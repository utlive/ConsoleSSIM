/*!
***********************************************************************
* \file MJ_SPNR.CPP
*
* \brief
*   kernel implementation of PSNR
*
* \note
*    It is not ssim family algorithms, just for comparison usage
*
* \author
*    - Ming-Jun Chen    <mjchen@utexas.edu>
***********************************************************************
*/

#include "MJ_PSNR.h"
#include "Common.h"
#include <cmath>
#include <algorithm>
#include <vector>
MJ_PSNRKL::MJ_PSNRKL(void)
{
		_DPRINTF(("MJ_PSNRKL  constructor"));
}

MJ_PSNRKL::~MJ_PSNRKL(void)
{
			_DPRINTF(("MJ_PSNRKL  destructor"));
}
bool MJ_PSNRKL::Init(const VFrameInfo vCurframe){

	if(vCurframe.nWidth<FRAME_SIZE_MIN || vCurframe.nHeight<FRAME_SIZE_MIN)
		return false;

	CSS_SSIMKL::Init(vCurframe);


	return true;
}

bool MJ_PSNRKL::ProcessOneFrame(const BYTE* pRef, const BYTE* pTest,double& dScore){


	if(m_PorcessFrameInfo.nWidth<FRAME_SIZE_MIN || m_PorcessFrameInfo.nHeight<FRAME_SIZE_MIN)
		return false;
	m_Width=m_PorcessFrameInfo.nWidth;

	m_Height=m_PorcessFrameInfo.nHeight;

	CTimeCost m_time;
	m_time.start();

	//
	GetEdgetMap((BYTE*)pRef,pRefGd);  
	ConvertEdge2Weight(pRefGd,pMeanRef);
	//

	int iCount(0);
	float fTemp1(0.0f),fTemp2(0.0f);
	int x,y;
	float fTMSE(0.0f);
	float fMaxI=255.0f;
	BYTE *pT1, *pT2;
	float *pQAMap,*pW;
	for(y=0;y<m_Height;y++){
		pT1=(BYTE*)pRef+(y*nAlignStride);
		pT2=(BYTE*)pTest+(y*nAlignStride);
		pQAMap = pQMap + (y*nAlignStride); // storing local QA map	
		pW = pMeanRef + (y*nAlignStride); // testing Weight in 

		for(x=0;x<m_Width;x++){
			fTMSE+=( (float)(*pT1)-(*pT2) )*(float)( (*pT1)-(*pT2) )*(*pW);
			float fT=sqrt(( (float)(*pT1)-(*pT2) )*(float)( (*pT1)-(*pT2) ));

			float fT1(0.0f);
			fT=std::max(fT,1.0f);
			fT1=20.0f*log10(fMaxI/fT);
			*pQAMap=fT1;
			pT1++;
			pT2++;
			pQAMap++;
			pW++;
		}
	}
	fTMSE/= ((float)m_Height*(float)m_Width);
	fTMSE=sqrt(fTMSE);
	fTMSE=std::max(fTMSE,0.01f);
	float fPSNR(0.0f);
	fPSNR=20.0f*log10(fMaxI/fTMSE);
//	fPSNR=fTMSE;  //calculate |x-x'|^2
	m_time.end();
	dScore=fPSNR;
//	dScore=fTMSE;


	return true;
}