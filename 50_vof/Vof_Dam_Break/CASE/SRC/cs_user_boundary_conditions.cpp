#include "cs_headers.h"

/*
 * Approximation of OpenFOAM pressureInletOutletVelocity
 * on the boundary zone "atmosphere".
 *
 * GUI configuration:
 *   atmosphere -> Imposed P outlet
 *   pressure   -> 0 Pa relative
 */

void
cs_user_boundary_conditions(cs_domain_t  *domain,
                            int           bc_type[])
{
  CS_UNUSED(bc_type);

  const cs_mesh_t *mesh = domain->mesh;

  const cs_lnum_t n_b_faces = mesh->n_b_faces;
  const cs_lnum_t *b_face_cells = mesh->b_face_cells;

  /*
   * In Code_Saturne, b_face_normal is stored as a flat array:
   *
   * normal[3*face_id + 0]
   * normal[3*face_id + 1]
   * normal[3*face_id + 2]
   */
  const cs_real_t *b_face_normal
    = domain->mesh_quantities->b_face_normal;

  /*
   * cs_boundary_zone_by_name returns a cs_zone_t.
   */
  const cs_zone_t *zone
    = cs_boundary_zone_by_name("atmosphere");

  cs_field_t *velocity = CS_F_(vel);

  cs_field_bc_coeffs_t *vel_bc = velocity->bc_coeffs;

  /*
   * Previous-time cell velocity.
   * Stored as a flat vector array.
   */
  const cs_real_t *cell_velocity = velocity->val_pre;

  for (cs_lnum_t i = 0; i < zone->n_elts; i++) {

    const cs_lnum_t face_id = zone->elt_ids[i];
    const cs_lnum_t cell_id = b_face_cells[face_id];

    const cs_real_t ux = cell_velocity[3*cell_id + 0];
    const cs_real_t uy = cell_velocity[3*cell_id + 1];
    const cs_real_t uz = cell_velocity[3*cell_id + 2];

    const cs_real_t nx = b_face_normal[3*face_id + 0];
    const cs_real_t ny = b_face_normal[3*face_id + 1];
    const cs_real_t nz = b_face_normal[3*face_id + 2];

    /*
     * The face-normal vector includes the face area.
     * Only the sign is used here.
     *
     * positive -> outflow
     * negative -> inflow
     */
    const cs_real_t normal_flux_predictor
      = ux*nx + uy*ny + uz*nz;

    if (normal_flux_predictor >= 0.0) {

      /*
       * Outflow:
       * homogeneous Neumann condition on velocity.
       */
      vel_bc->icodcl[face_id] = 3;

      for (int component_id = 0; component_id < 3; component_id++) {

        const cs_lnum_t bc_id
          = component_id*n_b_faces + face_id;

        vel_bc->rcodcl3[bc_id] = 0.0;
      }
    }
    else {

      /*
       * Inflow:
       *
       * icodcl = 11:
       * Neumann on the normal component and
       * Dirichlet on tangential components.
       *
       * Zero vector means zero imposed tangential velocity.
       */
      vel_bc->icodcl[face_id] = 11;

      for (int component_id = 0; component_id < 3; component_id++) {

        const cs_lnum_t bc_id
          = component_id*n_b_faces + face_id;

        vel_bc->rcodcl1[bc_id] = 0.0;
        vel_bc->rcodcl3[bc_id] = 0.0;
      }
    }
  }
}