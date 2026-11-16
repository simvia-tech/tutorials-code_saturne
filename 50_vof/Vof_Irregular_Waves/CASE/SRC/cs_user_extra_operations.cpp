/*============================================================================
 * Wave gauges: surface elevation at a few abscissas, at every time step.
 *
 * A gauge integrates the water fraction over the column of cells nearest
 * to its abscissa: the water depth is the water volume fraction of the
 * column times the tank height. Unlike a point probe, this measure does
 * not depend on the thickness of the interface. The total water volume of
 * the tank is recorded as well.
 * The sums are parallel; the file wave_gauges.csv is written by rank 0.
 *
 * Wave loads: when the breakwater is present, the pressure force of the
 * fluid on it is summed over the faces of the "structure" zone and written
 * per unit width (N/m) to structure_forces.csv.
 *============================================================================*/

#include "cs_headers.h"

#include "wave_tank.h"

void
cs_user_extra_operations(cs_domain_t  *domain)
{
  using namespace wave_tank;

  const cs_lnum_t n_cells = domain->mesh->n_cells;
  const cs_real_3_t *cell_cen = domain->mesh_quantities->cell_cen;
  const cs_real_t *cell_vol = domain->mesh_quantities->cell_vol;
  const cs_real_t *vf = cs_field_by_name("void_fraction")->val;
  const cs_time_step_t *ts = domain->time_step;

  /* First call: abscissa of the column of cells nearest to each gauge */

  static double x_col[n_gauges];
  static bool first_call = true;
  static FILE *f = nullptr, *f_forces = nullptr;

  const cs_zone_t *zn = cs_boundary_zone_by_name("structure");
  const bool structure = (zn->n_g_elts > 0);

  if (first_call) {
    first_call = false;

    double d_min[n_gauges];
    for (int i = 0; i < n_gauges; i++) {
      d_min[i] = 1e30;
      for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
        d_min[i] = cs::min(d_min[i], std::fabs(cell_cen[c_id][0] - gauges[i]));
    }
    cs_parall_min(n_gauges, CS_DOUBLE, d_min);

    /* If the gauge sits on a face, two columns tie: keep the left one */
    for (int i = 0; i < n_gauges; i++) {
      x_col[i] = 1e30;
      for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
        if (std::fabs(cell_cen[c_id][0] - gauges[i]) < d_min[i] + 1e-9)
          x_col[i] = cs::min(x_col[i], cell_cen[c_id][0]);
    }
    cs_parall_min(n_gauges, CS_DOUBLE, x_col);

    if (cs_glob_rank_id <= 0) {
      f = fopen("wave_gauges.csv", "w");
      fprintf(f, "time");
      for (int i = 0; i < n_gauges; i++)
        fprintf(f, ",eta_x%.3f", x_col[i]);
      fprintf(f, ",water_volume\n");

      if (structure) {
        f_forces = fopen("structure_forces.csv", "w");
        fprintf(f_forces, "time,fx,fy\n");
      }
    }
  }

  /* Pressure force of the fluid on the breakwater.
     - In a closed tank the pressure is defined up to a constant: it is
       counted from the pressure of the air under the lid.
     - The solved variable is the reduced pressure p* = p - ro0 g.(x - x0);
       it is brought from the cell centre to the face with its hydrostatic
       gradient (rho - ro0) g, and the ro0 part is added back.
     The vertical force is then the weight of the water above the block
     plus the wave load. */

  double force[3] = {0., 0., 0.};

  if (structure) {
    const cs_real_t *b_face_surf = domain->mesh_quantities->b_face_surf;
    const cs_real_3_t *b_face_cog = domain->mesh_quantities->b_face_cog;
    const cs_nreal_3_t *b_face_u_normal
      = domain->mesh_quantities->b_face_u_normal;
    const cs_lnum_t *b_face_cells = domain->mesh->b_face_cells;
    const cs_real_t *p = CS_F_(p)->val;
    const cs_real_t *rho = CS_F_(rho)->val;
    const double ro0 = cs_glob_fluid_properties->ro0;

    /* Reference: mean pressure of the air in the top 10 cm of the tank */
    const double y_ref = tank_height - 0.05;
    double p_air[2] = {0., 0.};
    for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++) {
      if (cell_cen[c_id][1] > tank_height - 0.1) {
        p_air[0] += p[c_id]*cell_vol[c_id];
        p_air[1] += cell_vol[c_id];
      }
    }
    cs_parall_sum(2, CS_DOUBLE, p_air);
    const double p_ref = p_air[0]/p_air[1];

    for (cs_lnum_t e_id = 0; e_id < zn->n_elts; e_id++) {
      const cs_lnum_t face_id = zn->elt_ids[e_id];
      const cs_lnum_t c_id = b_face_cells[face_id];
      const double dy_fc = b_face_cog[face_id][1] - cell_cen[c_id][1];
      const double p_face =   p[c_id] - (rho[c_id] - ro0)*g*dy_fc  /* p* at face */
                            - p_ref
                            - ro0*g*(b_face_cog[face_id][1] - y_ref);
      for (int i = 0; i < 3; i++)
        force[i] += p_face*b_face_u_normal[face_id][i]*b_face_surf[face_id];
    }
    cs_parall_sum(3, CS_DOUBLE, force);
  }

  /* Water volume and total volume of each column, water volume of the tank */

  double sums[2*n_gauges + 1];
  for (int i = 0; i < 2*n_gauges + 1; i++)
    sums[i] = 0.;

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++) {
    const double water = (1. - vf[c_id])*cell_vol[c_id];
    sums[2*n_gauges] += water;
    for (int i = 0; i < n_gauges; i++) {
      if (std::fabs(cell_cen[c_id][0] - x_col[i]) < 1e-9) {
        sums[2*i]     += water;
        sums[2*i + 1] += cell_vol[c_id];
      }
    }
  }
  cs_parall_sum(2*n_gauges + 1, CS_DOUBLE, sums);

  if (cs_glob_rank_id > 0)
    return;

  const double h = sea_state().h;

  fprintf(f, "%.6f", ts->t_cur);
  for (int i = 0; i < n_gauges; i++)
    fprintf(f, ",%.8e", sums[2*i]/sums[2*i + 1]*tank_height - h);
  fprintf(f, ",%.10e\n", sums[2*n_gauges]);

  if (structure)
    fprintf(f_forces, "%.6f,%.8e,%.8e\n",
            ts->t_cur, force[0]/tank_width, force[1]/tank_width);

  if (ts->nt_cur % 100 == 0) {
    fflush(f);
    if (structure)
      fflush(f_forces);
  }
  if (ts->nt_cur == ts->nt_max) {
    fclose(f);
    f = nullptr;
    if (structure)
      fclose(f_forces);
  }
}
