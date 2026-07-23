/*============================================================================
 * User extra operations: compute lift coefficient Cl(t)
 *============================================================================*/

#include "cs_headers.h"

#include <stdio.h>
#include <math.h>

/*----------------------------------------------------------------------------*/

void
cs_user_extra_operations(cs_domain_t *domain)
{
  /* ================= USER PARAMETERS ================= */

  /* Name of the boundary zone of the cylinder wall
     (the "wall" boundary group in this case's mesh). */
  const char boundary_zone_name[] = "wall";

  /* Reference values */
  const cs_real_t rho_ref = 1.0;  /* kg/m3: fluid density */
  const cs_real_t u_ref = 0.12;   /* m/s: free-stream velocity U_inf */
  const cs_real_t c_ref = 0.01;   /* m: cylinder diameter D */

  /* The mesh is a 2D slice extruded with a spanwise thickness of 1 m,
     so the reference frontal area is S_ref = D * 1.0. */
  const cs_real_t s_ref = c_ref * 1.0;

  /* Lift direction.
     If flow is along x and lift is vertical y: eL = (0,1,0). */
  const cs_real_t e_lift[3] = {0.0, 1.0, 0.0};

  /* =================================================== */

  /* Boundary stress field.
     In recent Code_Saturne versions, "boundary_stress" is a surface stress,
     so we multiply it by boundary face surface. */
  cs_field_t *b_stress = cs_field_by_name_try("boundary_stress");

  if (b_stress == NULL) {
    if (cs_glob_rank_id <= 0)
      bft_printf("Warning: field 'boundary_stress' not found. "
                 "Activate boundary stress/forces output.\n");
    return;
  }

  const cs_zone_t *zn = cs_boundary_zone_by_name(boundary_zone_name);

  if (zn == NULL) {
    if (cs_glob_rank_id <= 0)
      bft_printf("Warning: boundary zone '%s' not found.\n",
                 boundary_zone_name);
    return;
  }

  const cs_real_t *b_face_surf = domain->mesh_quantities->b_face_surf;
  const cs_real_3_t *stress = (const cs_real_3_t *)b_stress->val;

  cs_real_t force[3] = {0.0, 0.0, 0.0};

  for (cs_lnum_t e_id = 0; e_id < zn->n_elts; e_id++) {

    cs_lnum_t face_id = zn->elt_ids[e_id];

    for (int i = 0; i < 3; i++)
      force[i] += stress[face_id][i] * b_face_surf[face_id];

  }

  /* Parallel sum if running with MPI */
  cs_parall_sum(3, CS_REAL_TYPE, force);

  /* Lift force */
  cs_real_t lift =
      force[0]*e_lift[0]
    + force[1]*e_lift[1]
    + force[2]*e_lift[2];

  /* Lift coefficient */
  cs_real_t q_ref = 0.5 * rho_ref * u_ref * u_ref;
  cs_real_t cl = lift / (q_ref * s_ref);

  /* Current physical time */
  cs_real_t time = domain->time_step->t_cur;

  /* Write only from rank 0 in parallel */
  if (cs_glob_rank_id <= 0) {

    FILE *fp = NULL;

    if (domain->time_step->nt_cur == 1) {
      fp = fopen("cl_profile.dat", "w");
      fprintf(fp, "# time Fx Fy Fz Cl\n");
    }
    else {
      fp = fopen("cl_profile.dat", "a");
    }

    if (fp != NULL) {
      fprintf(fp, "%.12e %.12e %.12e %.12e %.12e\n",
              time, force[0], force[1], force[2], cl);
      fclose(fp);
    }

  }
}