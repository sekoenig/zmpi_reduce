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

#ifndef __TRACE_H__
#define __TRACE_H__


#ifdef TRACING
 #define TRACE_INIT(pid, tf)  trace_init(pid, tf)
 #define TRACE_CLOSE()        trace_close()
 #define TRACE_POINT(id)      trace_point(local_tracefile, id)
#else
 #define TRACE_INIT(pid, tf)
 #define TRACE_CLOSE()
 #define TRACE_POINT(id)
#endif


extern FILE *local_tracefile;
extern double local_tbase;

void trace_init(int pid, const char *tfilename);
void trace_close();
void trace_point(FILE *tfile, int id);


#endif /* __TRACE_H__ */
