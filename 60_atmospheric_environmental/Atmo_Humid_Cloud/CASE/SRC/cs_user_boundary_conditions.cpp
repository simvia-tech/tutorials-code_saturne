/*============================================================================
 * Atmo_Humid_Cloud - boundary conditions driven by the meteo file.
 *
 * The vertical profiles of wind, potential temperature and turbulence read from
 * the meteo file are imposed at the open boundaries: for a CS_INLET face left
 * unset, the atmospheric module (cs_atmo_bcond) fills the Dirichlet values by
 * interpolating the meteo profile at the face height. iautom = 1 lets the code
 * switch a face to outflow automatically if the meteo wind points outward.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_boundary_conditions([[maybe_unused]] cs_domain_t  *domain,
                            int                            bc_type[])
{
  int *iautom = cs_glob_bc_pm_info->iautom;

  /* Inlet (west) and top: meteo profile imposed automatically. */
  const char *open_zones[] = {"inlet", "top"};
  for (int iz = 0; iz < 2; iz++) {
    const cs_zone_t *zn = cs_boundary_zone_by_name(open_zones[iz]);
    for (cs_lnum_t e = 0; e < zn->n_elts; e++) {
      const cs_lnum_t face_id = zn->elt_ids[e];
      bc_type[face_id] = CS_INLET;   /* velocity/turb/theta left unset -> meteo */
      iautom[face_id] = 1;
    }
  }

  /* Outlet (east): free outlet. */
  {
    const cs_zone_t *zn = cs_boundary_zone_by_name("outlet");
    for (cs_lnum_t e = 0; e < zn->n_elts; e++)
      bc_type[zn->elt_ids[e]] = CS_OUTLET;
  }

  /* Symmetry sides (2D slice). */
  {
    const cs_zone_t *zn = cs_boundary_zone_by_name("sides");
    for (cs_lnum_t e = 0; e < zn->n_elts; e++)
      bc_type[zn->elt_ids[e]] = CS_SYMMETRY;
  }

  /* Rough-wall ground with dynamic roughness z0 and thermal roughness z0t. */
  {
    const cs_real_t  z0 = 0.1, z0t = 0.1;
    cs_real_t *bpro_roughness = nullptr, *bpro_roughness_t = nullptr;
    if (cs_field_by_name_try("boundary_roughness") != nullptr)
      bpro_roughness = cs_field_by_name_try("boundary_roughness")->val;
    if (cs_field_by_name_try("boundary_thermal_roughness") != nullptr)
      bpro_roughness_t = cs_field_by_name_try("boundary_thermal_roughness")->val;

    const cs_zone_t *zn = cs_boundary_zone_by_name("ground");
    for (cs_lnum_t e = 0; e < zn->n_elts; e++) {
      const cs_lnum_t face_id = zn->elt_ids[e];
      bc_type[face_id] = CS_ROUGHWALL;
      if (bpro_roughness != nullptr)
        bpro_roughness[face_id] = z0;
      if (bpro_roughness_t != nullptr)
        bpro_roughness_t[face_id] = z0t;
    }
  }
}

END_C_DECLS
