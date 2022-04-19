#include "fsi.h"
#include "SolverInterfaceC.h"
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifdef BOOL_TRUE
#error BOOL_TRUE already defined!
#endif

#ifdef BOOL_FALSE
#error BOOL_TRUE already defined!
#endif

#define BOOL_TRUE  1
#define BOOL_FALSE 0

/* Functions with interface only with FLUENT */

/* Helper function to plot FSI coordinates (not related to preCICE
 * functionality) */
void fsi_plot_coords()
{
  printf("(%d) Entering ON_DEMAND(plot_coords)\n", myid);

  #if !RP_HOST
  int i=0, n=0;
  Domain* domain = NULL;
  Dynamic_Thread* dynamic_thread = NULL;
  Thread* face_thread = NULL;
  face_t face;
  Node* node = NULL;

  domain = Get_Domain(1);
  if (domain == NULL){
    printf("  (%d) ERROR: domain == NULL\n", myid);
    fflush(stdout);
    exit(1);
  }
  if (domain->dynamic_threads == NULL){
    printf("  (%d) ERROR: domain.dynamic_threads == NULL\n", myid);
    fflush(stdout);
    exit(1);
  }
  dynamic_thread = domain->dynamic_threads;
  while (dynamic_thread != NULL){
    if (strncmp("gridmotions", dynamic_thread->profile_udf_name, 11) == 0){
      face_thread  = DT_THREAD(dynamic_thread);
      if (face_thread == NULL){
        printf("  (%d) ERROR: face_thread == NULL\n", myid);
        fflush(stdout);
        exit(1);
      }
      begin_f_loop (face, face_thread){
        if (PRINCIPAL_FACE_P(face,face_thread)){
          f_node_loop (face, face_thread, n){
            node = F_NODE ( face, face_thread, n );
            if ((NODE_MARK(node) != 0) && ((NODE_MARK(node) != 1234))){
              printf("  (%d) ERROR: Unexpected node mark!\n", myid);
              fflush(stdout);
              exit(1);
            }
            if (NODE_MARK(node) != 1234){
              NODE_MARK(node) = 1234;
              /*if (i < 2){*/
                /*Message("coords: %.16E, %.16E\n", NODE_COORD(node)[0], NODE_COORD(node)[1]);*/
                /*fflush(stdout);*/
              /*}*/
              i++;
            }
          }
        }
      } end_f_loop(face, face_thread);
    }
    dynamic_thread = dynamic_thread->next;
  }

  /* Reset node mark */
  while (dynamic_thread != NULL){
    if (strncmp("gridmotions", dynamic_thread->profile_udf_name, 11) == 0){
      face_thread  = DT_THREAD(dynamic_thread);
      if (face_thread == NULL){
        printf("  (%d) ERROR: face_thread == NULL\n", myid);
        fflush(stdout);
        exit(1);
      }
      begin_f_loop (face, face_thread){
        if (PRINCIPAL_FACE_P(face,face_thread)){
          f_node_loop (face, face_thread, n){
            node = F_NODE ( face, face_thread, n );
            if (NODE_MARK(node) == 1234){
              NODE_MARK(node) = 0;
            }
          }
        }
      } end_f_loop(face, face_thread);
    }
    dynamic_thread = dynamic_thread->next;
  }
  #endif /* ! RP_HOST */

  printf("(%d) Leaving ON_DEMAND(plot_coords)\n", myid);
}


