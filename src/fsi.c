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

//int did_gather_write_positions = BOOL_FALSE;
//int did_gather_read_positions = BOOL_FALSE;
//
/* GLOBAL VARIABLES */
int wet_nodes_size = 0;
int* displ_indices = NULL;
int thread_index = 0;
int dynamic_thread_size = 0;
int* dynamic_thread_node_size = NULL;
  int skip_grid_motion = BOOL_TRUE;
/* END GLOBAL VARIABLES */

//int wet_edges_size = 0;

#if ND_ND == 2
#define norm(a, b) sqrt((a[0]-b[0])*(a[0]-b[0]) + (a[1]-b[1])*(a[1]-b[1]))
#else
#error Not implemented!
#endif

/* Forward declarations of helper functions */
int count_dynamic_threads();
void write_forces();
void read_displacements(Dynamic_Thread* dt);
//int check_write_positions();
//int check_read_positions(Dynamic_Thread* dt);
int set_mesh_positions(Domain* domain);

/* This function creates the solver interface named "Fluent" and initializes
 * the interface
 * fsi_init is directly called by FLUENT UDF Functionality
 * */
void fsi_init(Domain* domain)
{
  double solve_dt = 0;
  int udf_convergence = 1;
  int udf_iterate = 0;

  /* Only Rank 0 Process handles the coupling interface */
  #if !RP_HOST
  printf("\n(%d) Entering fsi_init\n", myid);
  int solver_process_id = -1;
  int solver_process_size = 0;
  double timestep_limit = 0.0;

  #if !PARALLEL
  solver_process_id = 0;
  solver_process_size = 1;
  #else /* !PARALLEL*/
  solver_process_id = myid;
  solver_process_size = compute_node_count;
  #endif /* else !PARALLEL */

  printf("  (%d) Creating solver interface\n", myid);
  /* temporarily hard coding Solver name */
  if (RP_Variable_Exists_P("udf/config-location")){
    const char *config_loc = RP_Get_String("udf/config-location");
    precicec_createSolverInterface("Fluent", config_loc, solver_process_id,
            solver_process_size);
  }
  else {
    Error("Error reading 'udf/config-location' Scheme variable");
  }
  printf("  (%d) Solver interface created\n", myid);

  /* Count the number of dynamic threads */
  dynamic_thread_size = count_dynamic_threads();

  /* initialize array of nodes on each dynamic thread with 0s */
  dynamic_thread_node_size = (int*) malloc(dynamic_thread_size * sizeof(int));
  for (int i=0;  i < dynamic_thread_size; i++){
    dynamic_thread_node_size[i] = 0;
  }
  
  /* Set coupling mesh */
  wet_nodes_size = set_mesh_positions(domain);

  printf("  (%d) Initializing coupled simulation\n", myid);
  timestep_limit = precicec_initialize();
  /* Set the solver time step to be the minimum of the precice time step an the
   * current time step */
  solve_dt = fmin(timestep_limit, CURRENT_TIMESTEP);
  printf("  (%d) Initialization done\n", myid);
  #endif /* !RP_HOST */

  /* Communicate values from node(s) to host */
  node_to_host_double_1(solve_dt);
  #if !RP_NODE
  if (RP_Variable_Exists_P("solve/dt")){
    RP_Set_Real("solve/dt", solve_dt);
  }
  else {
    Error("Error reading 'solve/dt' Scheme variable");
  }
  #endif /* !RP_NODE */

  #if !RP_HOST
  if (precicec_isActionRequired(precicec_actionWriteIterationCheckpoint())){
    printf("  (%d) Implicit coupling\n", myid);
    udf_convergence = 0;
    udf_iterate = 1;
    precicec_markActionFulfilled(precicec_actionWriteIterationCheckpoint());
  }
  else {
    printf("  (%d) Explicit coupling\n", myid);
  }
  printf("  (%d) Synchronizing Fluent processes\n", myid);
  PRF_GSYNC();

  printf("(%d) Leaving INIT\n", myid);
  #endif /* !RP_HOST */
  
  node_to_host_int_2(udf_convergence, udf_iterate);

  #if !RP_NODE
  RP_Set_Integer("udf/convergence", udf_convergence);
  RP_Set_Integer("udf/iterate", udf_iterate);
  #endif /* !RP_NODE */
}

