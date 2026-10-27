/*============================================================================
 * Initial states of the two-phase Riemann problem.
 *
 * With the homogeneous two-phase equation of state, the thermodynamic state
 * of a cell is (density, total energy, volume/mass/energy fractions). The
 * GUI initialization by pressure and temperature is not available for this
 * model, so the states are set here and the indicator ithvar tells the
 * compressible module that density and energy are the imposed variables.
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

  cs_real_t *cpro_rom = CS_F_(rho)->val;
  cs_real_t *cvar_en  = CS_F_(e_tot)->val;
  cs_real_t *cvar_pr  = CS_F_(p)->val;
  cs_real_t *cvar_fv  = CS_F_(volume_f)->val;
  cs_real_t *cvar_fm  = CS_F_(mass_f)->val;
  cs_real_t *cvar_fe  = CS_F_(energy_f)->val;
  cs_real_3_t *cvar_vel = (cs_real_3_t *)CS_F_(vel)->val;

  /* Riemann problem of Helluy, Hurisse and Le Coupanec (2016), table 1.
     Fractions are those of the steam (phase 1). Note that the right-state
     velocity is -0.817 m/s: the Rankine-Hugoniot relations with the shock
     speed 857.774 m/s fix its sign (see the README). */

  const cs_real_t x_d = 0.5;   /* initial discontinuity */

  /*                           alpha                 y
                               z                     rho   u     p   */
  const cs_real_t st_l[6] = {1.38006860749359000e-1, 7.84348344805274079e-3,
                             1.51281566759211995e-2, 728., 5.0,  7.69e6};
  const cs_real_t st_r[6] = {1.48006860749359009e-1, 8.84348344805273995e-3,
                             1.61281566759211986e-2, 733., -0.817013411825560532,
                             4.02907811372492649e6};

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++) {

    const cs_real_t *st = (cell_cen[c_id][0] < x_d) ? st_l : st_r;

    const cs_real_t alpha = st[0], y = st[1], z = st[2];
    const cs_real_t rho = st[3], u = st[4], p = st[5];

    /* Mixture specific internal energy e(alpha, y, z, p, v) from the
       two-phase equation of state (inverse of the pressure law) */
    const cs_real_t e_int = cs_hgn_thermo_ie(alpha, y, z, p, 1./rho);

    cvar_vel[c_id][0] = u;
    cvar_vel[c_id][1] = 0.;
    cvar_vel[c_id][2] = 0.;

    cpro_rom[c_id] = rho;
    cvar_en[c_id]  = e_int + 0.5*u*u;
    cvar_pr[c_id]  = p;     /* recomputed by the module, kept for the log */

    cvar_fv[c_id] = alpha;
    cvar_fm[c_id] = y;
    cvar_fe[c_id] = z;
  }

  /* Density (3) and total energy (7) are the two imposed variables:
     ithvar = 10000 * 3 * 7. Pressure and temperature are deduced. */

  cs_get_glob_cf_model()->ithvar = 210000;
}

END_C_DECLS