//void gather_write_positions()
//{
//  printf("(%d) Entering gather_write_positions()\n", myid);
//  #if !RP_HOST
//  int meshID = precicec_getMeshID("moving_base");
//  int i = 0;
//  double center[ND_ND];
//  Domain* domain = NULL;
//  Dynamic_Thread* dynamic_thread = NULL;
//  Thread* face_thread = NULL;
//  int thread_counter = 0;
//  face_t face;
//
//  Message("  (%d) Counting wet edges...\n", myid);
//  domain = Get_Domain(1);
//  if (domain == NULL){
//    Message("  (%d) ERROR: domain == NULL\n", myid);
//    exit(1);
//  }
//  if (domain->dynamic_threads == NULL){
//    Message("  (%d) ERROR: domain.dynamic_threads == NULL\n", myid);
//    exit(1);
//  }
//  dynamic_thread = domain->dynamic_threads;
//  thread_counter = 0;
//  while (dynamic_thread != NULL){
//    if (strncmp("gridmotions", dynamic_thread->profile_udf_name, 11) == 0){
//      Message("\n  (%d) Thread index %d\n", myid, thread_counter);
//      face_thread  = DT_THREAD(dynamic_thread);
//      if (face_thread == NULL){
//        Message("  (%d) ERROR: face_thread == NULL\n", myid);
//        exit(1);
//      }
//      begin_f_loop (face, face_thread){
//        if (PRINCIPAL_FACE_P(face,face_thread)){
//          wet_edges_size++;
//        }
//      } end_f_loop(face, face_thread);
//      thread_counter++;
//    }
//    dynamic_thread = dynamic_thread->next;
//  }
//  Message("  (%d) ...done (counted %d wet edges)\n", myid, wet_edges_size);
//  Message("  (%d) Allocating %d force vector values\n", myid, wet_edges_size * ND_ND);
//  if (forces != NULL){
//    Message("  (%d) ERROR: Forces vector allocated multiple times!\n", myid);
//  }
//  forces = (double*) malloc(wet_edges_size * ND_ND * sizeof(double));
//  Message("  (%d) Allocating %d force indices\n", myid, wet_edges_size);
//  force_indices = (int*) malloc(wet_edges_size * sizeof(int));
//  /*force_indices_fluent = (int*) malloc(wet_edges_size * sizeof(int));*/
//  Message("  (%d) Setting write positions...", myid);
//  dynamic_thread = domain->dynamic_threads;
//  thread_counter = 0;
//  i = 0;
//  while (dynamic_thread != NULL){
//    if (strncmp("gridmotions", dynamic_thread->profile_udf_name, 11) == 0){
//      Message("\n  (%d) Thread index %d\n", myid, thread_counter);
//      face_thread  = DT_THREAD(dynamic_thread);
//      if (face_thread == NULL){
//        printf("  (%d) ERROR: face_thread == NULL\n", myid);
//        fflush(stdout);
//        exit(1);
//      }
//      begin_f_loop (face, face_thread){
//        if (PRINCIPAL_FACE_P(face,face_thread)){
//          F_CENTROID(center, face, face_thread);
//          force_indices[i] = precicec_setMeshVertex(meshID, center);
//          F_UDMI(face, face_thread, 0) = force_indices[i];
//          i++;
//        }
//      } end_f_loop(face, face_thread);
//      thread_counter++;
//    }
//    dynamic_thread = dynamic_thread->next;
//  }
//  Message("  (%d) ...done counting wet edges\n", myid);
//  did_gather_write_positions = BOOL_TRUE;
//  #endif /* ! RP_HOST */
//
//  /* Gather precice read and write indices */
//  #if PARALLEL
//  /*gather_precicec_write_indices();*/
//  #endif /* PARALLEL */
//
//  /* Setup precice index tables for checkpoint and load balancing */
//  #if !RP_NODE /* Host or serial */
//  #endif /* ! RP_NODE */
//  printf("(%d) Leaving gather_write_positions()\n", myid);
//}

