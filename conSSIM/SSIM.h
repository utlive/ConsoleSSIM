#pragma once
/*!
***********************************************************************
* \file SSIM.H
*
* \brief
*   Container for ssim implementation. It was designed for dll in the GUI version application, but I revised it for directly accessible.
*
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



#include "IKL_SSIM.h"
#include "SS_SSIM.h"
#include "MS_SSIM.h"
#include "MJ_PSNR.h"
#include "MJ_Correlation.h"
#include "ThreadTool.h"

typedef bool (CALLBACK *PFNPAINTCALLBACK)();
enum AL_MODE{SSIM=0,MS_SSIM,PSNR, COR1};
#pragma once

class CSSIM
{
public:
	// Exports Functions
	CSSIM();
	virtual ~CSSIM();
	virtual int  test( BYTE* pRef,  BYTE* pTest, double& dScore);

	virtual bool SetFrameInfo(int nWitdh, int nHeight, int nStride,int nBitCount,DWORD fourCC=FOURCC_YV12 );
	virtual bool ProcessOneFrame(const BYTE* pRef, const BYTE* pTest, double& pScore);
	virtual bool ProcessFile(LPCTSTR pRef,LPCTSTR pTest, double& avScore);
	virtual bool getTime(float& fTempTime);
	virtual bool SetMode(const AL_MODE mode );
	virtual bool GetSSIMArray(std::vector<double>& _SsimArray);
	virtual bool SetDisplayParameters(PFNPAINTCALLBACK  pfnEventCallback, bool bShow);

	
	//
	static DWORD WINAPI MultiCore(LPVOID lpParam);
	float fTime;
protected:
	//all variables
	IKL_SSIM **pSSIMKL;

	VideoFrame m_CurFrame;
	void cvtYV12toRGB24(BYTE *pDst, BYTE *pSrc, int width, int height);
	bool ConvertYUV2PNG(LPCTSTR pFile,LPCTSTR pPath2Save, VideoFrame& m_frame);
	int m_mode;
	float m_fTime;
	std::vector<double> m_ssimarray;

	void setcolorConvertUYVY2YUV(int width, int high);
	void colorConvertUYVY2YUV(BYTE* pbuf,int width, int high, int index,BYTE** pbufRz,IplImage* ResizeSzImgRef);
	BYTE **pTempbuf;
	bool m_bShow;
private:

	typedef struct _ThreadParam
	{
		int nID;
		CSSIM *pContext;
		BYTE* pRef, *pTest;
		BYTE* pRefRz, *pTestRz;
		IplImage *RzImgRef,*RzImgTest;
		double dScore;
		bool bAbort;
		bool bRun;

	}ThreadParam;
	CThreadControl *m_threads;
	ThreadParam *m_tParam;   

	DWORD m_tNumber;

	//Internal Functions
};

