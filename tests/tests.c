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

#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "dblv.h"
#include "zmpi_reduce.h"


#define VERBOSE 0
#define VERIFY  1
#define TIMING  1

typedef int (*MPI_Reduce_t)(const void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm);

void test_mpi_reduce(MPI_Reduce_t mpi_reduce, const char *name, int count, double non_zeros, int comm_size, int comm_rank, MPI_Comm comm)
{
  const int root = 0;

  double *sendbuf, *recvbuf;

  sendbuf = malloc(count * sizeof(double));
  recvbuf = malloc(count * sizeof(double));

  srand(comm_rank + 1);
  if (non_zeros < 1.0)
  {
    dblv_write_zeros(count, sendbuf);
    int nz = 0;
    dblv_write_random_random_next(count, sendbuf, (int) (count * non_zeros), 0.0, &nz);

  } else
  {
    dblv_write_random(count, sendbuf);
  }

#if VERBOSE
  printf("%d: %s: sendbuf:\n", comm_rank, name);
  dblv_print(count, sendbuf, "  ");
#endif

#if TIMING
  MPI_Barrier(comm);
  double t = MPI_Wtime();
#endif
  int ret = mpi_reduce(sendbuf, recvbuf, count, MPI_DOUBLE, MPI_SUM, root, comm);
#if TIMING
  MPI_Barrier(comm);
  t = MPI_Wtime() - t;
#endif

  if(ret != MPI_SUCCESS)
  {
    printf("%d: %s: failed\n", comm_rank, name);
  }

#if VERBOSE
  if (comm_rank == root)
  {
    printf("%d: %s: recvbuf:\n", comm_rank, name);
    dblv_print(count, recvbuf, "  ");
  }
#endif

#if VERIFY
  double *verify_recvbuf = malloc(count * sizeof(double));
  MPI_Reduce(sendbuf, verify_recvbuf, count, MPI_DOUBLE, MPI_SUM, root, comm);

  if (comm_rank == root)
  {
#if VERBOSE
    printf("%d: %s: verify_recvbuf:\n", comm_rank, name);
    dblv_print(count, verify_recvbuf, "  ");
#endif
    const double verify_absolute_diff = dblv_absdiff(count, recvbuf, verify_recvbuf);
    const double verfiy_allowed_diff = count * 1e-10;
    if (verify_absolute_diff > verfiy_allowed_diff)
    {
      printf("%d: %s: verification failed: absolute difference %e is greather than allowed difference %e", comm_rank, name, verify_absolute_diff, verfiy_allowed_diff);
    }
  }

  free(verify_recvbuf);
#endif

#if TIMING
  if (comm_rank == root)
  {
    printf("%d: %s: time: %f\n", comm_rank, name, t);
  }
#endif

  free(sendbuf);
  free(recvbuf);
}


int main(int argc, char *argv[])
{
  int size, rank;

  MPI_Comm comm = MPI_COMM_WORLD;

  MPI_Init(&argc,&argv);

  MPI_Comm_size(comm, &size);
  MPI_Comm_rank(comm, &rank);

  argc--; argv++;

  default_pa.packet_size = 1024 * 1024;

  const int count = 1000000;
  const double non_zeros = 0.01;

  // original
  test_mpi_reduce(MPI_Reduce, "MPI_Reduce", count, non_zeros, size, rank, comm);

  // Rabenseifner algorithm
  test_mpi_reduce(MPI_Reduce_rabenseifner, "MPI_Reduce_rabenseifner", count, non_zeros, size, rank, comm);

  // pipeline algorithm using blocking send/recv operations WITHOUT COMPRESSION
  // test_mpi_reduce(MPI_Reduce_pipe_send_recv, "MPI_Reduce_pipe_send_recv", count, non_zeros, size, rank, comm);

  // pipeline algorithm using blocking sendrecv operations WITHOUT COMPRESSION
  test_mpi_reduce(MPI_Reduce_pipe_sendrecv, "MPI_Reduce_pipe_sendrecv", count, non_zeros, size, rank, comm);

  // pipeline algorithm using blocking sendrecv operations WITH COMPRESSION
  test_mpi_reduce(MPI_Reduce_pipe_sendrecv_rle, "MPI_Reduce_pipe_sendrecv_rle", count, non_zeros, size, rank, comm);

  // pipeline stream algorithm using blocking send/recv operations (clean version) WITHOUT COMPRESSION
  // test_mpi_reduce(MPI_Reduce_pipe_stream_plain, "MPI_Reduce_pipe_stream_plain", count, non_zeros, size, rank, comm);

  // pipeline stream algorithm using blocking send/recv operations WITHOUT COMPRESSION
  test_mpi_reduce(MPI_Reduce_pipe_stream, "MPI_Reduce_pipe_stream", count, non_zeros, size, rank, comm);

  // pipeline stream algorithm using blocking send/recv operations WITH COMPRESSION
  test_mpi_reduce(MPI_Reduce_pipe_stream_rle, "MPI_Reduce_pipe_stream_rle", count, non_zeros, size, rank, comm);

  // gather to root algorithm using blocking send/recv operations WITHOUT COMPRESSION
  test_mpi_reduce(MPI_Reduce_gather, "MPI_Reduce_gather", count, non_zeros, size, rank, comm);

  // gather to root algorithm using blocking send/recv operations WITH COMPRESSION
  test_mpi_reduce(MPI_Reduce_gather_rle, "MPI_Reduce_gather_rle", count, non_zeros, size, rank, comm);

  MPI_Finalize();

  return 0;
}
