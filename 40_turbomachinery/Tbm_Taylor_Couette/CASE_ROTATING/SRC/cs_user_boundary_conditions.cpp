/*============================================================================
 * Tbm_Taylor_Couette, rotating frame: the outer cylinder turns backwards.
 *
 * The GUI can only impose a uniform velocity on a wall, so a cylinder turning
 * about its axis needs the velocity to be given face by face: u = omega x r.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

static const cs_real_t _omega = -1.0;  /* rad/s, about z: the outer cylinder
                                          is at rest in the absolute frame, so
                                          it turns at -omega in the frame that
                                          follows the inner one */

/*----------------------------------------------------------------------------
 * Solid-body velocity of a wall turning about the z axis.
 *----------------------------------------------------------------------------*/

static void
_rotating_wall([[maybe_unused]] cs_real_t   time,
               cs_lnum_t                    n_elts,
               const cs_lnum_t             *elt_ids,
               const cs_real_t             *coords,
               bool                         dense_output,
               void                        *input,
               cs_real_t                   *val)
{
  const cs_real_t omega = *(const cs_real_t *)input;
  const cs_real_3_t *xyz = (const cs_real_3_t *)coords;
  cs_real_3_t *v = (cs_real_3_t *)val;

  for (cs_lnum_t i = 0; i < n_elts; i++) {
    const cs_lnum_t e_id = (elt_ids == nullptr) ? i : elt_ids[i];
    const cs_lnum_t j = dense_output ? i : e_id;

    v[j][0] = -omega*xyz[e_id][1];
    v[j][1] =  omega*xyz[e_id][0];
    v[j][2] =  0.;
  }
}

/*----------------------------------------------------------------------------*/

void
cs_user_boundary_conditions_setup(cs_domain_t *domain)
{
  CS_UNUSED(domain);

  cs_equation_param_t *eqp = cs_equation_param_by_name("velocity");

  cs_equation_add_bc_by_analytic(eqp,
                                 CS_BC_DIRICHLET,
                                 "outer",
                                 _rotating_wall,
                                 (void *)&_omega);
}

END_C_DECLS
