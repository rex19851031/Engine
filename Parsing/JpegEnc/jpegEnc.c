/*
	jpegEnc.c: jpegEnc Library Code
	Author: Hao Hu [ihaohu@gmail.com]
*/

#include "stdio.h"
#include "jpegEnc.h"
#include "jpegEncPrivate.h"

#define SCALE_DCT_ON_QUANTIZATION		/* Combine 2 shiftting steps into 1 for performance */

/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* JPEG Header [START]                                                            */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

#define COPY_SIZE_DATA(_SRC_, _SIZE_)	\
do {									\
	memcpy(pOut, _SRC_, _SIZE_);		\
	pOut += _SIZE_;						\
} while(0)

#define COPY_DATA(_SRC_ARRAY_)			COPY_SIZE_DATA((_SRC_ARRAY_), sizeof((_SRC_ARRAY_)))

static int jpegEncFillHead(UINT8 *buf, int width, int height, BOOL isColor, BOOL doSubSample, UINT8 *tableQTLumin, UINT8 *tableQTChrom) {
	UINT8 *pOut = buf;

	/***************/
	/* SOI Section */
	/***************/
	COPY_DATA(section_SOI);

	/****************/
	/* APP0 Section */
	/****************/
	COPY_DATA(section_APP0);

	/***************/
	/* DQT Section */
	/***************/
	section_DQT[4] = 0;		/* Quantization Table 0 */
	COPY_DATA(section_DQT);
	COPY_SIZE_DATA(tableQTLumin, DCT_BLOCK_PIXS);

	if(isColor) {
		section_DQT[4] = 1;		/* Quantization Table 1 */
		COPY_DATA(section_DQT);
		COPY_SIZE_DATA(tableQTChrom, DCT_BLOCK_PIXS);
	}

	/****************/
	/* SOF0 Section */
	/****************/
	section_SOF0[3] = isColor? 17 : 11;		/* Color has 2 more section_SOF0_comp */
	section_SOF0[5] = (height >> 8) & 0xFF;
	section_SOF0[6] = height & 0xFF;
	section_SOF0[7] = (width >> 8) & 0xFF;
	section_SOF0[8] = width & 0xFF;
	section_SOF0[9] = isColor? 3 : 1;		/* How many components */
	COPY_DATA(section_SOF0);

	section_SOF0_comp[0] = 1;
	section_SOF0_comp[1] = (doSubSample)? 0x22 : 0x11;
	section_SOF0_comp[2] = 0;
	COPY_DATA(section_SOF0_comp);

	if(isColor) {
		section_SOF0_comp[1] = 0x11;
		section_SOF0_comp[2] = 1;

		section_SOF0_comp[0] = 2;
		COPY_DATA(section_SOF0_comp);

		section_SOF0_comp[0] = 3;
		COPY_DATA(section_SOF0_comp);
	}

	/***************/
	/* DHT Section */
	/***************/
	/* A DC HT */
	section_DHT[3] = 31;
	section_DHT[4] = 0x00;		/* First DC Table */
	COPY_DATA(section_DHT);
	COPY_DATA(section_DHT_codeLength_DC_Y);
	COPY_DATA(section_DHT_codeValue_DC_Y);

	/* A AC HT */
	section_DHT[3] = 181;
	section_DHT[4] = 0x10;		/* First AC Table */
	COPY_DATA(section_DHT);
	COPY_DATA(section_DHT_codeLength_AC_Y);
	COPY_DATA(section_DHT_codeValue_AC_Y);

	if(isColor) {
		/* A DC HT */
		section_DHT[3] = 31;
		section_DHT[4] = 0x01;		/* Second DC Table */
		COPY_DATA(section_DHT);
		COPY_DATA(section_DHT_codeLength_DC_C);
		COPY_DATA(section_DHT_codeValue_DC_C);

		/* A AC HT */
		section_DHT[3] = 181;
		section_DHT[4] = 0x11;		/* Second AC Table */
		COPY_DATA(section_DHT);
		COPY_DATA(section_DHT_codeLength_AC_C);
		COPY_DATA(section_DHT_codeValue_AC_C);
	}

	/***************/
	/* SOS Section */
	/***************/
	section_SOS_head[3] = isColor? 12 : 8;
	section_SOS_head[4] = isColor? 3 : 1;
	COPY_DATA(section_SOS_head);

	section_SOS_comp[0] = 1;		/* Y */
	section_SOS_comp[1] = 0x00;		/* First DC & AC Table */
	COPY_DATA(section_SOS_comp);

	if(isColor) {
		section_SOS_comp[1] = 0x11;		/* Second DC & AC Table */

		section_SOS_comp[0] = 2;		/* Cb */
		COPY_DATA(section_SOS_comp);

		section_SOS_comp[0] = 3;		/* Cr */
		COPY_DATA(section_SOS_comp);
	}

	COPY_DATA(section_SOS_tail);

	return (int)(pOut - buf);
}

static int jpegEncFillTail(UINT8 *buf) {
	UINT8 *pOut = buf;
	COPY_DATA(section_EOI);
	return (int)(pOut - buf);
}

#undef COPY_DATA

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* JPEG Header [END]                                                              */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */


/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* Color Conversion (with Downsampling of Cb/Cr and prepare for DCT) [START]      */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*
	Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
	Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  + 128
	Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  + 128
	Then all minus 128 for DCT Purpose.
*/

/*
	Input: Scan Lines (RGB or Grayscale)
	It will fill the internal workBand buffer as following:
	RGB: 
		 doSubSample: {Y(0,0)[8x8] Y(0,1)[8x8] Y(1,0)[8x8] Y(1,1)[8x8] Cb[8x8] Cr[8x8]}...
		!doSubSample: {Y(0,0)[8x8] Cb[8x8] Cr[8x8]}...
	Grayscale:
		{Y[8x8]}...
*/

#define RGB_R(_P_)		(*(_P_))
#define RGB_G(_P_)		(*(_P_ + 1))
#define RGB_B(_P_)		(*(_P_ + 2))

