/*============================================================================
 * Initial states of the depressurization tube.
 *
 * Each side of the tube is a steam-water mixture at a common pressure and
 * temperature with a given steam volume fraction; the interface is spread
 * over a width xw by a hyperbolic tangent. All values come from the notebook
 * of setup.xml so that the states can be changed from the command line.
 *
 * With the homogeneous two-phase equation of state, the thermodynamic state
 * of a cell is (density, total energy, volume/mass/energy fractions): the
 * states are built here from the phasic laws and the indicator ithvar tells
 * the compressible module that density and energy are the imposed variables.
 *============================================================================*/

#include "cs_headers.h"

#include <math.h>

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*!
 * \brief Initialize variables.
 *
 * \param[in, out]  domain   pointer to a cs_domain_t structure
 */
/*----------------------------------------------------------------------------*/

void
cs_user_initialization([[maybe_unused]] cs_domain_t   *domain)
{
  /* Do not reinitialize a restarted computation */
  if (domain->time_step->nt_prev > 0)
    return;

  const cs_lnum_t n_cells = domain->mesh->n_cells;
  const cs_real_3_t *cell_cen = domain->mesh_quantities->cell_cen;

  cs_real_t *cpro_rom = CS_F_(rho)->val;
  cs_real_t *cvar_en  = CS_F_(e_tot)->val;
  cs_real_t *cvar_pr  = CS_F_(p)->val;
  cs_real_t *cvar_tk  = CS_F_(t_kelvin)->val;
  cs_real_t *cvar_fv  = CS_F_(volume_f)->val;
  cs_real_t *cvar_fm  = CS_F_(mass_f)->val;
  cs_real_t *cvar_fe  = CS_F_(energy_f)->val;
  cs_real_3_t *cvar_vel = (cs_real_3_t *)CS_F_(vel)->val;

  /* Left and right states (pressure, temperature, steam volume fraction) */

  const cs_real_t x_d = 0.5;    /* interface position */
  const cs_real_t p_l = cs_notebook_parameter_value_by_name("p_left");
  const cs_real_t t_l = cs_notebook_parameter_value_by_name("t_left");
  const cs_real_t a_l = cs_notebook_parameter_value_by_name("alpha_left");
  const cs_real_t p_r = cs_notebook_parameter_value_by_name("p_right");
  const cs_real_t t_r = cs_notebook_parameter_value_by_name("t_right");
  const cs_real_t a_r = cs_notebook_parameter_value_by_name("alpha_right");
  const cs_real_t xw  = cs_notebook_parameter_value_by_name("xw");

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++) {

    /* Local (p, T, alpha): sharp jump, or smoothed over the width xw */

    cs_real_t p, t, alpha;
    if (xw > 0.) {
      const cs_real_t w = 0.5*(1. + tanh((cell_cen[c_id][0] - x_d)/xw));
      p     = (1.-w)*p_l + w*p_r;
      t     = (1.-w)*t_l + w*t_r;
      alpha = (1.-w)*a_l + w*a_r;
    }
    else if (cell_cen[c_id][0] < x_d) {
      p = p_l; t = t_l; alpha = a_l;
    }
    else {
      p = p_r; t = t_r; alpha = a_r;
    }

    /* Mixture at common (p, t): phase 1 (index 0) is the steam,
       phase 2 (index 1) the liquid. */

    const cs_real_t v1 = cs_hgn_phase_thermo_specific_volume_tp(t, p, 0);
    const cs_real_t v2 = cs_hgn_phase_thermo_specific_volume_tp(t, p, 1);
    const cs_real_t e1 = cs_hgn_phase_thermo_internal_energy_tp(t, p, 0);
    const cs_real_t e2 = cs_hgn_phase_thermo_internal_energy_tp(t, p, 1);

    const cs_real_t y  = (alpha/v1) / (alpha/v1 + (1.-alpha)/v2); /* mass fraction   */
    const cs_real_t v  = y*v1 + (1.-y)*v2;                         /* specific volume */
    const cs_real_t e  = y*e1 + (1.-y)*e2;                         /* internal energy */
    const cs_real_t z  = y*e1 / e;                                 /* energy fraction */

    cvar_vel[c_id][0] = 0.;
    cvar_vel[c_id][1] = 0.;
    cvar_vel[c_id][2] = 0.;

    cpro_rom[c_id] = 1./v;
    cvar_en[c_id]  = e;          /* total energy, fluid at rest */
    cvar_pr[c_id]  = p;          /* recomputed by the module, kept for the log */
    cvar_tk[c_id]  = t;

    cvar_fv[c_id] = alpha;
    cvar_fm[c_id] = y;
    cvar_fe[c_id] = z;
  }

  /* Density (3) and total energy (7) are the two imposed variables:
     ithvar = 10000 * 3 * 7. Pressure and temperature are deduced. */

  cs_get_glob_cf_model()->ithvar = 210000;
}

END_C_DECLS