/* Main function advances the interface time step and provides the mechanism
 * for proper coupling scheme to be applied
 * fsi_write_and_advance is directly called by FLUENT UDF functionality
 * */
void fsi_write_and_advance()
{
  int ongoing = 1;
  int udf_convergence = 0;
  double solve_dt = 0;

  /* Only the host process (Rank 0) handles the writing of data and advancing coupling */
  #if !RP_HOST
  printf("\n(%d) Entering ON_DEMAND(write_and_advance)\n", myid);
  int subcycling = !precicec_isWriteDataRequired(CURRENT_TIMESTEP);
  double timestep_limit = 0.0;

  if (subcycling){
    Message("  (%d) In subcycle, skip writing\n", myid);
  }
  else {
    if (wet_nodes_size > 0){
      write_forces();
    }
  }

  timestep_limit = precicec_advance(CURRENT_TIMESTEP);
  /* Send min of timestep_limit and CURRENT_TIMESTEP to TUI */
  solve_dt = fmin(timestep_limit, CURRENT_TIMESTEP);
  /* Read coupling state */
  ongoing = precicec_isCouplingOngoing();
  
  if (precicec_isActionRequired(precicec_actionWriteIterationCheckpoint())){
      udf_convergence = 1;
    precicec_markActionFulfilled(precicec_actionWriteIterationCheckpoint());
  }
  if (precicec_isActionRequired(precicec_actionReadIterationCheckpoint())){
      udf_convergence = 0;
    precicec_markActionFulfilled(precicec_actionReadIterationCheckpoint());
  }
  if (! precicec_isCouplingOngoing()){
    udf_convergence = 1;
  }
  printf("(%d) Leaving ON_DEMAND(write_and_advance)\n", myid);
  #endif /* !RP_HOST */  
  
  /* Pass data from compute nodes to host node */
  node_to_host_int_2(ongoing, udf_convergence);
  node_to_host_double_1(solve_dt);

  #if !RP_NODE
  RP_Set_Real("solve/dt", solve_dt);
  RP_Set_Integer("udf/ongoing", ongoing);
  RP_Set_Integer("udf/convergence", udf_convergence);
  Message("(%d) Setting solve/dt: %.6e, udf/ongoing: %ld, udf/convergence: %ld\n",
          myid, RP_Get_Real("solve/dt"), RP_Get_Integer("udf/ongoing"),
          RP_Get_Integer("udf/convergence"));
  #endif /* !RP_NODE */
}

/* Function to be attached to the Dynamic Mesh in FLUENT in the form of a UDF.
 * This function will read the displacements values from interface and move the
 * structural mesh accordingly
 * fsi_grid_motion is directly related to mesh motion in FLUENT UDF Functionality
 * */
void fsi_grid_motion(Domain* domain, Dynamic_Thread* dt, real time, real dtime)
{
  int ongoing = 1;
  
  /* Only the host process (Rank 0) handles grid motion and displacement calculations */
  #if !RP_HOST
  printf("\n(%d) Entering GRID_MOTION\n", myid);
  /* Restart the thread index so we can provide motion for more than one
   * boundary */
  if (thread_index == dynamic_thread_size){
    printf ("  (%d) Reset thread index\n", myid);
    thread_index = 0;
  }
  printf("  (%d) Thread index = %d\n", myid, thread_index);
  /* Create the Thread pointer */
  Thread* face_thread  = DT_THREAD(dt);

  /* Ways we can error out */
  if (strncmp("gridmotions", dt->profile_udf_name, 11) != 0){
    printf("  (%d) ERROR: called gridmotions for invalid dynamic thread: %s\n",
            myid, dt->profile_udf_name);
    exit(1);
  }
  if (face_thread == NULL){
    printf("  (%d) ERROR: face_thread == NULL\n", myid);
    exit(1);
  }
  /* If we skip the grid motion on the first round just return, but also
   * increase the thread index and change skip_grid_motion to FALSE */
  if (skip_grid_motion){
    if (thread_index >= dynamic_thread_size-1){
      skip_grid_motion = BOOL_FALSE;
    }
    thread_index++;
    printf("  (%d) Skipping first round grid motion\n", myid);
  }
  else {
    /* Read in displacements and move grid */
    SET_DEFORMING_THREAD_FLAG(THREAD_T0(face_thread));
    read_displacements(dt);
    thread_index++;
  }
  /* update the ongoing flag */
  ongoing = precicec_isCouplingOngoing();
  PRF_GSYNC();
  /* if precice is done, finalize */
  if (! precicec_isCouplingOngoing()){
    precicec_finalize();
  }
  printf("(%d) Leaving GRID_MOTION\n", myid);
  #endif /* !RP_HOST  */
  
  /* Send udf_convergence to RP_HOST so it can be changed in the TUI */
  node_to_host_int_1(ongoing);
  
  #if !RP_NODE
  /* update the ongoing flag */
  RP_Set_Integer("udf/ongoing", ongoing);
  /* print status of the relevant flags */
  Message("(%d) convergence=%ld, iterate=%ld, couplingOngoing=%ld\n",
          myid, RP_Get_Integer("udf/convergence"), RP_Get_Integer("udf/iterate"),
          RP_Get_Integer("udf/ongoing"));
  /* if ongoing, iterate, and convergence flags are all on turn the convergence
   * flag off */
  if (RP_Get_Integer("udf/convergence") &&
          RP_Get_Integer("udf/iterate") &&
          RP_Get_Integer("udf/ongoing")){
      RP_Set_Integer("udf/convergence", 0);
      Message("(%d) Setting udf/convergence: %ld\n",
              myid, RP_Get_Integer("udf/convergence"));
  }
  #endif
}

