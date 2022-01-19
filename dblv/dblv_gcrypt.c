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


#ifdef USE_GCRYPT

#include <gcrypt.h>


void dblv_gcrypt_md5(int nin, double *vin, char *md5sum)
{
  int algo = GCRY_MD_MD5;

  char md5sum_[16]; if (!md5sum) md5sum = md5sum_;

  DBLV_TSTART();
  gcry_md_hash_buffer(algo, md5sum, vin, nin * sizeof(double));
  DBLV_TEND();

  DBLV_PRINTF("dblv_gcrypt_md5", nin, ", md5sum: %llX%llX", *((long long *) &md5sum[0]), *((long long *) &md5sum[8]));
}


#endif /* USE_GCRYPT */
