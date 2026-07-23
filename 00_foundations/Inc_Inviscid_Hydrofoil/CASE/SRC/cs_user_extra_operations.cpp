/*============================================================================
 * User-defined functions called at the end of each time step.
 *
 * Computation of the lift (Cl) and drag (Cd) coefficients on the NACA0012
 * profile (boundary zone "body") for the incompressible, inviscid case
 * Inc_Inviscid_Hydrofoil.
 *============================================================================*/

/* code_saturne version 9.1 */

/*
  This file is part of code_saturne, a general-purpose CFD tool.
  Copyright (C) 1998-2025 EDF S.A.

  This program is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.
*/

/*----------------------------------------------------------------------------*/

#include "cs_headers.h"

/*----------------------------------------------------------------------------
 * Standard library headers
 *----------------------------------------------------------------------------*/

#include <assert.h>
#include <math.h>

/*----------------------------------------------------------------------------*/

BEGIN_C_DECLS

/*============================================================================
 * Case parameters (to be adapted by the user)
 *============================================================================*/

/* Name of the boundary zone matching the profile (see setup.xml). */
static const char *_airfoil_zone_name = "body";

/* Fluid density [kg/m3] (see setup.xml: Density = 998.2). */
static const cs_real_t _rho_ref = 998.2;

/* Free-stream (upstream) velocity magnitude [m/s]
   (see setup.xml: norm imposed at the inlet = 1.775). */
static const cs_real_t _u_ref = 1.775;

/* Profile chord [m] (NACA0012: chord = 1). */
static const cs_real_t _chord = 1.0;

/* Domain span along z [m] (extruded mesh, here from z=0 to z=1). */
static const cs_real_t _span = 1.0;

/* Unit vector of the free-stream direction.
   The flow is aligned with +x; the 5 deg angle of attack is carried by
   the geometry (profile tilted in the mesh). */
static const cs_real_t _flow_dir[3] = {1.0, 0.0, 0.0};

/*============================================================================
 * User function definitions
 *============================================================================*/

/*----------------------------------------------------------------------------*/
/*
 * User operations called at the end of each time step.
 *
 * \param[in, out]  domain   pointer to a cs_domain_t structure
 */
/*----------------------------------------------------------------------------*/

void
cs_user_extra_operations(cs_domain_t  *domain)
{
  /* Boundary zone representing the profile. */
  const cs_zone_t *zn = cs_boundary_zone_by_name(_airfoil_zone_name);

  /* "boundary_stress" field: force exerted by the fluid on the wall, per
     unit surface [Pa] (pressure + viscous friction). In an inviscid flow,
     only the pressure contributes. */
  cs_field_t *f_stress = cs_field_by_name_try("boundary_stress");

  if (f_stress == nullptr) {
    bft_printf("cs_user_extra_operations: field 'boundary_stress' not found.\n"
               "Enable the 'Stress' property (boundary support) in the GUI.\n");
    return;
  }

  const cs_real_3_t *b_stress     = (const cs_real_3_t *)f_stress->val;
  const cs_real_t   *b_face_surf  = domain->mesh_quantities->b_face_surf;

  /* Total aerodynamic force on the profile [N], summing the surface force
     over every boundary face of the zone. */
  cs_real_3_t f_tot = {0., 0., 0.};

  for (cs_lnum_t e_id = 0; e_id < zn->n_elts; e_id++) {
    cs_lnum_t face_id = zn->elt_ids[e_id];
    for (cs_lnum_t i = 0; i < 3; i++)
      f_tot[i] += b_stress[face_id][i] * b_face_surf[face_id];
  }

  /* Parallel sum (multi-rank MPI run). */
  cs_parall_sum(3, CS_REAL_TYPE, f_tot);

  /* Aerodynamic axes (in the x-y plane, span along z):
       - drag direction = free-stream direction
       - lift direction = drag direction rotated by +90 deg, i.e. the unit
         vector perpendicular to the drag in the (x, y) plane:
         (dx, dy) -> (-dy, dx). */
  const cs_real_t drag_dir[3] = { _flow_dir[0], _flow_dir[1], 0.0 };
  const cs_real_t lift_dir[3] = { -_flow_dir[1], _flow_dir[0], 0.0 };

  /* Projection of the total force onto the aerodynamic axes. */
  cs_real_t drag =   f_tot[0] * drag_dir[0]
                   + f_tot[1] * drag_dir[1]
                   + f_tot[2] * drag_dir[2];

  cs_real_t lift =   f_tot[0] * lift_dir[0]
                   + f_tot[1] * lift_dir[1]
                   + f_tot[2] * lift_dir[2];

  /* Reference dynamic pressure and reference (planform) area. */
  const cs_real_t q_ref    = 0.5 * _rho_ref * _u_ref * _u_ref;
  const cs_real_t area_ref = _chord * _span;
  const cs_real_t f_ref    = q_ref * area_ref;

  const cs_real_t cd = drag / f_ref;
  const cs_real_t cl = lift / f_ref;

  /* Print to the listing (run_solver.log) on the main rank. */
  bft_printf("\n"
             " ** Aerodynamic coefficients (zone \"%s\")\n"
             "    Fx = %12.5e N   Fy = %12.5e N   Fz = %12.5e N\n"
             "    Drag = %12.5e N   ->  Cd = %12.6f\n"
             "    Lift = %12.5e N   ->  Cl = %12.6f\n",
             _airfoil_zone_name,
             f_tot[0], f_tot[1], f_tot[2],
             drag, cd, lift, cl);

  /* Write a CSV history (main rank only): one line per time step, so the
     convergence of the coefficients is easy to plot. */
  if (cs_glob_rank_id <= 0) {
    const cs_time_step_t *ts = cs_glob_time_step;

    /* Header written on the first pass, data appended afterwards. */
    const char *mode = (ts->nt_cur <= ts->nt_prev + 1) ? "w" : "a";
    FILE *f = fopen("lift_drag.csv", mode);

    if (f != nullptr) {
      if (mode[0] == 'w')
        fprintf(f, "iteration, time, Fx, Fy, Fz, drag, lift, Cd, Cl\n");

      fprintf(f, "%d, %.6e, %.6e, %.6e, %.6e, %.6e, %.6e, %.6e, %.6e\n",
              ts->nt_cur, ts->t_cur,
              f_tot[0], f_tot[1], f_tot[2],
              drag, lift, cd, cl);

      fclose(f);
    }
  }
}

/*----------------------------------------------------------------------------*/

END_C_DECLS
