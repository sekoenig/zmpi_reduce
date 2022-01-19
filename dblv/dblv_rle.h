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

#ifndef __DBLV_RLE_H__
#define __DBLV_RLE_H__


#include <stdint.h>

typedef uint64_t dblv_int64;

#define DBL_NAN_MASK       0x7FF0000000000000LLU
#define DBL_FRACTION_MASK  0x000FFFFFFFFFFFFFLLU

#define DBL_IS_NAN_P(pv)         ((*((dblv_int64 *) (pv)) & DBL_NAN_MASK) == DBL_NAN_MASK)
#define DBL_ISN_NAN_P(pv)        ((*((dblv_int64 *) (pv)) & DBL_NAN_MASK) != DBL_NAN_MASK)

#define DBL_RLE_SET_P(pv, m)     (*((dblv_int64 *) (pv)) = (((dblv_int64) (m)) | (dblv_int64) ~DBL_FRACTION_MASK))
#define DBL_RLE2_SET_P(pv, m)    (*((dblv_int64 *) (pv)) = ((m)>1)?(((dblv_int64) (m)) | (dblv_int64) ~DBL_FRACTION_MASK):0)
#define DBL_RLE_GET_P(pv)        (*((dblv_int64 *) (pv)) & (dblv_int64) DBL_FRACTION_MASK)


#endif /* __DBLV_RLE_H__ */