#define ODD_PIX(_IDX_Y_, _IDX_C_)																							\
	out[subIndexY_ + _IDX_Y_] = (signed char)((YY_R[RGB_R(pPixel)] + YY_G[RGB_G(pPixel)] + YY_B[RGB_B(pPixel)]) >> 16);		\
	if(colorLine) {																											\
		out[subIndexCb + _IDX_C_] = (signed char)((CB_R[RGB_R(pPixel)] + CB_G[RGB_G(pPixel)] + CB_B[RGB_B(pPixel)]) >> 16);	\
		out[subIndexCr + _IDX_C_] = (signed char)((CR_R[RGB_R(pPixel)] + CR_G[RGB_G(pPixel)] + CR_B[RGB_B(pPixel)]) >> 16);	\
	}																														\
	pPixel += 3;

#define EVE_PIX(_IDX_)																										\
	out[subIndexY_ + _IDX_] = (signed char)((YY_R[RGB_R(pPixel)] + YY_G[RGB_G(pPixel)] + YY_B[RGB_B(pPixel)]) >> 16);		\
	pPixel += 3;

/* When band = NULL, we'll do padding for the last couple lines */
static void jpegEncRGBLine2YCCBlock(PJPEGENC pJpegEnc, UINT8 *band, int width, int height) {
	int i, j, k;
	int indexYY, indexCB, indexCR, indexLastYY, indexLastCB, indexLastCR;
	UINT8 *pPixel = band;
	_INT32 *out = pJpegEnc->workBand;
	int linePadding = pJpegEnc->widthAligned - width;
	int endPixels = pJpegEnc->encBlockSize - linePadding;
	int subBlockElements = DCT_BLOCK_PIXS;

	if(pJpegEnc->isColor && pJpegEnc->doSubSample) {
		BOOL colorLine;
		int lineOffSetY, lineOffSetC;
		int subIndexYY, subIndexCb, subIndexCr, subIndexY_;			/* Y Cb Cr */
		int lastSubIndexYY, lastSubIndexCb, lastSubIndexCr;
		int subBlockSize = pJpegEnc->bandLines / 2;					/* Should be 8 */

		if(NULL != band) {
			for(j=pJpegEnc->bandCurLines; j<(height + pJpegEnc->bandCurLines); j++) {
				lineOffSetY = (j % subBlockSize) * subBlockSize;
				lineOffSetC = (j / 2) * subBlockSize;
				subIndexYY  = (j / subBlockSize) * (2 * subBlockElements) + lineOffSetY;
				subIndexCb  = (4 * subBlockElements) + lineOffSetC;
				subIndexCr  = (5 * subBlockElements) + lineOffSetC;
				colorLine   = (0 == (j % 2));

				for(i=0; i<width/pJpegEnc->encBlockSize; i++) {

					subIndexY_   = subIndexYY;
					ODD_PIX(0, 0)
					EVE_PIX(1)
					ODD_PIX(2, 1)
					EVE_PIX(3)
					ODD_PIX(4, 2)
					EVE_PIX(5)
					ODD_PIX(6, 3)
					EVE_PIX(7)

					subIndexY_  += subBlockElements;

					ODD_PIX(0, 4)
					EVE_PIX(1)
					ODD_PIX(2, 5)
					EVE_PIX(3)
					ODD_PIX(4, 6)
					EVE_PIX(5)
					ODD_PIX(6, 7)
					EVE_PIX(7)

					subIndexYY += pJpegEnc->encBlockElements;
					subIndexCb += pJpegEnc->encBlockElements;
					subIndexCr += pJpegEnc->encBlockElements;
				}

				if(linePadding > 0) {
					int lastYY = 0, lastCB = 0, lastCR = 0;
					/* subIndexYY/subIndexCb/subIndexCr are set correctly already  */
					for(k=0; k<pJpegEnc->encBlockSize; k++) {
						int kOffSet = (k % subBlockSize) + ((k < subBlockSize)? 0 : subBlockElements);
						BOOL colorPix = (colorLine && (0 == k % 2));

						/* duplicate the last pixel for the the padding pixel */
						if(k < endPixels) {
							lastYY = (signed char)((YY_R[RGB_R(pPixel)] + YY_G[RGB_G(pPixel)] + YY_B[RGB_B(pPixel)]) >> 16);
							if( colorPix ) {
								lastCB = (signed char)((CB_R[RGB_R(pPixel)] + CB_G[RGB_G(pPixel)] + CB_B[RGB_B(pPixel)]) >> 16);
								lastCR = (signed char)((CR_R[RGB_R(pPixel)] + CR_G[RGB_G(pPixel)] + CR_B[RGB_B(pPixel)]) >> 16);
							}
							pPixel += 3;
						}

						out[subIndexYY + kOffSet] = lastYY;
						if( colorPix ) {
							int bOffSet = k / 2;
							out[subIndexCb + bOffSet] = lastCB;
							out[subIndexCr + bOffSet] = lastCR;
						}
					}

				}
			}
		} else {
			for(j=pJpegEnc->bandCurLines; j<pJpegEnc->bandLines; j++) {
				lineOffSetY = (j % subBlockSize) * subBlockSize;
				lineOffSetC = (j / 2) * subBlockSize;
				subIndexYY  = (j / subBlockSize) * (2 * subBlockElements) + lineOffSetY;
				subIndexCb  = (4 * subBlockElements) + lineOffSetC;
				subIndexCr  = (5 * subBlockElements) + lineOffSetC;
				colorLine   = (0 == (j % 2));

				for(i=0; i<pJpegEnc->horBlocks; i++) {
					lastSubIndexYY = subIndexYY - subBlockSize;
					lastSubIndexCb = subIndexCb - subBlockSize;
					lastSubIndexCr = subIndexCr - subBlockSize;

					for(k=0; k<subBlockSize; k++) {
						out[subIndexYY + k] = out[lastSubIndexYY + k];
						out[subIndexYY + subBlockElements + k] = out[lastSubIndexYY + k];
						if(colorLine) {
							out[subIndexCb + k] = out[lastSubIndexCb + k];
							out[subIndexCr + k] = out[lastSubIndexCr + k];
						}
					}

					subIndexYY += pJpegEnc->encBlockElements;
					subIndexCb += pJpegEnc->encBlockElements;
					subIndexCr += pJpegEnc->encBlockElements;
				}
			}
		}
	} else {
		if(NULL != band) {

			for(j=pJpegEnc->bandCurLines; j<(height + pJpegEnc->bandCurLines); j++) {
				indexYY = j * pJpegEnc->encBlockSize;

				/* For Speed Consideration. Put the big logic outside of the loops */
				if(pJpegEnc->isColor) {
					indexCB = indexYY + subBlockElements;
					indexCR = indexCB + subBlockElements;

					for(i=0; i<width/pJpegEnc->encBlockSize; i++) {
						for(k=0; k<pJpegEnc->encBlockSize; k++) {
							out[indexYY + k] = (signed char)((YY_R[RGB_R(pPixel)] + YY_G[RGB_G(pPixel)] + YY_B[RGB_B(pPixel)]) >> 16);
							out[indexCB + k] = (signed char)((CB_R[RGB_R(pPixel)] + CB_G[RGB_G(pPixel)] + CB_B[RGB_B(pPixel)]) >> 16);
							out[indexCR + k] = (signed char)((CR_R[RGB_R(pPixel)] + CR_G[RGB_G(pPixel)] + CR_B[RGB_B(pPixel)]) >> 16);
							pPixel += 3;
						}
						indexYY += pJpegEnc->encBlockElements;
						indexCB += pJpegEnc->encBlockElements;
						indexCR += pJpegEnc->encBlockElements;
					}

					/* index(s) are already correct at this point */
					if(linePadding > 0) {
						int lastYY = 0, lastCB = 0, lastCR = 0;
						for(k=0; k<pJpegEnc->encBlockSize; k++) {
							/* duplicate the last pixel for the the padding pixel */
							if(k < endPixels) {
								lastYY = (signed char)((YY_R[RGB_R(pPixel)] + YY_G[RGB_G(pPixel)] + YY_B[RGB_B(pPixel)]) >> 16);
								lastCB = (signed char)((CB_R[RGB_R(pPixel)] + CB_G[RGB_G(pPixel)] + CB_B[RGB_B(pPixel)]) >> 16);
								lastCR = (signed char)((CR_R[RGB_R(pPixel)] + CR_G[RGB_G(pPixel)] + CR_B[RGB_B(pPixel)]) >> 16);
								pPixel += 3;
							}

							out[indexYY + k] = lastYY;
							out[indexCB + k] = lastCB;
							out[indexCR + k] = lastCR;
						}
					}
				} else {
					for(i=0; i<width/pJpegEnc->encBlockSize; i++) {
						for(k=0; k<pJpegEnc->encBlockSize; k++) {
							out[indexYY + k] = (*pPixel++ - 128);
						}
						indexYY += pJpegEnc->encBlockElements;
					}

					/* indexYY is already correct at this point */
					if(linePadding > 0) {
						for(k=0; k<pJpegEnc->encBlockSize; k++) {
							/* duplicate the last pixel for the the padding pixel */
							out[indexYY + k] = ((k < endPixels)? (*pPixel++ - 128) : (*(pPixel-1) - 128));
						}
					}
				}
			}
		} else {
			for(j=pJpegEnc->bandCurLines; j<pJpegEnc->bandLines; j++) {
				indexYY = j * (pJpegEnc->encBlockSize * pJpegEnc->mcuPerEncBlock);
				if(pJpegEnc->isColor) {
					indexCB = indexYY + subBlockElements;
					indexCR = indexCB + subBlockElements;
				}
				for(i=0; i<pJpegEnc->horBlocks; i++) {
					indexLastYY = indexYY - pJpegEnc->encBlockSize;
					if(pJpegEnc->isColor) {
						indexLastCB = indexCB - pJpegEnc->encBlockSize;
						indexLastCR = indexCR - pJpegEnc->encBlockSize;
					}

					for(k=0; k<pJpegEnc->encBlockSize; k++) {
						out[indexYY + k] = out[indexLastYY + k];
						if(pJpegEnc->isColor) {
							out[indexCB + k] = out[indexLastCB + k];
							out[indexCR + k] = out[indexLastCR + k];
						}
					}

					indexYY += pJpegEnc->encBlockElements;
					if(pJpegEnc->isColor) {
						indexCB += pJpegEnc->encBlockElements;
						indexCR += pJpegEnc->encBlockElements;
					}
				}
			}
		}
	}

	return;
}

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* Color Conversion [END]                                                         */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */


