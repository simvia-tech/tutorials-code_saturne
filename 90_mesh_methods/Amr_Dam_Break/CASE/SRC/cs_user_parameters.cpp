/*============================================================================
 * Amr_Dam_Break - adaptive mesh refinement following a water-air interface.
 *
 * This routine IS the tutorial's showcased feature. Adaptive refinement has no
 * GUI page: it is declared here with cs_adaptive_refinement_define, and driven
 * by an indicator function that marks, at each adaptation step, the cells that
 * belong to the region worth resolving.
 *
 * How the module uses the indicator (src/mesh/cs_mesh_adaptive_refinement.cpp):
 *  - a marked cell is split once more at each adaptation, until it reaches the
 *    maximum level (first argument of cs_adaptive_refinement_define);
 *  - an unmarked cell is merged back one level at each adaptation, except the
 *    direct neighbours of marked cells, which are kept as they are;
 *  - neighbouring cells never differ by more than one level.
 *============================================================================*/

/*
  This file is part of code_saturne, a general-purpose CFD tool.

  Copyright (C) 1998-2026 EDF S.A.

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

#include <math.h>
#include <string.h>

#include "cs_headers.h"
#include "mesh/cs_mesh_adaptive_refinement.h"

/*----------------------------------------------------------------------------*/
/*
 * Settings
 *
 *  - AMR_ENABLE        : 1 = AMR active; 0 = fixed base mesh (reference run)
 *  - AMR_MAX_LEVEL     : maximum refinement level above the base mesh
 *  - AMR_INTERVAL      : time steps between two adaptations
 *  - AMR_ALPHA_MIN/MAX : cells with AMR_ALPHA_MIN < void_fraction <
 *                        AMR_ALPHA_MAX are interface cells
 *  - AMR_JUMP          : a face across which void_fraction jumps by more than
 *                        this value marks its two cells (sharp interface lying
 *                        on a face, invisible to the band test)
 *  - AMR_N_BUFFER      : extra layers of marked cells around the interface
 *  - AMR_LOAD_BALANCE  : redistribute cells between ranks after adaptation
 */
/*----------------------------------------------------------------------------*/

#define AMR_ENABLE        1
#define AMR_MAX_LEVEL     2
#define AMR_INTERVAL      5
#define AMR_ALPHA_MIN     0.1
#define AMR_ALPHA_MAX     0.9
#define AMR_JUMP          0.5
#define AMR_N_BUFFER      1
#define AMR_LOAD_BALANCE  false

/*----------------------------------------------------------------------------
 * Refinement indicator: vals[c] = 1 for the cells of the interface region.
 *----------------------------------------------------------------------------*/

static void
_interface_amr_indicator([[maybe_unused]] const void  *input,
                         int                          *vals)
{
  const cs_mesh_t *m = cs_glob_mesh;
  const cs_lnum_t n_cells = m->n_cells;
  const cs_lnum_t n_i_faces = m->n_i_faces;
  const cs_lnum_t n_cells_ext = m->n_cells_with_ghosts;
  const cs_lnum_2_t *i_face_cells = m->i_face_cells;

  for (cs_lnum_t c = 0; c < n_cells_ext; c++)
    vals[c] = 0;

  cs_real_t *alpha = cs_field_by_name("void_fraction")->val;

  if (m->halo != nullptr)
    cs_halo_sync_var(m->halo, CS_HALO_STANDARD, alpha);

  /* Interface cells */

  for (cs_lnum_t c = 0; c < n_cells; c++) {
    if (alpha[c] > AMR_ALPHA_MIN && alpha[c] < AMR_ALPHA_MAX)
      vals[c] = 1;
  }

  /* Sharp jumps across interior faces */

  for (cs_lnum_t f = 0; f < n_i_faces; f++) {
    const cs_lnum_t c0 = i_face_cells[f][0];
    const cs_lnum_t c1 = i_face_cells[f][1];
    if (fabs(alpha[c1] - alpha[c0]) > AMR_JUMP) {
      vals[c0] = 1;
      vals[c1] = 1;
    }
  }

  if (m->halo != nullptr)
    cs_halo_sync_untyped(m->halo, CS_HALO_STANDARD, sizeof(int), vals);

  /* Buffer layers around the marked cells */

  int *tmp = nullptr;
  CS_MALLOC(tmp, n_cells_ext, int);

  for (int layer = 0; layer < AMR_N_BUFFER; layer++) {
    memcpy(tmp, vals, n_cells_ext*sizeof(int));
    for (cs_lnum_t f = 0; f < n_i_faces; f++) {
      const cs_lnum_t c0 = i_face_cells[f][0];
      const cs_lnum_t c1 = i_face_cells[f][1];
      if (tmp[c0] > 0 || tmp[c1] > 0) {
        vals[c0] = 1;
        vals[c1] = 1;
      }
    }
    if (m->halo != nullptr)
      cs_halo_sync_untyped(m->halo, CS_HALO_STANDARD, sizeof(int), vals);
  }

  CS_FREE(tmp);

  /* Log the number of marked cells */

  cs_gnum_t n_flagged = 0;
  for (cs_lnum_t c = 0; c < n_cells; c++)
    if (vals[c] > 0)
      n_flagged++;
  cs_parall_counter(&n_flagged, 1);

  cs_log_printf(CS_LOG_DEFAULT,
                ">>> AMR indicator: %llu cells flagged for refinement "
                "(mesh: %llu cells)\n",
                (unsigned long long)n_flagged,
                (unsigned long long)m->n_g_cells);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Define or modify general numerical and physical user parameters.
 *
 * \param[in, out]   domain    pointer to a cs_domain_t structure
 */
/*----------------------------------------------------------------------------*/

void
cs_user_parameters([[maybe_unused]] cs_domain_t   *domain)
{
  if (AMR_ENABLE == 0) {
    cs_log_printf(CS_LOG_DEFAULT, ">>> AMR disabled (AMR_ENABLE = 0).\n");
    return;
  }

  cs_adaptive_refinement_define(AMR_MAX_LEVEL,
                                AMR_INTERVAL,
                                _interface_amr_indicator,
                                nullptr,            /* no indicator input */
                                0,                  /* P0 interpolation
                                                       (velocity is always
                                                       gradient-based) */
                                AMR_LOAD_BALANCE);
}

/*----------------------------------------------------------------------------*/
