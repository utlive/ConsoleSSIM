/*!
***********************************************************************
* \file SSIM.CPP
*
* \brief
*    Container of SSIM kernels 
*    Support CUP detect for SSE2 and multi-threads
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


#include "SSIM.h"
#include <iostream>
#include "UsingCV.h"
#include "Common.h"

//#define _SINGLE_CORE 1

//#define _VQEG_DB
//#define CROP_NUM 20
//#define _DUMP_LOG


CSSIM::CSSIM(){

	_DPRINTF(("SSIM CSSIM constructor"));

	m_mode=0;

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	m_tNumber = info.dwNumberOfProcessors;
#ifdef _SINGLE_CORE
	m_tNumber =1; //Ming for single core testing
#endif
	
	// init for parallel comupting, the multi-threading is implemented in frame level. 
	pSSIMKL=new IKL_SSIM*[m_tNumber];
	m_tParam = new ThreadParam[m_tNumber];

	m_threads = new CThreadControl[m_tNumber];

	for (int i = 0; i < m_tNumber; i++)
	{
		pSSIMKL[i]=new CSS_SSIMKL();
		m_tParam[i].nID = i;
		m_tParam[i].pContext = this;
		m_tParam[i].bRun = false;
		m_tParam[i].bAbort = false;
		m_tParam[i].dScore=0.0;
		m_tParam[i].pRef=NULL;
		m_tParam[i].pTest=NULL;
		m_tParam[i].pRefRz=NULL;
		m_tParam[i].pTestRz=NULL;
		m_threads[i].CreateThread_Run(CSSIM::MultiCore, &m_tParam[i]);
	}

	m_ssimarray.clear();

	pTempbuf=NULL;
	m_bShow=false;
}
CSSIM::~CSSIM(){
	int i=0;
	for ( i = 0; i < m_tNumber; i++)
		m_threads[i].CloseThread();
//	_DELETE_PTRS(pSSIMKL);
	for(i = 0 ; i<m_tNumber ; i++){
		_DELETE_PTR(pSSIMKL[i]);
	}

	delete [] m_tParam;
	delete [] m_threads;
	_DPRINTF(("SSIM CSSIM destructor"));
	if(pTempbuf!=NULL){
		for( i = 0 ; i<m_tNumber ; i++){
			_ALIGNED_FREE_PTR(pTempbuf[i]);
		}
		_ALIGNED_FREE_PTR(pTempbuf);
	}

	m_ssimarray.clear();

}
DWORD WINAPI CSSIM::MultiCore(LPVOID lpParam)
{
	ThreadParam *param = (ThreadParam*)lpParam;
	const int nTID   = param->nID;

	
	while(true)
	{
		param->pContext->m_threads[nTID].WaitBegin();

		if (param->pContext->m_threads[nTID].TimeToDie())
			break;

		if (param->nID<param->pContext->m_tNumber && param->bRun)
		{
			//CTimeCost ctimer;
			//ctimer.start();

			param->dScore=0.0;

			param->pContext->pSSIMKL[nTID]->ProcessOneFrame(param->pRef,param->pTest,param->dScore);

			//ctimer.end();
			//param->pContext->fTime+=ctimer.get_seconds();
		}
		

		param->pContext->m_threads[nTID].SignalEnd();
	}
	return 0;
}
bool CSSIM::SetFrameInfo(int nWitdh, int nHeight, int nStride,int nBitCount,DWORD fourCC ){
	m_CurFrame.SetFrameInfo(fourCC,nBitCount,nWitdh,nHeight,nStride);
	_DPRINTF(("ssim width%d height%d stride%d",nWitdh,nHeight,nStride));
	m_ssimarray.clear();
	return true;
}
bool CSSIM::SetMode(const AL_MODE mode ){
	if(m_mode==mode)
		return true;

	int i(0);
	m_mode=mode;
	for( i = 0 ; i<m_tNumber ; i++){
		_DELETE_PTR(pSSIMKL[i]);
	}
	if(m_mode==0){
		for(i = 0 ; i<m_tNumber ; i++){
			pSSIMKL[i]=new CSS_SSIMKL();
		}

	}else if(m_mode==1){
		for(i = 0 ; i<m_tNumber ; i++){
			pSSIMKL[i]=new CMS_SSIMKL();
		}


	}else if(m_mode==2){
		for(i = 0 ; i<m_tNumber ; i++){
			pSSIMKL[i]=new MJ_PSNRKL();
		}
	}else if(m_mode==3){
		for(i = 0 ; i<m_tNumber ; i++){
			pSSIMKL[i]=new MJ_CORRELATIONKL();
		}
	}
	else
		return false;
	return true;
}
bool CSSIM::ProcessOneFrame(const BYTE* pRef, const BYTE* pTest, double& pScore){


	pSSIMKL[0]->Init(m_CurFrame.m_FrmInfo);
	return pSSIMKL[0]->ProcessOneFrame(pRef,pTest,pScore);  //only luminance 

}
bool CSSIM::getTime(float& fTempTime){

	if(m_fTime>0){
		fTempTime=m_fTime;
		return true;
	}
	else
		return false;
	

}
bool CSSIM::ProcessFile(LPCTSTR pRef,LPCTSTR pTest, double& avScore){



	int width(m_CurFrame.m_FrmInfo.nWidth), height(m_CurFrame.m_FrmInfo.nHeight);
	BYTE *pOut = NULL;

	avScore=0.0;

	if(width<30||height<30)
		return false;
	m_ssimarray.clear();

	int y_size = width * height;

#ifdef _VQEG_DB
	int uv_size = y_size / 2;

#else
	int uv_size = y_size / 4;
#endif
	int frm_size = y_size + uv_size * 2;

	FILE *fpRef = _tfopen(pRef, "rb");
	FILE *fpTest = _tfopen(pTest, "rb");
	size_t rd_szRef(0),rd_szTest(0),rd_szRef1(0),rd_szTest1(0);
	int frm_cnt = 0;
#ifdef _VQEG_DB
	int newWidth((width-2*CROP_NUM)/2), newHeight((height-2*CROP_NUM)/2);

	setcolorConvertUYVY2YUV(width,height);
#endif
	_DPRINTF((" thread %d",m_tNumber));
	CTimeCost m_Timer;
	int i;
	for( i=0;i<m_tNumber;i++){
		m_tParam[i].pRef= new BYTE[frm_size];
#ifdef _VQEG_DB
		m_tParam[i].pRefRz= new BYTE[newWidth*height*2];
		m_tParam[i].RzImgRef = cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 1);
#endif
	}
	for (i=0;i<m_tNumber;i++){
		m_tParam[i].pTest= new BYTE[frm_size];
#ifdef _VQEG_DB
		m_tParam[i].pTestRz= new BYTE[newWidth*height*2];
		m_tParam[i].RzImgTest = cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 1);
#endif
	}
	int iT(0);

#ifdef _VQEG_DB
		m_CurFrame.m_FrmInfo.nWidth=newWidth*2;
		m_CurFrame.m_FrmInfo.nHeight=newHeight*2;
		m_CurFrame.m_FrmInfo.nStride=newWidth*2;

#endif
	for(i=0;i<m_tNumber;i++){
		pSSIMKL[i]->Init(m_CurFrame.m_FrmInfo);
	}

	int iP_num(1);

	fTime=0.0f;
	float fKernelTime(0.0f);
	CTimeCost cTimer;
	cTimer.start();

#if _DUMP_LOG
	 char *pRead, *pTemp;

	pRead= new  char[frm_size];
	fread(pRead, sizeof( char), frm_size, fpRef);
	pTemp = new  char[frm_size/2];


	int x,y;
	 char* pReadBuf,*pSavebuf;


	pReadBuf=pRead;
	for(y=0;y<height;y++){

		for(x=0;x<width/2;x++){
			pTemp[(x+(y*width/2))]= (*pReadBuf);
			pReadBuf+=4;
		}
	}

	_TCHAR fname[256];
	_stprintf(fname, "M:\\dump\\test%04d.jpg", 1);
	mj_SaveBufferToImage< char>(fname,( char*)pTemp,width/2,height,width/2,1);

	delete []pRead;
	delete []pTemp;
#endif
	

	while((rd_szRef = fread(m_tParam[0].pRef, sizeof(BYTE), frm_size, fpRef)) == frm_size && (rd_szTest = fread(m_tParam[0].pTest, sizeof(BYTE), frm_size, fpTest)) == frm_size /*&& iT<3*/) {

		
#ifdef _VQEG_DB

		//colorConvertUYVY2YUV(m_tParam[0].pRef,width,height,0,&m_tParam[0].pRefRz,m_tParam[0].RzImgRef);
		////////colorConvertUYVY2YUV(m_tParam[0].pTest,width,height,0,&m_tParam[0].pTestRz,m_tParam[0].RzImgTest);
#endif



		//_TCHAR fname[256];

		//_stprintf(fname, "M:\\dump\\ref%04d.jpg", frm_cnt);
		//mj_SaveBufferToImage<char>(fname,(char*)m_tParam[0].pRef,newWidth*2,newHeight*2,newWidth*2,1);

		//_stprintf(fname, "M:\\dump\\test%04d.jpg", frm_cnt);
		//mj_SaveBufferToImage<char>(fname,(char*)m_tParam[0].pTest,newWidth*2,newHeight*2,newWidth*2,1);

		frm_cnt++;

		iP_num=1;
		if(iP_num <m_tNumber ){

			while(  (rd_szRef1 = fread(m_tParam[iP_num].pRef, sizeof(BYTE), frm_size, fpRef)) == frm_size && (rd_szTest1 = fread(m_tParam[iP_num].pTest, sizeof(BYTE), frm_size, fpTest)) == frm_size ){

			//	_DPRINTF((" thread %d %d",iP_num,m_tNumber));

#ifdef _VQEG_DB
				colorConvertUYVY2YUV(m_tParam[iP_num].pRef,width,height,iP_num,&m_tParam[iP_num].pRefRz,m_tParam[iP_num].RzImgRef);
				colorConvertUYVY2YUV(m_tParam[iP_num].pTest,width,height,iP_num,&m_tParam[iP_num].pTestRz,m_tParam[iP_num].RzImgTest);

				//_stprintf(fname, "M:\\dump\\%dref%04d.jpg",iP_num, frm_cnt);
				//mj_SaveBufferToImage<char>(fname,(char*)m_tParam[0].pRefRz,newWidth,newHeight,newWidth,1);

				//_stprintf(fname, "M:\\dump\\%dtest%04d.jpg",iP_num, frm_cnt);
				//mj_SaveBufferToImage<char>(fname,(char*)m_tParam[0].pTestRz,newWidth,newHeight,newWidth,1);
#endif
				iP_num++;
				frm_cnt++;
				if(iP_num >=m_tNumber )
					break;
			}
		}
		CTimeCost Inner_T;
		Inner_T.start();

		m_Timer.start();
		for (i = 0; i < iP_num; i++)
		{
			m_tParam[i].bRun=true;
			m_threads[i].SignalBegin();
		}
		for (i = 0; i < iP_num; i++){
			m_threads[i].WaitComplete();
		}

		Inner_T.end();

		fKernelTime+=Inner_T.get_seconds();
		for (i = 0; i < iP_num; i++){
			avScore+=m_tParam[i].dScore;
			m_ssimarray.push_back(m_tParam[i].dScore);
			_DPRINTF(("Multi-thread %d %f time %f",i,m_tParam[i].dScore,	Inner_T.get_seconds()));
		}



		iT+=iP_num;
	}
	avScore = avScore/iT;

	cTimer.end();
	fTime=cTimer.get_seconds();

    m_fTime=fKernelTime;
	
	_DPRINTF((" %f  File Frame%d total time(include File I/O) %f average time %f thread num%d , average kernel time %f fps: %f ",avScore, frm_cnt,fTime,fTime/frm_cnt,m_tNumber,fKernelTime/frm_cnt, frm_cnt/fKernelTime));

	for( i=0 ; i < m_tNumber ;i++){
		delete [] m_tParam[i].pRef;
#ifdef _VQEG_DB		
		cvReleaseImage(&m_tParam[i].RzImgRef);
		delete [] m_tParam[i].pRefRz;
#endif
	}
	for( i=0 ; i < m_tNumber ; i++){
		delete [] m_tParam[i].pTest;
#ifdef _VQEG_DB	
		cvReleaseImage(&m_tParam[i].RzImgTest);
		delete [] m_tParam[i].pTestRz;
#endif
	}


	fclose(fpRef);
	fclose(fpTest);
	return true;
}
bool CSSIM::GetSSIMArray(std::vector<double>& _SsimArray){

	if(m_ssimarray.size()<1)
		return false;
	_SsimArray=m_ssimarray;
	return true;
}
bool ConvertYUV2PNG(LPCTSTR pFile,LPCTSTR pPath2Save, VideoFrame& m_frame){

	int width(m_frame.m_FrmInfo.nWidth), height(m_frame.m_FrmInfo.nHeight);
	BYTE *pOut = NULL;

	if(width<30||height<30)
		return false;

	IplImage *pimg = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	cvGetRawData(pimg, &pOut);
	//_DPRINTF(("CV SSIM "));
	int y_size = width * height;
	int uv_size = y_size / 4;
	int frm_size = y_size + uv_size * 2;

	FILE *fp = _tfopen(pFile, "rb");
	BYTE *frm_buffer = new BYTE[frm_size];

	using namespace std;
	size_t rd_sz = 0;
	int frm_cnt = 0;
	while((rd_sz = fread(frm_buffer, sizeof(BYTE), frm_size, fp)) == frm_size) {
		frm_cnt++;
		BYTE *ptrY = frm_buffer;
		BYTE *ptrU = frm_buffer + y_size;
		BYTE *ptrV = ptrU + uv_size;

		int i, j;
		for(i = 0; i < height; i++) {
			for(j = 0; j < width; j++) {
				int posY = i * width + j;
				int posUV = (i / 2) * (width / 2) + (j / 2);
				int Y = ptrY[posY];
				int U = ptrU[posUV];
				int V = ptrV[posUV];
				int iY  = (Y - 16) * iY2RGB;
				int iCb = U - 128;
				int iCr = V - 128;
				int px = i * pimg->widthStep + j * 3;
				pOut[px] = max(0, min(255, (iY +             iCb*iCb2B) >> 16)); // b
				pOut[px + 1] = max(0, min(255, (iY + iCr*iCr2G + iCb*iCb2G) >> 16)); // g
				pOut[px + 2] = max(0, min(255, (iY + iCr*iCr2R            ) >> 16)); // r
			}
		}

		char fname[MAX_PATH];
		sprintf(fname, "%sFRM_%08d.png", pPath2Save,frm_cnt);

		cvSaveImage(fname, pimg);
	}

	delete [] frm_buffer;

	cvReleaseImage(&pimg);

	fclose(fp);
	return true;
}
bool CSSIM::SetDisplayParameters(PFNPAINTCALLBACK pfnEventCallback, bool bShow){


		m_bShow=bShow;
		return true;
}
void CSSIM::setcolorConvertUYVY2YUV(int width, int high){

	int i(0);
	if(pTempbuf!=NULL){
		for( i = 0 ; i<m_tNumber ; i++){
			_ALIGNED_FREE_PTR(pTempbuf[i]);
		}
		_ALIGNED_FREE_PTR(pTempbuf);
	}
//	_ALIGNED_MALLOC_PTR(pTempbuf,BYTE*,m_tNumber);
	pTempbuf= new BYTE*[m_tNumber];
	for(i = 0 ; i<m_tNumber ; i++){
		pTempbuf[i]=NULL;
		_ALIGNED_MALLOC_PTR(pTempbuf[i],BYTE,width*high*2);
	}



}
void CSSIM::colorConvertUYVY2YUV(BYTE* pbuf,int width, int high, int index,BYTE** pbufRz,IplImage* ResizeSzImgRef){


	

	memcpy(pTempbuf[index],pbuf,sizeof(BYTE)*width*high*2);
	memset(pbuf,0,sizeof(BYTE)*width*high*2);
	int i,j;
	i=j=0;

	int x,y;
	BYTE *pY,*pYOri,*pYbottom,*pY1;
	pY=pbuf;

	pY1+=width;
	pYOri=pTempbuf[index];
	pYOri++;


	for(y=0;y<high;y++){

		for(x=0;x<width;x++){
			pbuf[(x+(y*width))]= (*pYOri);
			pYOri+=2;
		}
	}

	//_TCHAR fname[256];
	//_stprintf(fname, "M:\\dump\\oriref%04d.jpg", 1);
	//mj_SaveBufferToImage<char>(fname,(char*)pbuf,width,high,width,1);
	//crop
	memcpy(pTempbuf[index],pbuf,sizeof(BYTE)*width*high*2);
	memset(pbuf,0,sizeof(BYTE)*width*high*2);

#ifdef _VQEG_DB	
	int newWidth=width-2*CROP_NUM;
	int newHeight=high-2*CROP_NUM;
	i=j=0;
	for(y=CROP_NUM,j=0;y<high-CROP_NUM;y++){
		
		for(x=CROP_NUM,i=0;x<width-CROP_NUM;x++){
			pbuf[(i+(j*newWidth))]= pTempbuf[index][(x+(y*width))];
			i++;
		}
		j++;
	}
#endif
	//IplImage *img1_temp = cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 1);
	//BYTE* pData,*pTRef,*pTemp1;
	//int step;
	//CvSize size;

	//cvGetRawData( img1_temp, (uchar**)&pData, &step, &size );
	//step /= sizeof(*pData);

	//for( y = 0; y < size.height; y++, pData += step ){
	//	pTRef=(BYTE*)pbuf+newWidth*y;
	//	pTemp1=pData;
	//	for( x = 0; x < size.width; x++ ){
	//		*pTemp1 = *pTRef;
	//		pTemp1++;
	//		pTRef++;
	//	}
	//}

	//int ReWidth(newWidth/2),ReHieght(newHeight/2);
	//
	//cvResize(img1_temp, ResizeSzImgRef, CV_INTER_LINEAR);

	//cvReleaseImage(&img1_temp);


	//_TCHAR fname[256];
	//cvGetRawData( ResizeSzImgRef, pbufRz, &step, &size );

	//cvReleaseImage(&img1_temp);

	//_stprintf(fname, "M:\\dump\\Cropref%04d.jpg", 1);
	//mj_SaveBufferToImage<char>(fname,(char*)*pbufRz,ReWidth,ReHieght,ReWidth,1);
#if _DUMP_LOG

	_TCHAR fname[256];
	_stprintf(fname, "M:\\dump\\ColorConver%04d.jpg", 1);
	mj_SaveBufferToImage<char>(fname,(char*)pbuf,width,high,width,1);

#endif


	

}
// OpenCV SSIM for validation 
int CSSIM::test( BYTE* pRef,  BYTE* pTest, double& dScore)
{
	/*
	* The equivalent of Zhou Wang's SSIM matlab code using OpenCV.
	* from http://www.cns.nyu.edu/~zwang/files/research/ssim/index.html
	* The measure is described in :
	* "Image quality assessment: From error measurement to structural similarity"
	* C++ code by Rabah Mehdi. http://mehdi.rabah.free.fr
	*/

	//if(argc!=3)
	//	return -1;

	// default settings
		pSSIMKL[0]->Init(m_CurFrame.m_FrmInfo);
	double C1 = 6.5025, C2 = 58.5225;

	IplImage
		*img1=NULL, *img2=NULL, *img1_img2=NULL,
		*img1_temp=NULL, *img2_temp=NULL,
		*img1_sq=NULL, *img2_sq=NULL,
		*mu1=NULL, *mu2=NULL,
		*mu1_sq=NULL, *mu2_sq=NULL, *mu1_mu2=NULL,
		*sigma1_sq=NULL, *sigma2_sq=NULL, *sigma12=NULL,
		*ssim_map=NULL, *temp1=NULL, *temp2=NULL, *temp3=NULL;
	

	/***************************** INITS **********************************/
	//IplImage *img = cvLoadImage("C:\\FRM_00000250.png", 1);
	//img1_temp = cvLoadImage("C:\\FRM_00000250.png");
	//img2_temp = cvLoadImage("C:\\FRM_00000250.png");


	int x= m_CurFrame.m_FrmInfo.nWidth, y=m_CurFrame.m_FrmInfo.nHeight;
	int nChan=3, d=IPL_DEPTH_32F;

    BYTE *tmp1RGB24 = NULL;
	img1_temp = cvCreateImage(cvSize(x, y), IPL_DEPTH_8U, 3);
	cvGetRawData(img1_temp, &tmp1RGB24);
    cvtYV12toRGB24(tmp1RGB24, pRef, x,y);



    BYTE *tmp2RGB24 = NULL;
	img2_temp = cvCreateImage(cvSize(x, y), IPL_DEPTH_8U, 3);
	cvGetRawData(img2_temp, &tmp2RGB24);
    cvtYV12toRGB24(tmp2RGB24, pTest, x, y);

	CTimeCost m_time;
	m_time.start();
	if(img1_temp==NULL || img2_temp==NULL)
		return -1;

//	int x=img1_temp->width, y=img1_temp->height;

	CvSize size = cvSize(x, y);

	img1 = cvCreateImage( size, d, nChan);
	img2 = cvCreateImage( size, d, nChan);

	cvConvert(img1_temp, img1);
	cvConvert(img2_temp, img2);
	cvReleaseImage(&img1_temp);
	cvReleaseImage(&img2_temp);

	//cvSaveImage("C:\\temp.jpg",img1);
	img1_sq = cvCreateImage( size, d, nChan);
	img2_sq = cvCreateImage( size, d, nChan);
	img1_img2 = cvCreateImage( size, d, nChan);
	
	cvPow( img1, img1_sq, 2 );
	cvPow( img2, img2_sq, 2 );
	cvMul( img1, img2, img1_img2, 1 );

	mu1 = cvCreateImage( size, d, nChan);
	mu2 = cvCreateImage( size, d, nChan);

	mu1_sq = cvCreateImage( size, d, nChan);
	mu2_sq = cvCreateImage( size, d, nChan);
	mu1_mu2 = cvCreateImage( size, d, nChan);
	

	sigma1_sq = cvCreateImage( size, d, nChan);
	sigma2_sq = cvCreateImage( size, d, nChan);
	sigma12 = cvCreateImage( size, d, nChan);

	temp1 = cvCreateImage( size, d, nChan);
	temp2 = cvCreateImage( size, d, nChan);
	temp3 = cvCreateImage( size, d, nChan);

	ssim_map = cvCreateImage( size, d, nChan);
	/*************************** END INITS **********************************/


	//////////////////////////////////////////////////////////////////////////
	// PRELIMINARY COMPUTING
	cvSmooth( img1, mu1, CV_GAUSSIAN, 11, 11, 1.5 );
	cvSmooth( img2, mu2, CV_GAUSSIAN, 11, 11, 1.5 );
	
	cvPow( mu1, mu1_sq, 2 );
	cvPow( mu2, mu2_sq, 2 );
	cvMul( mu1, mu2, mu1_mu2, 1 );


	cvSmooth( img1_sq, sigma1_sq, CV_GAUSSIAN, 11, 11, 1.5 );
	cvAddWeighted( sigma1_sq, 1, mu1_sq, -1, 0, sigma1_sq );
	
	cvSmooth( img2_sq, sigma2_sq, CV_GAUSSIAN, 11, 11, 1.5 );
	cvAddWeighted( sigma2_sq, 1, mu2_sq, -1, 0, sigma2_sq );

	cvSmooth( img1_img2, sigma12, CV_GAUSSIAN, 11, 11, 1.5 );
	cvAddWeighted( sigma12, 1, mu1_mu2, -1, 0, sigma12 );
	

	//////////////////////////////////////////////////////////////////////////
	// FORMULA

	// (2*mu1_mu2 + C1)
	cvScale( mu1_mu2, temp1, 2 );
	cvAddS( temp1, cvScalarAll(C1), temp1 );

	// (2*sigma12 + C2)
	cvScale( sigma12, temp2, 2 );
	cvAddS( temp2, cvScalarAll(C2), temp2 );

	// ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
	cvMul( temp1, temp2, temp3, 1 );

	// (mu1_sq + mu2_sq + C1)
	cvAdd( mu1_sq, mu2_sq, temp1 );
	cvAddS( temp1, cvScalarAll(C1), temp1 );

	// (sigma1_sq + sigma2_sq + C2)
	cvAdd( sigma1_sq, sigma2_sq, temp2 );
	cvAddS( temp2, cvScalarAll(C2), temp2 );

	// ((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
	cvMul( temp1, temp2, temp1, 1 );

	// ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
	cvDiv( temp3, temp1, ssim_map, 1 );


	CvScalar index_scalar = cvAvg( ssim_map );
//	cvSaveImage("C:\\ssim.jpg",ssim_map);


	cvReleaseImage(&img1);
	cvReleaseImage(&img2);
	cvReleaseImage(&img1_img2);
	cvReleaseImage(&img1_sq);
	cvReleaseImage(&img2_sq);
	cvReleaseImage(&mu1);
	cvReleaseImage(&mu2);
	cvReleaseImage(&mu1_sq);
	cvReleaseImage(&mu2_sq); 
	cvReleaseImage(&mu1_mu2);
	cvReleaseImage(&sigma1_sq);
	cvReleaseImage(&sigma2_sq);
	cvReleaseImage(&sigma12);
	cvReleaseImage(&ssim_map);
	cvReleaseImage(&temp1);
	cvReleaseImage(&temp2);
	cvReleaseImage(&temp3);
	//cvSaveImage(fname, img);
	dScore =index_scalar.val[0];
	m_time.end();
	_DPRINTF(("CV SSIM %f %f %f time %f ",index_scalar.val[0],index_scalar.val[1],index_scalar.val[2],m_time.get_seconds()));
	// through observation, there is approximately 
	// 1% error max with the original matlab program

	//cout << "(R, G & B SSIM index)" << endl ;
	//cout << index_scalar.val[2] * 100 << "%" << endl ;
	//cout << index_scalar.val[1] * 100 << "%" << endl ;
	//cout << index_scalar.val[0] * 100 << "%" << endl ;

	// if you use this code within a program
	// don't forget to release the IplImages
	return 0;
}



void CSSIM::cvtYV12toRGB24(BYTE *pDst, BYTE *pSrc, int width, int height)
{
	BYTE *pSrcY = pSrc;
	BYTE *pSrcU = pSrc + (width * height * 5) / 4;
	BYTE *pSrcV = pSrc + width * height;

	using namespace std;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int posY = i * width + j;
			int Y = pSrcY[posY];
			int iY  = (Y - 16) * iY2RGB;

			int posUV = (i / 2) * (width / 2) + (j / 2);
			int U = pSrcU[posUV] - 128;
			int V = pSrcV[posUV] - 128;

			int px = (i * width + j) * 3;

			pDst[px] = max(0, min(255, (iY + U*iCb2B) >> 16));
			pDst[px + 1] = max(0, min(255, (iY + V*iCr2G + U*iCb2G) >> 16));
			pDst[px + 2] = max(0, min(255, (iY + V*iCr2R) >> 16));
		}
	}
}

