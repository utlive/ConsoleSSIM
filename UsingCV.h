//////////////////////////////////////////////////////////////////////////
/// Ming's function for manipulating OpenCV
/////////////////////////////////////////////////////////////////////////
#pragma once


#include <wtypes.h>

#include "cv.h"
#include "highgui.h"
#include "ThreadTool.h"


static CSync m_Critical_DumpImg;


template<typename _TYPE>
void mj_SaveBufferToImage(LPCTSTR fname, _TYPE *buffer, int width, int height, int stride = -1, int channels = -1)
{
	CLockGuard Gate(m_Critical_DumpImg);
	IplImage *p1 = NULL;
	mj_BufferToIplImage(&p1, buffer, width, height, stride, channels);

	cvSaveImage((const char*)fname, p1);
	cvReleaseImage(&p1);
}

template<typename _TYPE>
void mj_BufferToIplImage(IplImage **img, _TYPE *buffer, int width, int height, int stride = -1, int channels = -1)
{
	if(stride == -1) stride = width;
	if(channels != 3) channels = 1;

	CLockGuard Gate(m_Critical_DumpImg);

	if(*img != NULL) cvReleaseImage(img);
	*img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, channels);
	BYTE *pDest = NULL;
	cvGetRawData(*img, &pDest);
	int i, j, p;
	for(i = 0; i < height; i++) {
		for(j = 0; j < width; j++) {
			for(p = 0; p < channels; p++)
				pDest[i * (*img)->widthStep + j * channels + p] = (BYTE)(buffer[i * stride + j * channels + p]);
		}
	}


}
