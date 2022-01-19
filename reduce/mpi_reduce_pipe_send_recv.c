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

#include "mpi_reduce_pipe.h"


#define PIPE_SEND_RECV_TALL   0
#define PIPE_SEND_RECV_TLOOP  1
#define PIPE_SEND_RECV_TSEND  2
#define PIPE_SEND_RECV_TRECV  3
#define PIPE_SEND_RECV_TRED   4
#define PIPE_SEND_RECV_TIMES  5
double pipe_send_recv_times[PIPE_SEND_RECV_TIMES];
#undef current_times
#define current_times  pipe_send_recv_times


int MPI_Reduce_pipe_send_recv(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  int comm_rank, comm_size;
  int type_size;

  const int tag = 0;

  int max_packet, npackets, current_packet;
  int done, offset;

  int iam_first_in_pipe, iam_last_in_pipe;

  const char *sbuf = sendbuf;
  char *rbuf = recvbuf;
  char *buf0;

  MPI_Status status;

  pipe_attr local_pa, *my_pa;

#ifdef THREADED_REDUCE
  threaded_reduce_info tri;
#endif

#ifdef SEND_RECV_INIT
  MPI_Request reqs[2];
#endif

  timing_zero(PIPE_SEND_RECV_TIMES, current_times);
  timing_sstart();

  MPI_Comm_rank(comm, &comm_rank);
  MPI_Comm_size(comm, &comm_size);

  iam_first_in_pipe = (first_in_pipe == comm_rank);
  iam_last_in_pipe = (last_in_pipe == comm_rank);

  MPI_Type_size(datatype, &type_size);

  if (comm_size == 1)
  {
    memcpy(recvbuf, sendbuf, type_size * count);
    goto end;
  }

  if (default_pa.buf_size < default_pa.packet_size || !default_pa.buf[0])
  {
    local_pa.packet_size = default_pa.packet_size;
    pipe_attr_alloc_buf(&local_pa, local_pa.packet_size, 1);
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

#ifdef SEND_RECV_INIT
  MPI_Recv_init(buf0, max_packet, datatype, prev_in_pipe, tag, comm, &reqs[0]);
  MPI_Send_init(buf0, max_packet, datatype, next_in_pipe, tag, comm, &reqs[1]);
#endif

#ifdef THREADED_REDUCE
  if (first_in_pipe != comm_rank) threaded_reduce_init(&tri, 2);
#endif

  done = 0;

  timing_sstart();

  while (done < count)
  {
    current_packet = (count - done) / npackets--;

    offset = done * type_size;

    if (first_in_pipe == comm_rank)
    {
      timing_sstart();
#ifdef SEND_RECV_INIT
      MPI_Send(&sbuf[offset], max_packet, datatype, next_in_pipe, tag, comm);
#else
      MPI_Send(&sbuf[offset], current_packet, datatype, next_in_pipe, tag, comm);
#endif
      current_times[PIPE_SEND_RECV_TSEND] += timing_send();

    } else if (last_in_pipe == comm_rank)
    {
      timing_sstart();
#ifdef SEND_RECV_INIT
      MPI_Recv(&rbuf[offset], max_packet, datatype, prev_in_pipe, tag, comm, &status);
#else
      MPI_Recv(&rbuf[offset], current_packet, datatype, prev_in_pipe, tag, comm, &status);
#endif
      current_times[PIPE_SEND_RECV_TRECV] += timing_send();

      timing_sstart();
#ifdef THREADED_REDUCE
      threaded_reduce_op_2(&tri, current_packet, datatype, op, &sbuf[offset], &rbuf[offset]);
#else
      reduce_op_2(current_packet, 0, datatype, op, &sbuf[offset], &rbuf[offset]);
#endif
      current_times[PIPE_SEND_RECV_TRED] += timing_send();

    } else
    {
      timing_sstart();
#ifdef SEND_RECV_INIT
      MPI_Start(&reqs[0]);
      MPI_Wait(&reqs[0], &status);
#else
      MPI_Recv(buf0, current_packet, datatype, prev_in_pipe, tag, comm, &status);
#endif
      current_times[PIPE_SEND_RECV_TRECV] += timing_send();

      timing_sstart();
#ifdef THREADED_REDUCE
      threaded_reduce_op_2(&tri, current_packet, datatype, op, &sbuf[offset], buf0);
#else
      reduce_op_2(current_packet, 0, datatype, op, &sbuf[offset], buf0);
#endif
      current_times[PIPE_SEND_RECV_TRED] += timing_send();

      timing_sstart();
#ifdef SEND_RECV_INIT
      MPI_Start(&reqs[1]);
      MPI_Wait(&reqs[1], &status);
#else
      MPI_Send(buf0, current_packet, datatype, next_in_pipe, tag, comm);
#endif
      current_times[PIPE_SEND_RECV_TSEND] += timing_send();
    }

    done += current_packet;
  }

  current_times[PIPE_SEND_RECV_TLOOP] = timing_send();

#ifdef THREADED_REDUCE
  if (first_in_pipe != comm_rank) threaded_reduce_destroy(&tri);
#endif

#ifdef SEND_RECV_INIT
  MPI_Request_free(&reqs[0]);
  MPI_Request_free(&reqs[1]);
#endif

  if (my_pa == &local_pa) pipe_attr_free_buf(&local_pa);

end:

  current_times[PIPE_SEND_RECV_TALL] = timing_send();

/*  printf("%d here: %f  %f  %f  %f  %f\n", comm_rank, current_times[PIPE_SEND_RECV_TALL],
                                                     current_times[PIPE_SEND_RECV_TLOOP],
                                                     current_times[PIPE_SEND_RECV_TSEND],
                                                     current_times[PIPE_SEND_RECV_TRECV],
                                                     current_times[PIPE_SEND_RECV_TRED]);
*/
  return MPI_SUCCESS;
}
