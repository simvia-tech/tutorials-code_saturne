/*============================================================================
 * Ctwr_Counterflow_Tower - inlet values for the cooling-tower model variables.
 *
 * The boundary types and the standard inlet quantities (velocity, turbulence,
 * air temperature) are set in the GUI. The v9.1 GUI writes the cooling-tower
 * model-variable BCs (humidity, rain, packing liquid) but the solver does not
 * consume them, so they are prescribed here at the air inlet: incoming air
 * carries some humidity and no rain / no packing liquid. The boundary type set
 * by the GUI (CS_INLET) is left untouched.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_boundary_conditions([[maybe_unused]] cs_domain_t  *domain,
                            [[maybe_unused]] int           bc_type[])
{
  const cs_zone_t *zn = cs_boundary_zone_by_name("air_inlet");

  /* model variable name -> inlet Dirichlet value */
  const char *names[] = {"ym_water", "ym_l_r", "ymh_l_r",
                         "y_l_packing", "yh_l_packing"};
  const cs_real_t  vals[] = {0.006, 0.0, 0.0, 0.0, 0.0};

  for (int iv = 0; iv < 5; iv++) {
    cs_field_t *f = cs_field_by_name_try(names[iv]);
    if (f == nullptr) continue;
    for (cs_lnum_t e = 0; e < zn->n_elts; e++)
      f->bc_coeffs->rcodcl1[zn->elt_ids[e]] = vals[iv];
  }
}

END_C_DECLS
