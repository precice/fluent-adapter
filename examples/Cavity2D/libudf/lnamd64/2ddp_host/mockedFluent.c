#ifndef MOCK_FSI_H
#define MOCK_FSI_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#define RP_NODE 0
#define RP_HOST 0
#define ND_ND 2
#define CURRENT_TIMESTEP 0.1


typedef double real;

typedef double face_t;

typedef double Node;

static int compute_node_count = 1; 
static int myid = 0;              

typedef struct Thread {
    int id;
} Thread;

typedef struct Dynamic_Thread {
    char profile_udf_name[256];
    struct Dynamic_Thread* next;
    Thread* thread;
} Dynamic_Thread;

typedef struct {
    Dynamic_Thread* dynamic_threads;
} Domain;

static double _mock_storage_array[ND_ND] = {0.0, 0.0};

#define F_STORAGE_R_N3V(face, thread, var) (_mock_storage_array)

#define NV_VS(result, op, a, op2, b)                       \
  do {                                                     \
    for (int d = 0; d < ND_ND; d++) {                      \
      (result)[d] op (a)[d] op2 (b);                       \
    }                                                      \
  } while(0)

#define NV_VV(result, op, a, op2, b)                       \
  do {                                                     \
    for (int d = 0; d < ND_ND; d++) {                      \
      (result)[d] op (a)[d] op2 (b)[d];                    \
    }                                                      \
  } while(0)

#define F_AREA(area, face, thread) (area[0] = 1.0, area[1] = 1.0)
#define F_P(face, thread) (1.0)

#define begin_f_loop(face, thread) for (int face = 0; face < 10; face++) {
#define end_f_loop(face, thread) }

#define f_node_loop(face, thread, node_index) for (int node_index = 0; node_index < 5; node_index++)
#define end_f_node_loop()

static int node_mark_value = 0;
#define NODE_MARK(node) (node_mark_value)

#define DT_THREAD(dynamic_thread) ((dynamic_thread)->thread)
#define PRINCIPAL_FACE_P(face, thread) (1)
#define F_NODE(face, thread, node_index) ((Node*)NULL) 

/* 
 * Misc. stubs your code calls.
 */
static inline void SET_DEFORMING_THREAD_FLAG(Thread* t) {
    /* no-op in mock */
    printf("Mock: SET_DEFORMING_THREAD_FLAG called.\n");
}
static inline Thread* THREAD_T0(Thread* t) {
    return t;
}
static inline int BOUNDARY_FACE_THREAD_P(Thread* t) {
    return 1;
}
static inline Domain* Get_Domain(int id) {
    static Domain sDomain;
    return &sDomain;
}
static inline void F_CENTROID(double pos[], face_t face, Thread* thread) {
    pos[0] = 0.0;
    pos[1] = 0.0;
}
static inline int NODE_POS_NEED_UPDATE(Node* node) {
    return 1;
}
static inline void NODE_POS_UPDATED(Node* node) {
    /* no-op */
    printf("Mock: NODE_POS_UPDATED called.\n");
}

static int node_coords[ND_ND] = {0,0};
static inline int* NODE_COORD(Node* node) {
    return node_coords;
}

static inline int RP_Variable_Exists_P(const char* name) {
    printf("Mock: RP_Variable_Exists_P called with %s\n", name);
    return 1;  /* Always exist */
}
static inline const char* RP_Get_String(const char* name) {
    printf("Mock: RP_Get_String called with %s\n", name);
    return "mock_config.xml"; /* some default */
}
static inline void RP_Set_Real(const char* name, double value) {
    printf("Mock: RP_Set_Real called with %s = %f\n", name, value);
}
static inline void RP_Set_Integer(const char* name, int value) {
    printf("Mock: RP_Set_Integer called with %s = %d\n", name, value);
}
static inline double RP_Get_Real(const char* name) {
    printf("Mock: RP_Get_Real called with %s\n", name);
    return 0.1;  /* dummy value */
}
static inline int RP_Get_Integer(const char* name) {
    printf("Mock: RP_Get_Integer called with %s\n", name);
    return 1;    /* dummy value */
}
static inline void Message(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
static inline void Error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}
static inline void PRF_GSYNC() {
    printf("Mock: PRF_GSYNC called.\n");
}
static inline void node_to_host_int_1(int value) {
    printf("Mock: node_to_host_int_1 called with %d\n", value);
}
static inline void node_to_host_int_2(int value1, int value2) {
    printf("Mock: node_to_host_int_2 called with %d, %d\n", value1, value2);
}
static inline void node_to_host_double_1(double value) {
    printf("Mock: node_to_host_double_1 called with %f\n", value);
}
#endif
