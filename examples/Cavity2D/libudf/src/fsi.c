#include "fsi.h"
#include "precice/preciceC.h"
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

/* GLOBAL VARIABLES */
int wet_nodes_size = 0;
int wet_face_size = 0;
int* displ_indices = NULL;
int* face_indices = NULL;
double* initial_coords = NULL;
double* face_coords = NULL;
int thread_index = 0;
int dynamic_thread_size = 0;
int* dynamic_thread_node_size = NULL;
int* dynamic_thread_face_size = NULL;
int skip_grid_motion = BOOL_TRUE;
/* END GLOBAL VARIABLES */

#if ND_ND == 2
#define norm(a, b) sqrt((a[0]-b[0])*(a[0]-b[0]) + (a[1]-b[1])*(a[1]-b[1]))
#else
#error Not implemented!
#endif

/* Forward declarations of helper functions */
int count_dynamic_threads();
void write_forces();
void read_displacements(Dynamic_Thread* dt);
void set_mesh_positions(Domain* domain);

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
        precicec_createParticipant("Fluent", config_loc, solver_process_id,
                solver_process_size);
    }
    else {
        Error("Error reading 'udf/config-location' Scheme variable");
    }
    printf("  (%d) Solver interface created\n", myid);

    /* Count the number of dynamic threads */
    dynamic_thread_size = count_dynamic_threads();;

    /* initialize array of nodes on each dynamic thread with 0s */
    dynamic_thread_node_size = (int*) malloc(dynamic_thread_size * sizeof(int));
    dynamic_thread_face_size = (int*) malloc(dynamic_thread_size * sizeof(int));
    for (int i=0;  i < dynamic_thread_size; i++){
        dynamic_thread_node_size[i] = 0;
        dynamic_thread_face_size[i] = 0;
    }
    
    /* Set coupling mesh positions (faces and nodes)*/
    set_mesh_positions(domain);

    printf("  (%d) Initializing coupled simulation\n", myid);
    precicec_initialize();
    timestep_limit = precicec_getMaxTimeStepSize();

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
    if (precicec_requiresWritingCheckpoint()){
        printf("  (%d) Implicit coupling\n", myid);
        udf_convergence = 0;
        udf_iterate = 1;
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

    #if !RP_HOST
    printf("\n(%d) Entering ON_DEMAND(write_and_advance)\n", myid);
    double timestep_limit = 0.0;
    if (wet_face_size > 0){
          write_forces();
      }
    timestep_limit = precicec_getMaxTimeStepSize();
    solve_dt = fmin(timestep_limit, CURRENT_TIMESTEP);
    precicec_advance(solve_dt);

    ongoing = precicec_isCouplingOngoing();
    
    if (precicec_requiresWritingCheckpoint()){
        udf_convergence = 1;
    }
    if (precicec_requiresReadingCheckpoint()){
        udf_convergence = 0;
      
    }
    if (! precicec_isCouplingOngoing()){
        udf_convergence = 1;
    }
    printf("(%d) Leaving ON_DEMAND(write_and_advance)\n", myid);
    #endif /* !RP_HOST */  
    
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
    
    #if !RP_HOST
    printf("\n(%d) Entering GRID_MOTION\n", myid);

    if (thread_index == dynamic_thread_size){
        printf ("  (%d) Reset thread index\n", myid);
        thread_index = 0;
    }
    printf("  (%d) Thread index = %d\n", myid, thread_index);
    Thread* face_thread  = DT_THREAD(dt);

    if (strncmp("gridmotions", dt->profile_udf_name, 11) != 0){
        printf("  (%d) ERROR: called gridmotions for invalid dynamic thread: %s\n",
                myid, dt->profile_udf_name);
        exit(1);
    }
    if (face_thread == NULL){
        printf("  (%d) ERROR: face_thread == NULL\n", myid);
        exit(1);
    }

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
    ongoing = precicec_isCouplingOngoing();
    PRF_GSYNC();
    if (! precicec_isCouplingOngoing()){
        precicec_finalize();
    }
    printf("(%d) Leaving GRID_MOTION\n", myid);
    #endif /* !RP_HOST  */
    
    node_to_host_int_1(ongoing);
    
    #if !RP_NODE
    RP_Set_Integer("udf/ongoing", ongoing);
    Message("(%d) convergence=%ld, iterate=%ld, couplingOngoing=%ld\n",
            myid, RP_Get_Integer("udf/convergence"), RP_Get_Integer("udf/iterate"),
            RP_Get_Integer("udf/ongoing"));

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

void set_mesh_positions(Domain* domain)
{
     #if !RP_HOST
     printf("(%d) Entering set_mesh_positions()\n", myid);
     Thread* face_thread  = NULL;
     Dynamic_Thread* dynamic_thread = NULL;
     Node* node;
     face_t face;
     double pos[ND_ND];
     int n = 0, dim = 0, array_index = 0, face_index = 0;
     const char* nodeMeshID = "moving_base_nodes";
     const char* faceMeshID = "moving_base_faces";
 
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

     begin_f_loop(face, face_thread){
         if (PRINCIPAL_FACE_P(face, face_thread)) {
             wet_face_size++;
             dynamic_thread_face_size[thread_index]++;
             f_node_loop(face, face_thread, n){
                 node = F_NODE(face, face_thread, n);
                 if (NODE_MARK(node) == 0) {
                     wet_nodes_size++;
                     dynamic_thread_node_size[thread_index]++;
                     NODE_MARK(node) = 1;
                 }
             }
         }
     } end_f_loop(face, face_thread);
 
     initial_coords = (double*) malloc(wet_nodes_size * ND_ND * sizeof(double));
     face_coords = (double*) malloc(wet_face_size * ND_ND * sizeof(double));
 
     begin_f_loop(face, face_thread){
         if (PRINCIPAL_FACE_P(face,face_thread)){
             F_CENTROID(pos, face, face_thread);
             for (dim = 0; dim < ND_ND; dim++) {
                 face_coords[face_index * ND_ND + dim] = pos[dim];
             }
             f_node_loop(face, face_thread, n){
                 node = F_NODE(face, face_thread, n);
                 if (NODE_MARK(node) == 1) {
                     for (dim = 0; dim < ND_ND; dim++){
                         initial_coords[array_index * ND_ND + dim] = NODE_COORD(node)[dim];
                     }
                     NODE_MARK(node) = 0;
                     array_index++;
                 }
             }
             face_index++;
         }
     } end_f_loop(face, face_thread);
 
     printf("  (%d) Setting %d initial node positions ...\n", myid, wet_nodes_size);
     printf("  (%d) Setting %d initial face positions ...\n", myid, wet_face_size);
 
     displ_indices = (int*) malloc(wet_nodes_size * sizeof(int));
     face_indices = (int*) malloc(wet_face_size * sizeof(int));
     array_index = wet_nodes_size - dynamic_thread_node_size[thread_index];
 
     precicec_setMeshVertices(nodeMeshID, wet_nodes_size, initial_coords, displ_indices);
     precicec_setMeshVertices(faceMeshID, wet_face_size, face_coords, face_indices);
 
     printf("  (%d) Set %d (of %d) mesh positions ...\n", myid,
             array_index - wet_nodes_size + dynamic_thread_node_size[thread_index],
             dynamic_thread_node_size[thread_index]);
 
     printf("(%d) Leaving set_mesh_positions()\n", myid);
     #endif /* !RP_HOST  */
}

void read_displacements(Dynamic_Thread* dt)
{
    double* displacements = NULL;
    int offset = 0;
    int i = 0, n = 0, dim = 0;
    Thread* face_thread  = DT_THREAD(dt);
    Node* node;
    face_t face;
    real max_displ_delta = 0.0;

    displacements = (double*) malloc(wet_nodes_size * ND_ND * sizeof(double));
    
    if (dynamic_thread_node_size[thread_index] > 0){
        printf("  (%d) Reading displacements...\n", myid);
        for (i = 0; i < thread_index; i++){
            offset += dynamic_thread_node_size[i];
        }
        printf("  data size for readBlockVectorData = %d\n", dynamic_thread_node_size[thread_index]);

        double relativeReadTime = precicec_getMaxTimeStepSize();
        precicec_readData("moving_base_nodes","Displacements", dynamic_thread_node_size[thread_index], displ_indices,relativeReadTime, displacements);
        
        printf("After readBlockVectorData\n");
        Message("  (%d) Setting displacements...\n", myid);
        i = offset * ND_ND;
        begin_f_loop (face, face_thread){
            if (PRINCIPAL_FACE_P(face,face_thread)){
                f_node_loop (face, face_thread, n){
                    node = F_NODE(face, face_thread, n);
                    if (NODE_POS_NEED_UPDATE(node)){
                        for (dim=0; dim < ND_ND; dim++){
                            NODE_COORD(node)[dim] = initial_coords[i+dim] + displacements[i+dim];
                            if (fabs(displacements[i+dim]) > fabs(max_displ_delta)){
                                max_displ_delta = displacements[i + dim];
                            }
                        }
                        NODE_POS_UPDATED(node);
                        i += ND_ND;
                    }
                }
            }
        } end_f_loop (face, face_thread);

      Message("  (%d) ...done\n", myid);
    }
    Message("  (%d) Max displacement delta: %f\n", myid, max_displ_delta);
}

void write_forces()
{
    double* forces = NULL;
    int i=0, j=0;
    Domain* domain = NULL;
    Dynamic_Thread* dynamic_thread = NULL;
    int thread_counter = 0;
    int face_counter = 0;
    double area[ND_ND];
    double pressure_force[ND_ND];
    double viscous_force[ND_ND];
    double total_force[ND_ND];
    double sum_total_force[ND_ND];
    double max_force = 0.0;

    forces = (double*) malloc(wet_face_size * ND_ND * sizeof(double));
    
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
    printf("  (%d) Gather forces...\n", myid);
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
                 NV_VS(viscous_force, =, F_STORAGE_R_N3V(face,face_thread,SV_WALL_SHEAR),*,-1.0);
                 NV_VS(pressure_force, =, area, *, F_P(face,face_thread));
                 NV_VV(total_force, =, viscous_force, +, pressure_force);
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
           printf("  dynamic_thread->profile_udf_name is not 'gridmotions'");
       }
       dynamic_thread = dynamic_thread->next;
    }
    printf("  (%d) ...done (with %d force values)\n", myid, i);
    printf("  (%d) Writing forces...\n", myid);
    precicec_writeData("moving_base_faces","Forces", wet_face_size, face_indices, forces);
    printf("  (%d) ...done\n", myid );
    printf("  (%d) Max force: %f\n", myid, max_force);
    if (thread_counter != dynamic_thread_size){
        printf ( "  (%d) ERROR: Number of dynamic threads has changed to %d!\n", myid, thread_counter );
        exit(1);
    }
}