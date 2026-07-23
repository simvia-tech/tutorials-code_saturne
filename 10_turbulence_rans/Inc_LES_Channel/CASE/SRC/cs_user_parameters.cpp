/*============================================================================
 * Inc_LES_Channel - define time-averaged statistics.
 *
 * LES fields are instantaneous; the channel statistics are obtained by
 * time-averaging once the flow is statistically steady. Two moments are
 * accumulated in-solver from time step nt_start (after the initial transient):
 *   - u_mean     : mean velocity <U>            (vector)
 *   - u_variance : Reynolds stress tensor       <u_i u_j> - <u_i><u_j>
 * Both are written to the postprocessing output and used to build the
 * mean-velocity profile and the resolved Reynolds stresses.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_time_moments(void)
{
  const int nt_start = 5000;   /* begin averaging after the transient */

  int moment_f_id[] = {CS_F_(vel)->id};
  int moment_c_id[] = {-1};
  int n_fields = 1;

  /* Mean velocity <U> */
  cs_time_moment_define_by_field_ids("u_mean",
                                     n_fields,
                                     moment_f_id,
                                     moment_c_id,
                                     CS_TIME_MOMENT_MEAN,
                                     nt_start,
                                     -1,
                                     CS_TIME_MOMENT_RESTART_AUTO,
                                     nullptr);

  /* Reynolds stress tensor <u_i u_j> - <u_i><u_j> */
  cs_time_moment_define_by_field_ids("u_variance",
                                     n_fields,
                                     moment_f_id,
                                     moment_c_id,
                                     CS_TIME_MOMENT_VARIANCE,
                                     nt_start,
                                     -1,
                                     CS_TIME_MOMENT_RESTART_AUTO,
                                     nullptr);
}

END_C_DECLS
