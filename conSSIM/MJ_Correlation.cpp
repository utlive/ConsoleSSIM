/*!
***********************************************************************
* \file MJ_SPNR.CPP
*
* \brief
*   kernel implementation of correlation
*
* \note
*    It is not ssim family algorithms, just for comparison usage

* \author
*    - Ming-Jun Chen    <mjchen@utexas.edu>
***********************************************************************

*/

#include "MJ_Correlation.h"
#include "Common.h"
#include <cmath>
#include <algorithm>
#include <vector>
MJ_CORRELATIONKL::MJ_CORRELATIONKL(void)
{
		_DPRINTF(("MJ_CORRELATIONKL  constructor"));
}

MJ_CORRELATIONKL::~MJ_CORRELATIONKL(void)
{
			_DPRINTF(("MJ_CORRELATIONKL  destructor"));
}
bool MJ_CORRELATIONKL::Init(const VFrameInfo vCurframe){

	if(vCurframe.nWidth<FRAME_SIZE_MIN || vCurframe.nHeight<FRAME_SIZE_MIN)
		return false;

	CSS_SSIMKL::Init(vCurframe);


	return true;
}

bool MJ_CORRELATIONKL::ProcessOneFrame(const BYTE* pRef, const BYTE* pTest,double& dScore){


	if(m_PorcessFrameInfo.nWidth<FRAME_SIZE_MIN || m_PorcessFrameInfo.nHeight<FRAME_SIZE_MIN)
		return false;
	m_Width=m_PorcessFrameInfo.nWidth;

	m_Height=m_PorcessFrameInfo.nHeight;

	CTimeCost m_time;
	m_time.start();


	int iCount(0);
	float fTemp1(0.0f),fTemp2(0.0f);
	int x,y;
	float fCor(0.0f);
	float fMaxI=255.0f;
	BYTE *pT1, *pT2;

	float *pMap;
	for(y=0;y<m_Height;y++){
		pT1=(BYTE*)pRef+(y*nAlignStride);
		pT2=(BYTE*)pTest+(y*nAlignStride);
		pMap=pQMap +(y*nAlignStride);
		for(x=0;x<m_Width;x++){
			float ft; 
			ft=sqrt(abs( pow( float(*pT1), 2) - pow( float(*pT2), 2)));
			ft/=255.0f;
			ft=1.0f-ft;
			fCor += ft;

			*pMap=ft;
			pT1++;
			pT2++;
			pMap++;
		}
	}
	fCor/= ((float)m_Height*(float)m_Width);

	m_time.end();
	dScore=fCor;


	return true;
}