//void gather_read_positions(Dynamic_Thread* dt)
//{
//  printf("(%d) Entering gather_read_positions()\n", myid);
//  Thread* face_thread  = DT_THREAD(dt);
//  Node* node;
//  face_t face;
//  int n = 0, dim = 0;
//  int array_index = 0, node_index = 0;
//  double coords[ND_ND];
//  int meshID = precicec_getMeshID("moving_base");
//
//  /* Count not yet as updated (from other threads) marked nodes */
//  begin_f_loop(face, face_thread){
//    if (PRINCIPAL_FACE_P(face,face_thread)){
//      f_node_loop (face, face_thread, n){
//        node = F_NODE ( face, face_thread, n );
//        if (NODE_POS_NEED_UPDATE(node)){
//          NODE_MARK(node) = 12345;
//          wet_nodes_size++;
//          dynamic_thread_node_size[thread_index]++;
//        }
//      }
//    }
//  } end_f_loop(face, face_thread);
//
//  /* Get initial coordinates and reset update marking */
//  printf("  (%d) Reallocating %d initial positions ...\n", myid, wet_nodes_size);
//  initial_coords = (double*) realloc(initial_coords, wet_nodes_size * ND_ND * sizeof(double));
//  displacements = (double*) realloc(displacements, wet_nodes_size * ND_ND * sizeof(double));
//  displ_indices = (int*) realloc(displ_indices, wet_nodes_size * sizeof(int));
//  array_index = wet_nodes_size - dynamic_thread_node_size[thread_index];
//  begin_f_loop (face, face_thread){
//    if (PRINCIPAL_FACE_P(face,face_thread)){
//      f_node_loop(face, face_thread, n){
//        node = F_NODE(face, face_thread, n);
//        if (NODE_MARK(node) == 12345){
//          NODE_MARK(node) = 1;  /*Set node to need update*/
//          for (dim=0; dim < ND_ND; dim++){
//            coords[dim] = NODE_COORD(node)[dim];
//            initial_coords[array_index*ND_ND+dim] = coords[dim];
//          }
//          node_index = precicec_setMeshVertex(meshID,coords);
//          displ_indices[array_index] = node_index;
//          array_index++;
//        }
//      }
//    }
//  } end_f_loop(face, face_thread);
//  printf("  (%d) Set %d (of %d) displacement read positions ...\n", myid,
//          array_index - wet_nodes_size + dynamic_thread_node_size[thread_index],
//          dynamic_thread_node_size[thread_index]);
//
//  if (thread_index == dynamic_thread_size - 1){
//    did_gather_read_positions = BOOL_TRUE;
//  }
//  printf("(%d) Leaving gather_read_positions()\n", myid);
//}

