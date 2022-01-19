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

#ifdef USE_DBLV
 #include "dblv.h"
#endif


int MPI_Reduce_check(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  if (datatype != MPI_DOUBLE || op != MPI_SUM)
  {
    return 1;
  }

  int comm_size;
  MPI_Comm_size(comm, &comm_size);

  if (root >= comm_size)
  {
    return 1;
  }

  return MPI_SUCCESS;
}


int MPI_Reduce_self(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  int comm_size;

  if (comm_size > 1)
  {
    return 1;
  }

  dblv_copy(count, sendbuf, recvbuf);

  return MPI_SUCCESS;
}
