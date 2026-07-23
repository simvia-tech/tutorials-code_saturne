/*============================================================================
 * Pre_Mesh_Replication - read ONE pattern mesh three times.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The GUI lists the pattern mesh (tube_pattern.msh) once: the staging
 * step converts it to mesh_input.csm in the execution directory. This
 * routine then REPLACES that default input by three reads of the same
 * file, each with its own translation and group renames:
 *
 *   instance 1: x + 0.0,  Left -> Inlet, Right -> R1
 *   instance 2: x + 0.1,  Left -> L2,    Right -> R2
 *   instance 3: x + 0.2,  Left -> L3,    Right -> Outlet
 *
 * (The first cs_preprocessor_data_add_file call overrides the default
 * input; the following ones append.) Groups that are not renamed
 * (Walls, Tube, Sym) keep their name in every instance and merge into
 * single zones. The internal interfaces (R1/L2 and R2/L3) are then
 * glued by the two face joinings defined in the GUI.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Define mesh files to read and optional associated transformations.
 *----------------------------------------------------------------------------*/

void
cs_user_mesh_input(void)
{
  const double pattern_width = 0.1;  /* x extent of the pattern [m] */

  const char *renames[3][4] = {
    {"Left", "Inlet", "Right", "R1"},
    {"Left", "L2",    "Right", "R2"},
    {"Left", "L3",    "Right", "Outlet"}
  };

  for (int i = 0; i < 3; i++) {

    /* Homogeneous transformation: translation of i pattern widths */

    const double transf_matrix[3][4] = {{1., 0., 0., i * pattern_width},
                                        {0., 1., 0., 0.},
                                        {0., 0., 1., 0.}};

    cs_preprocessor_data_add_file("mesh_input.csm",
                                  2, renames[i],
                                  transf_matrix);

  }
}

END_C_DECLS
