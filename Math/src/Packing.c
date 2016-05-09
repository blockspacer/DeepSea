/*
 * Copyright 2016 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Half float implementation borrowed from GLM:
 * OpenGL Mathematics (glm.g-truc.net)
 *
 * Copyright (c) 2005 - 2015 G-Truc Creation (www.g-truc.net)
 *
 * This half implementation is based on OpenEXR which is Copyright (c) 2002,
 * Industrial Light & Magic, a division of Lucas Digital Ltd. LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Restrictions:
 *		By making use of the Software for military purposes, you choose to make
 *		a Bunny unhappy.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <DeepSea/Math/Packing.h>

typedef union FloatInt
{
	float f;
	uint32_t i;
} FloatInt;

static float overflow()
{
	float f = 1e10f;

	for(int i = 0; i < 10; ++i)
		f *= f; // this will overflow before the for loop terminates
	return f;
}

dsHalfFloat dsPackHalfFloat(float x)
{
	dsHalfFloat halfFloat;
	FloatInt entry;
	entry.f = x;
	int i = (int)entry.i;

	//
	// Our floating point number, f, is represented by the bit
	// pattern in integer i.  Disassemble that bit pattern into
	// the sign, s, the exponent, e, and the significand, m.
	// Shift s into the position where it will go in in the
	// resulting half number.
	// Adjust e, accounting for the different exponent bias
	// of float and half (127 versus 15).
	//

	int s =  (i >> 16) & 0x00008000;
	int e = ((i >> 23) & 0x000000ff) - (127 - 15);
	int m =   i        & 0x007fffff;

	//
	// Now reassemble s, e and m into a half:
	//

	if(e <= 0)
	{
		if(e < -10)
		{
			//
			// E is less than -10.  The absolute value of f is
			// less than half_MIN (f may be a small normalized
			// float, a denormalized float or a zero).
			//
			// We convert f to a half zero.
			//

			halfFloat.data = s;
			return halfFloat;
		}

		//
		// E is between -10 and 0.  F is a normalized float,
		// whose magnitude is less than __half_NRM_MIN.
		//
		// We convert f to a denormalized half.
		//

		m = (m | 0x00800000) >> (1 - e);

		//
		// Round to nearest, round "0.5" up.
		//
		// Rounding may cause the significand to overflow and make
		// our number normalized.  Because of the way a half's bits
		// are laid out, we don't have to treat this case separately;
		// the code below will handle it correctly.
		//

		if(m & 0x00001000)
			m += 0x00002000;

		//
		// Assemble the half from s, e (zero) and m.
		//

		halfFloat.data = s | (m >> 13);
		return halfFloat;
	}
	else if(e == 0xff - (127 - 15))
	{
		if(m == 0)
		{
			//
			// F is an infinity; convert f to a half
			// infinity with the same sign as f.
			//

			halfFloat.data = s | 0x7c00;
			return halfFloat;
		}
		else
		{
			//
			// F is a NAN; we produce a half NAN that preserves
			// the sign bit and the 10 leftmost bits of the
			// significand of f, with one exception: If the 10
			// leftmost bits are all zero, the NAN would turn
			// into an infinity, so we have to set at least one
			// bit in the significand.
			//

			m >>= 13;

			halfFloat.data = s | 0x7c00 | m | (m == 0);
			return halfFloat;
		}
	}
	else
	{
		//
		// E is greater than zero.  F is a normalized float.
		// We try to convert f to a normalized half.
		//

		//
		// Round to nearest, round "0.5" up
		//

		if(m &  0x00001000)
		{
			m += 0x00002000;

			if(m & 0x00800000)
			{
				m =  0;     // overflow in significand,
				e += 1;     // adjust exponent
			}
		}

		//
		// Handle exponent overflow
		//

		if (e > 30)
		{
			overflow();        // Cause a hardware floating point overflow;

			halfFloat.data = s | 0x7c00;
			return halfFloat;
			// if this returns, the half becomes an
		}   // infinity with the same sign as f.

		//
		// Assemble the half from s, e and m.
		//

		halfFloat.data = s | (e << 10) | (m >> 13);
		return halfFloat;
	}
}

float dsUnpackHalfFloat(dsHalfFloat x)
{
	int s = (x.data >> 15) & 0x00000001;
	int e = (x.data >> 10) & 0x0000001f;
	int m =  x.data        & 0x000003ff;

	if(e == 0)
	{
		if(m == 0)
		{
			//
			// Plus or minus zero
			//

			FloatInt result;
			result.i = (uint32_t)(s << 31);
			return result.f;
		}
		else
		{
			//
			// Denormalized number -- renormalize it
			//

			while(!(m & 0x00000400))
			{
				m <<= 1;
				e -=  1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}
	else if(e == 31)
	{
		if(m == 0)
		{
			//
			// Positive or negative infinity
			//

			FloatInt result;
			result.i = (uint32_t)((s << 31) | 0x7f800000);
			return result.f;
		}
		else
		{
			//
			// Nan -- preserve sign and significand bits
			//

			FloatInt result;
			result.i = (uint32_t)((s << 31) | 0x7f800000 | (m << 13));
			return result.f;
		}
	}

	//
	// Normalized number
	//

	e = e + (127 - 15);
	m = m << 13;

	//
	// Assemble s, e and m.
	//

	FloatInt Result;
	Result.i = (uint32_t)((s << 31) | (e << 23) | m);
	return Result.f;
}