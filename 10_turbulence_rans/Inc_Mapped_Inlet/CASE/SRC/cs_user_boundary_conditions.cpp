/*============================================================================
 * Inc_Mapped_Inlet - recycle a downstream profile onto the inlet.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * This routine IS the tutorial's showcased feature. The inlet values are not
 * prescribed by the user: at every time step they are copied from the cells
 * located a fixed distance downstream (the recycling plane). The channel
 * therefore feeds itself and converges towards its own fully developed
 * profile, which no analytical inlet condition could provide.
 *
 * Two calls are involved:
 *   - cs_boundary_conditions_map()        builds, once, the locator that
 *                                         links each inlet face to the cell
 *                                         found at (face centre + shift);
 *   - cs_boundary_conditions_mapped_set() copies the values at every step.
 *
 * The velocity is mapped with normalize = 1: the copied profile is rescaled
 * so that the mass flow rate imposed in setup.xml is preserved exactly.
 * The turbulence variables are mapped as they are (normalize = 0).
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/* Distance between the inlet and the recycling plane */

static const cs_real_t _recycling_length = 5.0;

/*----------------------------------------------------------------------------
 * User boundary conditions.
 *----------------------------------------------------------------------------*/

void
cs_user_boundary_conditions(cs_domain_t  *domain,
                            int           bc_type[])
{
  CS_UNUSED(bc_type);

  /* The locator is built once and kept between time steps */

  static ple_locator_t *_inlet_locator = nullptr;

  const cs_zone_t *zn = cs_boundary_zone_by_name("inlet");
  const cs_time_step_t *ts = domain->time_step;

  /* First time step: build the locator.
     Each inlet face is associated with the cell containing
     (face centre + _recycling_length e_x). */

  if (_inlet_locator == nullptr) {

    cs_real_3_t coord_shift[1] = {{_recycling_length, 0., 0.}};

    const cs_lnum_t n_cells = domain->mesh->n_cells;

    cs_lnum_t *cells_ids;
    CS_MALLOC(cells_ids, n_cells, cs_lnum_t);
    for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
      cells_ids[c_id] = c_id;

    _inlet_locator
      = cs_boundary_conditions_map(CS_MESH_LOCATION_CELLS,
                                   n_cells,
                                   zn->n_elts,
                                   cells_ids,
                                   zn->elt_ids,
                                   coord_shift,
                                   0,      /* uniform shift */
                                   0.10);  /* location tolerance */

    CS_FREE(cells_ids);
  }

  /* From the second time step on: copy the downstream profile
     onto the inlet, for the velocity and the turbulence variables. */

  else {

    const int n_fields = cs_field_n_fields();

    for (int f_id = 0; f_id < n_fields; f_id++) {

      cs_field_t *f = cs_field_by_id(f_id);

      if (!(f->type & CS_FIELD_VARIABLE))
        continue;

      /* Rescale the velocity so that the imposed mass flow rate is kept */

      const int normalize = (f == CS_F_(vel)) ? 1 : 0;

      cs_boundary_conditions_mapped_set(f,
                                        _inlet_locator,
                                        CS_MESH_LOCATION_CELLS,
                                        normalize,
                                        0,     /* no gradient interpolation */
                                        zn->n_elts,
                                        zn->elt_ids,
                                        nullptr);
    }
  }
}

/*----------------------------------------------------------------------------*/

END_C_DECLS
