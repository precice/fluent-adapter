#ifndef MOCK_DYNAMESH_TOOLS_H
#define MOCK_DYNAMESH_TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct _tg_node_struct Node;
typedef struct _tg_face_struct Face;
typedef struct _tg_cell_struct Cell;
typedef struct thread_struct Thread;
typedef struct Dynamic_Thread {
    char profile_udf_name[256];
    struct Dynamic_Thread* next;
    Thread* thread;  /* pointer to our mock Thread struct */
} Dynamic_Thread;

typedef struct Domain {
    Dynamic_Thread* dynamic_threads;
} Domain;
#define ND_ND 2

#ifndef _TG_NODE_STRUCT_DEFINED
#define _TG_NODE_STRUCT_DEFINED
struct _tg_node_struct {
    double coord[ND_ND];
    int mark;
};
#endif

#define F_AREA(area, face, thread) \
    do { for (int _i = 0; _i < ND_ND; _i++) (area)[_i] = 0.0; } while(0)
    #define F_CENTROID(pos, face, thread) \
    do { for (int _i = 0; _i < ND_ND; _i++) (pos)[_i] = 0.0; } while(0)

typedef double real;


#define DT_THREAD(dt) ((dt)->thread)
#define ND_ND 2
#define SET_DEFORMING_THREAD_FLAG(thread) do {} while(0)

#define NODE_POS_NEED_UPDATE(node) (0)
#define NODE_POS_UPDATED(node) do {} while(0)
#define NODE_MARK(node) (((Node*)(node))->mark)
#define NODE_COORD(node) (((Node*)(node))->coord)



#endif /* MOCK_DYNAMESH_TOOLS_H */
