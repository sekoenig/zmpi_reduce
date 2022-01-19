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

#include "dblv.h"


#ifdef USE_MPI

#include <mpi.h>


void dblv_mpi_send(int nin, double *vin, int dst, int tag, int rank, MPI_Comm comm)
{
  DBLV_TSTART();
  MPI_Send(vin, nin, MPI_DOUBLE, dst, tag, comm);
  DBLV_TEND();

  DBLV_PRINTF("dblv_mpi_send", nin, ", %d -> %d", rank, dst);
}


void dblv_mpi_recv(int nout, double *vout, int src, int tag, int rank, MPI_Comm comm, int *count)
{
  MPI_Status status;

  int count_; if (!count) count = &count_;

  DBLV_TSTART();
  MPI_Recv(vout, nout, MPI_DOUBLE, src, tag, comm, &status);
  DBLV_TEND();

  MPI_Get_count(&status, MPI_DOUBLE, count);

  DBLV_PRINTF("dblv_mpi_recv", *count, ", %d <- %d, count = %d", rank, src, *count);
}


#endif /* USE_MPI */
