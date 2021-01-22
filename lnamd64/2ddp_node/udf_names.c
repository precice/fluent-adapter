/* This file generated automatically. */
/*          Do not modify.            */
#include "udf.h"
#include "prop.h"
#include "dpm.h"
extern DEFINE_INIT(init,domain);
extern DEFINE_ON_DEMAND(write_and_advance);
extern DEFINE_GRID_MOTION(gridmotions,domain,dt,time,dtime);
extern DEFINE_ON_DEMAND(plot_coords);
UDF_Data udf_data[] = {
{"init", (void (*)(void))init, UDF_TYPE_INIT},
{"write_and_advance", (void (*)(void))write_and_advance, UDF_TYPE_ON_DEMAND},
{"gridmotions", (void (*)(void))gridmotions, UDF_TYPE_GRID_MOTION},
{"plot_coords", (void (*)(void))plot_coords, UDF_TYPE_ON_DEMAND},
};
int n_udf_data = sizeof(udf_data)/sizeof(UDF_Data);
#include "version.h"
void UDF_Inquire_Release(int *major, int *minor, int *revision)
{
  *major = RampantReleaseMajor;
  *minor = RampantReleaseMinor;
  *revision = RampantReleaseRevision;
}
