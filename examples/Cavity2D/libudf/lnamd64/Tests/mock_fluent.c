#include <stdio.h>
#include "mock_fluent.h"

int myid = 0;
int compute_node_count = 1;
Domain global_domain = { NULL };

Domain* Get_Domain(int dummy)
{
    return &global_domain;
}