/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* Forward DCT [START] (Borrowed from libjpeg jfdctint.c)                         */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
#define CONST_BITS  13
#define PASS1_BITS  2

#define FIX_0_298631336  ((_INT32)  2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ((_INT32)  3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ((_INT32)  4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ((_INT32)  6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ((_INT32)  7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ((_INT32)  9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  ((_INT32)  12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  ((_INT32)  15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  ((_INT32)  16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  ((_INT32)  16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  ((_INT32)  20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  ((_INT32)  25172)	/* FIX(3.072711026) */

/* To make the multiply faster by taking the advantage of 16bit ONLY DCT data */
#define MULTIPLY(var,const)		(((INT16) (var)) * ((INT16) (const)))
#define DESCALE(x, n)			( ((x) + (1 << ((n)-1))) >> (n) )

static void jpegEncforwardDCT(_INT32 *mcuBlock) {
	_INT32 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	_INT32 tmp10, tmp11, tmp12, tmp13;
	_INT32 z1, z2, z3, z4, z5;
	_INT32 *dataPtr;
	int ctr;

	/* Pass 1: process rows: Results are scaled up by sqrt(8) compared to a true DCT; Also scale the results by 2**PASS1_BITS. */
	dataPtr = mcuBlock;
	for (ctr = DCT_BLOCK_SIZE-1; ctr >= 0; ctr--) {
		tmp0 = dataPtr[0] + dataPtr[7];		tmp7 = dataPtr[0] - dataPtr[7];
		tmp1 = dataPtr[1] + dataPtr[6];		tmp6 = dataPtr[1] - dataPtr[6];
		tmp2 = dataPtr[2] + dataPtr[5];		tmp5 = dataPtr[2] - dataPtr[5];
		tmp3 = dataPtr[3] + dataPtr[4];		tmp4 = dataPtr[3] - dataPtr[4];
		tmp10 = tmp0 + tmp3;	tmp13 = tmp0 - tmp3;	tmp11 = tmp1 + tmp2;	tmp12 = tmp1 - tmp2;

		dataPtr[0] = (int) ((tmp10 + tmp11) << PASS1_BITS);
		dataPtr[4] = (int) ((tmp10 - tmp11) << PASS1_BITS);

		z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
		dataPtr[2] = (int) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),   CONST_BITS-PASS1_BITS);
		dataPtr[6] = (int) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065), CONST_BITS-PASS1_BITS);

		z1 = tmp4 + tmp7;		z2 = tmp5 + tmp6;		z3 = tmp4 + tmp6;		z4 = tmp5 + tmp7;
		z5 = MULTIPLY(z3 + z4, FIX_1_175875602);	/* sqrt(2) * c3 */

		tmp4 = MULTIPLY(tmp4, FIX_0_298631336);		/* sqrt(2) * (-c1+c3+c5-c7) */
		tmp5 = MULTIPLY(tmp5, FIX_2_053119869);		/* sqrt(2) * ( c1+c3-c5+c7) */
		tmp6 = MULTIPLY(tmp6, FIX_3_072711026);		/* sqrt(2) * ( c1+c3+c5-c7) */
		tmp7 = MULTIPLY(tmp7, FIX_1_501321110);		/* sqrt(2) * ( c1+c3-c5-c7) */
		z1 = MULTIPLY(z1, - FIX_0_899976223);		/* sqrt(2) * (c7-c3) */
		z2 = MULTIPLY(z2, - FIX_2_562915447);		/* sqrt(2) * (-c1-c3) */
		z3 = MULTIPLY(z3, - FIX_1_961570560);		/* sqrt(2) * (-c3-c5) */
		z4 = MULTIPLY(z4, - FIX_0_390180644);		/* sqrt(2) * (c5-c3) */

		z3 += z5;
		z4 += z5;

		dataPtr[7] = (int) DESCALE(tmp4 + z1 + z3, CONST_BITS-PASS1_BITS);
		dataPtr[5] = (int) DESCALE(tmp5 + z2 + z4, CONST_BITS-PASS1_BITS);
		dataPtr[3] = (int) DESCALE(tmp6 + z2 + z3, CONST_BITS-PASS1_BITS);
		dataPtr[1] = (int) DESCALE(tmp7 + z1 + z4, CONST_BITS-PASS1_BITS);

		dataPtr += DCT_BLOCK_SIZE;					/* advance pointer to next row */
	}

	/* Pass 2: process columns: Remove the PASS1_BITS scaling, but leave the results scaled up by an overall factor of 8. */
	dataPtr = mcuBlock;
	for (ctr = DCT_BLOCK_SIZE-1; ctr >= 0; ctr--) {
		tmp0 = dataPtr[DCT_BLOCK_SIZE*0] + dataPtr[DCT_BLOCK_SIZE*7];
		tmp7 = dataPtr[DCT_BLOCK_SIZE*0] - dataPtr[DCT_BLOCK_SIZE*7];
		tmp1 = dataPtr[DCT_BLOCK_SIZE*1] + dataPtr[DCT_BLOCK_SIZE*6];
		tmp6 = dataPtr[DCT_BLOCK_SIZE*1] - dataPtr[DCT_BLOCK_SIZE*6];
		tmp2 = dataPtr[DCT_BLOCK_SIZE*2] + dataPtr[DCT_BLOCK_SIZE*5];
		tmp5 = dataPtr[DCT_BLOCK_SIZE*2] - dataPtr[DCT_BLOCK_SIZE*5];
		tmp3 = dataPtr[DCT_BLOCK_SIZE*3] + dataPtr[DCT_BLOCK_SIZE*4];
		tmp4 = dataPtr[DCT_BLOCK_SIZE*3] - dataPtr[DCT_BLOCK_SIZE*4];

		tmp10 = tmp0 + tmp3;	tmp13 = tmp0 - tmp3;	tmp11 = tmp1 + tmp2;	tmp12 = tmp1 - tmp2;

		dataPtr[DCT_BLOCK_SIZE*0] = (int) DESCALE(tmp10 + tmp11, PASS1_BITS);
		dataPtr[DCT_BLOCK_SIZE*4] = (int) DESCALE(tmp10 - tmp11, PASS1_BITS);

		z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
		dataPtr[DCT_BLOCK_SIZE*2] = (int) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),   CONST_BITS+PASS1_BITS);
		dataPtr[DCT_BLOCK_SIZE*6] = (int) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065), CONST_BITS+PASS1_BITS);

		z1 = tmp4 + tmp7;
		z2 = tmp5 + tmp6;
		z3 = tmp4 + tmp6;
		z4 = tmp5 + tmp7;
		z5 = MULTIPLY(z3 + z4, FIX_1_175875602);	/* sqrt(2) * c3 */

		tmp4 = MULTIPLY(tmp4, FIX_0_298631336);		/* sqrt(2) * (-c1+c3+c5-c7) */
		tmp5 = MULTIPLY(tmp5, FIX_2_053119869);		/* sqrt(2) * ( c1+c3-c5+c7) */
		tmp6 = MULTIPLY(tmp6, FIX_3_072711026);		/* sqrt(2) * ( c1+c3+c5-c7) */
		tmp7 = MULTIPLY(tmp7, FIX_1_501321110);		/* sqrt(2) * ( c1+c3-c5-c7) */
		z1 = MULTIPLY(z1, - FIX_0_899976223);		/* sqrt(2) * (c7-c3) */
		z2 = MULTIPLY(z2, - FIX_2_562915447);		/* sqrt(2) * (-c1-c3) */
		z3 = MULTIPLY(z3, - FIX_1_961570560);		/* sqrt(2) * (-c3-c5) */
		z4 = MULTIPLY(z4, - FIX_0_390180644);		/* sqrt(2) * (c5-c3) */

		z3 += z5;	z4 += z5;

		dataPtr[DCT_BLOCK_SIZE*7] = (int) DESCALE(tmp4 + z1 + z3, CONST_BITS+PASS1_BITS);
		dataPtr[DCT_BLOCK_SIZE*5] = (int) DESCALE(tmp5 + z2 + z4, CONST_BITS+PASS1_BITS);
		dataPtr[DCT_BLOCK_SIZE*3] = (int) DESCALE(tmp6 + z2 + z3, CONST_BITS+PASS1_BITS);
		dataPtr[DCT_BLOCK_SIZE*1] = (int) DESCALE(tmp7 + z1 + z4, CONST_BITS+PASS1_BITS);

		dataPtr++;									/* advance pointer to next column */
	}

