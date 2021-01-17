/*!
***********************************************************************
* \file SS_SSIM.CPP
*
* \brief
*   kernel implementation of Fast-SSIM, please refer to my paper for my paper  
*  Ming-Jun Chen and Alan C. Bovik, “Fast structural similarity index algorithm”, Journal of Real-Time Image Processing, pp. 1-7. August, 2010
* \note
*  
*
* \author
*    - Ming-Jun Chen    <mjchen@utexas.edu>
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

#include "SS_SSIM.h"
#include "Common.h"
#include <cmath>
#include <emmintrin.h>
#include "cpuid.h"

//Configurations 
//#define USE_G_WINDOW 1

//#define _NO_SSE2 1
//#define _ORI_SSIM_LUMINANCE 1
//#define _ORI_SSIM_VARIANCE 1
//#define _DUMP_SSIM_MAP 1
//#define WRITE_LV 1



// BYTE W[]={
//0,0,0,1,1,0,0,0  
//,0,0,1,2,2,1,0,0
//,0,1,2,4,4,2,1,0
//,1,2,4,8,8,4,2,1
//,1,2,4,8,8,4,2,1
//,0,1,2,4,4,2,1,0
//,0,0,1,2,2,1,0,0
//,0,0,0,1,1,0,0,0 };

 BYTE W[]={
1,1,1,1,1,1,1,1  
,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1 };

// BYTE W[]={
//0,0,0,1,1,0,0,0  
//,0,0,1,2,2,1,0,0
//,0,1,2,4,4,2,1,0
//,1,2,4,8,8,4,2,1
//,1,2,4,8,8,4,2,1
//,0,1,2,4,4,2,1,0
//,0,0,1,2,2,1,0,0
//,0,0,0,1,1,0,0,0 };

CSS_SSIMKL::CSS_SSIMKL(){
	_DPRINTF(("CSS_SSIMKL base constructor"));
	//C1 = 6.5025f, C2 = 12.5225f; //0.6921  //ms-ssim 0.75
	C1 = 6.5025f, C2 = 24.5225f; //

	nAlignStride= nSrcStride=0;

	pRefSum=pTestSum=NULL;
	pMeanRef=pMeanTest=pMeanRefSq=pMeanTestSq=pSigmaRefSq=pSigmaTestSq=NULL; 
	pSigmaMul=pMeanMul=NULL;
	m_pSrcYTmp=NULL;
	pRefGd=pTestGd=NULL;
	pSigmaRef=pSigmaTest=NULL;

	pQMap=NULL;

	iSumW=0;
	for(int i=0;i<WINDOW_SZ*WINDOW_SZ;i++){
		iSumW+=W[i];
	}
#if _NO_SSE2
	IsSupportSSE2=false;
#else
	_p_info info;

	_cpuid(&info);
	IsSupportSSE2= info.feature&_CPU_FEATURE_SSE2;
#endif
	_DPRINTF(("support SSE2 %d",IsSupportSSE2));

}
CSS_SSIMKL::~CSS_SSIMKL(){
	_DPRINTF(("CSS_SSIMKL base destructor"));
	_ALIGNED_FREE_PTR(pRefSum);
	_ALIGNED_FREE_PTR(pTestSum);

	_ALIGNED_FREE_PTR(pMeanRef);
	_ALIGNED_FREE_PTR(pMeanTest);
	_ALIGNED_FREE_PTR(pMeanRefSq);
	_ALIGNED_FREE_PTR(pMeanTestSq);
	_ALIGNED_FREE_PTR(pSigmaRef);
	_ALIGNED_FREE_PTR(pSigmaTest);
	_ALIGNED_FREE_PTR(pSigmaRefSq);
	_ALIGNED_FREE_PTR(pSigmaTestSq);
	_ALIGNED_FREE_PTR(pSigmaMul);
	_ALIGNED_FREE_PTR(pMeanMul);
	_ALIGNED_FREE_PTR(pQMap);
	
	_ALIGNED_FREE_PTR(pRefGd);
	_ALIGNED_FREE_PTR(pTestGd);



	cvReleaseImage(&m_pSrcYTmp);
}
void CSS_SSIMKL::ResetBuf(){
	memset(pMeanRef,0,sizeof(float)*buf_sz);
	memset(pMeanTest,0,sizeof(float)*buf_sz);
	memset(pMeanRefSq,0,sizeof(float)*buf_sz);
	memset(pMeanTestSq,0,sizeof(float)*buf_sz);
	memset(pSigmaRef,0,sizeof(float)*buf_sz);
	memset(pSigmaTest,0,sizeof(float)*buf_sz);
	memset(pSigmaRefSq,0,sizeof(float)*buf_sz);
	memset(pSigmaTestSq,0,sizeof(float)*buf_sz);
	memset(pMeanMul,0,sizeof(float)*buf_sz);
	memset(pSigmaMul,0,sizeof(float)*buf_sz);
	memset(pQMap,0,sizeof(float)*buf_sz);

}
bool CSS_SSIMKL::Init(const VFrameInfo vCurframe){

	if(vCurframe.nWidth<FRAME_SIZE_MIN || vCurframe.nHeight<FRAME_SIZE_MIN)
		return false;

	m_PorcessFrameInfo=vCurframe;
	nSrcStride=vCurframe.nStride;
//	m_PorcessFrameInfo.nStride=(((vCurframe.nWidth-1)/8)+1)*8;
	m_PorcessFrameInfo.nStride=nSrcStride;
	nAlignStride=m_PorcessFrameInfo.nStride;
	 buf_sz=m_PorcessFrameInfo.nHeight*m_PorcessFrameInfo.nStride;
	 buf_szpad=(m_PorcessFrameInfo.nHeight+WINDOW_SZ)*(m_PorcessFrameInfo.nStride+WINDOW_SZ);
	nPadStride=nAlignStride+WINDOW_SZ;





	_ALIGNED_MALLOC_PTR(pRefGd,BYTE,buf_sz);
	_ALIGNED_MALLOC_PTR(pTestGd,BYTE,buf_sz);
	memset(pRefGd,0,sizeof(BYTE)*buf_sz);
	memset(pTestGd,0,sizeof(BYTE)*buf_sz);



	_ALIGNED_MALLOC_PTR(pRefSum,int,buf_sz);
	_ALIGNED_MALLOC_PTR(pTestSum,int,buf_sz);

	memset(pRefSum,0,sizeof(int)*buf_sz);
	memset(pTestSum,0,sizeof(int)*buf_sz);

	_ALIGNED_MALLOC_PTR(pMeanRef,float,buf_sz);
	_ALIGNED_MALLOC_PTR(pMeanTest,float,buf_sz);
	_ALIGNED_MALLOC_PTR(pMeanRefSq,float,buf_sz);
	_ALIGNED_MALLOC_PTR(pMeanTestSq,float,buf_sz);
	_ALIGNED_MALLOC_PTR(pSigmaRef,float,buf_sz);
	_ALIGNED_MALLOC_PTR(pSigmaTest,float,buf_sz);
	_ALIGNED_MALLOC_PTR(pSigmaRefSq,float,buf_sz);
	_ALIGNED_MALLOC_PTR(pSigmaTestSq,float,buf_sz);
	_ALIGNED_MALLOC_PTR(pSigmaMul,float,buf_sz);
	_ALIGNED_MALLOC_PTR(pMeanMul,float,buf_sz);
	_ALIGNED_MALLOC_PTR(pQMap,float,buf_sz);



	if(m_pSrcYTmp!=NULL)
		cvReleaseImage(&m_pSrcYTmp);
	m_pSrcYTmp=cvCreateImage(cvSize(m_PorcessFrameInfo.nWidth, m_PorcessFrameInfo.nHeight), IPL_DEPTH_8U, 1);

	iFrameCount=0;
	return true;
}
bool CSS_SSIMKL::ProcessOneFrame(const BYTE* pRef, const BYTE* pTest,double& dScore){


	if(m_PorcessFrameInfo.nWidth<FRAME_SIZE_MIN || m_PorcessFrameInfo.nHeight<FRAME_SIZE_MIN)
		return false;
	
	m_Width=m_PorcessFrameInfo.nWidth;
	m_Height=m_PorcessFrameInfo.nHeight;

#ifdef _DUMP_IMAGE

	_TCHAR fname[256];


	iFrameCount++;
	_stprintf(fname, "C:\\dump\\Ref_Frame%04d.jpg", iFrameCount);
	mj_SaveBufferToImage<BYTE>(fname,(BYTE*)pRef,m_Width,m_Height);
	_stprintf(fname, "C:\\dump\\Test_Frame%04d.jpg", iFrameCount);
	mj_SaveBufferToImage<BYTE>(fname,(BYTE*)pTest,m_Width,m_Height);

#endif
	CTimeCost m_time;
	m_time.start();
#ifdef _DUMP_SSIM_MAP
	BYTE* pDump(NULL);
	_ALIGNED_MALLOC_PTR(pDump,BYTE,buf_sz);
	memset(pDump,0,sizeof(BYTE)*buf_sz);
#endif 
	
	ResetBuf();
	memset(pRefSum,0,sizeof(int)*buf_sz);
	memset(pTestSum,0,sizeof(int)*buf_sz);

	//_DPRINTF(("SSIM format width %d height%d ",m_Width,m_Height));

	int x,y,m,n;
	CalculateFeatures((BYTE*)pRef,(BYTE*)pTest);
#ifdef _TEST_SSIM 
	for(y=0;y<m_Height-WINDOW_SZ+1;y++){
		for(x=0;x<m_Width-WINDOW_SZ+1;x++){
			float fT1,fT2,fTSigmaMul,fTM1,fTM2;
			fTM1=0.0f;
			fTM2=0.0f;
			fT1=0.0f;
			fT2=0.0f;
			fTSigmaMul=0.0f;



			//////////////////////////////////////////////////////////////////////////	
			// windowing
			for(m=0;m<WINDOW_SZ;m++){
				for(n=0;n<WINDOW_SZ;n++){
					fTM1+=pRef[ ((m+y)*nAlignStride)+n+x];
					fTM2+=pTest[ ((m+y)*nAlignStride)+n+x];
				}
			}
			//////////////////////////////////////////////////////////////////////////

			fTM1=fTM1*WINDOW_SZ_SQ_INVERSE;
			fTM2=fTM2*WINDOW_SZ_SQ_INVERSE;

			pMeanRef[ ((y)*nAlignStride)+x]=fTM1;
			pMeanTest[ ((y)*nAlignStride)+x]=fTM2;

			pMeanRefSq[ ((y)*nAlignStride)+x]=fTM1*fTM1;
			pMeanTestSq[ ((y)*nAlignStride)+x]=fTM2*fTM2;

			pMeanMul[ ((y)*nAlignStride)+x]=fTM1*fTM2;

			//if(x==10&&y<2){
			//	_DPRINTF(("SSIM Mean width %04d %d height%04d  %d mean 1%f mean 2%f  stride%d ",x,m_Width,y,m_Height,pMeanRef[ ((y)*nAlignStride)+x],pMeanTest[ ((y)*nAlignStride)+x],nAlignStride));
			//	//_DPRINTF((" pMean %d pMean2 %d  ",pTMean1,pTMean2));
			//}
			//_DPRINTF(("mean %02d %02d ref %4.2f",x,y,pMeanRef[ ((y)*m_Width)+x]));

		}
	}
	for(y=0;y<m_Height-WINDOW_SZ+1;y++){

		for(x=0;x<m_Width-WINDOW_SZ+1;x++){
			float fT1,fT2,fTSigmaMul,fTM1,fTM2,fVar2,fVar1;
			fTM1=0.0f;
			fTM2=0.0f;
			fT1=0.0f;
			fT2=0.0f;
			fTSigmaMul=0.0f;
			fVar2=fVar1=0.0f;



			//////////////////////////////////////////////////////////////////////////	
			// windowing
			for(m=0;m<WINDOW_SZ;m++){
				for(n=0;n<WINDOW_SZ;n++){
					//fT1=pRef[ ((m+y)*nAlignStride)+n+x]-pMeanRef[ ((y)*nAlignStride)+x];
					//fT2=pTest[ ((m+y)*nAlignStride)+n+x]-pMeanTest[ ((y)*nAlignStride)+x];
					//fTSigmaMul+=fabs(fT1*fT2);

					fT1+=pRef[ ((m+y)*nAlignStride)+n+x]*pRef[ ((m+y)*nAlignStride)+n+x];
					fT2+=pTest[ ((m+y)*nAlignStride)+n+x]*pTest[ ((m+y)*nAlignStride)+n+x];
					fTSigmaMul+=pRef[ ((m+y)*nAlignStride)+n+x]*pTest[ ((m+y)*nAlignStride)+n+x];
					//	fTSigmaMul+=(fT1*fT2);
					//	fTSigmaMul+= (float)(*pWinRef1) * (float)(*pWinRef2) ;
					//fVar1+=fT1*fT1;
					//fVar2+=fT2*fT2;

				}
			}
			//////////////////////////////////////////////////////////////////////////
			pSigmaMul[ ((y)*nAlignStride)+x]  = fTSigmaMul*WINDOW_SZ_SQ_INVERSE - pMeanMul[ ((y)*nAlignStride)+x];


			//	*pTSigmaMul = fTSigmaMul-((*pTMean1)*(*pTMean2));
			pSigmaRefSq[ ((y)*nAlignStride)+x] =fT1*WINDOW_SZ_SQ_INVERSE - pMeanRefSq[ ((y)*nAlignStride)+x];
			pSigmaTestSq[ ((y)*nAlignStride)+x] =fT2*WINDOW_SZ_SQ_INVERSE - pMeanTestSq[ ((y)*nAlignStride)+x];



		}
	}

	_TCHAR fname[256];
	_stprintf(fname, "C:\\dump\\Ref_mean_%04d.jpg", m_Width);
	mj_SaveBufferToImage<float>(fname,pMeanRefSq,m_Width,m_Height);
	_stprintf(fname, "C:\\dump\\test_mean_%04d.jpg", m_Width);
	mj_SaveBufferToImage<float>(fname,pMeanTestSq,m_Width,m_Height);
	_stprintf(fname, "C:\\dump\\test_mean12_%04d.jpg", m_Width);
	mj_SaveBufferToImage<float>(fname,pMeanMul,m_Width,m_Height);

	_stprintf(fname, "C:\\dump\\Ref_var_%04d.jpg", m_Width);
	mj_SaveBufferToImage<float>(fname,pSigmaRefSq,m_Width,m_Height);
	_stprintf(fname, "C:\\dump\\test_var_%04d.jpg", m_Width);
	mj_SaveBufferToImage<float>(fname,pSigmaTestSq,m_Width,m_Height);
	_stprintf(fname, "C:\\dump\\test_var12_%04d.jpg", m_Width);
	mj_SaveBufferToImage<float>(fname,pSigmaMul,m_Width,m_Height);

	_stprintf(fname, "C:\\dump\\1Ref_lu_%04d.jpg", m_Width);
	mj_SaveBufferToImage<BYTE>(fname,(BYTE*)pRef,m_Width,m_Height);
	_stprintf(fname, "C:\\dump\\1test_lu_%04d.jpg", m_Width);
	mj_SaveBufferToImage<BYTE>(fname,(BYTE*)pTest,m_Width,m_Height);
#endif
	int iCount(0);
	float fTemp1(0.0f),fTemp2(0.0f);
	//int x,y;
	float fTempSSIM(0.0f),fLuminace(0.0f),fVar(0.0f);
	float *pTVar1, *pTVar2, *pTSigmaMul, *pTMeanSq1, *pTMeanSq2, *pTMeanMul;
	float *pQAMap;
#ifdef _DUMP_SSIM_MAP
	BYTE *pDumpTemp;
#endif


	iCount=0;
	fLuminace=fVar=fTempSSIM=0.0f;
	for(y=0;y<m_Height-WINDOW_SZ+1;y++){
		pTSigmaMul=pSigmaMul+(y*nAlignStride);
		pTMeanMul=pMeanMul+(y*nAlignStride);
		pTMeanSq1=pMeanRefSq+(y*nAlignStride);
		pTMeanSq2=pMeanTestSq+(y*nAlignStride);
		pTVar1=pSigmaRefSq+(y*nAlignStride);
		pTVar2=pSigmaTestSq+(y*nAlignStride);

		pQAMap = pQMap + (y*nAlignStride); // storing local QA map

#ifdef _DUMP_SSIM_MAP
		pDumpTemp=pDump+(y*nAlignStride);
#endif
		for(x=0;x<m_Width-WINDOW_SZ+1;x++){
			iCount++;

			fTemp1=( 2 * (*pTMeanMul)+C1) /( (*pTMeanSq1) + (*pTMeanSq2)+C1) ;
			fTemp2=( 2 * (*pTSigmaMul)+C2 ) /( (*pTVar1) + (*pTVar2)+C2);
			fLuminace+=fTemp1;
			fVar+=fTemp2;

			fTempSSIM+=(fTemp1*fTemp2);
			*pQAMap=(fTemp1*fTemp2);


#ifdef _DUMP_SSIM_MAP
			*pDumpTemp=255*(1-(fTemp1*fTemp2));
			pDumpTemp++;
#endif
			pTMeanMul++;
			pTMeanSq1++;
			pTMeanSq2++;
			pTSigmaMul++;
			pTVar1++;
			pTVar2++;
			pQAMap++;


		}
	}


#ifdef _DUMP_SSIM_MAP
	//_TCHAR fname[256];
	_stprintf(fname, "C:\\dump\\SSIMMAP%04d.jpg", iFrameCount);
	mj_SaveBufferToImage<BYTE>(fname,(BYTE*)pDump,m_Width,m_Height);
	_ALIGNED_FREE_PTR(pDump);

#endif


	fTempSSIM/=(float)iCount;
	fLuminace/=(float)iCount;
	fVar/=(float)iCount;

	//_DPRINTF((" SSIM  %f %d ",fTempSSIM,iCount));
#ifdef WRITE_LV

	FILE *fp2=fopen("E://Luminance.txt","a+");

	fprintf(fp2,"%f ,",fLuminace);
	fclose(fp2);     
	fp2=fopen("E://Var.txt","a+");

	fprintf(fp2,"%f ,",fVar);
	fclose(fp2);     
#endif

	m_time.end();
	dScore=fTempSSIM;
//	_DPRINTF(("SSIM block %d score %f  time %f",iCount,fTempSSIM,m_time.get_seconds()));

	return true;
}
bool CSS_SSIMKL::CalculateFeatures( BYTE* pRef,  BYTE* pTest){




#if _ORI_SSIM_LUMINANCE
	GetLuminance(pRef,pMeanRef,pMeanRefSq,pTest,pMeanTest,pMeanTestSq,pMeanMul);
	//_TCHAR fname[256];
	//_stprintf(fname, "C:\\dump\\Ref_mean_%04d.jpg", m_Width);
	//mj_SaveBufferToImage<float>(fname,pMeanRef,m_Width,m_Height);
	//_stprintf(fname, "C:\\dump\\test_mean_%04d.jpg", m_Width);
	//mj_SaveBufferToImage<float>(fname,pMeanTest,m_Width,m_Height);

	//_stprintf(fname, "C:\\dump\\Ref_lu_%04d.jpg", m_Width);
	//mj_SaveBufferToImage<BYTE>(fname,pRef,m_Width,m_Height);
	//_stprintf(fname, "C:\\dump\\test_lu_%04d.jpg", m_Width);
	//mj_SaveBufferToImage<BYTE>(fname,pTest,m_Width,m_Height);
#else
	//////////////////////////////////////////////////////////////////////////
	//Pre-process  calculate Integral images
	//////////////////////////////////////////////////////////////////////////	
	GetIntegralMap((BYTE*)pRef,pRefSum);
	GetIntegralMap((BYTE*)pTest,pTestSum);


	//////////////////////////////////////////////////////////////////////////
	//  Calculate mean
	//////////////////////////////////////////////////////////////////////////

	GetMeanMap(pRefSum,pMeanRef);
	GetMeanMap(pTestSum,pMeanTest);	

	// Mean Sqrt
	GetMeanSqrtMap<float,float>(pMeanRef,pMeanRefSq);
	GetMeanSqrtMap<float,float>(pMeanTest,pMeanTestSq);
	GetMultMap<float,float>(pMeanRef,pMeanTest,pMeanMul);
#endif



#if _ORI_SSIM_VARIANCE
	GetVarMap((BYTE*)pRef,pMeanRef,pSigmaRefSq,(BYTE*)pTest,pMeanTest, pSigmaTestSq,pSigmaMul);
#else
	GetEdgetMap(pRef,pRefGd);


#ifdef _DUMP_IMAGE
	_TCHAR fname[256];


	//iFrameCount++;
	_stprintf(fname, "C:\\dump\\Ref_gradient1_%04d.jpg", m_Width);
	mj_SaveBufferToImage<BYTE>(fname,pRefGd,m_Width,m_Height);
#endif


	GetEdgetMap(pTest,pTestGd);

#ifdef _DUMP_IMAGE
	_stprintf(fname, "C:\\dump\\test_gradient1_%04d.jpg", m_Width);
	mj_SaveBufferToImage<BYTE>(fname,pTestGd,m_Width,m_Height);
#endif

	GetGradientVarMap(pRefGd,pSigmaRefSq,pTestGd, pSigmaTestSq,pSigmaMul); //Fast-SSIM
#endif

	return true;
}
bool CSS_SSIMKL:: SetFrameInfo(const VFrameInfo vCurFrame){
	if(m_PorcessFrameInfo.nWidth<FRAME_SIZE_MIN || m_PorcessFrameInfo.nHeight<FRAME_SIZE_MIN)
		return false;

	m_PorcessFrameInfo=vCurFrame;

	return true;
}
bool CSS_SSIMKL::GetIntegralMap(BYTE* pSrc , int* pDest){
	int x,y;

	int *pTSumCur,*pTUp,*pTLeft,*pTUpLeft;
	BYTE* pTSrc ;

	pTSrc=pSrc;
	pTLeft=pTSumCur=pDest;


	*pTSumCur = *pTSrc;

	for(x=1;x<m_Width;x++){
		pTSumCur++;
		pTSrc++;
		*pTSumCur = *pTSrc;
		*pTSumCur += *pTLeft ;
		pTLeft++;


	}
	for(y=1;y<m_Height;y++){
		pTSrc = pSrc+nSrcStride*y;

		pTUpLeft=pTUp=(pDest+nAlignStride*(y-1));
		pTLeft=pTSumCur=(pDest+nAlignStride*y);


		*pTSumCur = *pTSrc + *pTUp;

		for(x=1;x<m_Width;x++){
			
			pTSrc++;
			pTSumCur++;
			pTUp++;

			*pTSumCur = *pTSrc;
			*pTSumCur += *pTLeft ;
			*pTSumCur += *pTUp ;
			*pTSumCur -= *pTUpLeft ;

			pTLeft++;
			pTUpLeft++;


		}
	}
	return true;
}

bool CSS_SSIMKL::GetMeanMap(int* pSrc , float* pDest){
	int *pBL,*pTL, *pTR, *pBR;
	float *pMeanTemp;

	int x,y;
	float fWeight=float(WINDOW_SZ*WINDOW_SZ);
	for(y=0;y<m_Height-WINDOW_SZ;y++){
		pTL=pSrc+(y*nAlignStride);
		pTR=pTL+WINDOW_SZ;
		pBL=pTL+(nAlignStride*WINDOW_SZ);
		pBR=pBL+WINDOW_SZ;
		pMeanTemp=pDest+(y*nAlignStride);

		for(x=0;x<m_Width-WINDOW_SZ;x++){
			*pMeanTemp=( *pBR + *pTL - *pTR - *pBL) /fWeight;
			pMeanTemp++;
			pBR++;
			pTL++;
			pTR++;
			pBL++;

		}
	}

	return true;
}

bool CSS_SSIMKL::GetGradientVarMap(BYTE* pRef1 , float* pVar1, BYTE* pRef2 , float* pVar2, float* pSigmaMul){

	BYTE *pTRef1, *pTRef2,*pWinRef1,*pWinRef2;
	BYTE *pWindow;
	float *pTVar1, *pTVar2, *pTSigmaMul;

	int x,y,m,n;
	__asm emms;
	const bool bSSE2(IsSupportSSE2); // decided in constructing time
	for(y=0;y<m_Height-WINDOW_SZ+1;y++){
		pTRef1=pRef1+(y*nAlignStride);
		pTRef2=pRef2+(y*nAlignStride);
		pTVar1=pVar1+(y*nAlignStride);
		pTVar2=pVar2+(y*nAlignStride);
		pTSigmaMul=pSigmaMul+(y*nAlignStride);

		for(x=0;x<m_Width-WINDOW_SZ+1;x++){
			int iVar1(0),iVar2(0),iSigmamul(0);
			int iT1,iT2;
			//////////////////////////////////////////////////////////////////////////	
			// windowing
			pWindow=W;
			for(m=0;m<WINDOW_SZ;m++){
				pWinRef1=pTRef1+(m*nAlignStride);
				pWinRef2=pTRef2+(m*nAlignStride);
				pWindow=&W[m*8];

				if(bSSE2){

					__m128i RefVal, TestVal, WinVal;
					__m128i temp1; 
					__m128i Zero;

					Zero = _mm_setzero_si128();
					RefVal = _mm_loadl_epi64((__m128i*)pWinRef1);
					RefVal = _mm_unpacklo_epi8(RefVal, Zero);
					WinVal = _mm_loadl_epi64((__m128i*)pWindow );
					WinVal = _mm_unpacklo_epi8(WinVal, Zero);

					RefVal=_mm_mullo_epi16(RefVal,WinVal);  //windowing

					TestVal = _mm_loadl_epi64((__m128i*)pWinRef2);
					TestVal = _mm_unpacklo_epi8(TestVal, Zero);

					TestVal=_mm_mullo_epi16(TestVal,WinVal); //windowing

					int* pTemp;
					temp1=_mm_madd_epi16(RefVal,RefVal);
					pTemp=(int*)&temp1;
					iVar1 += *pTemp;
					pTemp++;
					iVar1 += *pTemp;
					pTemp++;
					iVar1 += *pTemp;
					pTemp++;
					iVar1 += *pTemp;





					temp1=_mm_madd_epi16(TestVal,TestVal);
					pTemp=(int*)&temp1;
					iVar2 += *pTemp;
					pTemp++;
					iVar2 += *pTemp;
					pTemp++;
					iVar2 += *pTemp;
					pTemp++;
					iVar2 += *pTemp;
					temp1=_mm_madd_epi16(RefVal,TestVal);
					pTemp=(int*)&temp1;
					iSigmamul += *pTemp;
					pTemp++;
					iSigmamul += *pTemp;
					pTemp++;
					iSigmamul += *pTemp;
					pTemp++;
					iSigmamul += *pTemp;
				}
				else{
				for(n=0;n<WINDOW_SZ;n++){
#ifdef USE_G_WINDOW
					iT1=(*pWinRef1++)*(*pWindow);
					iT2=(*pWinRef2++)*(*pWindow++);
#else
					iT1=(*pWinRef1++);
					iT2=(*pWinRef2++);
#endif
					iSigmamul+=iT1*iT2; 
					iVar1+=iT1*iT1;
					iVar2+=iT2*iT2;

				}
			}

			}
			//////////////////////////////////////////////////////////////////////////
			*pTSigmaMul = iSigmamul*WINDOW_SZ_SQ_INVERSE;
			*pTVar1 =iVar1*WINDOW_SZ_SQ_INVERSE;
			*pTVar2 =iVar2*WINDOW_SZ_SQ_INVERSE;


			pTRef1++;
			pTRef2++;
			pTVar1++;
			pTVar2++;
			pTSigmaMul++;

		}
	}

	return true;
}
bool CSS_SSIMKL::GetLuminance(BYTE* pRef1 , float* pMean1, float* pMeanSp1, BYTE* pRef2 , float* pMean2, float* pMeanSp2, float* pMul){

	BYTE *pTRef1, *pTRef2,*pWinRef1,*pWinRef2;

	BYTE *pWindow;

	float *pTMean1Sp, *pTMean2Sp, *pTSigmaMul, *pTMean1, *pTMean2;

	pTRef1=pTRef2=pWinRef1=pWinRef2=NULL;

	pTMean1Sp=pTMean2Sp=pTSigmaMul=pTMean1=pTMean2=NULL;
	float fMulSum,fMul;

	short T1,T2;
	int x,y,m,n;


	for(y=0;y<m_Height-WINDOW_SZ+1;y++){
		pTRef1=pRef1+(y*nAlignStride);
		pTRef2=pRef2+(y*nAlignStride);
		pTMean1=pMean1+(y*nAlignStride);
		pTMean2=pMean2+(y*nAlignStride);
		pTMean1Sp=pMeanSp1+(y*nAlignStride);
		pTMean2Sp=pMeanSp2+(y*nAlignStride);
		pTSigmaMul=pMul+(y*nAlignStride);

		for(x=0;x<m_Width-WINDOW_SZ+1;x++){
			float fT1,fT2,fTSigmaMul,fTM1,fTM2;
			fTM1=0.0f;
			fTM2=0.0f;
			fT1=0.0f;
			fT2=0.0f;
			fTSigmaMul=0.0f;


#ifdef USE_G_WINDOW
			pWindow=W;
#endif
			//////////////////////////////////////////////////////////////////////////	
			// windowing
			for(m=0;m<WINDOW_SZ;m++){
				pWinRef1=pTRef1+(m*nAlignStride);
				pWinRef2=pTRef2+(m*nAlignStride);
				for(n=0;n<WINDOW_SZ;n++){
					T1=(*pWinRef1);
					T2=(*pWinRef2);
#ifdef USE_G_WINDOW
					fT1= ((float)(*pWinRef1) )*(float)(*pWindow);
					fT2= ((float)(*pWinRef2) )*(float)(*pWindow);
#else

					fT1= ((float)T1);
					fT2= ((float)T2);
#endif
					pWinRef1++;
					pWinRef2++;


					fTM1+=fT1;
					fTM2+=fT2;

#ifdef USE_G_WINDOW
					pWindow++;
#endif
				}
			}
			//////////////////////////////////////////////////////////////////////////

			fTM1=fTM1*WINDOW_SZ_SQ_INVERSE;
			fTM2=fTM2*WINDOW_SZ_SQ_INVERSE;

			*pTMean1=fTM1;
			*pTMean2=fTM2;

			*pTMean1Sp =fTM1*fTM1;
			*pTMean2Sp =fTM2*fTM2;

			*pTSigmaMul=fTM1*fTM2;


			pTRef1++;
			pTRef2++;
			pTMean1++;
			pTMean2++;
			pTMean1Sp++;
			pTMean2Sp++;
			pTSigmaMul++;

		}
	}


	return true;
}
bool CSS_SSIMKL::GetVarMap(BYTE* pRef1 , float* pMean1, float* pVar1, BYTE* pRef2 , float* pMean2, float* pVar2, float* pSigmaMul){

	BYTE *pTRef1, *pTRef2,*pWinRef1,*pWinRef2;
	BYTE *pWindow;

	float *pTVar1(NULL), *pTVar2, *pTSigmaMul, *pTMean1, *pTMean2;

	pTRef1=pTRef2=pWinRef1=pWinRef2=NULL;
	pTVar1=pTVar2=pTSigmaMul=pTMean1=pTMean2=NULL;
	int x,y,m,n;

	for(y=0;y<m_Height-WINDOW_SZ+1;y++){
		pTRef1=pRef1+(y*nAlignStride);
		pTRef2=pRef2+(y*nAlignStride);
		pTMean1=pMean1+(y*nAlignStride);
		pTMean2=pMean2+(y*nAlignStride);
		pTVar1=pVar1+(y*nAlignStride);
		pTVar2=pVar2+(y*nAlignStride);
		pTSigmaMul=pSigmaMul+(y*nAlignStride);

		for(x=0;x<m_Width-WINDOW_SZ+1;x++){
			float fT1,fT2,fTSigmaMul,fTM1,fTM2,fVar1,fVar2;
			fTM1=(*pTMean1);
			fTM2=(*pTMean2);
			fT1=0.0f;
			fT2=0.0f;
			fTSigmaMul=0.0f;
			fVar1=0.0f;
			fVar2=0.0f;

#ifdef USE_G_WINDOW
			pWindow=W;
#endif


			//////////////////////////////////////////////////////////////////////////	
			// windowing
			for(m=0;m<WINDOW_SZ;m++){
				pWinRef1=pTRef1+(m*nAlignStride);
				pWinRef2=pTRef2+(m*nAlignStride);
				for(n=0;n<WINDOW_SZ;n++){
#ifdef USE_G_WINDOW
					fT1= ((float)(*pWinRef1) - fTM1)*(float)(*pWindow);
					fT2= ((float)(*pWinRef2) - fTM2)*(float)(*pWindow);
#else
					fT1= ((float)(*pWinRef1) - fTM1);
					fT2= ((float)(*pWinRef2) - fTM2);
#endif



				//	fTSigmaMul+=fabs(fT1*fT2);
					fTSigmaMul+=(fT1*fT2); //correct one
				//	fTSigmaMul+= (float)(*pWinRef1) * (float)(*pWinRef2) ;
					fVar1+=fT1*fT1;
					fVar2+=fT2*fT2;

					pWinRef1++;
					pWinRef2++;
#ifdef USE_G_WINDOW
					pWindow++;
#endif
				}
			}
			//////////////////////////////////////////////////////////////////////////
			*pTSigmaMul  = fTSigmaMul*WINDOW_SZ_SQ_INVERSE;


		//	*pTSigmaMul = fTSigmaMul-((*pTMean1)*(*pTMean2));
			*pTVar1 =fVar1*WINDOW_SZ_SQ_INVERSE;
			*pTVar2 =fVar2*WINDOW_SZ_SQ_INVERSE;

			pTRef1++;
			pTRef2++;
			pTMean1++;
			pTMean2++;
			pTVar1++;
			pTVar2++;
			pTSigmaMul++;

		}
	}
	
	return true;
}
bool CSS_SSIMKL::GetGradientMap(IplImage* pSrc , BYTE* pDest){

	IplImage *sobelDstX = cvCreateImage(cvSize(m_Width, m_Height), IPL_DEPTH_16S, 1);
	IplImage *sobelDstY = cvCreateImage(cvSize(m_Width, m_Height), IPL_DEPTH_16S, 1);

	cvSobel( pSrc, sobelDstX, 1, 0, 3);
	cvSobel( pSrc, sobelDstY, 0, 1, 3);


	short *pSrcX = (short*)(sobelDstX->imageData);
	short *pSrcY = (short*)(sobelDstY->imageData);
	BYTE *pDst = pDest;

	int i,j;
	for(i = 0; i < m_Height; i++)
	{
		for(j = 0; j < m_Width; j++)
		{
			int res = abs(pSrcX[j]) + abs(pSrcY[j]);
			pDst[j] = std::min(res, 255);
		}
		pSrcX += sobelDstX->widthStep/2;
		pSrcY += sobelDstY->widthStep/2;
		pDst += nAlignStride;
	}
	cvReleaseImage(&sobelDstX);
	cvReleaseImage(&sobelDstY);
	return true;
}
bool CSS_SSIMKL::GetEdgetMap(BYTE* pSrc , BYTE* pDest){




	BYTE *pSrcT,*pDst,*pSrcH,*pSrcV;

	int i,j;
	for(i = 0; i < m_Height-2; i++)
	{
		pSrcT =pSrc+(i+1)* nAlignStride+1;
		pSrcH =pSrc+(i)* nAlignStride;
		pSrcV = pSrcH+2;
		pDst = pDest+i* nAlignStride;
		for(j = 0; j < m_Width-2; j++)
		{
			int T1((*pSrcH)-(*pSrcT));
			int T2((*pSrcV)-(*pSrcT));
			int res = std::max( abs(T1) , abs(T2) )+ std::min( abs( T1) , abs(T2))/4;
		//	(*pDst)= res*4/5;

			*pDst = std::min(res,255);
			pDst++;			
			pSrcT ++;
			pSrcH ++;
			pSrcV++;
		}

	}

	return true;
}
void CSS_SSIMKL::Copy2PaddingBuffer(BYTE* pSrc, BYTE* pDst, int pad){
	BYTE* pTemp,*pTemp1;

	pTemp=pDst+pad+nPadStride*pad;
	pTemp1=pSrc;
	for(int i=0;i<m_Height;i++){
		memcpy(pTemp, pTemp1, m_Width*sizeof(BYTE));
		pTemp+=nPadStride;
		pTemp1+=nAlignStride;
	}
}
float* CSS_SSIMKL::GetQAMapPointer(){
	if(pQMap == NULL)
		return NULL;

	return pQMap;

}
bool CSS_SSIMKL::ConvertEdge2Weight(BYTE* pSrc , float* pDest){

	BYTE *pSrcT;
	float *pTemp;
	int i,j;
	for(i = 0; i < m_Height; i++)
	{
		pSrcT =pSrc+(i)* nAlignStride+1;
		pTemp =pDest+(i)* nAlignStride;

		for(j = 0; j < m_Width; j++)
		{
			*pTemp= ((255.0f - *pSrcT)/255.0f);
			pSrcT ++;
			pTemp++;
		}

	}
	return true;
}