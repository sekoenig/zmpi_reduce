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


void dblv_rle_zero_compress(int nin, double *vin, int *nout, double *vout)
{
  int m0, m = 0;
  double *vout_ = vout;

  int nout_; if (!nout) nout = &nout_;

  DBLV_TSTART();
  while (m < nin)
  {
    /* eq. 0.0 scan */
    m0 = m;
    while (*vin == 0.0 && m < nin)
    {
      vin++; m++;
    }

    /* 0.0 write */
    if (m - m0 > 0)
    {
      DBL_RLE2_SET_P(vout, m - m0);
      vout++;
    }

    /* neq. 0.0 copy */
    while (*vin != 0.0 && m < nin)
    {
      *(vout++) = *(vin++);
      m++;
    }
  }
  DBLV_TEND();

  *nout = vout - vout_;

/*  DBLV_PRINTF("dblv_rle_zero_compress", nin, ", nout: %d, ratio: %.1f%%", *nout, 100.0 * *nout / nin);*/
}


void dblv_rle_zero_compress2(int nin, double *vin, int *nout, double *vout)
{
  int m0, m = 0;
  double *vout_ = vout;

  int nout_; if (!nout) nout = &nout_;

  DBLV_TSTART();
  while (m < nin)
  {
    if (*vin != 0.0)
    {
      *(vout++) = *(vin++);
      m++;
      continue;
    }

    m0 = m;
    do
    {
      vin++; m++;

    } while (*vin == 0.0 && m < nin);

    DBL_RLE2_SET_P(vout, m - m0);
    vout++;
  }
  DBLV_TEND();

  *nout = vout - vout_;

/*  DBLV_PRINTF("dblv_rle_zero_compress", nin, ", nout: %d, ratio: %.1f%%", *nout, 100.0 * *nout / nin);*/
}


void dblv_rle_zero_compress3(int nin, double *vin, int nout, double *vout, int *nread, int *nwrite)
{
  double *vin_c = vin;
  double *vin_e = vin + nin;
  double *vin_z;
  double *vout_c = vout;
  double *vout_e = vout + nout;

  DBLV_TSTART();
  while (vin_c < vin_e && vout_c < vout_e)
  {
    if (*vin_c != 0.0)
    {
      *(vout_c++) = *(vin_c++);
      continue;
    }

    vin_z = vin_c;
    do
    {
      vin_c++;

    } while (*vin_c == 0.0 && vin_c < vin_e);

    DBL_RLE2_SET_P(vout_c, vin_c - vin_z);
    vout_c++;
  }
  DBLV_TEND();

  if (nread) *nread = vin_c - vin;
  if (nwrite) *nwrite = vout_c - vout;

/*  DBLV_PRINTF("dblv_rle_zero_compress3", *nread, ", nread: %d, nwrite: %d, ratio: %.1f%%", *nread, *nwrite, 100.0 * *nwrite / *nread);*/
}


void dblv_rle_zero_uncompress(int nin, double *vin, int *nout, double *vout)
{
  int i, m = 0;
  double *vout_ = vout;

  int nout_; if (!nout) nout = &nout_;

  DBLV_TSTART();
  while (m < nin)
  {
    /* neq. NaN copy */
    while (DBL_ISN_NAN_P(vin) && m < nin)
    {
      *(vout++) = *(vin++);
      m++;
    }

    if (m < nin)
    {
      for (i = DBL_RLE_GET_P(vin); i > 0; i--) *(vout++) = 0.0;

      vin++;
      m++;
    }
  }
  DBLV_TEND();

  *nout = vout - vout_;

  DBLV_PRINTF("dblv_rle_zero_uncompress", *nout, ", nout: %d", *nout);
}


void dblv_rle_zero_uncompress2(int nin, double *vin, int *nout, double *vout)
{
  int i, m = 0;
  double *vout_ = vout;

  int nout_; if (!nout) nout = &nout_;

  DBLV_TSTART();
  while (m < nin)
  {
    if (DBL_ISN_NAN_P(vin))
    {
      *(vout++) = *(vin++);
      m++;
      continue;
    }

    for (i = DBL_RLE_GET_P(vin); i > 0; i--) *(vout++) = 0.0;

    vin++;
    m++;
  }
  DBLV_TEND();

  *nout = vout - vout_;

/*  DBLV_PRINTF("dblv_rle_zero_uncompress", *nout, ", nout: %d", *nout);*/
}
