#ifndef MOCK_UDF_H
#define MOCK_UDF_H

#include "dynamesh_tools.h"

#define THREAD_T0(thread) (thread)
#define F_NODE(face, thread, index) ((Node*)NULL)
#define f_node_loop(face, thread, index) for ((index) = 0; (index) < 0; (index)++)
#define begin_f_loop(face, thread) for (int _i = 0; _i < 0; _i++) {
#define end_f_loop(face, thread) }

#define PRINCIPAL_FACE_P(face, thread) (0)


#define BOUNDARY_FACE_THREAD_P(thread) (1)


#define RP_Get_Real(key)           0.0
#define RP_Set_Real(key, value)    do {} while(0)
#define RP_Set_Integer(key, value) do {} while(0)
#define RP_Variable_Exists_P(key)  (0)
#define RP_Get_String(key)         ""
#define RP_Get_Integer(key)        (long int)0

#define CURRENT_TIMESTEP 0.01

#define Message(...)               printf(__VA_ARGS__)
#define Error(fmt, ...)            do { printf("Error: " fmt, ##__VA_ARGS__); exit(1); } while(0)
#define PRF_GSYNC()                do {} while(0)

typedef void (*UDF_Function)(void);

/* Structure that holds UDF information:
   - name: a string to identify the UDF
   - function: pointer to the UDF function
   - type: an integer indicating the UDF type (e.g., init, on-demand, grid motion)
*/
typedef struct {
    const char *name;
    UDF_Function function;
    int type;
} UDF_Data;

/* Define UDF type constants */
#define UDF_TYPE_INIT         0
#define UDF_TYPE_ON_DEMAND    1
#define UDF_TYPE_GRID_MOTION  2

#define node_to_host_int_1(x)      do {} while(0)
#define node_to_host_int_2(x, y)   do {} while(0)
#define node_to_host_double_1(x)   do {} while(0)

extern int myid;
extern int compute_node_count;
typedef double face_t;
extern Domain global_domain;
Domain* Get_Domain(int dummy);
static double _mock_storage_array[ND_ND] = {0.0, 0.0};
#define F_STORAGE_R_N3V(face, thread, var) (_mock_storage_array)
#define F_P(face, thread) (0.0)
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

#endif /* MOCK_UDF_H */