#ifndef SCALE_DCT_ON_QUANTIZATION
	dataPtr = mcuBlock;
	for(i = 0; i< DCT_BLOCK_SIZE*DCT_BLOCK_SIZE; i++) {
		dataPtr[i] = (dataPtr[i] < 0)? -( (4-dataPtr[i]) >> 3) : ((dataPtr[i] + 4) >> 3);		/* 4 is for rounding */
	}
#endif

	return;
}
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* Forward DCT [END]                                                              */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */


/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* ZigZag Quantization Related [START]                                            */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/* Borrowed from libjpeg jcparam.c for mapping quality (0-100) to a QT Table */
static void jpegEncGetScaledQTTable(int scaleFactor, const UINT8 *standardTable, UINT8 *dstTable, UINT8 *dstZigZagTable) {
	int i, n, helpShift = 0, temp, val;

#ifdef SCALE_DCT_ON_QUANTIZATION
	helpShift = 3;
#endif

	/* First Generate the QT Table (Convert the values to shiftting for performance) */
	for (i = 0; i < DCT_BLOCK_PIXS; i++) {
		temp = ((int) standardTable[zigzagIndex[i]] * scaleFactor + 50) / 100;
		if (temp <= 0)  temp = 1;
		if (temp > 255) temp = 255;
		/* Get the best shiftting for the given value */
		n = 0; val = temp;
		while( val >>= 1 ) { n++; }									/* The biggest 2^n that is smaller than temp value */
		n += ( ((1 << (n+1)) - temp) < (temp - (1 << n)) )? 1 : 0;	/* See if 2^(n+1) is closer to temp or not */
		val = 1 << n;

		dstZigZagTable[i] = n + helpShift;
		dstTable[i] = (val > 255)? 255 : val;
	}
}

