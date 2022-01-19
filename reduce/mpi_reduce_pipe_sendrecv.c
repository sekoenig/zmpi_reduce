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


// #define RLE
// #define RLE_FIRST

#ifndef RLE
 #undef RLE_FIRST
#endif

#ifndef MOD_PIPE
 #define MOD_PIPE(s) s
#endif

#define PIPE_SENDRECV_TALL       0
#define PIPE_SENDRECV_TLOOP      1
#define PIPE_SENDRECV_TSEND      2
#define PIPE_SENDRECV_TRECV      3
#define PIPE_SENDRECV_TSENDRECV  4
#define PIPE_SENDRECV_TRED       5
#define PIPE_SENDRECV_TIMES      6
double MOD_PIPE(pipe_sendrecv_times)[PIPE_SENDRECV_TIMES];
#undef current_times
#define current_times  MOD_PIPE(pipe_sendrecv_times)

int MOD_PIPE(MPI_Reduce_pipe_sendrecv)(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  int comm_rank, comm_size;
  int type_size;

  const int tag = 0;

  int max_packet, npackets, current_packet, prev_packet;
  int done, offset;

  int iam_first_in_pipe, iam_last_in_pipe;

  const char *sbuf = sendbuf;
  char *rbuf = recvbuf;
  char *buf0, *buf1, *buft;

#ifdef RLE
  int rle_sendcount, rle_recvcount;
  double *rle_sendbuf;
  int rle_sendcounts = 0, rle_recvcounts = 0;
#endif

  MPI_Status status;

  pipe_attr local_pa, *my_pa;

  timing_zero(PIPE_SENDRECV_TIMES, current_times);
  timing_sstart();

  MPI_Comm_rank(comm, &comm_rank);
  MPI_Comm_size(comm, &comm_size);

  iam_first_in_pipe = (first_in_pipe == comm_rank);
  iam_last_in_pipe = (last_in_pipe == comm_rank);

  MPI_Type_size(datatype, &type_size);

  if (default_pa.logging) mainlog_printf("MPI_Reduce_pipe_sendrecv: %d  %d  %d\n", count, type_size, comm_size);

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

  if (!max_packet)
  {
    verbose_printf("%d here: size of datatype (%d bytes) exceeds packet size (%d bytes)!\n", comm_rank, type_size, my_pa->packet_size);
    max_packet = 1;
  }

  npackets = count / max_packet;
  if (count % max_packet) npackets++;

  buf0 = my_pa->buf[0];
  buf1 = my_pa->buf[1];

  done = prev_packet = 0;

  timing_sstart();

  while (done < count || prev_packet > 0)
  {
    if (npackets == 0) current_packet = 0;
    else current_packet = (count - done) / npackets--;

    offset = done * type_size;

    if (iam_first_in_pipe)
    {
      if (current_packet > 0)
      {
#ifdef RLE_FIRST
        rle_sendbuf = (double *) buf0;

        timing_sstart();
        dblv_rle_zero_compress(current_packet, (double *) &sbuf[offset], &rle_sendcount, rle_sendbuf);
        current_times[PIPE_SENDRECV_TRED] += timing_send();

        rle_sendcounts += rle_sendcount;

        timing_sstart();
        MPI_Send(rle_sendbuf, rle_sendcount, datatype, next_in_pipe, tag, comm);
        current_times[PIPE_SENDRECV_TSEND] += timing_send();
#else
        timing_sstart();
        MPI_Send(&sbuf[offset], current_packet, datatype, next_in_pipe, tag, comm);
        current_times[PIPE_SENDRECV_TSEND] += timing_send();
#endif
      }

    } else if (iam_last_in_pipe)
    {
      if (current_packet > 0)
      {
        timing_sstart();
        MPI_Recv(&rbuf[offset], current_packet, datatype, prev_in_pipe, tag, comm, &status);
        current_times[PIPE_SENDRECV_TRECV] += timing_send();

#ifdef RLE
        MPI_Get_count(&status, datatype, &rle_recvcount);
        rle_sendcount = current_packet;

        timing_sstart();
        dblv_rle_zero_cf_uc_add2_ub(rle_recvcount, (double *) &rbuf[offset], current_packet, (double *) &sbuf[offset], &rle_sendcount, &rle_sendbuf);
        current_times[PIPE_SENDRECV_TRED] += timing_send();

        rle_sendcounts += rle_sendcount;
        rle_recvcounts += rle_recvcount;
#else
        timing_sstart();
        reduce_op_2(current_packet, 0, datatype, op, &sbuf[offset], &rbuf[offset]);
        current_times[PIPE_SENDRECV_TRED] += timing_send();
#endif
      }

    } else
    {
      if (current_packet > 0)
      {
        if (done == 0)
        {
          timing_sstart();
          MPI_Recv(buf0, current_packet, datatype, prev_in_pipe, tag, comm, &status);
          current_times[PIPE_SENDRECV_TRECV] += timing_send();

        } else
        {
          timing_sstart();
#ifdef RLE
          MPI_Sendrecv(rle_sendbuf, rle_sendcount, datatype, next_in_pipe, tag, buf0, current_packet, datatype, prev_in_pipe, tag, comm, &status);
#else
          MPI_Sendrecv(buf1, prev_packet, datatype, next_in_pipe, tag, buf0, current_packet, datatype, prev_in_pipe, tag, comm, &status);
#endif
          current_times[PIPE_SENDRECV_TSENDRECV] += timing_send();
        }

#ifdef RLE
        MPI_Get_count(&status, datatype, &rle_recvcount);
        rle_sendcount = current_packet;

        timing_sstart();
        dblv_rle_zero_cf_uc_add2_cb(rle_recvcount, (double *) buf0, current_packet, (double *) &sbuf[offset], &rle_sendcount, &rle_sendbuf);
        current_times[PIPE_SENDRECV_TRED] += timing_send();

        rle_sendcounts += rle_sendcount;
        rle_recvcounts += rle_recvcount;
#else
        timing_sstart();
        reduce_op_2(current_packet, 0, datatype, op, &sbuf[offset], buf0);
        current_times[PIPE_SENDRECV_TRED] += timing_send();
#endif

      } else
      {
        timing_sstart();
#ifdef RLE
        MPI_Send(rle_sendbuf, rle_sendcount, datatype, next_in_pipe, tag, comm);
#else
        MPI_Send(buf1, prev_packet, datatype, next_in_pipe, tag, comm);
#endif
        current_times[PIPE_SENDRECV_TSEND] += timing_send();
      }

      buft = buf1;
      buf1 = buf0;
      buf0 = buft;
    }

    done += current_packet;

    prev_packet = current_packet;
  }

  current_times[PIPE_SENDRECV_TLOOP] = timing_send();

  if (my_pa == &local_pa) pipe_attr_free_buf(&local_pa);

end:

  current_times[PIPE_SENDRECV_TALL] = timing_send();

/*  printf("%d here: %f  %f  %f  %f  %f  %f\n", comm_rank, current_times[PIPE_SENDRECV_TALL],
                                                         current_times[PIPE_SENDRECV_TLOOP],
                                                         current_times[PIPE_SENDRECV_TSEND],
                                                         current_times[PIPE_SENDRECV_TRECV],
                                                         current_times[PIPE_SENDRECV_TSENDRECV],
                                                         current_times[PIPE_SENDRECV_TRED]);
*/

  if (default_pa.logging)
  {
    mainlog_printf("T  %f  %f  %f  %f  %f  %f\n", current_times[PIPE_SENDRECV_TALL],
                                                  current_times[PIPE_SENDRECV_TLOOP],
                                                  current_times[PIPE_SENDRECV_TSEND],
                                                  current_times[PIPE_SENDRECV_TRECV],
                                                  current_times[PIPE_SENDRECV_TSENDRECV],
                                                  current_times[PIPE_SENDRECV_TRED]);

#define BW(t) (count * type_size / t / 1000000.0)

    mainlog_printf("B  %f  %f  %f  %f  %f  %f\n", BW(current_times[PIPE_SENDRECV_TALL]),
                                                  BW(current_times[PIPE_SENDRECV_TLOOP]),
                                                  BW(current_times[PIPE_SENDRECV_TSEND]),
                                                  BW(current_times[PIPE_SENDRECV_TRECV]),
                                                  BW(current_times[PIPE_SENDRECV_TSENDRECV]),
                                                  BW(current_times[PIPE_SENDRECV_TRED]));

#ifdef RLE
    mainlog_printf("B  %d  %f  %d  %f\n", rle_sendcounts, 100.0 * rle_sendcounts / count, rle_recvcounts, 100.0 * rle_recvcounts / count);
#endif
  }

  return MPI_SUCCESS;
}
