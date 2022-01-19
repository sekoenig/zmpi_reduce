/*
 *  Copyright (C) 2019-2022 Michael Hofmann
 *  Copyright (C) 2008-2018 Michael Hofmann, Chemnitz University of Technology
 *
 *  This file is part of the ZMPI Reduce Library.
 *
 *  The ZMPI-Reduce is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The ZMPI-Reduce is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>

#include "dblv.h"
#include "dblv_rle.h"


void dblv_rle_zero_cf_uc_add2_cb(int nin0, double *vin0, int nin1, double *vin1, int *nout, double **vout)
{
  int m, n, m0;

  double *vin0_, *vin1_, *vout_;

  vin0_ = vin0 + nin0 - 1;
  vin1_ = vin1 + nin1 - 1;
  vout_ = vin0 + *nout - 1;

  m = 0;
  while (m < nin1)
  {
    if (DBL_ISN_NAN_P(vin0_))
    {
      *(vout_--) = *(vin0_--) + *(vin1_--);
      m++;
      continue;
    }

    n = m + DBL_RLE_GET_P(vin0_);
    vin0_--;

    while (m < n)
    {
      if (*vin1_ != 0.0)
      {
        *(vout_--) = *(vin1_--);
        m++;
        continue;
      }

      m0 = m;
      do
      {
        vin1_--; m++;

      } while (*vin1_ == 0.0 && m < n);

      DBL_RLE2_SET_P(vout_, m - m0);
      vout_--;
    }
  }

  *nout -= vout_ + 1 - vin0;
  *vout = vout_ + 1;
}


void dblv_rle_zero_cf_uc_add2_ub(int nin0, double *vin0, int nin1, double *vin1, int *nout, double **vout)
{
  int m, n;

  double *vin0_, *vin1_, *vout_;

  vin0_ = vin0 + nin0 - 1;
  vin1_ = vin1 + nin1 - 1;
  vout_ = vin0 + *nout - 1;

  m = 0;
  while (m < nin1)
  {
    if (DBL_ISN_NAN_P(vin0_))
    {
      *(vout_--) = *(vin0_--) + *(vin1_--);
      m++;
      continue;
    }

    n = m + DBL_RLE_GET_P(vin0_);
    vin0_--;

    for (; m < n; m++) *(vout_--) = *(vin1_--);
  }

  *nout -= vout_ + 1 - vin0;
  if (vout) *vout = vout_ + 1;
}


void dblv_rle_zero_uc_cf_add2_uc(int nin0, double *vin0, int nin1, double *vin1, int *nout, double **vout)
{
  int m, n;

  double *vin0_, *vin1_, *vout_;

  vin0_ = vin0 + nin0 - 1;
  vin1_ = vin1 + nin1 - 1;
  vout_ = vin0 + *nout - 1;

  m = 0;
  while (m < nin0)
  {
    if (DBL_ISN_NAN_P(vin1_))
    {
      *(vout_--) = *(vin0_--) + *(vin1_--);
      m++;
      continue;
    }

    n = m + DBL_RLE_GET_P(vin1_);
    vin1_--;

    for (; m < n; m++) *(vout_--) = *(vin0_--);
  }

  *nout -= vout_ + 1 - vin0;
  if (vout) *vout = vout_ + 1;
}


void dblv_rle_zero_uc_uc_add2_cf(int nin0, double *vin0, int nin1, double *vin1, int *nout, double **vout)
{
  int m0, m = 0;

  double *vin0_, *vin1_, *vout_;
  double v;

  vin0_ = vin0;
  vin1_ = vin1;
  vout_ = vin0;

  while (m < nin0)
  {
    v = *(vin0_++) + *(vin1_++); m++;

    if (v != 0.0)
    {
      *(vout_++) = v;
      continue;
    }

    m0 = m;
    while ((v = *vin0_ + *vin1_) == 0.0 && m < nin0)
    {
      vin0_++; vin1_++; m++;
    }

    DBL_RLE2_SET_P(vout_, m - m0 + 1);
    vout_++;
  }

  *nout = vout_ - vin0;
  *vout = vin0;
}


void dblv_rle_zero_cf_uc_add3_cf(int nin0, double *vin0, int nin1, double *vin1, int nout, double *vout, int *nread0, int *nread1, int *nwrite, double *vin0_next)
{
  double *vin0_c = vin0;
  double *vin0_e = vin0 + nin0;
  double *vin1_c = vin1;
  double *vin1_e = vin1 + nin1;
  double *vin1_z;
  double *vout_c = vout;
  double *vout_e = vout + nout;
  int n;

  if (DBL_IS_NAN_P(vin0_next))
  {
    n = DBL_RLE_GET_P(vin0_next);

    while (n > 0 && vin1_c < vin1_e && vout_c < vout_e)
    {
      if (*vin1_c != 0.0)
      {
        *(vout_c++) = *(vin1_c++); n--;
        continue;
      }

      vin1_z = vin1_c;
      do
      {
        vin1_c++; n--;

      } while (n > 0 && *vin1_c == 0.0 && vin1_c < vin1_e);

      DBL_RLE2_SET_P(vout_c, vin1_c - vin1_z);
      vout_c++;
    }

    *vin0_next = 0.0;
    if (n > 0) DBL_RLE_SET_P(vin0_next, n);
  }

  while (vin0_c < vin0_e && vin1_c < vin1_e && vout_c < vout_e)
  {
    if (DBL_ISN_NAN_P(vin0_c))
    {
      *(vout_c++) = *(vin0_c++) + *(vin1_c++);
      continue;
    }

    n = DBL_RLE_GET_P(vin0_c);
    vin0_c++;

    while (n > 0 && vin1_c < vin1_e && vout_c < vout_e)
    {
      if (*vin1_c != 0.0)
      {
        *(vout_c++) = *(vin1_c++); n--;
        continue;
      }

      vin1_z = vin1_c;
      do
      {
        vin1_c++; n--;

      } while (n > 0 && *vin1_c == 0.0 && vin1_c < vin1_e);

      DBL_RLE2_SET_P(vout_c, vin1_c - vin1_z);
      vout_c++;
    }

    *vin0_next = 0.0;
    if (n > 0) DBL_RLE_SET_P(vin0_next, n);
  }

  if (nread0) *nread0 = vin0_c - vin0;
  if (nread1) *nread1 = vin1_c - vin1;
  if (nwrite) *nwrite = vout_c - vout;
}


void dblv_rle_zero_cf_uc_add3_uc(int nin0, double *vin0, int nin1, double *vin1, int nout, double *vout, int *nread0, int *nread1, int *nwrite, double *vin0_next)
{
  double *vin0_c = vin0;
  double *vin0_e = vin0 + nin0;
  double *vin1_c = vin1;
  double *vin1_e = vin1 + nin1;
  double *vout_c = vout;
  double *vout_e = vout + nout;
  int n;

  if (DBL_IS_NAN_P(vin0_next))
  {
    n = DBL_RLE_GET_P(vin0_next);

    while (n > 0 && vin1_c < vin1_e && vout_c < vout_e)
    {
      *(vout_c++) = *(vin1_c++); n--;
    }

    *vin0_next = 0.0;
    if (n > 0) DBL_RLE_SET_P(vin0_next, n);
  }

  while (vin0_c < vin0_e && vin1_c < vin1_e && vout_c < vout_e)
  {
    if (DBL_ISN_NAN_P(vin0_c))
    {
      *(vout_c++) = *(vin0_c++) + *(vin1_c++);
      continue;
    }

    n = DBL_RLE_GET_P(vin0_c);
    vin0_c++;

    while (n > 0 && vin1_c < vin1_e && vout_c < vout_e)
    {
      *(vout_c++) = *(vin1_c++); n--;
    }

    *vin0_next = 0.0;
    if (n > 0) DBL_RLE_SET_P(vin0_next, n);
  }

  if (nread0) *nread0 = vin0_c - vin0;
  if (nread1) *nread1 = vin1_c - vin1;
  if (nwrite) *nwrite = vout_c - vout;
}
