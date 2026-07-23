/*============================================================================
 * Ctwr_Counterflow_Tower - define the packing (exchange) zone.
 *
 * The mesh, model, fluid properties and boundary conditions are set through the
 * GUI (setup.xml). Only the packing exchange zone must be declared in C: the
 * cooling tower module builds its exchange zones from cs_ctwr_define, there is
 * no GUI panel for them. Counter-current fill, water injected at 36 degC.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_zones(void)
{
  const cs_real_t  q_l = 2.0;      /* injected water mass flow rate [kg/s] */
  const cs_real_t  surf = 8.0;     /* water exchange surface [m2] */
  cs_ctwr_define("z > 3.0 and z < 7.0",   /* packing band */
                 -1,                       /* create the zone */
                 CS_CTWR_COUNTER_CURRENT,
                 -1.0,                     /* use the exchange law (not imposed dT) */
                 0.1,                      /* relaxation */
                 36.0,                     /* injected water temperature [degC] */
                 q_l,
                 0.2,                      /* exchange law constant A */
                 0.5,                      /* exchange law exponent n */
                 surf,
                 -1.0);                    /* no leakage */
}

END_C_DECLS