static void jpegEncGenerateQTTable(PJPEGENC pJpegEnc) {
	int scaleFactor;

	if (pJpegEnc->quality <= 0)  pJpegEnc->quality = 1;
	if (pJpegEnc->quality > 100) pJpegEnc->quality = 100;
	scaleFactor = (pJpegEnc->quality < 50)? (5000 / pJpegEnc->quality) : (200 - pJpegEnc->quality * 2);

	jpegEncGetScaledQTTable(scaleFactor, table_QT_Lumin, pJpegEnc->tableQT[0], pJpegEnc->tableQTShiftZigzag[0]);
	if(pJpegEnc->isColor) {
		jpegEncGetScaledQTTable(scaleFactor, table_QT_Chrom, pJpegEnc->tableQT[1], pJpegEnc->tableQTShiftZigzag[1]);
	}
}

static void jpegEncZigZagQuantization(PJPEGENC pJpegEnc, _INT32 dctBlock[DCT_BLOCK_PIXS], INT16 huffBlock[DCT_BLOCK_PIXS], int color) {
	int i, pixVal;
	UINT8 *zigzagQTTable = (COLOR_YY == color)? pJpegEnc->tableQTShiftZigzag[0] : pJpegEnc->tableQTShiftZigzag[1];

	for(i=0; i<DCT_BLOCK_PIXS; i++) {
		pixVal = dctBlock[zigzagIndex[i]];
		huffBlock[i] = (pixVal > 0)? (pixVal >> zigzagQTTable[i]) : -((-pixVal) >> zigzagQTTable[i]);
	}
}

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* ZigZag Quantization Related [END]                                              */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */


/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* Huffman Encoding Related [START]                                               */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/* Assume the values are always in the sequential order (0x0->0xB) */
static void jpegEncGenerateHTInfo(UINT8 htLen[DHT_MAX_BITS], UINT8 *htVal, _INT32 valCount, UINT8 *tbLen, UINT16 *tbBin, _UINT32 tbSize) {
	UINT8  codeBitLen[DHT_ENCODE_ELEMENT_MAX];
	INT16 codeBitStr[DHT_ENCODE_ELEMENT_MAX], binCode;
	int i, thisCodeBitLen, lenCnt, index, htValue;

	/* Generate the binary information first */
	for(i=0, index=0, binCode=0; i<DHT_MAX_BITS; i++) {
		thisCodeBitLen = i + 1;
		lenCnt = htLen[i];
		while(lenCnt-- > 0) {
			codeBitLen[index] = thisCodeBitLen;
			codeBitStr[index] = binCode;
			binCode++;
			index++;
			if(DHT_ENCODE_ELEMENT_MAX == index) { printf("Invalid HT Table!\n"); return; }
		}

		binCode <<= 1;
	}

	memset(tbLen, 0, tbSize * sizeof(UINT8));
	memset(tbBin, 0, tbSize * sizeof(UINT16));
	/* Set values to the correct positon based on given htVal */
	for(i=0, index=0; i<valCount; i++) {
		htValue = htVal[i];
		tbLen[htValue] = codeBitLen[index];
		tbBin[htValue] = codeBitStr[index];
		index++;
	}
}

