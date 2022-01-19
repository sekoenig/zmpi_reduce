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

#include "dblv.h"


#ifdef USE_ZLIB

#include <zlib.h>


void dblv_zlib_deflate(int nin, double *vin, int *nout, unsigned char *vout, int level)
{
  z_stream strm;

  int nout_; if (!nout) nout = &nout_;

  DBLV_TSTART();
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  deflateInit(&strm, level);

  strm.next_in = (Bytef *) vin;
  strm.avail_in = nin * sizeof(double);

  strm.next_out = (Bytef *) vout;
  strm.avail_out = nin * sizeof(double);

  deflate(&strm, Z_FINISH);

  *nout = nin * sizeof(double) - strm.avail_out;

  deflateEnd(&strm);
  DBLV_TEND();

/*  DBLV_PRINTF("dblv_zlib_deflate", nin, ", nout = %d Bytes", *nout);*/
}


void dblv_zlib_inflate(int nin, unsigned char *vin, int *nout, double *vout)
{
}


#endif /* USE_ZLIB */
