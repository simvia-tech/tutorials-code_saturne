/*============================================================================
 * Initial states of the air-helium shock tube.
 *
 * Each side is given by its pressure, temperature and composition; density
 * and total energy are deduced by the compressible module once the gas mix
 * model has computed the local specific heat and molar mass.
 *============================================================================*/

#include "cs_headers.h"

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

  cs_real_t *cvar_pr = CS_F_(p)->val;
  cs_real_t *cvar_tk = CS_F_(t_kelvin)->val;
  cs_real_t *cvar_yo2 = cs_field_by_name("y_o2")->val;
  cs_real_t *cvar_yn2 = cs_field_by_name("y_n2")->val;
  cs_real_3_t *cvar_vel = (cs_real_3_t *)CS_F_(vel)->val;

  /* Left: air at (p_l, t_l); right: helium at (p_r, t_r) */

  const cs_real_t x_d = 0.;
  const cs_real_t p_l = 1.e5, t_l = 300.;
  const cs_real_t p_r = 1.e4, t_r = 300.;

  /* Air composition by mass */
  const cs_real_t y_o2_air = 0.233, y_n2_air = 0.767;

  /* Trace of air in the helium. Two auxiliary properties of the gas mix
     (molar mass of the non-condensable gases, steam binary diffusion) are
     computed by a division that gives 0/0 where no solved species is
     present. They are only used by the wall condensation model, but their
     NaN values would stop the run. */
  const cs_real_t y_air_he = 1.e-6;

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++) {

    const bool left = (cell_cen[c_id][0] < x_d);

    cvar_pr[c_id] = left ? p_l : p_r;
    cvar_tk[c_id] = left ? t_l : t_r;
    cvar_yo2[c_id] = (left ? 1. : y_air_he) * y_o2_air;
    cvar_yn2[c_id] = (left ? 1. : y_air_he) * y_n2_air;

    cvar_vel[c_id][0] = 0.;
    cvar_vel[c_id][1] = 0.;
    cvar_vel[c_id][2] = 0.;
  }

  /* Pressure (2) and temperature (5) are the two imposed variables:
     ithvar = 10000 * 2 * 5. Density and total energy are deduced. */

  cs_get_glob_cf_model()->ithvar = 100000;
}

END_C_DECLS
