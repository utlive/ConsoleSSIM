/*!
***********************************************************************
* \file MS_SSIM.CPP
*
* \brief
*  kernel implementation of fast MS-SSIM and MS-SSIM
*  For fast MS-SSIM , refer to Ming-Jun Chen and Alan C. Bovik, “Fast structural similarity index algorithm”, Journal of Real-Time Image Processing, pp. 1-7. August, 2010
* \note
*  
*
* \author
*    - Ming-Jun Chen    <mjchen@utexas.edu>
***********************************************************************
***********************************************************************

Copyright Notice

-----------COPYRIGHT NOTICE STARTS WITH THIS LINE------------
Copyright (c) 2012 The University of Texas at Austin
All rights reserved.

Permission is hereby granted, without written agreement and without license or royalty fees, 
to use, copy, modify, and distribute the software provided and its documentation for research purpose only,
provided that this copyright notice and the original authors' names appear on all copies and supporting documentation.
The software provided may not be commercially distributed.


The following papers are to be cited in the bibliography whenever the software is used as:

Ming-Jun Chen and Alan C. Bovik, “Fast structural similarity index algorithm”, Journal of Real-Time Image Processing, pp. 1-7. August, 2010


IN NO EVENT SHALL THE UNIVERSITY OF TEXAS AT AUSTIN BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, 
SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS DATABASE AND ITS 
DOCUMENTATION, EVEN IF THE UNIVERSITY OF TEXAS AT AUSTIN HAS BEEN ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

THE UNIVERSITY OF TEXAS AT AUSTIN SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE DATABASE 
PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF TEXAS AT AUSTIN HAS NO OBLIGATION
TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

-----------COPYRIGHT NOTICE ENDS WITH THIS LINE------------
*/

#include "MS_SSIM.h"
#include "Common.h"
#include "UsingCV.h"

//#define SKIP_FIRST_LAYER 1
//#define _DUMP_IMAGE 1

