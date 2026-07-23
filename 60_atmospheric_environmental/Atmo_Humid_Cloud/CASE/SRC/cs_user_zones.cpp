/*============================================================================
 * Atmo_Humid_Cloud - define the boundary zones.
 *
 * The cartesian mesher names the six boundary groups X0/X1/Y0/Y1/Z0/Z1
 * (z is the vertical). The boundary conditions are then applied by name in
 * cs_user_boundary_conditions.cpp:
 *   - inlet  (X0) and top (Z1) : meteo inlet (profile from the meteo file)
 *   - outlet (X1)              : free outlet
 *   - ground (Z0)              : rough wall
 *   - sides  (Y0, Y1)          : symmetry (2D slice)
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_zones(void)
{
  cs_boundary_zone_define("inlet",  "X0", 0);
  cs_boundary_zone_define("outlet", "X1", 0);
  cs_boundary_zone_define("top",    "Z1", 0);
  cs_boundary_zone_define("ground", "Z0", 0);
  cs_boundary_zone_define("sides",  "Y0 or Y1", 0);
}

END_C_DECLS
