#ifndef MOCK_FLUENT_H
#define MOCK_FLUENT_H

#include "dynamesh_tools.h" 

extern int myid;
extern int compute_node_count;

extern Domain global_domain;

Domain* Get_Domain(int dummy);

#endif /* MOCK_FLUENT_H */
