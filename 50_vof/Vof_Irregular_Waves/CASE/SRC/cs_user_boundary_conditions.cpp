/*============================================================================
 * Wavemaker: linear irregular waves imposed on the "wavemaker" boundary.
 *
 * Below the instantaneous free surface the velocity is the sum of the Airy
 * components, with Wheeler stretching (the linear profile computed between
 * the bottom and the still water level is stretched up to the moving
 * surface). The void fraction follows the surface elevation.
 *
 * Two corrections make the wavemaker conservative, so that the tank can be
 * closed by a lid and needs no open boundary:
 *   - a uniform return current in the water brings the water flow rate
 *     back to its linear value, which has a zero time average: the tank
 *     does not fill up (it compensates the Stokes drift of the waves);
 *   - a uniform counter-flow in the air balances the water flow rate at
 *     every instant: the net volume flow rate through the boundary is zero.
 *============================================================================*/

#include "cs_headers.h"

#include "wave_tank.h"

void
cs_user_boundary_conditions(cs_domain_t  *domain,
                            int           bc_type[])
{
  const cs_mesh_t *m = domain->mesh;
  const cs_mesh_quantities_t *mq = domain->mesh_quantities;
  const cs_lnum_t n_b_faces = m->n_b_faces;
  const cs_real_3_t *b_face_cog = mq->b_face_cog;
  const cs_real_t *b_face_surf = mq->b_face_surf;

  const cs_zone_t *zn = cs_boundary_zone_by_name("wavemaker");

  const wave_tank::sea_state_t &s = wave_tank::sea_state();
  const double h = s.h;
  const double t = domain->time_step->t_cur;
  const double r = wave_tank::ramp(t);

  /* Surface elevation and linear water flow rate (per unit width) at x = 0 */

  double eta = 0., q_lin = 0.;
  for (const wave_tank::component_t &w : s.c) {
    const double c = std::cos(-w.omega*t + w.phi);
    eta   += r*w.a*c;
    q_lin += r*w.a*w.omega/w.k*c;
  }

  /* First pass: Airy velocity and air fraction of each face */

  std::vector<double> u(zn->n_elts), v(zn->n_elts), alpha(zn->n_elts);

  /* sums: wet area, water flow rate, total area, air-only area */
  double sums[4] = {0., 0., 0., 0.};
  double y_min = 1e30, y_max = -1e30;

  for (cs_lnum_t e_id = 0; e_id < zn->n_elts; e_id++) {
    const cs_lnum_t f_id = zn->elt_ids[e_id];

    /* Vertical extent of the face, from its vertices */
    double y0 = 1e30, y1 = -1e30;
    for (cs_lnum_t i = m->b_face_vtx_idx[f_id];
         i < m->b_face_vtx_idx[f_id+1]; i++) {
      const double y = m->vtx_coord[3*m->b_face_vtx_lst[i] + 1];
      y0 = cs::min(y0, y);
      y1 = cs::max(y1, y);
    }
    y_min = cs::min(y_min, y0);
    y_max = cs::max(y_max, y1);

    /* Air fraction of the face cut by the surface at y = h + eta */
    alpha[e_id] = cs::min(cs::max((y1 - (h + eta))/(y1 - y0), 0.), 1.);

    /* Wheeler stretching: the wet height [0, h + eta] maps to [0, h] */
    const double y  = cs::min(b_face_cog[f_id][1], h + eta);
    const double ys = y*h/(h + eta);

    u[e_id] = 0.;
    v[e_id] = 0.;
    if (alpha[e_id] < 1.) {
      for (const wave_tank::component_t &w : s.c) {
        const double theta = -w.omega*t + w.phi;
        const double amp = r*w.a*w.omega/std::sinh(w.k*h);
        u[e_id] += amp*std::cosh(w.k*ys)*std::cos(theta);
        v[e_id] += amp*std::sinh(w.k*ys)*std::sin(theta);
      }
      sums[0] += (1. - alpha[e_id])*b_face_surf[f_id];
      sums[1] += (1. - alpha[e_id])*b_face_surf[f_id]*u[e_id];
    }
    else
      sums[3] += b_face_surf[f_id];
    sums[2] += b_face_surf[f_id];
  }

  cs_parall_sum(4, CS_DOUBLE, sums);
  cs_parall_min(1, CS_DOUBLE, &y_min);
  cs_parall_max(1, CS_DOUBLE, &y_max);

  const double width = sums[2]/(y_max - y_min);

  /* Return current: the water flow rate recovers its linear value */
  const double u_return = (q_lin*width - sums[1])/sums[0];

  /* Flow rate through the wet faces, to be balanced by the air */
  double q_wet = 0.;
  for (cs_lnum_t e_id = 0; e_id < zn->n_elts; e_id++) {
    if (alpha[e_id] < 1.) {
      u[e_id] += u_return;
      q_wet += u[e_id]*b_face_surf[zn->elt_ids[e_id]];
    }
  }
  cs_parall_sum(1, CS_DOUBLE, &q_wet);

  const double u_air = -q_wet/sums[3];

  /* Second pass: impose the velocity and the void fraction */

  cs_field_t *f_vf = cs_field_by_name("void_fraction");
  cs_real_t *vel_rcodcl1 = CS_F_(vel)->bc_coeffs->rcodcl1;

  for (cs_lnum_t e_id = 0; e_id < zn->n_elts; e_id++) {
    const cs_lnum_t f_id = zn->elt_ids[e_id];

    bc_type[f_id] = CS_INLET;

    vel_rcodcl1[n_b_faces*0 + f_id] = (alpha[e_id] < 1.) ? u[e_id] : u_air;
    vel_rcodcl1[n_b_faces*1 + f_id] = v[e_id];
    vel_rcodcl1[n_b_faces*2 + f_id] = 0.;

    f_vf->bc_coeffs->icodcl[f_id] = 1;
    f_vf->bc_coeffs->rcodcl1[f_id] = alpha[e_id];
  }
}
