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
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef USE_MPI
 #include <mpi.h>
#endif


FILE *mainlog = NULL;


void timestamp(char *logname)
{
  char t[256], ts[256], *p;

  struct tm *pstime;
  time_t ttime;
  long ltime_s, ltime_r;

  ltime_s = time(NULL);

#ifdef USE_MPI
  MPI_Allreduce(&ltime_s, &ltime_r, 1, MPI_LONG, MPI_MIN, MPI_COMM_WORLD);
  ltime_s = ltime_r;
#endif

  ttime = ltime_s;
  pstime = localtime(&ttime);
  sprintf(ts, "%d%.2d%.2d-%.2d%.2d", pstime->tm_year + 1900, pstime->tm_mon, pstime->tm_mday, pstime->tm_hour, pstime->tm_min);

  while ((p = strstr(logname, "%T")))
  {
    *(p + 1) = 's';
    strcpy(t, logname);
    sprintf(logname, t, ts);
  }
}


FILE *log_open(const char *fmtstr, ...)
{
  char logname[256];
  va_list argp;

  va_start(argp, fmtstr);
  vsprintf(logname, fmtstr, argp);
  va_end(argp);

  printf("logname: %s\n", logname);

  return fopen(logname, "r+");
}


void log_close(FILE *log)
{
  if (!log) return;

  fclose(log);
}


void log_printf(FILE *log, const char *fmtstr, ...)
{
  va_list argp;

  if (!log) return;

  va_start(argp, fmtstr);
  vfprintf(log, fmtstr, argp);
  va_end(argp);
}


void mainlog_open(const char *fmtstr, ...)
{
  char logname[256];
  va_list argp;

  va_start(argp, fmtstr);
  vsprintf(logname, fmtstr, argp);
  va_end(argp);

  timestamp(logname);

  mainlog = fopen(logname, "w");
}


void mainlog_close()
{
  log_close(mainlog);
}


void mainlog_printf(const char *fmtstr, ...)
{
  va_list argp;

  if (!mainlog) return;

  va_start(argp, fmtstr);
  vfprintf(mainlog, fmtstr, argp);
  va_end(argp);
}