static void jpegEncGenerateHTCodeBook(PJPEGENC pJpegEnc) {

	jpegEncGenerateHTInfo(section_DHT_codeLength_DC_Y, section_DHT_codeValue_DC_Y, DHT_VAL_CNT_DC,
					pJpegEnc->htCodeBitLenDC[0], pJpegEnc->htCodeBitStrDC[0], DHT_ENCODE_ELEMENT_DC);


	jpegEncGenerateHTInfo(section_DHT_codeLength_AC_Y, section_DHT_codeValue_AC_Y, DHT_VAL_CNT_AC,
					pJpegEnc->htCodeBitLenAC[0], pJpegEnc->htCodeBitStrAC[0], DHT_ENCODE_ELEMENT_AC);


	if(pJpegEnc->isColor) {
		jpegEncGenerateHTInfo(section_DHT_codeLength_DC_C, section_DHT_codeValue_DC_C, DHT_VAL_CNT_DC,
						pJpegEnc->htCodeBitLenDC[1], pJpegEnc->htCodeBitStrDC[1], DHT_ENCODE_ELEMENT_DC);

		jpegEncGenerateHTInfo(section_DHT_codeLength_AC_C, section_DHT_codeValue_AC_C, DHT_VAL_CNT_AC,
						pJpegEnc->htCodeBitLenAC[1], pJpegEnc->htCodeBitStrAC[1], DHT_ENCODE_ELEMENT_AC);

	}
}

#define OUT_BYTE(_c_, _count_)							\
do{														\
	*(pJpegEnc->pCurrent++) = (_c_); (_count_)++;		\
	if ((_c_) == 0xFF) {								\
		*(pJpegEnc->pCurrent++) = 0x00; (_count_)++;	\
	}													\
} while(0)

static const _UINT32 bitMask[17] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

static int jpegEncHuffmanEncodeFinish(PJPEGENC pJpegEnc) {
	UINT8 thisByte;
	int outBytes = 0;

	if (pJpegEnc->vlcRemainBits >= 8) {
		thisByte = (UINT8)(pJpegEnc->vlcRemain >> (pJpegEnc->vlcRemainBits - 8));
		OUT_BYTE(thisByte, outBytes);

		pJpegEnc->vlcRemainBits -= 8;
	}

	if (pJpegEnc->vlcRemainBits >= 0) {
		thisByte = (UINT8)(pJpegEnc->vlcRemain << (8 - pJpegEnc->vlcRemainBits));
		thisByte |= bitMask[8 - pJpegEnc->vlcRemainBits];
		OUT_BYTE((thisByte & 0xff), outBytes);
	}

	return outBytes;
}

/* It seems that the MACRO is much faster than function on my enviroment */
#define HUFFMAN_OUTBITS16(_LEN_, _BITVAL_)											\
do {																				\
	UINT16 out16;																	\
	UINT8  out8;																	\
																					\
	pJpegEnc->vlcRemain <<= (_LEN_);												\
	pJpegEnc->vlcRemain  |= (_BITVAL_) & bitMask[(_LEN_)];							\
	pJpegEnc->vlcRemainBits += (_LEN_);												\
																					\
	if (pJpegEnc->vlcRemainBits >= 16) {											\
		out16 = (UINT16)(pJpegEnc->vlcRemain >> (pJpegEnc->vlcRemainBits - 16));	\
																					\
		out8  = (out16 & 0xFF00) >> 8;												\
		OUT_BYTE(out8, outBytes);													\
																					\
		out8 = out16 & 0xFF;														\
		OUT_BYTE(out8, outBytes);													\
																					\
		pJpegEnc->vlcRemainBits -= 16;												\
	}																				\
} while(0)

static const bitLenTable[256] = {
	0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};

/* It seems that the MACRO is much faster than function on my enviroment */
#define HUFFMAN_GET_BITCODE(_VAL_, _LEN_, _BITVAL_)					\
do {																\
	INT16 value = (_VAL_);											\
	UINT8 hi, lo;													\
																	\
	if (value >=0) {												\
		(_BITVAL_) = value;											\
	} else {														\
		value      = -value;										\
		(_BITVAL_) = ~value;										\
	}																\
																	\
	hi = value >> 8;												\
	lo = value & 0xFF;												\
	(_LEN_) = (0 != hi)? (bitLenTable[hi] + 8) : bitLenTable[lo];	\
} while(0)

