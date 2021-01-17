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

#pragma once

#include <wtypes.h>
#include <time.h>
#include <string>
#include <tchar.h>
#include "VideoFrame.h"
class IKL_SSIM 
{
public:
	/// \Process one frame
	/// \param[in] pRef, pTest
	/// \param[out] pScore
	/// \return true   Process success
	/// \return false  Non-support format
	virtual bool ProcessOneFrame(const BYTE* pRef, const BYTE* pTest, double& dScore)=0;

	virtual bool SetFrameInfo(const VFrameInfo vCurFrame)=0;

	/// \Allocate buffer must be called before ProcessOneFrame
	/// \param[in] vCurFrame
	/// \return true   Init success
	/// \return false  Non-support format
	virtual bool Init(const VFrameInfo vCurFrame)=0;

	// Get QA map buffer pointer, only availbe for SSIM algorithm, for others return NULL
	virtual float* GetQAMapPointer()=0;
	
	virtual ~IKL_SSIM(){;};
};