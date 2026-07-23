/*============================================================================
 * Gwf_Unsaturated_VanGenuchten - define the mesh zones for the CDO GWF model.
 *
 * Vertical soil column, y is the vertical (gravity along -y). The cartesian
 * mesher names the six boundary groups X0/X1/Y0/Y1/Z0/Z1. Only two zones are
 * needed by the setup (cs_user_parameters.cpp):
 *   - volume zone "cells" : the whole column (Van Genuchten soil)
 *   - boundary  "bottom"  : Y0, the water table (imposed hydraulic head = 0)
 * Every other boundary defaults to no-flow (symmetry), set in cs_user_model.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_zones(void)
{
  cs_volume_zone_define("cells", "all[]", 0);
  cs_boundary_zone_define("bottom", "Y0", 0);
}

END_C_DECLS
