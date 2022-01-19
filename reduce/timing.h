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

#ifndef __TIMING_H__
#define __TIMING_H__


#ifdef TIMING

 #define ntimer              8
 #define TIMING_INITIALIZE   double tt[ntimer]; \
                             int ti, tts = 0;
 #define timing_zero(n, t)   for (ti = 0; ti < n; ti++) t[ti] = 0.0
 #define timing_start(t)     (tt[t] = MPI_Wtime())
 #define timing_end(t)       (tt[t] = MPI_Wtime() - tt[t])
 #define timing_end_s(t, s)  (s += MPI_Wtime() - tt[t])

 #define timing_sstart()     (tt[tts++] = MPI_Wtime())
 #define timing_send()       (tts--, tt[tts] = MPI_Wtime() - tt[tts])
 #define timing_send_s(s)    (tts--, s += MPI_Wtime() - tt[tts])
 #define timing_slast()      tt[tts]

extern double tt[ntimer];
extern int ti, tts;

#else

 #define TIMING_INITIALIZE

 #define timing_zero(n, t)
 #define timing_start(t)     (0.0)
 #define timing_end(t)       (0.0)
 #define timing_end_s(t, s)  (0.0)

 #define timing_sstart()     (0.0)
 #define timing_send()       (0.0)
 #define timing_send_s(s)    (0.0)
 #define timing_slast()      (0.0)

#endif


#endif /* __TIMING_H__ */
