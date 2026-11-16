/*============================================================================
 * Wave gauges: surface elevation at a few abscissas, at every time step.
 *
 * A gauge integrates the water fraction over the column of cells nearest
 * to its abscissa: the water depth is the water volume fraction of the
 * column times the tank height. Unlike a point probe, this measure does
 * not depend on the thickness of the interface. The total water volume of
 * the tank is recorded as well.
 * The sums are parallel; the file wave_gauges.csv is written by rank 0.
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
  static FILE *f = nullptr;

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
    }
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

  if (ts->nt_cur % 100 == 0)
    fflush(f);
  if (ts->nt_cur == ts->nt_max) {
    fclose(f);
    f = nullptr;
  }
}
