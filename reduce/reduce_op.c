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
#include <pthread.h>
#include <mpi.h>

#ifdef USE_NUMA
 #include <numa.h>
#endif

#include "reduce_op.h"

#define REDUCE_LOOP_2(i, n, in, out, op)        for (i = 0; i < n; i++) *(out++) op *(in++)
#define REDUCE_LOOP_3(i, n, in0, in1, out, op)  for (i = 0; i < n; i++) *(out++) = *(in0++) op *(in1++)


void reduce_op_2(int count, int offset, MPI_Datatype datatype, MPI_Op op, const void *in, void *out)
{
  int i;

  const double *dbl_in;
  double *dbl_out, t;

  t = MPI_Wtime();
  if (datatype == MPI_DOUBLE)
  {
    dbl_in = in;
    dbl_out = out;

    dbl_in += offset;
    dbl_out += offset;

    if (op == MPI_SUM)
    {
      REDUCE_LOOP_2(i, count, dbl_in, dbl_out, +=);
    }
  }
  reduce_times[1] += MPI_Wtime() - t;
}


void reduce_op_3(int count, int offset, MPI_Datatype datatype, MPI_Op op, const void *in0, const void *in1, void *out)
{
  int i;

  const double *dbl_in0, *dbl_in1;
  double *dbl_out, t;

  t = MPI_Wtime();
  if (datatype == MPI_DOUBLE)
  {
    dbl_in0 = in0;
    dbl_in1 = in1;
    dbl_out = out;

    dbl_in0 += offset;
    dbl_in1 += offset;
    dbl_out += offset;

    if (op == MPI_SUM)
    {
      REDUCE_LOOP_3(i, count, dbl_in0, dbl_in1, dbl_out, +);
    }
  }
  reduce_times[1] += MPI_Wtime() - t;
}


#define bitmap_set(bm, n)    ((bm) |= (1 << (n)))
#define bitmap_unset(bm, n)  ((bm) &= ~(1 << (n)))
#define bitmap_get(bm, n)    (((bm) >> (n)) & 1)
#define bitmap_setall(bm)    ((bm) = 0xFF)
#define bitmap_unsetall(bm)  ((bm) = 0)

double rt[8], rtimes[8];

void *reduce_task(void *arg)
{
  reduce_task_info *rti = (reduce_task_info *) arg;

#ifdef USE_NUMA
/*  numa_run_on_node((rti->tidx + 1) / 2);*/
  numa_run_on_node(rti->tidx);
#endif

  while (1)
  {
    pthread_mutex_lock(&rti->tri->mutex);
    while (!bitmap_get(rti->tri->run, rti->tidx))
    {
      pthread_cond_wait(&rti->tri->cond, &rti->tri->mutex);
    }
    pthread_mutex_unlock(&rti->tri->mutex);

    if (rti->tri->exit) break;

    rt[rti->tidx] = MPI_Wtime();
    reduce_op_2(rti->count, rti->offset, rti->datatype, rti->op, rti->in, rti->out);
    rtimes[rti->tidx] += MPI_Wtime() - rt[rti->tidx];

/*    pthread_mutex_lock(&rti->tri->mutex_done);
    bitmap_unset(rti->tri->run, rti->tidx);
    rti->tri->done++;
    pthread_mutex_unlock(&rti->tri->mutex_done);

    pthread_cond_signal(&rti->tri->cond_done);*/

    pthread_mutex_lock(&rti->tri->mutex);
    bitmap_unset(rti->tri->run, rti->tidx);
    if (++rti->tri->done >= rti->tri->nreduce_tasks) pthread_cond_signal(&rti->tri->cond_done);
/*    rti->tri->done++;*/
    pthread_mutex_unlock(&rti->tri->mutex);
  }

  return NULL;
}

void threaded_reduce_init(threaded_reduce_info *tri, int nthreads)
{
  int i;

/*  printf("threaded_reduce_init\n");*/

/*  numa_run_on_node(0);*/

  pthread_mutex_init(&tri->mutex, NULL);
  pthread_mutex_init(&tri->mutex_done, NULL);
  pthread_cond_init(&tri->cond, NULL);
  pthread_cond_init(&tri->cond_done, NULL);

  tri->exit = 0;
  tri->done = 1;
  bitmap_unsetall(tri->run);

  tri->nreduce_tasks = nthreads;
  tri->rtis = (reduce_task_info *) malloc(nthreads * sizeof(reduce_task_info));

  for (i = 0; i < nthreads - 1; i++)
  {
    tri->rtis[i].tri = tri;
    tri->rtis[i].tidx = i;
    pthread_create(&tri->rtis[i].tid, NULL, reduce_task, (void *) &tri->rtis[i]);
  }
  tri->rtis[i].tri = tri;
  tri->rtis[i].tidx = nthreads - 1;

  rtimes[0] = rtimes[1] = rtimes[2] = rtimes[3] = 0.0;

/*  printf("threaded_reduce_init done\n");*/
}

