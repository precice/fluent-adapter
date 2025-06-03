#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "fsi.h"

#undef node_to_host_double_1
#undef node_to_host_int_2
#undef PRF_GSYNC
#undef RP_Set_Real
#undef RP_Set_Integer
#undef RP_Get_Real
#undef RP_Get_Integer
#undef Message

extern int count_dynamic_threads(void);

int myid = 0;  // Used in fsi.c

#ifndef CURRENT_TIMESTEP
#define CURRENT_TIMESTEP 0.1
#endif

static Domain test_domain;

void node_to_host_double_1(double value) {
    check_expected(value);
}

void node_to_host_int_2(int a, int b) {
    (void)a; (void)b;
}

void PRF_GSYNC(void) { }

void RP_Set_Real(const char* key, double value) {
    (void)key; (void)value;
}

void RP_Set_Integer(const char* key, int value) {
    (void)key; (void)value;
}

double RP_Get_Real(const char* key) {
    (void)key;
    return 0.0;
}

int RP_Get_Integer(const char* key) {
    (void)key;
    return 0;
}

void Message(const char* fmt, ...) {
}

int mock_createParticipant_call_count = 0;
void precicec_createParticipant(const char* participantName,
                                const char* config,
                                int solver_process_id,
                                int solver_process_size)
{
    mock_createParticipant_call_count++;
    check_expected_ptr(participantName);
    check_expected_ptr(config);
    check_expected(solver_process_id);
    check_expected(solver_process_size);
}

int mock_finalize_call_count = 0;
void precicec_finalize(void) {
    mock_finalize_call_count++;
}

double precicec_getMaxTimeStepSize(void) {
    return 0.05;
}

int precicec_isCouplingOngoing(void) {
    return (int) mock();
}

int precicec_getDataDimensions(const char* meshID, const char* dataName) { 
    (void)meshID; (void)dataName; 
    return 1; 
}
int precicec_requiresInitialData(void) { 
    return 0; 
}
void precicec_writeData(const char* meshID, const char* dataName, 
                        int vertexSize, int* vertexIDs, double* forces) 
{
    (void)meshID; (void)dataName; (void)vertexSize; (void)vertexIDs; (void)forces;
}
void precicec_initialize(void) { }
void precicec_advance(double timestep) { (void)timestep; }
void precicec_setMeshVertices(const char* meshID, int size, 
                              const double* coords, int* indices)
{
    (void)meshID; (void)size; (void)coords; (void)indices;
}
void precicec_readData(const char* meshID, const char* dataName, 
                       int count, int* indices, double timestep, double* data)
{
    (void)meshID; (void)dataName; (void)count; (void)indices; 
    (void)timestep; (void)data;
}

Domain* Get_Domain(int dummy) {
    (void)dummy;
    return &test_domain;
}

int precicec_requiresWritingCheckpoint(void) {
    return 1; 
}

int precicec_requiresReadingCheckpoint(void) {
    return 1;
}

static void test_fsi_init_parameters(void **state)
{
    (void)state;


    expect_string(precicec_createParticipant, participantName, "Fluent");
    expect_string(precicec_createParticipant, config, "udf/config-location");
    expect_value(precicec_createParticipant, solver_process_id, 0);
    expect_value(precicec_createParticipant, solver_process_size, 1);

    Dynamic_Thread dt;
    snprintf(dt.profile_udf_name, sizeof(dt.profile_udf_name), "gridmotions");
    dt.next = NULL;
    test_domain.dynamic_threads = &dt;

    // Reset counter
    mock_createParticipant_call_count = 0;

    fsi_init(&test_domain);

    assert_int_equal(mock_createParticipant_call_count, 1);
}

static void test_fsi_grid_motion_finalize(void **state)
{
    (void)state;

    will_return(precicec_isCouplingOngoing, 0); 
    will_return(precicec_isCouplingOngoing, 0);

    Dynamic_Thread dt;
    Thread thread;
    dt.thread = &thread;
    snprintf(dt.profile_udf_name, sizeof(dt.profile_udf_name), "gridmotions");
    dt.next = NULL;

    Domain dom;
    dom.dynamic_threads = &dt;

    mock_finalize_call_count = 0;

    fsi_grid_motion(&dom, &dt, 0.1, 0.01);

    assert_int_equal(mock_finalize_call_count, 1);
}

static void test_count_dynamic_threads(void **state)
{
    (void) state;

    Dynamic_Thread dt1, dt2;
    snprintf(dt1.profile_udf_name, sizeof(dt1.profile_udf_name), "gridmotions");
    dt1.next = &dt2;
    snprintf(dt2.profile_udf_name, sizeof(dt2.profile_udf_name), "other");
    dt2.next = NULL;

    test_domain.dynamic_threads = &dt1;

    int count = count_dynamic_threads();
    assert_int_equal(count, 1);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_fsi_init_parameters),
        cmocka_unit_test(test_fsi_grid_motion_finalize),
        cmocka_unit_test(test_count_dynamic_threads),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
