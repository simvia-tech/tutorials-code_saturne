/*============================================================================
 * Amr_Vortex_Wake - adaptive mesh refinement following a vortex street.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * This routine IS the tutorial's showcased feature. Adaptive refinement asks
 * the user for one thing only: a function that says, for every cell, whether
 * that cell belongs to the region worth resolving finely. Everything else, the
 * splitting, the interpolation of the fields onto the new cells, the merging
 * back when the region moves away, is the solver's business.
 *
 * The indicator describes a region to keep refined, not an order to refine one
 * more level. A cell that is marked and already refined stays as it is; a cell
 * that stops being marked is coarsened back. That is what lets the refined
 * patch travel with the vortices instead of accumulating behind them, and it is
 * also why the indicator must keep marking the cells it wants to keep fine.
 *
 * The criterion used here is the Q-criterion, the second invariant of the
 * velocity gradient: positive in a vortex core, small in pure shear.
 *============================================================================*/

/* code_saturne version 9.2 */

#include "cs_headers.h"

BEGIN_C_DECLS

/* Refine where Q exceeds this value, in units of (U_inf / D)^2. The number has
   no universal value: it is tuned by running the case and reading the resulting
   mesh size out of the log. */

static const cs_real_t _q_threshold = 0.5;

/*----------------------------------------------------------------------------
 * Refinement indicator: 1 marks a cell of the region to keep refined.
 *
 * Q is formed from the full velocity gradient, so the criterion carries over
 * unchanged to a three-dimensional case.
 *----------------------------------------------------------------------------*/

static void
_wake_indicator([[maybe_unused]] const void  *input,
                int                          *vals)
{
  const cs_lnum_t n_cells = cs_glob_mesh->n_cells;
  const cs_lnum_t n_cells_ext = cs_glob_mesh->n_cells_with_ghosts;

  cs_real_33_t *gradv;
  CS_MALLOC(gradv, n_cells_ext, cs_real_33_t);

  cs_field_gradient_vector(CS_F_(vel),
                           false,   /* current time step */
                           1,       /* not an increment */
                           gradv);

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++) {
    /* symmetric and antisymmetric parts of the velocity gradient */

    cs_real_t s2 = 0., o2 = 0.;

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        const cs_real_t sij = 0.5*(gradv[c_id][i][j] + gradv[c_id][j][i]);
        const cs_real_t oij = 0.5*(gradv[c_id][i][j] - gradv[c_id][j][i]);
        s2 += sij*sij;
        o2 += oij*oij;
      }
    }

    const cs_real_t q = 0.5*(o2 - s2);

    vals[c_id] = (q > _q_threshold) ? 1 : 0;
  }

  CS_FREE(gradv);
}

/*----------------------------------------------------------------------------*/

void
cs_user_parameters(cs_domain_t *domain)
{
  CS_UNUSED(domain);

  cs_adaptive_refinement_define(2,        /* layers kept around the marked
                                             region, so the refined patch is
                                             slightly ahead of the vortices */
                                20,       /* adapt every 20 time steps */
                                _wake_indicator,
                                nullptr,  /* the indicator needs no extra input */
                                1,        /* gradient-based interpolation of the
                                             fields onto the new cells */
                                true);    /* rebalance the partitions after
                                             each adaptation: the refined
                                             region moves, so the cell count
                                             per rank would otherwise drift */
}

END_C_DECLS
