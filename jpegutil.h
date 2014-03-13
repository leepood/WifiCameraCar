/*
 * jpegutil.h
 *
 *  Created on: 2013-12-13
 *      Author: leepood
 */

#ifndef JPEGUTIL_H_
#define JPEGUTIL_H_

#define BYTE unsigned char
#define DECODE_ERROR -1
#define DECODE_SUCC 0

typedef struct _IplImage IplImage;

int ipl2Jpeg(IplImage *, unsigned char **,
		long unsigned int *outlen);

#endif /* JPEGUTIL_H_ */
