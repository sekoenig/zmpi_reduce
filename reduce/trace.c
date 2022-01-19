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
#include <mpi.h>


FILE *local_tracefile = NULL;
double local_tbase = 0.0;


void trace_init(int pid, const char *tfilename)
{
  char s[256];

  if (!tfilename) tfilename = "tracefile";

  sprintf(s, "%s_p%.3d", tfilename, pid);

  local_tracefile = fopen(s, "w");

  local_tbase = MPI_Wtime();
}


void trace_close()
{
  fclose(local_tracefile);

  local_tracefile = NULL;
}


void trace_point(FILE *tfile, int id)
{
  fprintf(tfile, "%.10f  %d\n", MPI_Wtime() - local_tbase, id);
}
