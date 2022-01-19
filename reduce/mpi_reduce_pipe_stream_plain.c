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

#include "mpi_reduce_pipe.h"


int MPI_Reduce_pipe_stream_plain(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  int comm_rank, comm_size;
  int type_size;

  const int tag = 0;

  int max_packet, sends, recvs, offset, current, next;

  int iam_first_in_pipe, iam_last_in_pipe;

  const char *sbuf = sendbuf;
  char *rbuf = recvbuf;
  char *spbuf, *rpbuf, *tpbuf;

  MPI_Status status;

  pipe_attr local_pa, *my_pa;

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

  if (default_pa.buf_size < default_pa.packet_size || !default_pa.buf[0] || !default_pa.buf[1])
  {
    local_pa.packet_size = default_pa.packet_size;
    pipe_attr_alloc_buf(&local_pa, local_pa.packet_size, 2);
    my_pa = &local_pa;

  } else my_pa = &default_pa;

  max_packet = my_pa->packet_size / type_size;

  spbuf = my_pa->buf[0];
  rpbuf = my_pa->buf[1];

  sends = recvs = 0;
  offset = 0;
  current = next = 0;

  while (sends < count || recvs < count)
  {
    if (iam_first_in_pipe)
    {
      /* recv */
      next = count - sends;
      if (next > max_packet) next = max_packet;
      recvs += next;

      /* op */
      current = next; next = 0;

      /* send */
      MPI_Send(&sbuf[offset], current, datatype, next_in_pipe, tag, comm);
      sends += current;

      offset += current * type_size;

    } else if (iam_last_in_pipe)
    {
      /* recv */
      MPI_Recv(&rbuf[offset], max_packet, datatype, prev_in_pipe, tag, comm, &status);
      MPI_Get_count(&status, datatype, &next);
      recvs += next;

      /* op */
      reduce_op_2(next, 0, datatype, op, &sbuf[offset], &rbuf[offset]);
      offset += next * type_size;
      current = next; next = 0;

      /* send */
      sends += current;

    } else
    {
      if (next <= 0 && recvs < count)  /* nothing received, but still something left ot receive? */
      {
        if (current <= 0)  /* nothing to send? */
        {
          /* recv */
          MPI_Recv(rpbuf, max_packet, datatype, prev_in_pipe, tag, comm, &status);

        } else  /* something to send! */
        {
          /* send / recv */
          MPI_Sendrecv(spbuf, current, datatype, next_in_pipe, tag, rpbuf, max_packet, datatype, prev_in_pipe, tag, comm, &status);
          sends += current;
        }

        MPI_Get_count(&status, datatype, &next);
        recvs += next;

      } else  /* something received or nothing left to receive! */
      {
        if (current > 0)  /* something to send? */
        {
          /* send */
          MPI_Send(spbuf, current, datatype, next_in_pipe, tag, comm);
          sends += current;
        }
      }

      /* op */
      reduce_op_2(next, 0, datatype, op, &sbuf[offset], rpbuf);
      offset += next * type_size;
      current = next; next = 0;

      tpbuf = rpbuf; rpbuf = spbuf; spbuf = tpbuf;
    }
  }

  if (my_pa == &local_pa) pipe_attr_free_buf(&local_pa);

end:

  return MPI_SUCCESS;
}
