#include "udf.h"
void plot_coords(void);

DEFINE_ON_DEMAND(plot_coords)
{
  /* mock implementation */
}

DEFINE_INIT(init, domain)
{
  /* mock implementation */
}

DEFINE_ON_DEMAND(write_and_advance)
{
  /* mock implementation */
}

DEFINE_GRID_MOTION(gridmotions, domain, dt, time, dtime)
{
  /* mock implementation */
}