int count_dynamic_threads()
{
  printf("(%d) Entering count_dynamic_threads()\n", myid);
  Domain *domain = NULL;
  Dynamic_Thread* dynamic_thread = NULL;
  Thread* face_thread = NULL;
  face_t face;
  int node_index;
  int dynamic_thread_size = 0;
  Node* node = NULL;

  printf( "  (%d) counting dynamic threads:\n", myid);
  domain = Get_Domain(1);
  if (domain == NULL){
    printf("  (%d) ERROR: domain == NULL\n", myid);
    exit(1);
  }
  if (domain->dynamic_threads == NULL){
    printf("  (%d) ERROR: domain.dynamic_threads == NULL\n", myid);
    exit (1);
  }
  dynamic_thread = domain->dynamic_threads;
  while (dynamic_thread != NULL){
    if (strncmp("gridmotions", dynamic_thread->profile_udf_name, 11) == 0){
      face_thread  = DT_THREAD(dynamic_thread);
      begin_f_loop (face, face_thread){ /* Thread face loop */
        if (PRINCIPAL_FACE_P(face,face_thread)){
          f_node_loop (face, face_thread, node_index){ /* Face node loop */
            node = F_NODE(face, face_thread, node_index);
            NODE_MARK(node) = 11111;
          }
        }
      } end_f_loop(face, face_thread)
      dynamic_thread_size++;
    }
    dynamic_thread = dynamic_thread->next;
  }

  /* Reset node marks */
  dynamic_thread = domain->dynamic_threads;
  while (dynamic_thread != NULL){
    if (strncmp("gridmotions", dynamic_thread->profile_udf_name, 11) == 0){
      face_thread  = DT_THREAD(dynamic_thread);
      begin_f_loop (face, face_thread){ /* Thread face loop */
        f_node_loop (face, face_thread, node_index){ /* Face node loop */
          node = F_NODE(face, face_thread, node_index);
          NODE_MARK(node) = 0;
        }
      } end_f_loop(face, face_thread)
    }
    dynamic_thread = dynamic_thread->next;
  }
  printf("  (%d) ... %d\n", myid, dynamic_thread_size);
  printf("(%d) Leaving count_dynamic_threads()\n", myid);
  return dynamic_thread_size;
}

