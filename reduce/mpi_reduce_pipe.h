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

#ifndef __MPI_REDUCE_PIPE_H__
#define __MPI_REDUCE_PIPE_H__


#define xswap(x0, x1, t)  do { t = x0; x0 = x1; x1 = t; } while (0)

#define last_in_pipe   (root)
#define first_in_pipe  ((root - 1 + comm_size) % comm_size)
#define second_in_pipe ((root - 2 + comm_size) % comm_size)
#define next_in_pipe   ((last_in_pipe == comm_rank)?-1:((comm_rank - 1 + comm_size) % comm_size))
#define prev_in_pipe   ((first_in_pipe == comm_rank)?-1:((comm_rank + 1) % comm_size))


#define PIPE_ATTR_NBUFS  3


typedef struct _pipe_attr
{
  int packet_size;

  int buf_size;
  void *buf[PIPE_ATTR_NBUFS], *buf_free[PIPE_ATTR_NBUFS];

  int logging;

} pipe_attr;


extern pipe_attr default_pa;


void pipe_attr_alloc_buf(pipe_attr *pa, int buf_size, int nbufs);
void pipe_attr_free_buf(pipe_attr *pa);

int MPI_Reduce_pipe_send_recv(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_Reduce_pipe_sendrecv(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_Reduce_pipe_sendrecv_rle(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_Reduce_pipe_isend_irecv(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);

int MPI_Reduce_pipe_stream(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_Reduce_pipe_stream_rle(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_Reduce_pipe_stream_plain(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);


extern int sendc_global, recvc_global;

#endif /* __MPI_REDUCE_PIPE_H__ */