static int jpegEncHuffmanEnc(PJPEGENC pJpegEnc, INT16 mcuBlock[64], int color) {
	INT16 dcDiff, zeroRun, i;
	UINT8 bitLen;
	_UINT32 bitVal, totalBitLen, outBytes = 0;
	UINT8  *htCodeBitLenTable;
	UINT16 *htCodeBitStrTable;

	/* DC */
	dcDiff = mcuBlock[0] - pJpegEnc->prevDCValue[color];
	pJpegEnc->prevDCValue[color] = mcuBlock[0];
	htCodeBitLenTable = (COLOR_YY == color)? pJpegEnc->htCodeBitLenDC[0] : pJpegEnc->htCodeBitLenDC[1];
	htCodeBitStrTable = (COLOR_YY == color)? pJpegEnc->htCodeBitStrDC[0] : pJpegEnc->htCodeBitStrDC[1];

	HUFFMAN_GET_BITCODE(dcDiff, bitLen, bitVal);
	HUFFMAN_OUTBITS16(htCodeBitLenTable[bitLen], htCodeBitStrTable[bitLen]);
	HUFFMAN_OUTBITS16(bitLen, bitVal);

	/* AC */
	htCodeBitLenTable = (COLOR_YY == color)? pJpegEnc->htCodeBitLenAC[0] : pJpegEnc->htCodeBitLenAC[1];
	htCodeBitStrTable = (COLOR_YY == color)? pJpegEnc->htCodeBitStrAC[0] : pJpegEnc->htCodeBitStrAC[1];
	zeroRun=0;
	for(i=1; i<64; i++) {
		if (mcuBlock[i] != 0) {
			while ( zeroRun >= 16 ) {
				HUFFMAN_OUTBITS16(htCodeBitLenTable[0xF0], htCodeBitStrTable[0xF0]);
				zeroRun -= 16;
			}

			HUFFMAN_GET_BITCODE(mcuBlock[i], bitLen, bitVal);
			totalBitLen = (zeroRun * 16) + bitLen;
			HUFFMAN_OUTBITS16(htCodeBitLenTable[totalBitLen], htCodeBitStrTable[totalBitLen]);
			HUFFMAN_OUTBITS16(bitLen, bitVal);
			zeroRun=0;
		} else zeroRun++;
	}

	if ( 0 != zeroRun )	{
		HUFFMAN_OUTBITS16(htCodeBitLenTable[0x00], htCodeBitStrTable[0x00]);
	}

	return outBytes;
}
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* Huffman Encoding Related [END]                                                 */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

static void jpegEncUpdateOutInfo(PJPEGENC pJpegEnc, int bytesOut, BOOL updateCurrentAddr) {
	*(pJpegEnc->pJpegSize)	+= bytesOut;
	pJpegEnc->bytesLeft		-= bytesOut;
	if(updateCurrentAddr) { pJpegEnc->pCurrent += bytesOut; }
	return;
}

#define PROC_WORK_BUF(_COLOR_IDX_)											\
{																			\
	_INT32 *pBlock = pJpegEnc->workBand + blockOffSet;						\
	INT16 *pHuffBlock = pJpegEnc->huffBand + blockOffSet;					\
	jpegEncforwardDCT(pBlock);												\
	jpegEncZigZagQuantization(pJpegEnc, pBlock, pHuffBlock, _COLOR_IDX_);	\
	totalOutBytes += jpegEncHuffmanEnc(pJpegEnc, pHuffBlock, _COLOR_IDX_);  \
	blockOffSet += 64;														\
}

/* Always process bandLines at a time */
static void jpegEncProcWorkBuf(PJPEGENC pJpegEnc) {
	int i, totalOutBytes = 0, blockOffSet = 0;

	/* Use big logic to avoid branching within loop */
	if(pJpegEnc->isColor) {
		if(pJpegEnc->doSubSample) {
			for(i=0; i<pJpegEnc->horBlocks; i++) {
				PROC_WORK_BUF(COLOR_YY);
				PROC_WORK_BUF(COLOR_YY);
				PROC_WORK_BUF(COLOR_YY);
				PROC_WORK_BUF(COLOR_YY);
				PROC_WORK_BUF(COLOR_CB);
				PROC_WORK_BUF(COLOR_CR);
			}
		} else {
			for(i=0; i<pJpegEnc->horBlocks; i++) {
				PROC_WORK_BUF(COLOR_YY);
				PROC_WORK_BUF(COLOR_CB);
				PROC_WORK_BUF(COLOR_CR);
			}
		}
	} else {
		for(i=0; i<pJpegEnc->horBlocks; i++) {
			PROC_WORK_BUF(COLOR_YY);
		}
	}

	jpegEncUpdateOutInfo(pJpegEnc, totalOutBytes, FALSE);

	return;
}

int jpegEncGetLeftMemory (PJPEGENC pJpegEnc) {
	return (NULL != pJpegEnc)? pJpegEnc->bytesLeft : 0;
}

