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
#include <string.h>
#include <mpi.h>

#include "debug.h"
#include "timing.h"
#include "trace.h"
#include "reduce_op.h"
#include "logging.h"

#ifdef USE_DBLV
 #include "dblv.h"
#endif


// #define RLE

#ifndef MOD_GATHER
 #define MOD_GATHER(s) s
#endif

int MOD_GATHER(MPI_Reduce_gather)(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  int comm_rank, comm_size;
  int type_size;

  const int tag = 0;

  int recvs = 0;
  int received, receivedc, processed, processedc;

  const char *sbuf;
  char *tbuf;

  int sendc = 0, recvc = 0;

  MPI_Status status;


  MPI_Comm_rank(comm, &comm_rank);
  MPI_Comm_size(comm, &comm_size);

  MPI_Type_size(datatype, &type_size);

  if (comm_size == 1)
  {
    memcpy(recvbuf, sendbuf, type_size * count);
    goto end;
  }

  tbuf = malloc(count * type_size);

  if (comm_rank == root)
  {
    memcpy(recvbuf, sendbuf, type_size * count);

    while (recvs < count * (comm_size - 1) )
    {
      MPI_Recv(tbuf, count, datatype, MPI_ANY_SOURCE, tag, comm, &status);
      MPI_Get_count(&status, datatype, &receivedc); recvc += receivedc;

#ifndef RLE
      processed = processedc = received = receivedc;
      reduce_op_2(received, 0, datatype, op, tbuf, recvbuf);
#else
      processed = processedc = received = count;
      dblv_rle_zero_uc_cf_add2_uc(received, (double *) recvbuf, receivedc, (double *) tbuf, &processedc, NULL);
#endif

      recvs += processed;
    }

  } else
  {
    received = receivedc = count;

#ifndef RLE
    processed = processedc = received;
    sbuf = sendbuf;
#else
    processed = processedc = received;
    dblv_rle_zero_compress2(received, (double *) sendbuf, &processedc, (double *) tbuf);
    sbuf = tbuf;
#endif

    MPI_Send(sbuf, processedc, datatype, root, tag, comm); sendc += processedc;
  }

  free(tbuf);

end:

/*  printf("%d here: %f%% in, %f%% out\n", comm_rank, 100.0 * recvc / count, 100.0 * sendc / count);*/

  return MPI_SUCCESS;
}
