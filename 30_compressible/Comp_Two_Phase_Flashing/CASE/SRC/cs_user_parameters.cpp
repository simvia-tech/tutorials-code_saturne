/*============================================================================
 * Compressible homogeneous two-phase model: activation and thermodynamics.
 *
 * The model is not exposed in the GUI. Everything below is mandatory:
 *  - the model flag (value 2 of the compressible module),
 *  - the equation of state (one stiffened gas per phase),
 *  - the parameters of the two stiffened gases.
 *============================================================================*/

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*!
 * \brief Select physical model options, including user fields.
 */
/*----------------------------------------------------------------------------*/

void
cs_user_model(void)
{
  /* Compressible module, homogeneous two-phase variant:
     -1 off, 0 single-phase, 2 homogeneous two-phase (volume, mass and
     energy fractions of phase 1 are solved). Must be set here, before the
     variable fields are created. */

  cs_glob_physical_model_flag[CS_COMPRESSIBLE] = 2;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Define or modify general numerical and physical user parameters.
 *
 * \param[in, out] domain    pointer to a cs_domain_t structure
 */
/*----------------------------------------------------------------------------*/

void
cs_user_parameters([[maybe_unused]] cs_domain_t   *domain)
{
  cs_cf_model_t *cf_model = cs_get_glob_cf_model();

  /* Mixture equation of state: homogeneous two-phase, each phase following
     a stiffened gas law (overrides the "constant gamma" choice of the GUI). */

  cf_model->ieos = CS_EOS_HOMOGENEOUS_TWO_PHASE;

  /* Return-to-equilibrium source terms for the fractions:
     -1 disabled, 0 enabled. The step itself is applied from
     cs_user_extra_operations.cpp (see the README). */

  cf_model->hgn_relax_eq_st = 0;

  /* Stiffened gas parameters of each phase:
       iph, cv [J/kg/K], gamma [-], pinf [Pa], q' [J/kg/K], q [J/kg]
     Phase 1 (index 0) is the phase whose fractions are transported: here
     the steam. Phase 2 (index 1) is the liquid water.

     cv, gamma and pinf are those of Helluy, Hurisse and Le Coupanec (2016),
     representative of steam-water flows around 81 bar. The reference energy
     q must be kept to 0 in this version of the code. The entropy constant q'
     only shifts the saturation curve: q' of the steam is set so that
     Tsat(81 bar) = 569 K, as in the reference. */

  cs_hgn_thermo_define_stiffened_gas(0,                     /* steam  */
                                     4.477815802223535e3,
                                     1.084875362318841,
                                     0.,
                                     -49292.11,
                                     0.);

  cs_hgn_thermo_define_stiffened_gas(1,                     /* liquid */
                                     1.395286166711847e3,
                                     1.665128030303030,
                                     3.725876146842836e8,
                                     0.,
                                     0.);
}

END_C_DECLS
