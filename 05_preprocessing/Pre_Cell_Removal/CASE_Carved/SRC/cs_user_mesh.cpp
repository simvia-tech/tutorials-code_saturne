/*============================================================================
 * Pre_Cell_Removal - carve an obstacle out of the mesh.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * This is the whole difference between CASE_Full and CASE_Carved: the
 * cells of a square block in the middle of the channel are REMOVED
 * from the mesh at run time. The faces freed by the removal become
 * boundary faces, and the function stamps the given group name on
 * them ("Obstacle"): the boundary-condition zone then selects that
 * name, as if the mesh had been generated with the hole.
 *
 * The same function also accepts a user-computed flag array
 * (cs_mesh_remove_cells): that variant is the rescue tool for
 * amputating degenerate cells from an imported mesh (see the README).
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
  /* Square obstacle: 0.15 < x < 0.19, 0.03 < y < 0.07 */

  const char criteria[]  = "box[0.15, 0.03, -0.01, 0.19, 0.07, 0.02]";
  const char new_group[] = "Obstacle";  /* group for the freed faces */

  cs_mesh_remove_cells_from_selection_criteria(mesh,
                                               criteria,
                                               new_group);
}

END_C_DECLS
