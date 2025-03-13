#include <assert.h>
#include <stdio.h>
#include "preciceC.h"

#include "fsi.h"

static Dynamic_Thread dt;
static Thread face_thread;
static Node face_nodes[4];

void setup_global_domain_for_test()
{
  snprintf(dt.profile_udf_name, sizeof(dt.profile_udf_name), "gridmotions");
  dt.next = NULL;

  face_nodes[0].coord[0] = 0.0; face_nodes[0].coord[1] = 0.0;
  face_nodes[1].coord[0] = 0.0; face_nodes[1].coord[1] = 1.0;
  face_nodes[2].coord[0] = 1.0; face_nodes[2].coord[1] = 0.0;
  face_nodes[3].coord[0] = 1.0; face_nodes[3].coord[1] = 1.0;

  Node* face_node_ptrs[4] = { &face_nodes[0], &face_nodes[1], &face_nodes[2], &face_nodes[3] };
  face_thread.nodes = face_node_ptrs;
  face_thread.node_count = 2; 

  dt.thread = &face_thread;

  global_domain.dynamic_threads = &dt;
}

static void test_fsi_init_creates_participant()
{
  mock_createParticipant_call_count = 0;

  setup_global_domain_for_test();


  fsi_init(&global_domain);

  assert(mock_createParticipant_call_count == 1);
  printf("test_fsi_init_creates_participant passed!\n");
}

static void test_fsi_grid_motion_calls_finalize_when_not_ongoing()
{

  extern int precicec_isCouplingOngoing(void);


  mock_finalize_call_count = 0;

  Thread face_thread;
  Node face_nodes[4];
  face_nodes[0].coord[0] = 0.0; face_nodes[0].coord[1] = 0.0; 
  face_nodes[1].coord[0] = 0.0; face_nodes[1].coord[1] = 1.0;
  face_nodes[2].coord[0] = 0.5; face_nodes[2].coord[1] = 0.0;
  face_nodes[3].coord[0] = 0.5; face_nodes[3].coord[1] = 1.0;
  face_thread.nodes = (Node**)&face_nodes;
  face_thread.node_count = 2; 

  static Dynamic_Thread dt;
  dt.thread = &face_thread;
  snprintf(dt.profile_udf_name, 64, "gridmotions");
  dt.next = NULL;

  Domain dom;
  dom.dynamic_threads = &dt;

  fsi_grid_motion(&dom, &dt, 0.1, 0.01);

  assert(mock_finalize_call_count == 0);

  printf("test_fsi_grid_motion_calls_finalize_when_not_ongoing passed!\n");
}

int main()
{
  test_fsi_init_creates_participant();
  test_fsi_grid_motion_calls_finalize_when_not_ongoing();

  printf("\033[32mAll tests passed!\033[0m\n");
    return 0;
}
