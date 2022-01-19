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

/*#define RLE*/
/*#define RLE_PACKET*/
/*#define RLE_PACKET_FIRST_UNCOMPRESSED*/
/*#define RLE_PACKET_THRESHOLD  1.0*/

#ifndef MOD_PIPE_STREAM
 #define MOD_PIPE_STREAM(s) s
#endif


int MOD_PIPE_STREAM(MPI_Reduce_pipe_stream)(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  int comm_rank, comm_size;
  int type_size;

  const int tag = 0;

  int max_packet, sends, recvs;
  int received, receivedc, processed, processedc;

  int iam_first_in_pipe, iam_last_in_pipe;

  const char *sbuf = sendbuf;
  char *rbuf = recvbuf;
  char *pbufs, *pbufr, *pbuf0, *pbuf1, *pbuft;

#ifdef RLE
 #ifndef RLE_PACKET
  double vin0_next = 0.0;
 #endif
#endif

  int sendc = 0, recvc = 0;

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

  pbuf0 = my_pa->buf[0];
  pbuf1 = my_pa->buf[1];

  pbufr = pbuf0;
  pbufs = pbuf1;

  sends = recvs = 0;
  received = receivedc = processed = processedc = 0;

  while (sends < count || recvs < count)
  {
    if (iam_first_in_pipe)
    {
      /* recv */

      /* op */
#ifndef RLE
      received = count - recvs; if (received > max_packet) received = max_packet;
      processed = processedc = received;
      /* prepare send-buffer */
      pbufs = (char *) sbuf;
#else
 #ifdef RLE_PACKET
      received = count - recvs; if (received > max_packet) received = max_packet;
      processed = processedc = received;
  #ifndef RLE_PACKET_FIRST_UNCOMPRESSED
      dblv_rle_zero_compress2(received, (double *) sbuf, &processedc, (double *) pbufs);
  #else
      pbufs = (char *) sbuf;
  #endif
      /* prepare send-buffer */
 #else
      received = count - recvs;
      dblv_rle_zero_compress3(received, (double *) sbuf, max_packet, (double *) pbufs, &processed, &processedc);
/*      processed = processedc = (received > max_packet)?max_packet:received;*/
      /* prepare send-buffer */
 #endif
#endif
      sbuf += processed * type_size;

      recvs += processed;

      /* send */
      MPI_Send(pbufs, processedc, datatype, next_in_pipe, tag, comm); sendc += processedc;
      sends += processed;

    } else if (iam_last_in_pipe)
    {
#ifndef RLE
      pbufr = rbuf;
#else
 #ifdef RLE_PACKET
      pbufr = rbuf;
 #endif
#endif

      /* recv */
      MPI_Recv(pbufr, max_packet, datatype, prev_in_pipe, tag, comm, &status);
      MPI_Get_count(&status, datatype, &receivedc); recvc += receivedc;

      /* op */
#ifndef RLE
      processed = processedc = received = receivedc;
      reduce_op_2(received, 0, datatype, op, sbuf, pbufr);
      /* prepare recv-buffer */
#else
 #ifdef RLE_PACKET
      /* calculate packet size, for backward-decompression! */
      received = count - recvs; if (received > max_packet) received = max_packet;
      processed = processedc = received;
      dblv_rle_zero_cf_uc_add2_ub(receivedc, (double *) pbufr, received, (double *) sbuf, &processedc, NULL);
      /* prepare recv-buffer */
 #else
      dblv_rle_zero_cf_uc_add3_uc(receivedc, (double *) pbufr, count - recvs, (double *) sbuf, count - recvs, (double *) rbuf, &received, &processed, &processedc, &vin0_next);
/*      received = receivedc;
      processed = processedc = received;*/
      receivedc -= received; if (receivedc != 0) printf("ERROR: root hast receivedc != 0 (%d)\n", receivedc);
 #endif
#endif
      sbuf += processed * type_size;
      rbuf += processed * type_size;

      recvs += processed;

      /* send */

      sends += processed;

    } else
    {
      if (receivedc <= 0 && recvs < count)  /* nothing received, but still something left to receive? */
      {
        if (processedc <= 0)  /* nothing to send? */
        {
          /* recv */
          MPI_Recv(pbufr, max_packet, datatype, prev_in_pipe, tag, comm, &status);

        } else  /* something to send! */
        {
          /* send / recv */
          MPI_Sendrecv(pbufs, processedc, datatype, next_in_pipe, tag, pbufr, max_packet, datatype, prev_in_pipe, tag, comm, &status); sendc += processedc;
          sends += processed;
        }

        MPI_Get_count(&status, datatype, &receivedc); recvc += receivedc;

      } else  /* something received or nothing left to receive! */
      {
        if (processedc > 0)  /* something to send? */
        {
          /* send */
          MPI_Send(pbufs, processedc, datatype, next_in_pipe, tag, comm); sendc += processedc;
          sends += processed;
        }
      }

      /* op */
#ifndef RLE
      processed = processedc = received = receivedc;
      reduce_op_2(received, 0, datatype, op, sbuf, pbufr);
      /* prepare send-buffer */
      pbufs = pbufr;
      /* prepare recv-buffer */
      xswap(pbuf0, pbuf1, pbuft);
      pbufr = pbuf0;
      received = receivedc = 0;
#else
 #ifdef RLE_PACKET
/*      printf("%d here: count = %d, recvs = %d\n", comm_rank, count, recvs);*/
      /* calculate packet size, for backward-decompression! */
      received = count - recvs; if (received > max_packet) received = max_packet;
      processed = processedc = received;
#ifndef RLE_PACKET_THRESHOLD
/*      printf("%d here: X  %d\n", comm_rank, receivedc);*/
  #ifdef RLE_PACKET_FIRST_UNCOMPRESSED
      if (second_in_pipe == comm_rank) dblv_rle_zero_uc_uc_add2_cf(receivedc, (double *) pbufr, received, (double *) sbuf, &processedc, (double **) &pbufs);
      else
  #endif
        dblv_rle_zero_cf_uc_add2_cb(receivedc, (double *) pbufr, received, (double *) sbuf, &processedc, (double **) &pbufs);
#else
/*      printf("%d here: %d >= %d (max_packet = %d)\n", comm_rank, receivedc, received, max_packet);*/
      /* uncompressed? */
      if (receivedc >= received)
      {
/*        printf("%d here: Z  %d\n", comm_rank, received);*/
  #ifdef RLE_PACKET_FIRST_UNCOMPRESSED
        if (second_in_pipe == comm_rank) dblv_rle_zero_uc_uc_add2_cf(receivedc, (double *) pbufr, received, (double *) sbuf, &processedc, (double **) &pbufs);
        else
  #endif
          reduce_op_2(received, 0, datatype, op, sbuf, pbufr);

      } else
      {
        /* compression rate to small? */
        if (received * RLE_PACKET_THRESHOLD < receivedc)
        {
/*          printf("%d here: X  %d\n", comm_rank, receivedc);*/
          dblv_rle_zero_cf_uc_add2_ub(receivedc, (double *) pbufr, received, (double *) sbuf, &processedc, (double **) &pbufs);

        } else
        {
/*          printf("%d here: Y\n", comm_rank);*/
          dblv_rle_zero_cf_uc_add2_cb(receivedc, (double *) pbufr, received, (double *) sbuf, &processedc, (double **) &pbufs);
        }
      }
#endif
      /* prepare send-buffer */
      /* prepare recv-buffer */
      xswap(pbuf0, pbuf1, pbuft);
      pbufr = pbuf0;
      received = receivedc = 0;
 #else
      dblv_rle_zero_cf_uc_add3_cf(receivedc, (double *) pbufr, count - recvs, (double *) sbuf, max_packet, (double *) pbufs, &received, &processed, &processedc, &vin0_next);
      receivedc -= received; if (receivedc > 0) pbufr += received * type_size; else pbufr = pbuf0;
 #endif
#endif
      sbuf += processed * type_size;

      recvs += processed;
    }
  }

  if (my_pa == &local_pa) pipe_attr_free_buf(&local_pa);

end:

  sendc_global = sendc;
  recvc_global = recvc;

/*  printf("%d here: %f%% in, %f%% out\n", comm_rank, 100.0 * recvc / count, 100.0 * sendc / count);*/

  return MPI_SUCCESS;
}
