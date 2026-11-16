/*============================================================================
 * Submerged breakwater: a rectangular block carved out of the cartesian mesh.
 *
 * When the notebook parameter "structure" is 1, the cells of the block are
 * removed from the mesh. The boundary faces created around the hole are
 * put in the group "structure", which the boundary zone of the same name
 * in setup.xml selects (a wall). With "structure" at 0 the tank is empty
 * and the zone has no face.
 *============================================================================*/

#include "cs_headers.h"

#include "wave_tank.h"

void
cs_user_mesh_modify(cs_mesh_t  *mesh)
{
  if (cs_notebook_parameter_value_by_name("structure") < 0.5)
    return;

  char criteria[128];
  snprintf(criteria, 127, "box[%g, %g, %g, %g, %g, %g]",
           wave_tank::structure_x0, -1., -1.,
           wave_tank::structure_x1, wave_tank::structure_height, 1.);

  cs_mesh_remove_cells_from_selection_criteria(mesh, criteria, "structure");

  mesh->modified |= CS_MESH_MODIFIED_BALANCE;  /* re-partition */
}