void threaded_reduce_destroy(threaded_reduce_info *tri)
{
  int i;

/*  printf("threaded_reduce_destroy\n");*/

  double t = MPI_Wtime();

  pthread_mutex_lock(&tri->mutex);
  tri->exit = 1;
  bitmap_setall(tri->run);
  pthread_cond_broadcast(&tri->cond);
  pthread_mutex_unlock(&tri->mutex);

  for (i = 0; i < tri->nreduce_tasks - 1; i++) pthread_join(tri->rtis[i].tid, NULL);

  free(tri->rtis);

  pthread_mutex_destroy(&tri->mutex);
  pthread_mutex_destroy(&tri->mutex_done);
  pthread_cond_destroy(&tri->cond);
  pthread_cond_destroy(&tri->cond_done);

  printf("threaded_reduce_destroy = %f\n", MPI_Wtime() - t);

  printf("threaded_reduce times: %f  %f  %f  %f  %f  %f  %f  %f\n", rtimes[0], rtimes[1], rtimes[2], rtimes[3], rtimes[4], rtimes[5], rtimes[6], rtimes[7]);
}

void threaded_reduce_op_2(threaded_reduce_info *tri, int count, MPI_Datatype datatype, MPI_Op op, void *in, void *out)
{
  int i;
  reduce_task_info *rti_last = &tri->rtis[tri->nreduce_tasks - 1];

/*  printf("threaded_reduce_op_2\n");*/

  for (i = 0; i < tri->nreduce_tasks; i++)
  {
    tri->rtis[i].offset = (i * count) / tri->nreduce_tasks;
    tri->rtis[i].count = ((i + 1) * count) / tri->nreduce_tasks;
    tri->rtis[i].count -= tri->rtis[i].offset;

    tri->rtis[i].datatype = datatype;
    tri->rtis[i].op = op;
    tri->rtis[i].in = in;
    tri->rtis[i].out = out;
  }

  pthread_mutex_lock(&tri->mutex);
  tri->done = 1;
  tri->exit = 0;
  bitmap_setall(tri->run);
  pthread_mutex_unlock(&tri->mutex);

  pthread_cond_broadcast(&tri->cond);

  rt[rti_last->tidx] = MPI_Wtime();
  reduce_op_2(rti_last->count, rti_last->offset, rti_last->datatype, rti_last->op, rti_last->in, rti_last->out);
  rtimes[rti_last->tidx] += MPI_Wtime() - rt[rti_last->tidx];

/*  printf("thread %d done at %f\n", rti_last->tidx, MPI_Wtime());*/

  double t = MPI_Wtime();
  pthread_mutex_lock(&tri->mutex);
  while (tri->done < tri->nreduce_tasks) pthread_cond_wait(&tri->cond_done, &tri->mutex);
  pthread_mutex_unlock(&tri->mutex);
  t = MPI_Wtime() - t;

  printf("threaded_reduce_op_2 %f\n", t);
}


#define REDUCE_THREADS  4

typedef struct _double_reduce_task_info
{
  double *in, *out;
  int count;

} double_reduce_task_info;

reduce_task_info reduce_task_infos[REDUCE_THREADS];

double reduce_times[3];

void *static_reduce_task(void *arg)
{
  reduce_task_info *rti = (reduce_task_info *) arg;

  int i;

  const double *dbl_in = rti->in;
  double *dbl_out = rti->out;

  REDUCE_LOOP_2(i, rti->count, dbl_in, dbl_out, +=);

  return 0;
}


void static_threaded_reduce_op_2(int count, int offset, MPI_Datatype datatype, MPI_Op op, const void *in, void *out)
{
  int i, n;

  const double *dbl_in = in;
  double *dbl_out = out;

  double t;

  pthread_t ths[REDUCE_THREADS - 1];

  n = count / REDUCE_THREADS;

  t = MPI_Wtime();
  for (i = 0; i < REDUCE_THREADS - 1; i++)
  {
    reduce_task_infos[i].in = dbl_in + (i * n);
    reduce_task_infos[i].out = dbl_out + (i * n);
    reduce_task_infos[i].count = n;

    pthread_create(&ths[i], NULL, static_reduce_task, (void *) &reduce_task_infos[i]);
  }
  reduce_times[0] += MPI_Wtime() - t;

  dbl_in += (REDUCE_THREADS - 1) * n;
  dbl_out += (REDUCE_THREADS - 1) * n;

  t = MPI_Wtime();
  REDUCE_LOOP_2(i, count - ((REDUCE_THREADS - 1) * n), dbl_in, dbl_out, +=);
  reduce_times[1] += MPI_Wtime() - t;

  t = MPI_Wtime();
  for (i = 0; i < REDUCE_THREADS - 1; i++) pthread_join(ths[i], NULL);
  reduce_times[2] += MPI_Wtime() - t;
}
