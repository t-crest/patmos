/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2014 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/

/**
 * @file sin_cos_q15.c
 *
 * @brief Fixed point trig functions
 */

#include "includes.h"

/*!
 * \addtogroup MC
 *
 * @{
 */

/*!
 * \addtogroup DSPFIXED
 *
 * @{
 */

#define TABLE_LENGTH		256
#define TABLE_SPACING_Q15	0x80	// 1/TABLE_LENGTH

short sinTableQ15[259] TABLE_SECTION ;
short cosTableQ15[259] TABLE_SECTION ;

/**
 *
 */
void init_sin_cos_tables(void) {
	double value;
	int n;
	int table_size = TABLE_LENGTH;
	for(n = -1; n < (table_size + 2); n++) {
		value = sin(2*3.14159265358979*n/table_size);
		value = value * pow(2, 15);
		value += value > 0 ? 0.5 : -0.5;
		value = value >=32768 ? 32767 : value;
		sinTableQ15[n+1] = value;
		value = cos(2*3.14159265358979*n/table_size);
		value = value * pow(2, 15);
		value += value > 0 ? 0.5 : -0.5;
		value = value >=32768 ? 32767 : value;
		cosTableQ15[n+1] = value;
	}
}

/**
 * Fast approximation to the trigonometric sine function for Q15 data.
 *
 * The Q15 input value is in the range [0 +1) and is mapped to a radian value in the range [0 2*pi).
 *
 * @param x	Scaled input value in radians.
 * @return	output value in 1.15(q15) format
 */
short sin_q15(short x)
{
  int sinVal;                                  /* Temporary variables output */
  short *tablePtr;                               /* Pointer to table */
  short fract, in, in2;                          /* Temporary variables for input, output */
  int wa, wb, wc, wd;                          /* Cubic interpolation coefficients */
  short a, b, c, d;                              /* Four nearest output values */
  short fractCube, fractSquare;                  /* Temporary values for fractional value */
  short oneBy6 = 0x1555;                         /* Fixed point value of 1/6 */
  int index;                                 /* Index variable */

  in = x;

  /* Calculate the nearest index */
  index = (int) in / TABLE_SPACING_Q15;

  /* Calculate the nearest value of input */
  in2 = (short) ((index) * TABLE_SPACING_Q15);

  /* Calculation of fractional value */
  fract = (in - in2) << 8;

  /* fractSquare = fract * fract */
  fractSquare = (short) ((fract * fract) >> 15);

  /* fractCube = fract * fract * fract */
  fractCube = (short) ((fractSquare * fract) >> 15);

  /* Initialise table pointer */
  tablePtr = (short *) & sinTableQ15[index];

  /* Cubic interpolation process */
  /* Calculation of wa */
  /* wa = -(oneBy6)*fractCube + (fractSquare >> 1u) - (0x2AAA)*fract; */
  wa = (int) oneBy6 *fractCube;
  wa += (int) 0x2AAA * fract;
  wa = -(wa >> 15);
  wa += ((int) fractSquare >> 1u);

  /* Read first nearest value of output from the sin table */
  a = *tablePtr++;

  /* sinVal = a * wa */
  sinVal = a * wa;

  /* Calculation of wb */
  wb = (((int) fractCube >> 1u) - (int) fractSquare) -
       (((int) fract >> 1u) - 0x7FFF);

  /* Read second nearest value of output from the sin table */
  b = *tablePtr++;

  /*      sinVal += b*wb */
  sinVal += b * wb;

  /* Calculation of wc */
  wc = -(int) fractCube + fractSquare;
  wc = (wc >> 1u) + fract;

  /* Read third nearest value of output from the sin table */
  c = *tablePtr++;

  /*      sinVal += c*wc */
  sinVal += c * wc;

  /* Calculation of wd */
  /* wd = (oneBy6)*fractCube - (oneBy6)*fract; */
  fractCube = fractCube - fract;
  wd = ((short) (((int) oneBy6 * fractCube) >> 15));

  /* Read fourth nearest value of output from the sin table */
  d = *tablePtr++;

  /* sinVal += d*wd; */
  sinVal += d * wd;
 
  /* Return the output value in 1.15(q15) format */
  return ((short) (sinVal >> 15u));

}

/**
 * Fast approximation to the trigonometric cosine function for Q15 data.
 * The Q15 input value is in the range [0 +1) and is mapped to a radian value in the range [0 2*pi). 
 * 
 * @param x	Scaled input value in radians.
 * @return	output value in 1.15(q15) format
 */
short cos_q15(short x)
{ 
  int cosVal;                                  /* Temporary variables output */ 
  short *tablePtr;                               /* Pointer to table */ 
  short fract, in, in2;                          /* Temporary variables for input, output */ 
  int wa, wb, wc, wd;                          /* Cubic interpolation coefficients */ 
  short a, b, c, d;                              /* Four nearest output values */ 
  short fractCube, fractSquare;                  /* Temporary values for fractional value */ 
  short oneBy6 = 0x1555;                         /* Fixed point value of 1/6 */ 
  int index;                                 /* Index variable */
 
  in = x; 
 
  /* Calculate the nearest index */ 
  index = (int) in / TABLE_SPACING_Q15;
 
  /* Calculate the nearest value of input */ 
  in2 = (short) ((index) * TABLE_SPACING_Q15);
 
  /* Calculation of fractional value */ 
  fract = (in - in2) << 8; 
 
  /* fractSquare = fract * fract */ 
  fractSquare = (short) ((fract * fract) >> 15); 
 
  /* fractCube = fract * fract * fract */ 
  fractCube = (short) ((fractSquare * fract) >> 15); 
 
  /* Initialise table pointer */ 
  tablePtr = (short *) & cosTableQ15[index]; 
 
  /* Cubic interpolation process */ 
  /* Calculation of wa */ 
  /* wa = -(oneBy6)*fractCube + (fractSquare >> 1u) - (0x2AAA)*fract; */ 
  wa = (int) oneBy6 *fractCube; 
  wa += (int) 0x2AAA * fract; 
  wa = -(wa >> 15); 
  wa += ((int) fractSquare >> 1u);
 
  /* Read first nearest value of output from the cos table */ 
  a = *tablePtr++; 
 
  /* cosVal = a * wa */ 
  cosVal = a * wa; 
 
  /* Calculation of wb */ 
  wb = (((fractCube >> 1u) - fractSquare) - (fract >> 1u)) + 0x7FFF;
 
  /* Read second nearest value of output from the cos table */ 
  b = *tablePtr++; 
 
  /*      cosVal += b*wb */ 
  cosVal += b * wb; 
 
  /* Calculation of wc */ 
  wc = -(int) fractCube + fractSquare;
  wc = (wc >> 1u) + fract;
 
  /* Read third nearest value of output from the cos table */ 
  c = *tablePtr++;
 
  /*      cosVal += c*wc */ 
  cosVal += c * wc;
 
  /* Calculation of wd */ 
  /* wd = (oneBy6)*fractCube - (oneBy6)*fract; */ 
  fractCube = fractCube - fract;
  wd = ((short) (((int) oneBy6 * fractCube) >> 15));
 
  /* Read fourth nearest value of output from the cos table */ 
  d = *tablePtr++;
 
  /* cosVal += d*wd; */ 
  cosVal += d * wd;
 
  /* Return the output value in 1.15(q15) format */ 
  return ((short) (cosVal >> 15u)); 
}

/*!
 * @}
 */

/*!
 * @}
 */