int set_mesh_positions(Domain* domain)
{
  /* Only the host process (Rank 0) handles grid motion and displacement calculations */
  #if !RP_HOST
  double* initial_coords = NULL;
  printf("(%d) Entering set_mesh_positions()\n", myid);
  Thread* face_thread  = NULL;
  Dynamic_Thread* dynamic_thread = NULL;
  Node* node;
  face_t face;
  int n = 0, dim = 0, array_index = 0;
  int meshID = precicec_getMeshID("moving_base");

  if (domain->dynamic_threads == NULL){
    Message("  (%d) ERROR: domain.dynamic_threads == NULL\n", myid);
    exit(1);
  }
  dynamic_thread = domain->dynamic_threads;

  face_thread = DT_THREAD(dynamic_thread);
  if (face_thread == NULL){
	printf("  (%d) ERROR: face_thread == NULL\n", myid);
    fflush(stdout);
    exit(1);
  }

  /* Count number of interface vertices and dynamic_thread_node_size */
  begin_f_loop(face, face_thread){
    if (PRINCIPAL_FACE_P(face,face_thread)){
      f_node_loop(face, face_thread, n){
        node = F_NODE(face, face_thread, n);
        printf("Count of wet_nodes_size = %d\n", wet_nodes_size);
        wet_nodes_size++;
        dynamic_thread_node_size[thread_index]++;
      }
    }
  } end_f_loop(face, face_thread);

  printf("  (%d) Setting %d initial positions ...\n", myid, wet_nodes_size);

  /* Providing mesh information to preCICE */
  initial_coords = (double*) malloc(wet_nodes_size * ND_ND * sizeof(double));
  displ_indices = (int*) malloc(wet_nodes_size * sizeof(int));
  array_index = wet_nodes_size - dynamic_thread_node_size[thread_index];

  begin_f_loop (face, face_thread){
    if (PRINCIPAL_FACE_P(face,face_thread)){
      f_node_loop(face, face_thread, n){
		    node = F_NODE(face, face_thread, n);
        NODE_MARK(node) = 1;  /*Set node to need update*/
        for (dim = 0; dim < ND_ND; dim++){
           initial_coords[array_index*ND_ND+dim] = NODE_COORD(node)[dim];
        }
        array_index++;
      }
    }
  } end_f_loop(face, face_thread);

  for (int i=0; i<wet_nodes_size; i++) {
    printf("initial_coords[%d] = (%f, %f)\n", i, initial_coords[i],
            initial_coords[i+1]);
  }

  precicec_setMeshVertices(meshID, wet_nodes_size, initial_coords, displ_indices);

  printf("  (%d) Set %d (of %d) mesh positions ...\n", myid,
          array_index - wet_nodes_size + dynamic_thread_node_size[thread_index],
          dynamic_thread_node_size[thread_index]);

  printf("(%d) Leaving set_mesh_positions()\n", myid);

  return wet_nodes_size;
  #endif /* !RP_HOST  */
  
  #if RP_HOST
  return -1;
  #endif /* RP_HOST */
}

/* This functions reads the new displacements provided by the structural
 * solver and moves the mesh coordinates in FLUENT with the corresponding
 * values
 * */
void read_displacements(Dynamic_Thread* dt)
{
  double* displacements = NULL;
  int meshID = precicec_getMeshID("moving_base");
  int displID = precicec_getDataID("Displacements", meshID);
  int offset = 0;
  int i = 0, n = 0, dim = 0;
  Thread* face_thread  = DT_THREAD(dt);
  Node* node;
  face_t face;
  real max_displ_delta = 0.0;

  displacements = (double*) malloc(wet_nodes_size * ND_ND * sizeof(double));
  
  if (dynamic_thread_node_size[thread_index] > 0){
    Message("  (%d) Reading displacements...\n", myid);
    offset = 0;
    for (i = 0; i < thread_index; i++){
      offset += dynamic_thread_node_size[i];
    }
    printf("data size for readBlockVectorData = %d\n",dynamic_thread_node_size[thread_index]);
  precicec_readBlockVectorData(displID, dynamic_thread_node_size[thread_index],
        displ_indices + offset, displacements + ND_ND * offset);
	printf("After readBlockVectorData\n");

  Message("  (%d) Setting displacements...\n", myid);
  i = offset * ND_ND;
  begin_f_loop (face, face_thread){
    if (PRINCIPAL_FACE_P(face,face_thread)){
      f_node_loop (face, face_thread, n){
        node = F_NODE(face, face_thread, n);
        if (NODE_POS_NEED_UPDATE(node)){
          NODE_POS_UPDATED(node);
          for (dim=0; dim < ND_ND; dim++){
            /* NODE_COORD(node)[dim] = initial_coords[i+dim] + displacements[i+dim]; */
            if (fabs(displacements[i+dim]) > fabs(max_displ_delta)){
              max_displ_delta = displacements[i + dim];
            }
          }
          i += ND_ND;
        }
      }
    }
  } end_f_loop (face, face_thread);

  Message("  (%d) ...done\n", myid);
  }
  Message("  (%d) Max displacement delta: %f\n", myid, max_displ_delta);
}

