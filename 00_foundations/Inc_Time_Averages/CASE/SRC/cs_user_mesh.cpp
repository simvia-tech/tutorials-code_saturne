/*============================================================================
 * Inc_Time_Averages - carve the square cylinder out of the Cartesian mesh.
 *
 * The built-in Cartesian mesher generates a plain box; the cells inside the
 * square section are removed at run time and the freed faces are tagged with
 * the "obstacle" group, used by the wall boundary zone.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_mesh_modify(cs_mesh_t  *mesh)
{
  /* Square cylinder of side D = 0.01 m centred at the origin */

  const char criteria[]  = "box[-0.005, -0.005, -1.0, 0.005, 0.005, 1.0]";
  const char new_group[] = "obstacle";

  cs_mesh_remove_cells_from_selection_criteria(mesh, criteria, new_group);
}

END_C_DECLS
