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

#ifndef __MPI_REDUCE_RABENSEIFNER_H__
#define __MPI_REDUCE_RABENSEIFNER_H__


#define MPI_Reduce_rabenseifner  MPI_MYreduce

int MPI_MYreduce(const void* Sendbuf, void* Recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);


#endif /* __MPI_REDUCE_RABENSEIFNER_H__ */
