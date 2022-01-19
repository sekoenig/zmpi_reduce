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
#include "memory.h"
#include "reduce_op.h"

#include "mpi_reduce_pipe.h"


pipe_attr default_pa;


void pipe_attr_alloc_buf(pipe_attr *pa, int buf_size, int nbufs)
{
  int i;

  if (!pa) pa = &default_pa;

  pa->buf_size = buf_size;

  for (i = 0; i < PIPE_ATTR_NBUFS; i++) pa->buf[i] = pa->buf_free[i] = NULL;

  for (i = 0; i < nbufs; i++)
  {
    if (pa->buf_free[i]) free(pa->buf_free[i]);

#ifdef BUFFER_ALIGNMENT
    pa->buf[i] = malloc_aligned(buf_size, BUFFER_ALIGNMENT, &pa->buf_free[i]);
#else
    pa->buf[i] = pa->buf_free[i] = malloc(buf_size);
#endif
  }
}


void pipe_attr_free_buf(pipe_attr *pa)
{
  int i;

  if (!pa) pa = &default_pa;

  pa->buf_size = 0;
  for (i = 0; i < PIPE_ATTR_NBUFS; i++)
  {
    if (pa->buf_free[i]) free(pa->buf_free[i]);
    pa->buf[i] = pa->buf_free[i] = NULL;
  }
}
