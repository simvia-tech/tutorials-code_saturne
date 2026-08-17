/*============================================================================
 * Th_1D_Wall_Thermal - configure the 1D wall thermal module.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * This routine IS the tutorial's showcased feature: the 1D wall thermal
 * module has no GUI path and activates as soon as nfpt1d > 0 is declared
 * here. The bottom wall of the channel receives an embedded 1D conduction
 * mesh across the wall thickness; its fluid-side temperature is coupled to
 * the flow, and its exterior side sees a cold environment (Dirichlet +
 * exchange coefficient) from t = 0, quenching the initially hot wall.
 *
 * The routine is called with three purposes (iappel):
 *   1 - count the coupled boundary faces (nfpt1d);
 *   2 - list them (ifpt1d) and build the 1D meshes (points, thickness,
 *       grading, initial temperature);
 *   3 - every time step: exterior boundary condition and wall properties.
 *============================================================================*/

/* code_saturne version 9.1 */

#include <algorithm>

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Wall and exterior parameters (see the README and validation/sizing.py).
 *----------------------------------------------------------------------------*/

static const cs_real_t  _e      = 5.0e-3;  /* wall thickness [m] */
static const cs_real_t  _lam_s  = 0.03;    /* wall conductivity [W/m/K] */
static const cs_real_t  _rcp_s  = 4.2e4;   /* wall rho*cp [J/m3/K] */
static const cs_real_t  _h_ext  = 50.0;    /* exterior exchange coef. [W/m2/K] */
static const cs_real_t  _t_ext  = 290.0;   /* exterior temperature [K] */
static const cs_real_t  _t_init = 340.0;   /* initial wall temperature [K] */

/*----------------------------------------------------------------------------
 * Data Entry of the 1D wall thermal module.
 *----------------------------------------------------------------------------*/

void
cs_user_1d_wall_thermal(int iappel)
{
  cs_1d_wall_thermal_t *wt = cs_get_glob_1d_wall_thermal();

  wt->use_restart = cs_restart_present() ? true : false;

  /* Calls 1 and 2: select the coupled faces (the "bottom_wall" GUI zone) */

  if (iappel == 1 || iappel == 2) {

    const cs_zone_t *z = cs_boundary_zone_by_name("bottom_wall");

    if (iappel == 1)
      wt->nfpt1d = z->n_elts;   /* nfpt1d > 0 activates the module */

    else if (iappel == 2) {
      for (cs_lnum_t ii = 0; ii < z->n_elts; ii++)
        wt->ifpt1d[ii] = z->elt_ids[ii] + 1;
      /* the module requires ifpt1d in increasing order */
      std::sort(wt->ifpt1d, wt->ifpt1d + z->n_elts);
    }
  }

  /* Call 2: build the embedded 1D meshes across the thickness */

  if (iappel == 2) {
    for (cs_lnum_t ii = 0; ii < wt->nfpt1d; ii++) {
      wt->local_models[ii].nppt1d = 20;      /* points across the thickness */
      wt->local_models[ii].eppt1d = _e;      /* wall thickness */
      wt->local_models[ii].rgpt1d = 1.1;     /* grading (>1: fine fluid side) */
      wt->tppt1d[ii] = _t_init;              /* initial wall temperature */
    }
  }

  /* Call 3 (every time step): exterior BC and wall properties */

  if (iappel == 3) {
    const cs_lnum_t *b_face_cells = cs_glob_mesh->b_face_cells;

    for (cs_lnum_t ii = 0; ii < wt->nfpt1d; ii++) {
      wt->local_models[ii].iclt1d = 1;       /* Dirichlet + exchange coef. */
      wt->local_models[ii].tept1d = _t_ext;  /* exterior temperature */
      wt->local_models[ii].hept1d = _h_ext;  /* exterior exchange coef. */
      wt->local_models[ii].xlmbt1 = _lam_s;  /* wall conductivity */
      wt->local_models[ii].rcpt1d = _rcp_s;  /* wall rho*cp */

      /* 1D conduction advanced with the local fluid time step */
      cs_lnum_t c_id = b_face_cells[wt->ifpt1d[ii] - 1];
      wt->local_models[ii].dtpt1d = CS_F_(dt)->val[c_id];
    }
  }
}

/*----------------------------------------------------------------------------*/

END_C_DECLS
