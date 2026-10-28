/*============================================================================
 * Bulk quantities of the periodic heated channel at the last time step.
 *
 *   - bulk velocity and bulk temperature (velocity-weighted average);
 *   - wall averages of the temperature, the shear stress, y+ and the heat
 *     flux (the latter from the "boundary_thermal_flux" function, counted
 *     positive when it leaves the fluid);
 *   - friction coefficient Cf = 2 tau_w / (rho U_b^2), Re_Dh, and
 *     Nu = q_w D_h / (lambda (T_w - T_b)) with D_h = 4 h.
 * Written to channel_summary.csv; the wall-normal profiles of velocity,
 * temperature and turbulent viscosity of one column of cells are written to
 * profiles.csv. The routine is written for a serial run.
 *============================================================================*/

#include "cs_headers.h"

#include <stdio.h>
#include <math.h>

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is called at the end of each time step.
 *
 * \param[in, out]  domain   pointer to a cs_domain_t structure
 */
/*----------------------------------------------------------------------------*/

void
cs_user_extra_operations(cs_domain_t  *domain)
{
  const cs_time_step_t *ts = domain->time_step;
  if (ts->nt_cur != ts->nt_max || cs_glob_rank_id > 0)
    return;

  const cs_mesh_t *m = domain->mesh;
  const cs_mesh_quantities_t *mq = domain->mesh_quantities;
  const cs_lnum_t n_cells = m->n_cells;
  const cs_real_t *cell_vol = mq->cell_vol;
  const cs_real_3_t *cell_cen = mq->cell_cen;
  const cs_real_t *b_face_surf = mq->b_face_surf;

  const cs_real_t *t = cs_thermal_model_field()->val;
  const cs_real_3_t *vel = (const cs_real_3_t *)CS_F_(vel)->val;
  const cs_real_t *mut = CS_F_(mu_t)->val;
  const cs_real_t *t_bnd = cs_field_by_name("boundary_temperature")->val;
  const cs_real_t *yplus = cs_field_by_name("yplus")->val;
  const cs_real_3_t *stress = (const cs_real_3_t *)cs_field_by_name("boundary_stress")->val;

  const cs_fluid_properties_t *fp = cs_glob_fluid_properties;
  const cs_real_t rho = fp->ro0, mu = fp->viscl0, cp = fp->cp0, lambda = fp->lambda0;
  const cs_real_t h = 0.05, d_h = 4.*h, q_w = 100.;

  /* bulk velocity and temperature */
  cs_real_t su = 0., sut = 0., sv = 0.;
  for (cs_lnum_t c = 0; c < n_cells; c++) {
    su += vel[c][0]*cell_vol[c]; sut += vel[c][0]*t[c]*cell_vol[c]; sv += cell_vol[c];
  }
  const cs_real_t u_b = su/sv, t_b = sut/su;

  /* wall averages */
  const cs_zone_t *z = cs_boundary_zone_by_name("walls");
  cs_real_t *q = nullptr; CS_MALLOC(q, z->n_elts, cs_real_t);
  cs_function_evaluate(cs_function_by_name("boundary_thermal_flux"), ts,
                       CS_MESH_LOCATION_BOUNDARY_FACES, z->n_elts, z->elt_ids, q);
  cs_real_t s_tw = 0., s_tau = 0., s_yp = 0., s_q = 0., s_a = 0.;
  for (cs_lnum_t k = 0; k < z->n_elts; k++) {
    const cs_lnum_t f_id = z->elt_ids[k]; const cs_real_t a = b_face_surf[f_id];
    s_tw += t_bnd[f_id]*a; s_tau += fabs(stress[f_id][0])*a; s_yp += yplus[f_id]*a; s_q += -q[k]*a; s_a += a;
  }
  const cs_real_t t_w = s_tw/s_a, tau_w = s_tau/s_a, yp = s_yp/s_a, q_mean = s_q/s_a;
  const cs_real_t u_tau = sqrt(tau_w/rho);
  const cs_real_t re_dh = rho*u_b*d_h/mu, pr = mu*cp/lambda;
  const cs_real_t cf = 2.*tau_w/(rho*u_b*u_b);
  const cs_real_t nu = q_w*d_h/(lambda*(t_w - t_b));

  FILE *f = fopen("channel_summary.csv", "w");
  fprintf(f, "u_b,t_b,t_w,tau_w,u_tau,re_tau,yplus,cf,re_dh,pr,q_wall_mean,nusselt\n");
  fprintf(f, "%.6e,%.6e,%.6e,%.6e,%.6e,%.6e,%.6e,%.6e,%.6e,%.6e,%.6e,%.6e\n",
          u_b, t_b, t_w, tau_w, u_tau, u_tau*h/(mu/rho), yp, cf, re_dh, pr, q_mean, nu);
  fclose(f);
  bft_printf("Channel summary: Re_Dh = %.0f, Re_tau = %.1f, y+ = %.1f, Cf = %.5f, Nu = %.2f (T_w - T_b = %.3f K)\n",
             re_dh, u_tau*h/(mu/rho), yp, cf, nu, t_w - t_b);

  /* profiles: cells of the column nearest to x = 0.05 */
  f = fopen("profiles.csv", "w");
  fprintf(f, "y,u,t,mu_t\n");
  for (cs_lnum_t c = 0; c < n_cells; c++)
    if (fabs(cell_cen[c][0] - 0.055) < 0.006)
      fprintf(f, "%.6e,%.6e,%.6e,%.6e\n", cell_cen[c][1], vel[c][0], t[c], mut[c]);
  fclose(f);
  CS_FREE(q);
}

END_C_DECLS
