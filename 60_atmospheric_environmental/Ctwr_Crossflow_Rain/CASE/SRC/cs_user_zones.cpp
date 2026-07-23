/*============================================================================
 * Ctwr_Crossflow_Rain - packing (cross-current) + rain zone.
 *
 * A cross-flow cooling tower: air flows horizontally (+x) while water falls
 * (-z) through the packing, so the two streams cross. A positive leakage factor
 * turns part of the injected water into rain: droplets leak out of the packing
 * and fall through the rest of the domain (the rain model, ym_l_r), a second
 * cooling-tower feature exercised here.
 *
 * Only the exchange zone is set in C (cs_ctwr_define); the model, mesh and
 * boundary conditions are set through the GUI (setup.xml).
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_zones(void)
{
  const cs_real_t  q_l = 3.0;      /* injected water mass flow rate [kg/s] */
  const cs_real_t  surf = 8.0;     /* water exchange surface [m2] */
  const cs_real_t  xleak = 0.15;   /* leakage factor -> rain (15% of the flow) */

  cs_ctwr_define("x > 1.0 and x < 3.0 and z > 2.0 and z < 5.0",  /* packing block */
                 -1,                       /* create the zone */
                 CS_CTWR_CROSS_CURRENT,    /* air horizontal, water vertical */
                 -1.0,                     /* use the exchange law */
                 0.1,                      /* relaxation */
                 36.0,                     /* injected water temperature [degC] */
                 q_l,
                 0.2,                      /* exchange law constant A */
                 0.5,                      /* exchange law exponent n */
                 surf,
                 xleak);                   /* leakage -> rain */
}

END_C_DECLS
