/*============================================================================
 * Th_Balance_By_Zone - print scalar and pressure-drop balances by zone.
 *
 * This routine IS the tutorial's showcased feature: at the last time step it
 * asks code_saturne for
 *  - the term-by-term budget of the temperature equation over the whole
 *    domain and over the downstream half of the channel
 *    (cs_balance_by_zone), and
 *  - the pressure drop over the whole channel (cs_pressure_drop_by_zone).
 * The tables are printed in run_solver.log.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_extra_operations(cs_domain_t  *domain)
{
  const cs_time_step_t *ts = domain->time_step;

  /* Only at the last time step (steady state reached) */

  if (ts->nt_cur == ts->nt_max) {

    /* Temperature budget over the whole domain */
    cs_balance_by_zone("all[]", "temperature");

    /* Temperature budget over the downstream half of the channel */
    cs_balance_by_zone("box[0.5, -0.1, -0.1, 1.1, 0.1, 0.1]", "temperature");

    /* Pressure drop over the whole channel */
    cs_pressure_drop_by_zone("all[]");
  }
}

END_C_DECLS
