#include "fsi.h"

/* This is a general purpose macro which does basic initialisation 
 * of all UDF relevant variables after the flow field has been 
 * initialised in Fluent
 * Ref. ANSYS v19.1 ANSYS_Fluent_UDF_Manual.pdf Section 2.2.8
 * */
DEFINE_INIT(init,domain)
{
  fsi_init(domain);
}

/* This is a general purpose macro to have "on_demand" behavior.
 * This macro will be executed immediately on activation but will
 * not be available while the solver is iterating
 * Ref. ANSYS v19.1 ANSYS_Fluent_UDF_Manual.pdf Section 2.2.9
 * */
DEFINE_ON_DEMAND(write_and_advance)
{
  fsi_write_and_advance();
}

/* This macro is part of Dynamic Mesh DEFINE Marcos to define 
 * the behavior of a dynamic mesh. FLUENT dictates that this 
 * Macro works only as a compiled UDF
 * Ref: ANSYS v19.1 ANSYS_Fluent_UDF_Manual.pdf Section 2.6.4 
 * */
DEFINE_GRID_MOTION(gridmotions,domain,dt,time,dtime)
{
  fsi_grid_motion(domain, dt, time, dtime);
}

/* Same as above */
DEFINE_ON_DEMAND(plot_coords)
{
  fsi_plot_coords();
}
 
