/*============================================================================
 * Pre_Zone_Refinement - refine the mesh locally around the tracer band.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * This is the whole difference between CASE_Coarse and CASE_Refined.
 * cs_mesh_refine_simple_selected subdivides the selected cells (each
 * hexahedron into 8 children) at the start of the run: the corridor
 * carrying the tracer band is refined from 5 mm to 2.5 mm cells,
 * while the rest of the mesh is left untouched. With the "conforming"
 * flag, the transition to the unrefined neighbourhood is handled by
 * the code.
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
  /* Corridor around the tracer band (band: 0.0375 < y < 0.0625) */

  const char criteria[] = "box[-0.01, 0.02, -0.01, 1.01, 0.08, 0.02]";

  cs_lnum_t   n_selected_cells = 0;
  cs_lnum_t  *selected_cells = nullptr;

  CS_MALLOC(selected_cells, mesh->n_cells, cs_lnum_t);

  cs_selector_get_cell_list(criteria, &n_selected_cells, selected_cells);

  cs_mesh_refine_simple_selected(mesh,
                                 true,   /* conforming transition */
                                 n_selected_cells,
                                 selected_cells);

  CS_FREE(selected_cells);
}

END_C_DECLS
