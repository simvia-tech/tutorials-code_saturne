/*============================================================================
 * User initialization prior to solving time steps.
 *============================================================================*/

/*
  This file is part of code_saturne, a general-purpose CFD tool.

  Copyright (C) 1998-2024 EDF S.A.

  This program is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
  Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*----------------------------------------------------------------------------*/

#include "cs_defs.h"

/*----------------------------------------------------------------------------
 * Standard C library headers
 *----------------------------------------------------------------------------*/

#include <assert.h>
#include <math.h>

#if defined(HAVE_MPI)
#include <mpi.h>
#endif

/*----------------------------------------------------------------------------
 * PLE library headers
 *----------------------------------------------------------------------------*/

#include <ple_coupling.h>

/*----------------------------------------------------------------------------
 * Local headers
 *----------------------------------------------------------------------------*/

#include "cs_headers.h"

/*----------------------------------------------------------------------------*/

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*!
 * \file cs_user_initialization.cpp
 *
 * \brief Sedov blast wave initial condition.
 *
 * Uniform gas at rest (rho = 1, u = 0) with a high-pressure disc of radius
 * r0 centered on the origin: p = p_hot for r < r0, p = p_inf elsewhere.
 * The origin is the corner of the quarter-plane domain, whose x = 0 and
 * y = 0 boundaries are symmetry planes.
 *
 * This routine is called after the GUI initialization and therefore
 * overrides the uniform pressure formula defined in setup.xml.
 */
/*----------------------------------------------------------------------------*/

/*============================================================================
 * User function definitions
 *============================================================================*/

/*----------------------------------------------------------------------------*/
/*!
 * \brief Define initial conditions for variables.
 *
 * \param[in, out]  domain   pointer to a cs_domain_t structure
 */
/*----------------------------------------------------------------------------*/

void
cs_user_initialization(cs_domain_t *domain)
{
  CS_UNUSED(domain);

  const cs_lnum_t n_cells = cs_glob_mesh->n_cells;
  const cs_real_3_t *cell_cen =
    (const cs_real_3_t *)cs_glob_mesh_quantities->cell_cen;

  cs_real_t *rho = CS_F_(rho)->val;
  cs_real_t *p   = CS_F_(p)->val;
  cs_real_3_t *vel = (cs_real_3_t *)CS_F_(vel)->val;

  const cs_real_t rho0   = 1.0;    /* uniform density                    */
  const cs_real_t pinfty = 0.01;   /* ambient pressure                   */
  const cs_real_t phot   = 100.0;  /* pressure inside the energy deposit */
  const cs_real_t r0     = 0.005;  /* radius of the energy deposit [m]   */

  cs_log_printf(CS_LOG_DEFAULT, ">>> cs_user_initialization: Sedov init.\n");

  for (cs_lnum_t c = 0; c < n_cells; c++) {
    double x = cell_cen[c][0];
    double y = cell_cen[c][1];
    double r = sqrt(x*x + y*y);

    rho[c] = rho0;
    vel[c][0] = 0.0;
    vel[c][1] = 0.0;
    vel[c][2] = 0.0;

    if (r < r0)
      p[c] = phot;
    else
      p[c] = pinfty;
  }
}

/*----------------------------------------------------------------------------*/

END_C_DECLS
