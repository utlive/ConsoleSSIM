/*!
***********************************************************************
* \fileIKL_SSIM.h
*
* \brief
*   Interfaces of kernels of SSIM families
*
* \note
*  
*
* \author
*    - Ming-Jun Chen    <mjchen@utexas.edu>
***********************************************************************
*/
#pragma once

#include <wtypes.h>
#include <time.h>
#include <string>
#include <tchar.h>
#include "VideoFrame.h"
class IKL_STEREO
{
public:
	virtual ~IKL_STEREO(){;};

	virtual bool ProcessOneFrame(const BYTE* pRefL, const BYTE* pTestL,const BYTE* pRefR, const BYTE* pTestR, double& dScore)=0;

	virtual bool SetFrameInfo(const VFrameInfo vCurFrame)=0;

	virtual bool Init(const VFrameInfo vCurFrame)=0;

	virtual bool CalculateDisparity(const BYTE* pL, const BYTE* pR, BYTE* pDepth)=0;

	virtual bool GenDisplayMap(BYTE* pSrc , BYTE* pDest, int scale)=0;

	virtual bool ProcessOneFrame(const BYTE* pRefL, const BYTE* pTestL,const BYTE* pRefR, const BYTE* pTestR, const BYTE* pRefDH, const BYTE* pTestDH, double& dScore)=0;

};