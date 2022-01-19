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

#include <string.h>
#include <stdlib.h>

#ifdef USE_DBLV
 #include "dblv.h"
#endif

#include "memory.h"


void *malloc_aligned(size_t size, size_t alignment, void **free_ptr)
{
  char *ptr;

  ptr = (char *) calloc(size + alignment, 1);

  if (free_ptr) *free_ptr = ptr;

  if (alignment)
  if (((unsigned long) ptr) % alignment) return ptr + alignment - (((unsigned long) ptr) % alignment);

  return ptr;
}


void *memcpy_org(void *dest, const void *src, size_t n)
{
  return memcpy(dest, src, n);
}


void tuned_STREAM_Copy(int n, double *c, double *a)
{
  int j;
  for (j=0; j<n; j++) c[j] = a[j];
}


void *memcpy_dbl(void *dest, const void *src, size_t n)
{
  tuned_STREAM_Copy(n / sizeof(double), (double *) dest, (double *) src);

  return dest;
}


void *memcpy_dblv(void *dest, const void *src, size_t n)
{
  dblv_copy(n / sizeof(double), (double *) src, dest);

  return dest;
}


void *memcpy_opt(void *b, const void *a, size_t n)
{
  char *s1 = b;
  const char *s2 = a;
  for(; 0<n; --n)*s1++ = *s2++;
  return b;
}


void *memcpy_sse2(void* dest, const void* src, size_t n)
{

  asm
  ( "shrl $7, %%ecx\n\t"
    "prefetchnta 128(%%esi)\n\t"
    "prefetchnta 160(%%esi)\n\t"
    "prefetchnta 192(%%esi)\n\t"
    "prefetchnta 224(%%esi)\n\t"

    "loop_copy:\n\t"
    "movdqa   0(%%esi), %%xmm0\n\t"
    "movdqa  16(%%esi), %%xmm1\n\t"
    "movdqa  32(%%esi), %%xmm2\n\t"
    "movdqa  48(%%esi), %%xmm3\n\t"
    "movdqa  64(%%esi), %%xmm4\n\t"
    "movdqa  80(%%esi), %%xmm5\n\t"
    "movdqa  96(%%esi), %%xmm6\n\t"
    "movdqa 112(%%esi), %%xmm7\n\t"

    "movntdq %%xmm0,   0(%%edi)\n\t"
    "movntdq %%xmm1,  16(%%edi)\n\t"
    "movntdq %%xmm2,  32(%%edi)\n\t"
    "movntdq %%xmm3,  48(%%edi)\n\t"
    "movntdq %%xmm4,  64(%%edi)\n\t"
    "movntdq %%xmm5,  80(%%edi)\n\t"
    "movntdq %%xmm6,  96(%%edi)\n\t"
    "movntdq %%xmm7, 112(%%edi)\n\t"

    "add $128, %%esi\n\t"
    "add $128, %%edi\n\t"
    "dec %%ebx\n\t"

    "jnz loop_copy\n\t"
    "loop_copy_end:\n\t"

  :
  : "c" (n), "S" (src), "D" (dest)
  );
}
