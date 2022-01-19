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

#ifndef __DBLV_H__
#define __DBLV_H__


#include <stdio.h>
#include <string.h>


#ifdef USE_MPI

 #include <mpi.h>

 #define DBLV_TVAL      double tvals, tvale
 extern DBLV_TVAL;
 #define DBLV_TSTART()  (tvals = MPI_Wtime())
 #define DBLV_TEND()    (tvale = MPI_Wtime())
 #define DBLV_TDIFF()   (tvale - tvals)

#else

 #include <sys/time.h>

 #define DBLV_TVAL      struct timeval tvals, tvale
 extern DBLV_TVAL;
 #define DBLV_TSTART()  (gettimeofday(&(tvals), NULL))
 #define DBLV_TEND()    (gettimeofday(&(tvale), NULL))
 #define DBLV_TDIFF()   ((double) (((tvale).tv_sec - (tvals).tv_sec) + ((tvale).tv_usec - (tvals).tv_usec) * 1e-6))

#endif

#define DBLV_PRINT_SPACE 30

#define DBLV_PRINTF(name, n, sfmt, args...)  printf("%s: %*.6f MB/s" sfmt "\n", name, (int) (DBLV_PRINT_SPACE - strlen(name)), (double) n * sizeof(double) / DBLV_TDIFF() *1e-6, args);
#define DBLV_PRINT(name, n)                  printf("%s: %*.6f MB/s\n", name, (int) (DBLV_PRINT_SPACE - strlen(name)), (double) n * sizeof(double) / DBLV_TDIFF() *1e-6);


/* dbvl_io.c */
void dblv_bin_fread(int nout, double *vout, const char *fname, int *n);
void dblv_bin_fwrite(int nin, double *vin, const char *fname);
void dblv_plain_lines(int *count, const char *fname);
void dblv_hex_write(int nin, double *vin, FILE *f);
void dblv_hex_fwrite(int nin, double *vin, const char *fname);
void dblv_dec_fread(int nout, double *vout, const char *fname, int *n);
void dblv_dec_write(int nout, double *vout, FILE *f);
void dblv_dec_fwrite(int nout, double *vout, const char *fname);

/* dbvl_rw.c */
void dblv_print(int n, double *v, const char *prefix);
void dblv_read(int nin, double *vin);
void dblv_write_zeros(int nin, double *vin);
void dblv_write_random(int nin, double *vin);
void dblv_write_random_random(int nin, double *vin, int nrandoms, double nonx, int *newnonx);
void dblv_write_random_random_next(int nin, double *vin, int nrandoms, double nonx, int *newnonx);
void dblv_write_random_random_step(int nin, double *vin, int nrandoms, int offset, int step, double nonx, int *newnonx);
void dblv_copy(int nin, const double *vin, double *vout);
void dblv_scan_zeros(int nin, double *vin, int *nz);
void dblv_scan_cont_zeros(int nin, double *vin, double *cz);
void dblv_scan_values(int nin, double *vin, int *nz, double v);
void dblv_copy_nonzeros(int nin, double *vin, int *nout, double *vout);
void dblv_copy_nonvalues(int nin, double *vin, int *nout, double *vout, double v);
int dblv_equal(int nin, double *vin0, double *vin1);
double dblv_absdiff(int nin, double *vin0, double *vin1);

/* dblv_rle_zero.c */
void dblv_rle_zero_compress(int nin, double *vin, int *nout, double *vout);
void dblv_rle_zero_compress2(int nin, double *vin, int *nout, double *vout);
void dblv_rle_zero_compress3(int nin, double *vin, int nout, double *vout, int *nread, int *nwrite);
void dblv_rle_zero_uncompress(int nin, double *vin, int *nout, double *vout);
void dblv_rle_zero_uncompress2(int nin, double *vin, int *nout, double *vout);
/* dblv_rle_zero_add.c */
void dblv_rle_zero_cf_uc_add2_cb(int nin0, double *vin0, int nin1, double *vin1, int *nout, double **vout);
void dblv_rle_zero_cf_uc_add2_ub(int nin0, double *vin0, int nin1, double *vin1, int *nout, double **vout);
void dblv_rle_zero_uc_cf_add2_uc(int nin0, double *vin0, int nin1, double *vin1, int *nout, double **vout);
void dblv_rle_zero_uc_uc_add2_cf(int nin0, double *vin0, int nin1, double *vin1, int *nout, double **vout);
void dblv_rle_zero_cf_uc_add3_cf(int nin0, double *vin0, int nin1, double *vin1, int nout, double *vout, int *nread0, int *nread1, int *nwrite, double *vin0_next);
void dblv_rle_zero_cf_uc_add3_uc(int nin0, double *vin0, int nin1, double *vin1, int nout, double *vout, int *nread0, int *nread1, int *nwrite, double *vin0_next);

#ifdef USE_ZLIB

/* dblv_zlib.c */
void dblv_zlib_deflate(int nin, double *vin, int *nout, unsigned char *vout, int level);
void dblv_zlib_inflate(int nin, unsigned char *vin, int *nout, double *vout);

#endif

#ifdef USE_GCRYPT

/* dblv_gcrypt.c */
void dblv_gcrypt_md5(int nin, double *vin, char *md5sum);

#endif

#ifdef USE_MPI

/* dblv_mpi.c */
void dblv_mpi_send(int nin, double *vin, int dst, int tag, int rank, MPI_Comm comm);
void dblv_mpi_recv(int nout, double *vout, int src, int tag, int rank, MPI_Comm comm, int *count);

#endif


#endif /* __DBLV_H__ */
