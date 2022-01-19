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

#ifndef __REDUCE_OP_H__
#define __REDUCE_OP_H__


struct _reduce_task_info;

typedef struct _threaded_reduce_info
{
  pthread_mutex_t mutex, mutex_done;
  pthread_cond_t cond, cond_done;
  int exit, done;

  int run;

  int nreduce_tasks;
  struct _reduce_task_info *rtis;

} threaded_reduce_info;


typedef struct _reduce_task_info
{
  threaded_reduce_info *tri;

  int tidx;
  pthread_t tid;

  int count, offset;
  MPI_Datatype datatype;
  MPI_Op op;
  const void *in;
  void *out;

} reduce_task_info;


void reduce_op_2(int count, int offset, MPI_Datatype datatype, MPI_Op op, const void *in, void *out);
void reduce_op_3(int count, int offset, MPI_Datatype datatype, MPI_Op op, const void *in0, const void *in1, void *out);

void threaded_reduce_init(threaded_reduce_info *tri, int nthreads);
void threaded_reduce_destroy(threaded_reduce_info *tri);
void threaded_reduce_op_2(threaded_reduce_info *tri, int count, MPI_Datatype datatype, MPI_Op op, void *in, void *out);

extern double reduce_times[3];

void static_threaded_reduce_op_2(int count, int offset, MPI_Datatype datatype, MPI_Op op, const void *in, void *out);


#endif /* __REDUCE_OP_H__ */
