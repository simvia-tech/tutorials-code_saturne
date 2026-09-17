/*============================================================================
 * Cpl_FMI_Heater_Control - the sensor of the control loop.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The FMU exchanges notebook variables and nothing else. The controller needs
 * a measurement, so at the end of every time step this routine computes the
 * mixing (bulk) temperature over the outlet faces,
 *
 *     T_out = sum(mass_flux * T) / sum(mass_flux),
 *
 * and stores it in the notebook variable `t_out`. The FMU then reads that
 * variable and hands it to the controller as the FMI output.
 *
 * `t_out` must be declared *editable* in the notebook even though nothing
 * outside the solver ever sets it: cs_notebook_parameter_set_value refuses
 * non-editable entries.
 *
 * The cell value next to each outlet face is used rather than the boundary
 * temperature property, so the measurement is the value the solver has just
 * converged, with no one-step lag.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_extra_operations(cs_domain_t  *domain)
{
  const cs_mesh_t      *m  = domain->mesh;
  const cs_time_step_t *ts = domain->time_step;

  const cs_zone_t *z = cs_boundary_zone_by_name_try("outlet");
  if (z == nullptr)
    return;

  /* mass flux through the boundary faces, carried by the velocity field */
  const int kbmasf = cs_field_key_id("boundary_mass_flux_id");
  const int iflmab = cs_field_get_key_int(CS_F_(vel), kbmasf);
  const cs_real_t *b_mass_flux = cs_field_by_id(iflmab)->val;

  const cs_real_t *t_cell = CS_F_(t)->val;

  cs_real_t sum[2] = {0., 0.};
  for (cs_lnum_t i = 0; i < z->n_elts; i++) {
    const cs_lnum_t f_id = z->elt_ids[i];
    const cs_lnum_t c_id = m->b_face_cells[f_id];
    sum[0] += b_mass_flux[f_id]*t_cell[c_id];
    sum[1] += b_mass_flux[f_id];
  }
  cs_parall_sum(2, CS_REAL_TYPE, sum);

  if (fabs(sum[1]) < 1e-12)
    return;

  const cs_real_t t_out = sum[0]/sum[1];
  cs_notebook_parameter_set_value("t_out", t_out);

  if (ts->nt_cur % 100 == 0)
    bft_printf("Cpl_FMI_Heater_Control: t = %.3f s, t_out = %.3f K\n",
               ts->t_cur, t_out);
}

END_C_DECLS
