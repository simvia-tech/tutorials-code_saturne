/*============================================================================
 * Gwf_Darcy_Tracer - define the mesh zones used by the CDO groundwater model.
 *
 * The cartesian mesher auto-names the six boundary groups X0/X1/Y0/Y1/Z0/Z1.
 * The CDO groundwater setup (cs_user_parameters.cpp) refers to zones by name:
 *   - volume zone "cells" : the whole soil block (soil law)
 *   - boundary "left"      : X0, imposed high hydraulic head + tracer inlet
 *   - boundary "right"     : X1, imposed low  hydraulic head
 * All other boundaries default to no-flow (symmetry), set in cs_user_model.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_zones(void)
{
  /* Soil volume zone: the whole domain */
  cs_volume_zone_define("cells", "all[]", 0);

  /* Inlet / outlet boundary zones (streamwise x faces) */
  cs_boundary_zone_define("left",  "X0", 0);
  cs_boundary_zone_define("right", "X1", 0);
}

END_C_DECLS