//void regather_read_positions(Dynamic_Thread* dt, int thread_new_size)
//{
//  Message("  (%d) Regathering read positions...\n", myid);
//  int i = 0, j = 0, n = 0, dim = 0;
//  Thread* face_thread = DT_THREAD(dt);
//  Node* node;
//  face_t face;
//  int meshID = precicec_getMeshID("moving_base");
//  int all_size = 100;/*precicec_getReadNodesSize(meshID);    TODO: read mesh size from Fluent, 100 is just a dummy value!*/
//  int displID = precicec_getDataID("Displacements", meshID);
//  double* new_coords = (double*) malloc(thread_new_size * ND_ND * sizeof(double));
//  int* new_indices = (int*) malloc(thread_new_size * sizeof(double));
//  double* all_coords = (double*) malloc(all_size * ND_ND * sizeof(double));
//  double* all_displ = (double*) malloc(all_size * ND_ND * sizeof(double));
//  int* vertexIDs = (int*) malloc(all_size * sizeof(int));
//  double* tail_coords = NULL;
//  int* tail_indices = NULL;
//  /*double coords[ND_ND];*/
//  int left_i, right_i;
//  int front_size, tail_size, new_size;
//
//  for (i=0; i < all_size; i++){
//    vertexIDs[i] = i;
//  }
//  precicec_readBlockVectorData(displID, all_size, vertexIDs, all_displ);
//  for (i=0; i < all_size*ND_ND; i++){
//    if (i < all_size*ND_ND) {
//      printf("  (%d) coods %.16E\n", myid, all_coords[i]);
//      fflush(stdout);
//    }
//    all_coords[i] += all_displ[i];
//  }
//
//  /* Determine new indices, positions, and displacements */
//  i = 0;
//  begin_f_loop(face, face_thread){
//    if (PRINCIPAL_FACE_P(face,face_thread)){
//      f_node_loop (face, face_thread, n){
//        node = F_NODE(face, face_thread, n);
//        if (NODE_POS_NEED_UPDATE(node)){
//          NODE_MARK(node) = 12345;
//          for (j=0; j < all_size; j++){ /* Find position index in all positions */
//            for (dim=0; dim < ND_ND; dim++){ /* Vector equality comp */
//              if (i < 10){
//                printf("  (%d) %.16E == %.16E\n", myid, NODE_COORD(node)[dim],
//                       all_coords[j*ND_ND+dim]);
//                fflush(stdout);
//              }
//
//              if (fabs(NODE_COORD(node)[dim] - all_coords[j*ND_ND+dim]) > 1e-7){
//                break;
//              }
//            }
//            if (dim == ND_ND){ /* If equal */
//              /*printf("  (%d) Equal!\n", myid); fflush(stdout);*/
//              new_indices[i] = vertexIDs[j];
//              for (dim=0; dim < ND_ND; dim++){
//                left_i = i*ND_ND+dim;
//                right_i = j*ND_ND+dim;
//                new_coords[left_i] = all_coords[right_i] - all_displ[right_i];
//              }
//              break;
//            }
//          }
//          if (j == all_size){
//            printf("  (%d) ERROR: Didn't find suitable index while regathering read indices!\n", myid);
//            fflush(stdout);
//            exit(1);
//          }
//          i++;
//        }
//      }
//    }
//  } end_f_loop(face, face_thread);
//
//  /* Reset node marks */
//  begin_f_loop(face, face_thread){
//    if (PRINCIPAL_FACE_P(face,face_thread)){
//      f_node_loop (face, face_thread, n){
//        node = F_NODE(face, face_thread, n);
//        if (NODE_MARK(node) == 12345){
//          NODE_MARK(node) = 1; /* Set node to need update*/
//        }
//      }
//    }
//  } end_f_loop(face, face_thread);
//
//  /* Count entries before this face thread and after it */
//  front_size = 0;
//  for (i=0; i < thread_index; i++){
//    front_size += dynamic_thread_node_size[i];
//  }
//  tail_size = 0;
//  for (i=thread_index+1; i < dynamic_thread_size; i++){
//    tail_size += dynamic_thread_node_size[i];
//  }
//  tail_coords = (double*) malloc(tail_size*ND_ND*sizeof(double));
//  tail_indices = (int*) malloc(tail_size*sizeof(int));
//
//  /* Store tail entries */
//  for (i=0; i < tail_size; i++){
//    j = front_size + dynamic_thread_node_size[thread_index] + i;
//    for (dim=0; dim < ND_ND; dim++){
//      tail_coords[i*ND_ND+dim] = initial_coords[j*ND_ND+dim];
//    }
//    tail_indices[i] = displ_indices[j];
//  }
//
//  /* Insert new and tail entries */
//  new_size = front_size + thread_new_size + tail_size;
//  initial_coords = (double*) realloc(initial_coords, new_size * ND_ND * sizeof(double));
//  displacements = (double*) realloc(displacements, new_size * ND_ND * sizeof(double));
//  displ_indices = (int*) realloc(displ_indices, new_size * sizeof(int));
//  for (i=0; i < thread_new_size; i++){
//    j = front_size + i;
//    for (dim=0; dim < ND_ND; dim++){
//      initial_coords[j*ND_ND+dim] = new_coords[i*ND_ND+dim];
//    }
//    printf("  (%d) new index: %d\n", myid, new_indices[i]);
//    displ_indices[j] = new_indices[i];
//  }
//  for (i=0; i < tail_size; i++){
//    j = front_size + thread_new_size + i;
//    for (dim=0; dim < ND_ND; dim++){
//      initial_coords[j*ND_ND+dim] = tail_coords[i*ND_ND+dim];
//    }
//    displ_indices[j] = tail_indices[i];
//  }
//
//  wet_nodes_size -= dynamic_thread_node_size[thread_index];
//  wet_nodes_size += thread_new_size;
//  dynamic_thread_node_size[thread_index] = thread_new_size;
//
//  free(vertexIDs);
//  free(all_coords);
//  free(new_indices);
//  free(new_coords);
//  free(tail_indices);
//  free(tail_coords);
//
//  Message("  (%d) ... done regathering read positions...\n", myid); fflush(stdout);
//}

