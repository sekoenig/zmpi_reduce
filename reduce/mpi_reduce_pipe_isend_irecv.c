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

#include "mpi_reduce_common.h"
#include "mpi_reduce_pipe.h"


#define PIPE_ISEND_IRECV_TALL          0
#define PIPE_ISEND_IRECV_TLOOP         1
#define PIPE_ISEND_IRECV_TSENDRECVRED  2
#define PIPE_ISEND_IRECV_TRED          3
#define PIPE_ISEND_IRECV_TIMES         4
double pipe_isend_irecv_times[PIPE_ISEND_IRECV_TIMES];
#undef current_times
#define current_times  pipe_isend_irecv_times


int MPI_Reduce_pipe_isend_irecv(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  int comm_rank, comm_size;
  int type_size;

  const int tag = 0;

  int max_packet, npackets, current_packet, prev_packet, pprev_packet;
  int done, offset;

  int iam_first_in_pipe, iam_last_in_pipe;

  const char *sbuf = sendbuf;
  char *rbuf = recvbuf;
  char *buf0, *buf1, *buf2, *buft;

  MPI_Request reqs[2];
  MPI_Status stats[2];

  pipe_attr local_pa, *my_pa;

  int ret = MPI_Reduce_check(sendbuf, recvbuf, count, datatype, op, root, comm);
  if (ret != MPI_SUCCESS)
  {
    return ret;
  }

  if (MPI_SUCCESS == MPI_Reduce_self(sendbuf, recvbuf, count, datatype, op, root, comm))
  {
    return MPI_SUCCESS;
  }

  timing_zero(PIPE_ISEND_IRECV_TIMES, current_times);
  timing_sstart();

  MPI_Comm_rank(comm, &comm_rank);
  MPI_Comm_size(comm, &comm_size);

  iam_first_in_pipe = (first_in_pipe == comm_rank);
  iam_last_in_pipe = (last_in_pipe == comm_rank);

  MPI_Type_size(datatype, &type_size);

  if (default_pa.buf_size < default_pa.packet_size || !default_pa.buf[0] || !default_pa.buf[1] || !default_pa.buf[2])
  {
    local_pa.packet_size = default_pa.packet_size;
    pipe_attr_alloc_buf(&local_pa, local_pa.packet_size, 2);
    my_pa = &local_pa;

  } else my_pa = &default_pa;

  max_packet = my_pa->packet_size / type_size;

  if (!max_packet)
  {
    verbose_printf("%d here: size of datatype (%d bytes) exceeds packet size (%d bytes)!\n", comm_rank, type_size, my_pa->packet_size);
    max_packet = 1;
  }

  npackets = count / max_packet;
  if (count % max_packet) npackets++;

  buf0 = my_pa->buf[0];
  buf1 = my_pa->buf[1];
  buf2 = my_pa->buf[2];

  done = prev_packet = pprev_packet = 0;

  timing_sstart();

  while (done < count || prev_packet > 0 || pprev_packet > 0)
  {
    if (npackets == 0) current_packet = 0;
    else current_packet = (count - done) / npackets--;

    offset = done * type_size;

    timing_sstart();

    if (iam_first_in_pipe)
    {
      if (current_packet > 0) MPI_Send(&sbuf[offset], current_packet, datatype, next_in_pipe, tag, comm);

    } else if (iam_last_in_pipe)
    {
      if (current_packet > 0) MPI_Irecv(&rbuf[offset], current_packet, datatype, prev_in_pipe, tag, comm, &reqs[0]);
      else reqs[0] = MPI_REQUEST_NULL;

      if (prev_packet > 0)
      {
        timing_sstart();
        reduce_op_2(prev_packet, 0, datatype, op, &sbuf[offset - (prev_packet * type_size)], &rbuf[offset - (prev_packet * type_size)]);
        current_times[PIPE_ISEND_IRECV_TRED] = timing_send();
      }

      MPI_Waitall(1, reqs, stats);

    } else
    {
      if (current_packet > 0) MPI_Irecv(buf0, current_packet, datatype, prev_in_pipe, tag, comm, &reqs[0]);
      else reqs[0] = MPI_REQUEST_NULL;

      if (pprev_packet > 0) MPI_Isend(buf2, pprev_packet, datatype, next_in_pipe, tag, comm, &reqs[1]);
      else reqs[1] = MPI_REQUEST_NULL;

      if (prev_packet > 0)
      {
        timing_sstart();
        reduce_op_2(prev_packet, 0, datatype, op, &sbuf[offset - (prev_packet * type_size)], buf1);
        current_times[PIPE_ISEND_IRECV_TRED] = timing_send();
      }

      MPI_Waitall(2, reqs, stats);

      buft = buf2;
      buf2 = buf1;
      buf1 = buf0;
      buf0 = buft;
    }
    current_times[PIPE_ISEND_IRECV_TSENDRECVRED] = timing_send();

    done += current_packet;

    pprev_packet = prev_packet;
    prev_packet = current_packet;
  }

  current_times[PIPE_ISEND_IRECV_TLOOP] = timing_send();

  if (my_pa == &local_pa) pipe_attr_free_buf(&local_pa);

  current_times[PIPE_ISEND_IRECV_TALL] = timing_send();

  // printf("%d here: %f  %f  %f  %f\n", comm_rank, current_times[PIPE_ISEND_IRECV_TALL],
  //                                                current_times[PIPE_ISEND_IRECV_TLOOP],
  //                                                current_times[PIPE_ISEND_IRECV_TSENDRECVRED],
  //                                                current_times[PIPE_ISEND_IRECV_TRED]);

  return MPI_SUCCESS;
}
