#ifndef MOCK_UDF_H
#define MOCK_UDF_H
#include "mockedFluent.c"
/*
 * If you already have Domain, Dynamic_Thread, real, etc. in mock_fsi.h,
 * you can include it here:
 */
// #include "mock_fsi.h"

/* 
 * DEFINE_INIT(name, domain_var):
 * Expands to a C function with signature: 
 *   void name(Domain *domain_var)
 */
#define DEFINE_INIT(name, domain_arg) \
  void name(Domain *domain_arg)

/* 
 * DEFINE_ON_DEMAND(name):
 * Expands to a C function with signature:
 *   void name(void)
 */
#define DEFINE_ON_DEMAND(name) \
  void name(void)

/*
 * DEFINE_GRID_MOTION(name, domain, dt, time, dtime):
 * Expands to:
 *   void name(Domain *domain, Dynamic_Thread *dt, real time, real dtime)
 */
#define DEFINE_GRID_MOTION(name, domain_arg, dt_arg, time_arg, dtime_arg) \
  void name(Domain *domain_arg, Dynamic_Thread *dt_arg, real time_arg, real dtime_arg)

/* 
 * DEFINE_EXECUTE_AT_END, DEFINE_ADJUST, etc., can be mocked similarly if needed.
 */

#endif /* MOCK_UDF_H */