CMS_SSIMKL::CMS_SSIMKL(){
	_DPRINTF(("CMS_SSIMKL MS constructor"));
#ifndef SKIP_FIRST_LAYER
	fPara[0]=0.0448f;
	fPara[1]=0.2856f;
#else
	fPara[0]=0.00f;
	fPara[1]=0.3303f;
#endif
	fPara[2]=0.3001f;
	fPara[3]=0.2363f;
	fPara[4]=0.1333f;
	//fPara[4]=1.0f;
	pReSizeRef=ReSizeTest=NULL;

}
CMS_SSIMKL::~CMS_SSIMKL(){
	_DPRINTF(("CMS_SSIMKL MS destructor"));
	_ALIGNED_FREE_PTR(pReSizeRef);
	_ALIGNED_FREE_PTR(ReSizeTest);

}
bool CMS_SSIMKL::Init(const VFrameInfo vCurframe){

	if(vCurframe.nWidth<FRAME_SIZE_MIN || vCurframe.nHeight<FRAME_SIZE_MIN)
		return false;

	m_PorcessFrameInfoInit=vCurframe;
	CSS_SSIMKL::Init(vCurframe);

	nInitSrcStride=m_PorcessFrameInfo.nStride;
	int buf_sz_temp=m_PorcessFrameInfo.nHeight*m_PorcessFrameInfo.nStride;
	buf_sz_temp=buf_sz_temp>>2;

	_ALIGNED_MALLOC_PTR(pReSizeRef,BYTE,buf_sz_temp);
	_ALIGNED_MALLOC_PTR(ReSizeTest,BYTE,buf_sz_temp);
	iFrameCount=0;
	return true;
}
bool CMS_SSIMKL::ProcessOneFrame(const BYTE* pRef, const BYTE* pTest,double& dScore){

	if(m_PorcessFrameInfo.nWidth<FRAME_SIZE_MIN || m_PorcessFrameInfo.nHeight<FRAME_SIZE_MIN)
		return false;

	iFrameCount++;

	CTimeCost m_time;
	m_time.start();

	m_PorcessFrameInfo=m_PorcessFrameInfoInit;

	nSrcStride=m_PorcessFrameInfo.nStride;
	nAlignStride=m_PorcessFrameInfo.nStride;
	nPadStride=nAlignStride+WINDOW_SZ;


	m_Width=m_PorcessFrameInfo.nWidth;
	m_Height=m_PorcessFrameInfo.nHeight;

	float fSSIM(1.0f), fLayer(0.0f);
	int iCount(0);
	float fTemp1(0.0f),fTemp2(0.0f);
	int x,y;
	float fTempSSIM(0.0f);
	float *pTVar1, *pTVar2, *pTSigmaMul, *pTMeanSq1, *pTMeanSq2, *pTMeanMul;
#ifndef SKIP_FIRST_LAYER

	CalculateFeatures((BYTE*)pRef,(BYTE*)pTest);  //skip Layer one analysis for optimization 

	////////////////////////////////////////////////////////////////////////
	// Get contrast and structure 

	for(y=0;y<m_Height-WINDOW_SZ+1;y++){
		pTSigmaMul=pSigmaMul+(y*nAlignStride);
		pTMeanMul=pMeanMul+(y*nAlignStride);
		pTMeanSq1=pMeanRefSq+(y*nAlignStride);
		pTMeanSq2=pMeanTestSq+(y*nAlignStride);
		pTVar1=pSigmaRefSq+(y*nAlignStride);
		pTVar2=pSigmaTestSq+(y*nAlignStride);

		for(x=0;x<m_Width-WINDOW_SZ+1;x++){
			iCount++;
			fTemp1= ( 2 * (*pTSigmaMul)+C2 );
			fTemp2= ( *pTVar1+ *pTVar2+C2);
			fLayer+=(fTemp1/fTemp2);
			pTSigmaMul++;
			pTVar1++;
			pTVar2++;

		}
	}
	fLayer/=(float)iCount;
	//_DPRINTF(("L1 c %f",fLayer));
	fSSIM*= pow(fLayer,fPara[0]);
	fSSIM*= 1.0;
#endif
	//////////////////////////////////////////////////////////////////////////
	// Prepare for resize by OpenCV
	IplImage *img1_temp = cvCreateImage(cvSize(m_PorcessFrameInfo.nWidth, m_PorcessFrameInfo.nHeight), IPL_DEPTH_8U, 1);
	BYTE* pData,*pTRef,*pTemp1;
	int step;
	CvSize size;

	cvGetRawData( img1_temp, (uchar**)&pData, &step, &size );
	step /= sizeof(*pData);

	for( y = 0; y < size.height; y++, pData += step ){
		pTRef=(BYTE*)pRef+nSrcStride*y;
		pTemp1=pData;
		for( x = 0; x < size.width; x++ ){
			*pTemp1 = *pTRef;
			pTemp1++;
			pTRef++;
		}
	}

	IplImage *img2_temp = cvCreateImage(cvSize(m_PorcessFrameInfo.nWidth, m_PorcessFrameInfo.nHeight), IPL_DEPTH_8U, 1);
	BYTE *pTTest,*pDataTest,*pTemptest;


	cvGetRawData( img2_temp, &pDataTest, &step, &size );
	step /= sizeof(*pDataTest);

	for( y = 0; y < size.height; y++, pDataTest += step ){
		pTTest=(BYTE*)pTest+nSrcStride*y;
		pTemptest=pDataTest;
		for( x = 0; x < size.width; x++ ){
			*pTemptest = *pTTest;
			pTemptest++;
			pTTest++;
		}
	}


	int i;
	IplImage *ResizeSzImgRef =NULL;
	IplImage *ResizeSzImgTest=NULL;

	for(i=1;i<MAX_LEVEL;i++){

		iCount=0;
		BYTE* pReSzRef, *pReSzTest;

		int ReWidth,ReHieght;
		m_Width=m_Width>>1;
		m_Height=m_Height>>1;

		ReWidth=m_Width;
		ReHieght=m_Height;


		ResizeSzImgRef = cvCreateImage(cvSize(ReWidth ,ReHieght), IPL_DEPTH_8U, 1);
		cvResize(img1_temp, ResizeSzImgRef, CV_INTER_LINEAR);

		ResizeSzImgTest = cvCreateImage(cvSize(ReWidth, ReHieght), IPL_DEPTH_8U, 1);
		cvResize(img2_temp, ResizeSzImgTest, CV_INTER_LINEAR);

		#ifdef _DUMP_IMAGE
		
				_TCHAR fname[256];
		
				_stprintf(fname, "C:\\dump\\Ref_Ori_Layer%d.jpg", i);
				cvSaveImage(fname,img1_temp);
				_stprintf(fname, "C:\\dump\\Test_Ori_Layer%d.jpg", i);
				cvSaveImage(fname,img2_temp);
		
				_stprintf(fname, "C:\\dump\\Ref_Resize_Layer%d.jpg", i);
				cvSaveImage(fname,ResizeSzImgRef);
				_stprintf(fname, "C:\\dump\\Test_Resize_Layer%d.jpg", i);
				cvSaveImage(fname,ResizeSzImgTest);
		
		#endif
		cvReleaseImage(&img1_temp);
		cvReleaseImage(&img2_temp);

		cvGetRawData( ResizeSzImgRef, &pReSzRef, &step, &size );
		cvGetRawData( ResizeSzImgTest, &pReSzTest, &step, &size );

#ifdef _DUMP_IMAGE

		_TCHAR fname[256];

		_stprintf(fname, "C:\\dump\\Ref_Frame%04dlayer%d.jpg", iFrameCount,i);
		mj_SaveBufferToImage<BYTE>(fname,pReSzRef,m_Width,m_Height);
		_stprintf(fname, "C:\\dump\\Test_Frame%04dlayer%d.jpg", iFrameCount,i);
		mj_SaveBufferToImage<BYTE>(fname,pReSzTest,m_Width,m_Height);

#endif
		step /= sizeof(*pReSzRef);

		m_PorcessFrameInfo.nStride=step;
		nSrcStride=step;
		nAlignStride=step;
		nPadStride=nAlignStride+WINDOW_SZ;
		CalculateFeatures((BYTE*)pReSzRef,(BYTE*)pReSzTest);

		//////////////////////////////////////////////////////////////////////////
		// Get contrast and structure for multi-scales

		fLayer=0.0f;
		for(y=0;y<m_Height-WINDOW_SZ+1;y++){
			pTSigmaMul=pSigmaMul+(y*nAlignStride);
			pTMeanMul=pMeanMul+(y*nAlignStride);
			pTMeanSq1=pMeanRefSq+(y*nAlignStride);
			pTMeanSq2=pMeanTestSq+(y*nAlignStride);
			pTVar1=pSigmaRefSq+(y*nAlignStride);
			pTVar2=pSigmaTestSq+(y*nAlignStride);

			for(x=0;x<m_Width-WINDOW_SZ+1;x++){

				fTemp1= ( 2 * (*pTSigmaMul)+C2 );
				fTemp2= ( (*pTVar1)+ (*pTVar2)+C2);
				fLayer+=(fTemp1/fTemp2);
				iCount++;
				pTSigmaMul++;
				pTVar1++;
				pTVar2++;

			}
		}

		fLayer/=(float)iCount;
	//	_DPRINTF(("S layer%d s%f",i,fLayer));
		float ft=pow((float)fLayer,(float)fPara[i]);
		fSSIM*= ft;
		if(i==MAX_LEVEL-1){
			fLayer=0.0f;
			iCount=0;
			for(y=0;y<m_Height-WINDOW_SZ+1;y++){

				pTMeanMul=pMeanMul+(y*nAlignStride);
				pTMeanSq1=pMeanRefSq+(y*nAlignStride);
				pTMeanSq2=pMeanTestSq+(y*nAlignStride);


				for(x=0;x<m_Width-WINDOW_SZ+1;x++){
					fTemp1=2 * (*pTMeanMul)+C1;
					fTemp2= ( (*pTMeanSq1)+ (*pTMeanSq2)+C1) ;
					fLayer+=(fTemp1/fTemp2);
					pTMeanMul++;
					pTMeanSq1++;
					pTMeanSq2++;
					iCount++;

				}
			}
			fLayer/=(float)iCount;
			float ft=pow((float)fLayer,(float)fPara[i]);
	//		_DPRINTF(("Luminance s%f",ft));
			fSSIM*= ft;

		}

		img1_temp=ResizeSzImgRef;
		img2_temp=ResizeSzImgTest;

	}
	cvReleaseImage(&ResizeSzImgRef);
	cvReleaseImage(&ResizeSzImgTest);

	m_time.end();
	if(fSSIM>1.0f)
		fSSIM=1.0f;
	dScore=fSSIM;
	//_DPRINTF(("MS-SSIM block %d score %f  time %f",iCount,fSSIM,m_time.get_seconds()));
	return true;
}
bool CMS_SSIMKL::DownSizeImageByTwo(IplImage* pImgSrc, IplImage* pImgDest, DWORD nWidth, DWORD nHeight, DWORD nStride,float& fSSIM){

	pImgDest = cvCreateImage(cvSize(m_PorcessFrameInfo.nWidth>>1, m_PorcessFrameInfo.nHeight>>1), IPL_DEPTH_8U, 1);
	cvResize(pImgSrc, pImgDest, CV_INTER_LINEAR);
	return false;

}