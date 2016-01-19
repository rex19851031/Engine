/*
	jpegEnc.h: API header file for jpegEnc
	Author: Hao Hu [ihaohu@gmail.com]
	
	Supported Mode:
		2x2:1x1:1x1		Most Common Mode (isColor is TRUE && doSubSample is TRUE)
		1x1:1x1:1x1		Best Quality (isColor is TRUE && doSubSample is FALSE)
		1x1				Grayscale (isColor is FALSE)
*/

#ifndef _JPEGENC_H_
#define _JPEGENC_H_

/* ############################################ */
/* IF the system does not defined all these     */
/* ############################################ */

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef UINT8
typedef unsigned char UINT8;
#endif

#ifndef INT16
typedef signed short INT16;
#endif

#ifndef UINT16
typedef unsigned short UINT16;
#endif

#ifndef _INT32
typedef signed long _INT32;
#endif

#ifndef _UINT32
typedef unsigned long _UINT32;
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef ROUND_UP
#define ROUND_UP(_VAL_, _ALIGN_)		(((_VAL_) + (_ALIGN_ - 1)) & (-_ALIGN_))
#endif

#ifndef min
#define min(_a_, _b_)					(((_a_) < (_b_))? (_a_) : (_b_))
#endif

#ifndef max
#define max(_a_, _b_)					(((_a_) > (_b_))? (_a_) : (_b_))
#endif

/* ############################################ */
/* MACROs for jpegEnc                           */
/* ############################################ */

#define DCT_BLOCK_SIZE			8
#define DCT_BLOCK_PIXS			(DCT_BLOCK_SIZE * DCT_BLOCK_SIZE)

#define DHT_MAX_BITS			16
#define DHT_VAL_CNT_DC			12
#define DHT_VAL_CNT_AC			162

#define DHT_ENCODE_ELEMENT_DC	16
#define DHT_ENCODE_ELEMENT_AC	256
#define DHT_ENCODE_ELEMENT_MAX	256

#define SAFTY_EXTRA_BYTES		512
#define MIN_JPEG_SIZE			(1024 + SAFTY_EXTRA_BYTES)	/* A typical color JPEG file header is 620 bytes + 2bytes file tail */

/* ############################################ */
/* jpegEnc Object and API                       */
/* ############################################ */

typedef struct {
	int				width, widthAligned;
	int				height, heightAligned;
	BOOL			isColor, doSubSample;
	int				quality;
	int				pixelBytes;
	int				encBlockSize, horBlocks;
	int				linesDone;
	int				*pJpegSize;
	UINT8			*dstBuf;
	int				dstBufSize;
	UINT8			*pCurrent;
	int				bytesLeft;
	int				bandLines, bandCurLines;
	int				mcuPerEncBlock, encBlock8x8Count;
	int				encBlockElements;
	int				workBandElements;
	_INT32			*workBand;		/* Used for Color Conversion and DCT */
	INT16			*huffBand;		/* Use for Huffman Encoding */
	UINT8			tableQT[2][DCT_BLOCK_PIXS];				/* Generated Quantization Table for given quality */
	UINT8			tableQTShiftZigzag[2][DCT_BLOCK_PIXS];	/* Actually reformated QT Table used for better performance */
	UINT8			htCodeBitLenDC[2][DHT_ENCODE_ELEMENT_DC];		/* Generate table for Huffman Encoding. Y & C */
	UINT16			htCodeBitStrDC[2][DHT_ENCODE_ELEMENT_DC];		/* Generate table for Huffman Encoding  Y & C */
	UINT8			htCodeBitLenAC[2][DHT_ENCODE_ELEMENT_AC];		/* Generate table for Huffman Encoding. Y & C */
	UINT16			htCodeBitStrAC[2][DHT_ENCODE_ELEMENT_AC];		/* Generate table for Huffman Encoding  Y & C */
	_UINT32			vlcRemain, vlcRemainBits;
	INT16			prevDCValue[3];
} JPEGENC, *PJPEGENC;

/* doSubSample: TRUE: 2x2:1x1:1x1 (Default Mode);  FALSE: 1x1:1x1:1x1 (Best Quality) */
PJPEGENC jpegEncAlloc (int imgW, int imgH, UINT8 *jpegBuf, int *pJpegBufSize, int quality, BOOL isColor, BOOL doSubSample);
int jpegEncFeed (PJPEGENC pJpegEnc, UINT8 *bandBuf, int bandH, int bandW);
BOOL jpegEncFree (PJPEGENC pJpegEnc, BOOL padTailLines, BOOL isAbort);

/* To report memory usage */
int jpegEncGetLeftMemory (PJPEGENC pJpegEnc);

#endif /* _JPEGENC_H_ */