//void regather_write_positions(int current_size)
//{
//  #if !RP_HOST
//  int i = 0;
//  Domain* domain = NULL;
//  Dynamic_Thread* dynamic_thread = NULL;
//  Thread* face_thread = NULL;
//  int thread_counter = 0;
//  face_t face;
//
//  Message("  (%d) Regather write positions...\n", myid);
//
//  forces = (double*) realloc(forces, current_size * ND_ND * sizeof(double));
//  force_indices = (int*) realloc(force_indices, current_size * sizeof(int));
//
//  domain = Get_Domain(1);
//  if (domain == NULL){
//    Message("  (%d) ERROR: domain == NULL\n", myid);
//    exit(1);
//  }
//  if (domain->dynamic_threads == NULL){
//    Message("  (%d) ERROR: domain.dynamic_threads == NULL\n", myid);
//    exit(1);
//  }
//  dynamic_thread = domain->dynamic_threads;
//  thread_counter = 0;
//  i = 0;
//  while (dynamic_thread != NULL){
//    if (strncmp("gridmotions", dynamic_thread->profile_udf_name, 11) == 0){
//      Message("\n  (%d) Thread index %d\n", myid, thread_counter);
//      face_thread  = DT_THREAD(dynamic_thread);
//      if (face_thread == NULL){
//        Message("  (%d) ERROR: face_thread == NULL\n", myid);
//        exit(1);
//      }
//      begin_f_loop (face, face_thread){
//        if (PRINCIPAL_FACE_P(face,face_thread)){
//          force_indices[i] = F_UDMI(face, face_thread, 0);
//          i++;
//        }
//      } end_f_loop(face, face_thread);
//      thread_counter++;
//    }
//    dynamic_thread = dynamic_thread->next;
//  }
//  wet_edges_size = current_size;
//  Message("  (%d) ...done regathering write positions\n", myid);
//  #endif /* ! RP_HOST */
//}

int check_write_positions()
{
  #if !RP_HOST
  Domain* domain = NULL;
  Dynamic_Thread* dynamic_thread = NULL;
  Thread* face_thread = NULL;
  int thread_counter = 0;
  face_t face;
  int wet_edges_check_size = 0;

  Message("  (%d) Checking write positions...\n", myid);
  domain = Get_Domain(1);
  if (domain == NULL){
    Message("  (%d) ERROR: domain == NULL\n", myid);
    exit(1);
  }
  if (domain->dynamic_threads == NULL){
    Message("  (%d) ERROR: domain.dynamic_threads == NULL\n", myid);
    exit(1);
  }
  dynamic_thread = domain->dynamic_threads;
  thread_counter = 0;
  while (dynamic_thread != NULL){
    if (strncmp("gridmotions", dynamic_thread->profile_udf_name, 11) == 0){
      Message("\n  (%d) Thread index %d\n", myid, thread_counter);
      face_thread  = DT_THREAD(dynamic_thread);
      if (face_thread == NULL){
        Message("  (%d) ERROR: Thread %d: face_thread == NULL\n", myid, thread_counter);
        exit(1);
      }
      begin_f_loop (face, face_thread){
        if (PRINCIPAL_FACE_P(face,face_thread)){
          wet_edges_check_size++;
        }
      } end_f_loop(face, face_thread);
      thread_counter++;
    }
    dynamic_thread = dynamic_thread->next;
  }
  Message("  (%d) ...done (currently %d wet edges, old is %d)", myid,
          wet_edges_check_size, wet_edges_size);
  if (wet_edges_check_size != wet_edges_size) {
    return wet_edges_check_size;
  }
  #endif // ! RP_HOST 
  return -1;
}

int check_read_positions(Dynamic_Thread* dt)
{
  Message("  (%d) Checking read positions...\n", myid);
  int i = 0, n = 0;
  Thread* face_thread = DT_THREAD(dt);
  Node* node;
  face_t face;

  // Count nodes 
  begin_f_loop(face, face_thread){
    if (PRINCIPAL_FACE_P(face,face_thread)){
      f_node_loop (face, face_thread, n){
        node = F_NODE(face, face_thread, n);
        if (NODE_POS_NEED_UPDATE(node)){
          NODE_MARK(node) = 12345;
          i++;
        }
      }
    }
  } end_f_loop(face, face_thread);

  // Reset node marks 
  begin_f_loop(face, face_thread){
    if (PRINCIPAL_FACE_P(face,face_thread)){
      f_node_loop (face, face_thread, n){
        node = F_NODE(face, face_thread, n);
        if (NODE_MARK(node) == 12345){
          NODE_MARK(node) = 1; // Set node to need update
        }
      }
    }
  } end_f_loop(face, face_thread);

  if (i != dynamic_thread_node_size[thread_index]){
    Message("  (%d) Wet node count has changed for dynamic thread %d!\n",
            myid, thread_index);
    return i;
  }
  return -1;
}
