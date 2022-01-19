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
#include <stdlib.h>
#include <math.h>

#include "dblv.h"

DBLV_TVAL;

#undef DBLV_PRINTF
#define DBLV_PRINTF(name, n, sfmt, args...)
#undef DBLV_PRINT
#define DBLV_PRINT(name, n)

double v;


void dblv_print(int n, double *v, const char *prefix)
{
  int i = n;

  while (i-- > 0) printf("%s%f\n", v[i], ((prefix != NULL)?prefix:""));
}


void dblv_read(int nin, double *vin)
{
  int i = nin;

  DBLV_TSTART();
  while (i-- > 0) v = *(vin++);
  DBLV_TEND();

  DBLV_PRINT("dblv_read", nin);
}


void dblv_write_zeros(int nin, double *vin)
{
  int i = nin;

  DBLV_TSTART();
  while (i-- > 0) *(vin++) = 0.0;
  DBLV_TEND();

  DBLV_PRINT("dblv_write_zeros", nin);
}


void dblv_write_random(int nin, double *vin)
{
  int i = nin;

  DBLV_TSTART();
  while (i-- > 0) *(vin++) = (double) rand();
/*  while (i-- > 0) *(vin++) = (double) rand() / RAND_MAX;*/
  DBLV_TEND();

  DBLV_PRINT("dblv_write_random", nin);
}


void dblv_write_random_random(int nin, double *vin, int nrandoms, double nonx, int *newnonx)
{
  int j, i = nrandoms;

  DBLV_TSTART();
  while (i-- > 0)
  {
    j = (int) (((double) rand() / RAND_MAX * (nin - 1)) + 0.5);
    if (newnonx && vin[j] == nonx) (*newnonx)++;
    vin[j] = (double) rand();
  }
  DBLV_TEND();

  DBLV_PRINT("dblv_write_random_random", nrandoms);
}


void dblv_write_random_random_next(int nin, double *vin, int nrandoms, double nonx, int *newnonx)
{
  int j, k, i = nrandoms;

  DBLV_TSTART();
  while (i-- > 0)
  {
    j = (int) (((double) rand() / RAND_MAX * (nin - 1)) + 0.5);
    k = nin;
    while (vin[j] != nonx && k-- > 0) j = (j + 1) % nin;
    if (newnonx && vin[j] == nonx) (*newnonx)++;
    vin[j] = (double) rand();
  }
  DBLV_TEND();

  DBLV_PRINT("dblv_write_random_random", nrandoms);
}


void dblv_write_random_random_step(int nin, double *vin, int nrandoms, int offset, int step, double nonx, int *newnonx)
{
  int j = offset, i = nrandoms;

  DBLV_TSTART();
  while (i-- > 0)
  {
    if (newnonx && vin[j] == nonx) (*newnonx)++;
    vin[j] = (double) rand();
    j = (j + step) % nin;
  }
  DBLV_TEND();

  DBLV_PRINT("dblv_write_random_random", nrandoms);
}


void dblv_copy(int nin, const double *vin, double *vout)
{
  int i = nin;

  DBLV_TSTART();
  while (i-- > 0) *(vout++) = *(vin++);
  DBLV_TEND();

  DBLV_PRINT("dblv_copy", nin);
}


void dblv_scan_zeros(int nin, double *vin, int *nz)
{
  int i = nin;

  int nz_; if (!nz) nz = &nz_;

  *nz = 0;

  DBLV_TSTART();
  while (i-- > 0) *nz += (*(vin++) == 0.0);
  DBLV_TEND();

  DBLV_PRINTF("dblv_scan_zeros", nin, ", nz = %d", *nz);
}


void dblv_scan_cont_zeros(int nin, double *vin, double *cz)
{
  int i = nin, c = 0, t = 0, s = -1;

  double cz_; if (!cz) cz = &cz_;

  *cz = 0.0;

  DBLV_TSTART();
  while (i-- > 0)
  {
    if (s < 0)
    {
      if (*vin == 0.0)
      {
        s = t;
      }

    } else
    {
      if (*vin != 0.0)
      {
        *cz += t - s;
        s = -1;
        c++;
      }
    }
    t++; vin++;
  }
  DBLV_TEND();

  if (s >= 0)
  {
    *cz += t - s;
    s = -1;
    c++;
  }

  if (c) *cz /= c;

  DBLV_PRINTF("dblv_scan_zeros", nin, ", cz = %d", *cz);
}


void dblv_scan_values(int nin, double *vin, int *nz, double v)
{
  int i = nin;

  int nz_; if (!nz) nz = &nz_;

  *nz = 0;

  DBLV_TSTART();
  while (i-- > 0) *nz += (*(vin++) == v);
  DBLV_TEND();

  DBLV_PRINTF("dblv_scan_values", nin, ", nz = %d", *nz);
}


void dblv_copy_nonzeros(int nin, double *vin, int *nout, double *vout)
{
  int i = nin;
  double *vout_ = vout;

  int nout_; if (!nout) nout = &nout_;

  DBLV_TSTART();
  while (i-- > 0)
  {
    if (*vin != 0.0) *(vout++) = *vin;

    vin++;
  }
  *nout = vout - vout_;
  DBLV_TEND();

  DBLV_PRINTF("dblv_copy_nonzeros", nin, ", nout = %d", *nout);
}


void dblv_copy_nonvalues(int nin, double *vin, int *nout, double *vout, double v)
{
  int i = nin;
  double *vout_ = vout;

  int nout_; if (!nout) nout = &nout_;

  DBLV_TSTART();
  while (i-- > 0)
  {
    if (*vin != v) *(vout++) = *vin;

    vin++;
  }
  *nout = vout - vout_;
  DBLV_TEND();

  DBLV_PRINTF("dblv_copy_nonvalues", nin, ", nout = %d", *nout);
}


int dblv_equal(int nin, double *vin0, double *vin1)
{
  int i = nin;

  while (i-- > 0)
  {
    if (*vin0++ != *vin1++) return 0;
  }

  return 1;
}


double dblv_absdiff(int nin, double *vin0, double *vin1)
{
  int i = nin;
  double ad = 0.0;

  while (i-- > 0)
  {
    ad += fabs(*vin0++ - *vin1++);
  }

  return ad;
}