PJPEGENC jpegEncAlloc (int imgW, int imgH, UINT8 *jpegBuf, int *pJpegBufSize, int quality, BOOL isColor, BOOL doSubSample) {
	BOOL allOK = FALSE;
	PJPEGENC pJpegEnc = NULL;
	int bytesOut;

	if( (imgW <= 0) || (imgH <=0) || (NULL == jpegBuf) || (*pJpegBufSize < MIN_JPEG_SIZE) ) {
		printf("Invalid Input\n"); goto allocExit;
	}

	pJpegEnc = (PJPEGENC)malloc(sizeof(JPEGENC));
	if(NULL == pJpegEnc) { printf("Can't allocate memory\n"); goto allocExit; }

	memset(pJpegEnc, 0, sizeof(JPEGENC));
	pJpegEnc->width			= imgW;
	pJpegEnc->height		= imgH;
	pJpegEnc->isColor		= isColor;
	pJpegEnc->doSubSample	= doSubSample;
	pJpegEnc->quality		= quality;
	pJpegEnc->dstBuf		= jpegBuf;
	pJpegEnc->dstBufSize    = *pJpegBufSize;
	pJpegEnc->bytesLeft		= pJpegEnc->dstBufSize;
	pJpegEnc->pCurrent		= pJpegEnc->dstBuf;
	pJpegEnc->pJpegSize		= pJpegBufSize;
	*(pJpegEnc->pJpegSize)	= 0;

	pJpegEnc->encBlockSize	= (pJpegEnc->isColor && pJpegEnc->doSubSample) ? 16 : 8;	/* Color SubSample Mode: 16x16 block;  Grayscale Mode & Non-SubSample Mode: 8x8 block */
	pJpegEnc->pixelBytes	= pJpegEnc->isColor? 3 : 1;
	pJpegEnc->widthAligned	= ROUND_UP(pJpegEnc->width,  pJpegEnc->encBlockSize);
	pJpegEnc->heightAligned	= ROUND_UP(pJpegEnc->height, pJpegEnc->encBlockSize);
	pJpegEnc->bandLines		= pJpegEnc->encBlockSize;
	pJpegEnc->horBlocks		= pJpegEnc->widthAligned / pJpegEnc->encBlockSize;

	pJpegEnc->mcuPerEncBlock   = (pJpegEnc->isColor)? ( (pJpegEnc->doSubSample)? 6 : 3 ) : 1;
	pJpegEnc->encBlockElements = DCT_BLOCK_PIXS * pJpegEnc->mcuPerEncBlock;
	pJpegEnc->workBandElements = (pJpegEnc->widthAligned * DCT_BLOCK_SIZE) * pJpegEnc->mcuPerEncBlock;
	pJpegEnc->workBand = (_INT32 *)malloc(sizeof(_INT32) * pJpegEnc->workBandElements);
	if(NULL == pJpegEnc->workBand) { printf("Can't allocate memory\n"); goto allocExit; }
	memset(pJpegEnc->workBand, 0, sizeof(_INT32) * pJpegEnc->workBandElements);

	pJpegEnc->huffBand = (INT16 *)malloc(sizeof(INT16) * pJpegEnc->workBandElements);
	if(NULL == pJpegEnc->huffBand) { printf("Can't allocate memory\n"); goto allocExit; }

	/* Generate Quantization Table based on given quality */
	jpegEncGenerateQTTable(pJpegEnc);

	/* Generate Huffman encoding book based on given Huffman Table */
	jpegEncGenerateHTCodeBook(pJpegEnc);

	bytesOut = jpegEncFillHead(pJpegEnc->pCurrent, pJpegEnc->width, pJpegEnc->height, pJpegEnc->isColor, pJpegEnc->doSubSample, pJpegEnc->tableQT[0], pJpegEnc->tableQT[1]);
	jpegEncUpdateOutInfo(pJpegEnc, bytesOut, TRUE);

	allOK = TRUE;
allocExit:
	if(!allOK) { jpegEncFree(pJpegEnc, FALSE, TRUE); pJpegEnc = NULL; }
	return pJpegEnc;
}

int jpegEncFeed (PJPEGENC pJpegEnc, UINT8 *bandBuf, int bandH, int bandW) {
	int procLines, stripLines, doneLines = 0;
	UINT8 *feedAddr = bandBuf;

	if( (NULL == pJpegEnc) || (NULL == bandBuf) || (0 > bandH) || (0 > bandW) ) { printf("Invalid input\n"); return 0; }
	/* Do a safe buffer estimation here. To avoid a lot of logic checking on the pixel level */
	if( pJpegEnc->bytesLeft < (pJpegEnc->workBandElements + SAFTY_EXTRA_BYTES) ) { printf("Buffer is too small to proceed.\n"); return 0; }
	if(bandW != pJpegEnc->width) { printf("WARNING: band width is not the same as specified image width.\n"); }

	procLines = min( bandH, (pJpegEnc->height - pJpegEnc->linesDone) );
	while(procLines > 0) {
		stripLines = min( (pJpegEnc->bandLines - pJpegEnc->bandCurLines), procLines );

		/* RGB -> YCC to the internal band buffer. Also do downsampling and convert lines to 8x8 blocks */
		jpegEncRGBLine2YCCBlock(pJpegEnc, feedAddr, bandW, stripLines);

		feedAddr += bandW * stripLines * pJpegEnc->pixelBytes;
		pJpegEnc->bandCurLines += stripLines;
		pJpegEnc->linesDone += stripLines;
		doneLines += stripLines;
		procLines -= stripLines;

		/* Only Start to process the YCC internal band buffer when it's filled or at very end of the image */
		if( (pJpegEnc->bandCurLines == pJpegEnc->bandLines) || (pJpegEnc->linesDone == pJpegEnc->height) ) {
			/* For the last band of the image. Fill the empty lines with the last line (by passing NULL ad band pointer) */
			if(pJpegEnc->bandCurLines < pJpegEnc->bandLines) jpegEncRGBLine2YCCBlock(pJpegEnc, NULL, 0, 0);

			/* Start to process the internal band work buffer */
			jpegEncProcWorkBuf(pJpegEnc);
			pJpegEnc->bandCurLines = 0;
		}
	}

	return doneLines;
}

BOOL jpegEncFree (PJPEGENC pJpegEnc, BOOL padTailLines, BOOL isAbort) {
	BOOL result = FALSE;

	if(NULL != pJpegEnc) {
		if(!isAbort) {
			if((pJpegEnc->linesDone < pJpegEnc->height) && padTailLines) {
				int whileLineByteSize = pJpegEnc->width * pJpegEnc->pixelBytes;
				UINT8 *whiteLine = malloc(whileLineByteSize);
				if(NULL != whiteLine) {
					int i = (pJpegEnc->height - pJpegEnc->linesDone);
					memset(whiteLine, 0xFF, whileLineByteSize);
					while(i-- > 0) { jpegEncFeed(pJpegEnc, whiteLine, 1, pJpegEnc->width); }
					free(whiteLine);
				}
			}

			if(pJpegEnc->linesDone != pJpegEnc->height) {
				printf("Input Lines are less than speicified\n");
			} else {
				if( pJpegEnc->bytesLeft >= 4 ) {
					int bytesOut;

					bytesOut = jpegEncHuffmanEncodeFinish(pJpegEnc);	/* Output upto 2 Bytes */
					jpegEncUpdateOutInfo(pJpegEnc, bytesOut, FALSE);

					bytesOut = jpegEncFillTail(pJpegEnc->pCurrent);		/* Output 2 Bytes */
					jpegEncUpdateOutInfo(pJpegEnc, bytesOut, TRUE);
					result = TRUE;
				}
			}
		}

		if(NULL != pJpegEnc->workBand) free(pJpegEnc->workBand);
		if(NULL != pJpegEnc->huffBand) free(pJpegEnc->huffBand);

		free(pJpegEnc);
	}

	return result;
}
