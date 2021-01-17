/*!
***********************************************************************
* \file USingCV.CPP
*
* \brief
*    Functions of using OpenCV
*
* \note
*  
*
* \author
*    - Ming-Jun Chen    <mjchen@utexas.edu>
***********************************************************************
*/
#include "UsingCV.h"


void _ResizeImage(IplImage **img, int new_width, int new_height)
{
	IplImage *p2 = cvCreateImage(cvSize(new_width, new_height), IPL_DEPTH_8U, (*img)->nChannels);
	cvResize(*img, p2);
	cvReleaseImage(img);
	*img = p2;
}