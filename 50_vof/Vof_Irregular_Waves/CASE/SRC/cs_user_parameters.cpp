/*============================================================================
 * Reference density of the hydrostatic pressure split.
 *
 * code_saturne solves for the reduced pressure p* = p - ro0 g.(x - x0).
 * With the VOF model ro0 defaults to the density of the heavier fluid, so
 * that the light air above the free surface is left with a large
 * gradient of p* to balance numerically, which shakes a water surface that
 * should stay at rest. Taking the air density as reference removes it: the
 * hydrostatic gradient is then carried by the water, where it is harmless.
 *============================================================================*/

#include "cs_headers.h"

#include "wave_tank.h"

void
cs_user_finalize_setup([[maybe_unused]] cs_domain_t  *domain)
{
  cs_get_glob_fluid_properties()->ro0 = wave_tank::rho_air;
}
