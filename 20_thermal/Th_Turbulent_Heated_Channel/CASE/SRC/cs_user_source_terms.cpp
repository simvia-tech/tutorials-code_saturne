/*============================================================================
 * Energy sink of the streamwise-periodic heated channel.
 *
 * The walls inject a uniform heat flux; in a periodic domain the temperature
 * would rise indefinitely. Following Patankar, Liu and Sparrow (1977), the
 * periodic part of the temperature is solved with a volumetric sink equal to
 * the wall heat input, weighted by the local streamwise velocity. The sign
 * follows the GUI convention for the wall flux (negative = heating), so that
 * the sink is the opposite of the wall input.
 *============================================================================*/

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*!
 * \brief Additional source terms for variable equations.
 *
 * \param[in, out]  domain   pointer to a cs_domain_t structure
 * \param[in]       f_id     field id of the variable
 * \param[out]      st_exp   explicit source term
 * \param[out]      st_imp   implicit part of the source term
 */
/*----------------------------------------------------------------------------*/

void
cs_user_source_terms(cs_domain_t  *domain,
                     int           f_id,
                     cs_real_t    *st_exp,
                     cs_real_t    *st_imp)
{
  const cs_field_t *f = cs_field_by_id(f_id);
  if (f != cs_thermal_model_field())
    return;

  const cs_lnum_t n_cells = domain->mesh->n_cells;
  const cs_real_t *cell_vol = domain->mesh_quantities->cell_vol;
  const cs_real_t *b_face_surf = domain->mesh_quantities->b_face_surf;
  const cs_real_3_t *vel = (const cs_real_3_t *)CS_F_(vel)->val;
  const cs_real_t vol_total = domain->mesh_quantities->tot_vol;

  const cs_real_t q_wall = -100.;   /* GUI convention: negative = heating */
  const cs_zone_t *z = cs_boundary_zone_by_name("walls");
  cs_real_t surface = 0.;
  for (cs_lnum_t j = 0; j < z->n_elts; j++)
    surface += b_face_surf[z->elt_ids[j]];
  cs_parall_sum(1, CS_REAL_TYPE, &surface);

  cs_real_t u_mean = 0.;
  for (cs_lnum_t c = 0; c < n_cells; c++)
    u_mean += vel[c][0]*cell_vol[c];
  cs_parall_sum(1, CS_REAL_TYPE, &u_mean);
  u_mean /= vol_total;

  const cs_real_t sink = q_wall*surface/vol_total;   /* W/m3 (rho cp cancel) */
  for (cs_lnum_t c = 0; c < n_cells; c++) {
    st_imp[c] = 0.;
    st_exp[c] = cell_vol[c]*vel[c][0]*sink/u_mean;
  }
}

END_C_DECLS
