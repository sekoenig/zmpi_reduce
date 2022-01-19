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
#include <sys/stat.h>

#include "dblv.h"


void dblv_bin_count(int *count, const char *fname)
{
  struct stat buf;

  int count_; if (!count) count = &count_;

  stat(fname, &buf);

  *count = buf.st_size / sizeof(double);
}


void dblv_bin_fread(int nout, double *vout, const char *fname, int *count)
{
  FILE *f;

  *count = 0;

  if (!(f = fopen(fname, "r"))) return;

  *count = fread(vout, sizeof(double), nout, f);

  fclose(f);
}


void dblv_bin_fwrite(int nin, double *vin, const char *fname)
{
  FILE *f;

  if (!(f = fopen(fname, "w"))) return;

  fwrite(vin, sizeof(double), nin, f);

  fclose(f);
}


void dblv_plain_lines(int *count, const char *fname)
{
  FILE *f;
#define LINE_MAX 1024
  char line[LINE_MAX];

  int count_; if (!count) count = &count_;

  *count = 0;

  if (!(f = fopen(fname, "r"))) return;

  while (fgets(line, LINE_MAX, f)) (*count)++;

  fclose(f);
}


void dblv_hex_write(int nin, double *vin, FILE *f)
{
  int i = nin;

  if (!f) return;

  while (i-- > 0) fprintf(f, "0x%.16llX\n", *((long long *) vin++));
}


void dblv_hex_fwrite(int nin, double *vin, const char *fname)
{
  FILE *f;

  if (!(f = fopen(fname, "w"))) return;

  dblv_hex_write(nin, vin, f);

  fclose(f);
}


void dblv_dec_fread(int nout, double *vout, const char *fname, int *count)
{
  FILE *f;

  *count = 0;

  if (!(f = fopen(fname, "r"))) return;

  while (nout-- > 0 && fscanf(f, "%le", vout++) == 1) (*count)++;

  fclose(f);
}


#include "dblv_rle.h"

void dblv_dec_write(int nout, double *vout, FILE *f)
{
  int i = nout;

  if (!f) return;

/*   while (i-- > 0) fprintf(f, "%.16le\n", *(vout++)); */

  while (i-- > 0)
  {
    fprintf(f, "%.16le", *vout);

    if (DBL_IS_NAN_P(vout)) fprintf(f, " (%ld)\n", (long int) DBL_RLE_GET_P(vout)); else fprintf(f, "\n");

    vout++;
  }
}


void dblv_dec_fwrite(int nout, double *vout, const char *fname)
{
  FILE *f;

  if (!(f = fopen(fname, "w"))) return;

  dblv_dec_write(nout, vout, f);

  fclose(f);
}
