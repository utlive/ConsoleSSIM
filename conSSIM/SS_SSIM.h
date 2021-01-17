/***********************************************************************

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
#pragma once

#include "UsingCV.h"
#include "IKL_SSIM.h"
#include "VideoFrame.h"
#include <vector>
const int FRAME_SIZE_MIN=12;
const int WINDOW_SZ=8;
const float WINDOW_SZ_SQ_INVERSE=1.0f/64.0f;

//#define _DUMP_IMAGE 1

class CSS_SSIMKL: public IKL_SSIM{
public:
	CSS_SSIMKL();
	virtual ~CSS_SSIMKL();
	virtual bool ProcessOneFrame(const BYTE* pRef, const BYTE* pTest,double& dScore);
	virtual bool SetFrameInfo(const VFrameInfo vCurFrame);
	virtual bool Init(const VFrameInfo vCurFrame);
	virtual float* GetQAMapPointer();

protected:
	int IsSupportSSE2;
	float C1, C2;
	float K1,K2,L1,L2;
	VFrameInfo m_PorcessFrameInfo; 
	int nSrcStride, nAlignStride,m_Width,m_Height,nPadStride,m_WidthPad,m_HeightPad;

	//Internal buffer 
	BYTE *pRefGd, *pTestGd;

	int iSumW;
	int *pRefSum, *pTestSum; // design for 8bits, image size < 2^11 * 2^11, be carefully of overflow if size is bigger than the design or data is not 8 bits
	float *pMeanRef,*pMeanTest,*pMeanMul;
	float *pMeanRefSq,*pMeanTestSq;
	float *pSigmaRef,*pSigmaTest,*pSigmaMul;
	float *pSigmaRefSq,*pSigmaTestSq;  
	float fLuminace, fContrast;

	// local Quality map
	float *pQMap;



	IplImage *m_pSrcYTmp;

	//functions
	bool CalculateFeatures( BYTE* pRef,  BYTE* pTest);
	bool GetIntegralMap(BYTE* pSrc , int* pDest);
	bool GetMeanMap(int* pSrc, float* pDest);
	bool GetVarMap(BYTE* pRef1 , float* pMean1, float* pVar1, BYTE* pRef2 , float* pMean2, float* pVar2, float* pSigmaMul); //Ref1 ,  var1, ref2, var2, sigmamul
	bool GetLuminance(BYTE* pRef1 , float* pMean1, float* pMeanSp1, BYTE* pRef2 , float* pMean2, float* pMeanSp2, float* pMul);
	bool GetGradientMap(IplImage* pSrc , BYTE* pDest);
	bool GetEdgetMap(BYTE* pSrc , BYTE* pDest);
	bool GetGradientVarMap(BYTE* pRef1 ,float* pVar1, BYTE* pRef2 , float* pVar2, float* pSigmaMul);
	void Copy2PaddingBuffer(BYTE* pSrc, BYTE* pDst, int pad);
	void ResetBuf();
	bool ConvertEdge2Weight(BYTE* pSrc , float* pDest);

	int buf_sz,buf_szpad;
	template<typename T1, typename T2>
	void GetMeanSqrtMap(T1* pSrc , T2* pDest){
		int x,y;
		T1 *pfSrc;
		T2 *pfDest;

		for(y=0;y<m_PorcessFrameInfo.nHeight-WINDOW_SZ;y++){
			pfSrc=pSrc+(y*nAlignStride);
			pfDest=pDest+(y*nAlignStride);

			for(x=0;x<m_PorcessFrameInfo.nWidth-WINDOW_SZ;x++){
				*pfDest = T2(*pfSrc) * T2(*pfSrc);
				pfDest++;
				pfSrc++;

			}
		}
	}
	template<typename T1, typename T2>
	void GetMultMap(T1* pSrc , T1* pSrc1, T2*pDest){
		int x,y;
		T1 *pfSrc,*pfSrc1;
		T2 *pfDest;

		for(y=0;y<m_PorcessFrameInfo.nHeight-WINDOW_SZ;y++){
			pfSrc=pSrc+(y*nAlignStride);
			pfSrc1=pSrc1+(y*nAlignStride);
			pfDest=pDest+(y*nAlignStride);

			for(x=0;x<m_PorcessFrameInfo.nWidth-WINDOW_SZ;x++){
				*pfDest = T2(*pfSrc) * T2(*pfSrc1);
				pfDest++;
				pfSrc++;
				pfSrc1++;

			}
		}
	}
	int iFrameCount;

};