/*============================================================================
 * Pre_Boundary_Layer - insert prism layers along the channel walls.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * This is the whole difference between CASE_Uniform and CASE_Layers.
 * Starting from the same uniform cartesian mesh (10 cells across the
 * channel height), cs_mesh_boundary_layer_insert deflates the interior
 * mesh away from the selected walls and fills the freed space with
 * layers of prisms. The expansion factor is the ratio between
 * SUCCESSIVE layers along the insertion direction (from the deflated
 * interior surface toward the wall), so a value < 1 puts the thinnest
 * layer against the wall.
 *
 * Here: 4 layers over a total thickness of 15 mm with a 0.7 ratio,
 * i.e. a wall-adjacent layer of about 2 mm, dropping the first-cell
 * y+ from about 150 (uniform mesh) into the log-law range.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Modify geometry and mesh.
 *----------------------------------------------------------------------------*/

void
cs_user_mesh_modify(cs_mesh_t  *mesh)
{
  const char  criteria[] = "Y0 or Y1";  /* channel walls */
  const int     n_layers  = 4;
  const double  thickness = 0.015;  /* total inserted thickness [m] */
  const float   expansion = 0.7;    /* ratio between successive layers;
                                       < 1: thinnest layer at the wall */

  /* Per-face extrusion parameters for the selected walls */

  cs_mesh_extrude_face_info_t *efi = cs_mesh_extrude_face_info_create(mesh);

  cs_lnum_t  n_faces;
  cs_lnum_t  *face_list;

  CS_MALLOC(face_list, mesh->n_b_faces, cs_lnum_t);

  cs_selector_get_b_face_list(criteria, &n_faces, face_list);

  cs_mesh_extrude_set_info_by_zone(efi,
                                   n_layers,
                                   thickness,
                                   expansion,
                                   n_faces,
                                   face_list);

  CS_FREE(face_list);

  /* Vertex-based extrusion vectors, then insertion */

  cs_mesh_extrude_vectors_t *e = cs_mesh_extrude_vectors_create(efi);

  cs_mesh_extrude_face_info_destroy(&efi);

  cs_mesh_boundary_layer_insert(mesh,
                                e,
                                0.2,      /* min volume factor safeguard */
                                false,    /* do not group interior faces */
                                0,        /* no fixed vertices */
                                nullptr);

  cs_mesh_extrude_vectors_destroy(&e);
}

END_C_DECLS
