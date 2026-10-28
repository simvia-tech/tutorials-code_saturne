/*============================================================================
 * Local Nusselt number along the bottom wall (uniform wall heat flux).
 *
 * At the last time step, for every boundary face of the bottom wall:
 *   - the wall heat flux q_w comes from the "boundary_thermal_flux" function
 *     of code_saturne (the flux used by the solver, counted positive when it
 *     leaves the fluid): it must equal the imposed value;
 *   - the wall temperature T_w is the boundary temperature field;
 *   - the bulk temperature T_b is the velocity-weighted average of the
 *     temperature over the column of cells at the same x (cartesian mesh),
 *     and is compared with its exact value from the energy balance,
 *     T_b(x) = T_in + q_w x / (rho cp U_m h) for two heated walls;
 *   - Nu = q_w D_h / (lambda (T_w - T_b)), with D_h = 4 h.
 * The results are written to nusselt_bottom_wall.csv in the run directory.
 * The routine is written for a serial run.
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
  const cs_real_3_t *cell_cen = mq->cell_cen;
  const cs_real_t *cell_vol = mq->cell_vol;
  const cs_real_3_t *b_face_cog = mq->b_face_cog;

  const cs_real_t *t = cs_thermal_model_field()->val;
  const cs_real_3_t *vel = (const cs_real_3_t *)CS_F_(vel)->val;

  /* Case parameters (notebook of setup.xml and wall/inlet values) */

  const cs_real_t h = cs_notebook_parameter_value_by_name("h");
  const cs_real_t u_mean = cs_notebook_parameter_value_by_name("u_mean");
  const cs_real_t lambda = cs_notebook_parameter_value_by_name("lambda");
  const cs_real_t q_set = 100., t_in = 300.;   /* as in setup.xml */
  const cs_real_t *t_bnd = cs_field_by_name("boundary_temperature")->val;

  const cs_real_t d_h = 4.*h;
  const cs_fluid_properties_t *fp = cs_glob_fluid_properties;
  const cs_real_t pr = fp->viscl0*fp->cp0/lambda;
  const cs_real_t re = fp->ro0*u_mean*d_h/fp->viscl0;

  /* Columns of cells: cells of a column share the x of their centre */

  cs_real_t *xc = nullptr;
  cs_lnum_t *order = nullptr;
  CS_MALLOC(xc, n_cells, cs_real_t);
  CS_MALLOC(order, n_cells, cs_lnum_t);
  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
    xc[c_id] = cell_cen[c_id][0];
  cs_order_real_allocated(nullptr, xc, order, n_cells);

  cs_real_t *x_col = nullptr;
  int *col = nullptr;
  CS_MALLOC(x_col, n_cells, cs_real_t);
  CS_MALLOC(col, n_cells, int);
  int n_col = 0;
  for (cs_lnum_t k = 0; k < n_cells; k++) {
    const cs_lnum_t c_id = order[k];
    if (n_col == 0 || fabs(xc[c_id] - x_col[n_col-1]) > 1e-9) {
      x_col[n_col] = xc[c_id];
      n_col++;
    }
    col[c_id] = n_col - 1;
  }

  /* Bulk temperature of each column: int(u T dV) / int(u dV) */

  cs_real_t *s_ut = nullptr, *s_u = nullptr;
  CS_MALLOC(s_ut, n_col, cs_real_t);
  CS_MALLOC(s_u, n_col, cs_real_t);
  for (int i = 0; i < n_col; i++) {
    s_ut[i] = 0.;
    s_u[i] = 0.;
  }
  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++) {
    s_ut[col[c_id]] += vel[c_id][0]*t[c_id]*cell_vol[c_id];
    s_u[col[c_id]]  += vel[c_id][0]*cell_vol[c_id];
  }

  /* Wall heat flux on the bottom wall */

  const cs_zone_t *z = cs_boundary_zone_by_name("bottom_wall");
  cs_real_t *q_w = nullptr;
  CS_MALLOC(q_w, z->n_elts, cs_real_t);
  cs_function_evaluate(cs_function_by_name("boundary_thermal_flux"),
                       ts, CS_MESH_LOCATION_BOUNDARY_FACES,
                       z->n_elts, z->elt_ids, q_w);

  /* Local Nusselt number */

  FILE *f = fopen("nusselt_bottom_wall.csv", "w");
  fprintf(f, "x,x_star,q_wall,t_wall,t_bulk,t_bulk_exact,nusselt\n");

  for (cs_lnum_t k = 0; k < z->n_elts; k++) {
    const cs_lnum_t face_id = z->elt_ids[k];
    const cs_real_t x = b_face_cog[face_id][0];

    /* column of the face: nearest column abscissa */
    int i = 0;
    cs_real_t d_min = fabs(x - x_col[0]);
    for (int j = 1; j < n_col; j++) {
      const cs_real_t d_j = fabs(x - x_col[j]);
      if (d_j < d_min) {
        d_min = d_j;
        i = j;
      }
    }

    const cs_real_t t_b = s_ut[i]/s_u[i];
    const cs_real_t t_w = t_bnd[face_id];
    /* exact bulk temperature from the energy balance (both walls heated) */
    const cs_real_t t_b_ex = t_in + q_set*x/(fp->ro0*fp->cp0*u_mean*h);
    /* the function returns the flux leaving the fluid: minus sign */
    const cs_real_t nu = -q_w[k]*d_h/(lambda*(t_w - t_b));

    fprintf(f, "%.6e,%.6e,%.6e,%.6e,%.6e,%.6e,%.6e\n",
            x, x/(d_h*re*pr), -q_w[k], t_w, t_b, t_b_ex, nu);
  }
  fclose(f);

  bft_printf("Local Nusselt number written to nusselt_bottom_wall.csv"
             " (Re_Dh = %g, Pr = %g)\n", re, pr);

  CS_FREE(xc);
  CS_FREE(order);
  CS_FREE(x_col);
  CS_FREE(col);
  CS_FREE(s_ut);
  CS_FREE(s_u);
  CS_FREE(q_w);
}

END_C_DECLS