/* This function writes the new forces on the structure calculated in FLUENT to the
 * Structural solver
 */
void write_forces()
{
  double* forces = NULL;
  int* force_indices = NULL;
  int meshID = precicec_getMeshID("moving_base");
  int forceID = precicec_getDataID("Forces", meshID);
  int i=0, j=0;
  Domain* domain = NULL;
  Dynamic_Thread* dynamic_thread = NULL;
  int thread_counter = 0;
  int face_counter = 0;
  real area[ND_ND];
  real pressure_force[ND_ND];
  real viscous_force[ND_ND];
  double total_force[ND_ND];
  double sum_total_force[ND_ND];
  double max_force = 0.0;

  forces = (double*) malloc(wet_nodes_size * ND_ND * sizeof(double));
  force_indices = displ_indices;
  
  domain = Get_Domain(1);
  if (domain == NULL){
    printf("  (%d) ERROR: domain == NULL\n", myid);
    exit(1);
  }
  if (domain->dynamic_threads == NULL){
    printf("  (%d) ERROR: domain.dynamic_threads == NULL\n", myid);
    exit(1);
  }

  dynamic_thread = domain->dynamic_threads;
  thread_counter = 0;
  printf("  (%d) Gather forces...\n", myid);
  i = 0;
  while (dynamic_thread != NULL){
    if (strncmp("gridmotions", dynamic_thread->profile_udf_name, 11) == 0){
      printf("  (%d) Thread index %d\n", myid, thread_counter);
      Thread* face_thread  = DT_THREAD(dynamic_thread);
      if (face_thread == NULL){
        Error("  (%d) face_thread == NULL\n", myid);
      }
      if (!BOUNDARY_FACE_THREAD_P(face_thread)){
          Error("  (%d) face_thread is not a boundary face\n", myid);
      }
      face_counter = 0;
      sum_total_force[0] = 0;
      sum_total_force[1] = 0;
      face_t face;
      begin_f_loop (face, face_thread){
        if (PRINCIPAL_FACE_P(face, face_thread)){
          F_AREA(area, face, face_thread);
          printf("area: %f %f\n", area[0], area[1]);
          NV_VS(viscous_force, =, F_STORAGE_R_N3V(face,face_thread,SV_WALL_SHEAR),*,-1.0);
          printf("viscous_force: %f %f\n", viscous_force[0], viscous_force[1]);
          /* F_P is NOT available in the density-based solver in Fluent; MUST
           * use the pressure-based solver */
          NV_VS(pressure_force, =, area, *, F_P(face,face_thread));
          printf("pressure_force: %f %f\n", pressure_force[0], pressure_force[1]);
          NV_VV(total_force, =, viscous_force, +, pressure_force);
          printf("total_force: %f %f\n", total_force[0], total_force[1]);
          for (j=0; j < ND_ND; j++){
            forces[i + j] = total_force[j];
            if (fabs(total_force[j]) > fabs(max_force)){
              max_force = total_force[j];
            }
          }
          i += ND_ND;
        }
        face_counter++;
        sum_total_force[0] += total_force[0];
        sum_total_force[1] += total_force[1];
      } end_f_loop(face, face_thread);
      printf("  (%d) thread: %d had %d faces\n", myid, thread_counter, face_counter);
      printf("  (%d) thread: %d had sum_total_force: %f %f\n",
              myid, thread_counter, sum_total_force[0], sum_total_force[1]);
      thread_counter++;
    }
    else {
        Error("  dynamic_thread->profile_udf_name is not 'gridmotions'");
    }
    dynamic_thread = dynamic_thread->next;
  }
  printf("  (%d) ...done (with %d force values)\n", myid, i);
  printf("  (%d) Writing forces...\n", myid);
  precicec_writeBlockVectorData(forceID, wet_nodes_size, force_indices, forces);
  printf("  (%d) ...done\n", myid );
  printf("  (%d) Max force: %f\n", myid, max_force);
  if (thread_counter != dynamic_thread_size){
    printf ( "  (%d) ERROR: Number of dynamic threads has changed to %d!\n", myid, thread_counter );
    exit(1);
  }
}

/*
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
//      Message("\n  (%d) Thread index %d\n", myid, thread_counter);
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
*/
/*
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
*/
