// conSSIM.cpp : Defines the entry point for the console application.
//
/*!
***********************************************************************
* \file SS_SSIM.CPP
*
* \brief
*   Using SSIM in command line application 
*
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


#pragma warning(disable:4996)

#include "stdafx.h"
#include "Common.h"
#include "SSIM.h"
#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include <ctime>
int gResizePercent=100;

bool ProcessVideoBatch( int iMode, _TCHAR* batchname){
	using namespace std;
	//Create 
	CSSIM pTest;

	_TCHAR path[MAX_PATH];

	_TCHAR spe[]=_T(" \n");

	GetCurrentDirectory(sizeof(path), path);

	_DPRINTF(("Batch file should be placed under %s ",path));

	_TCHAR pathfile[MAX_PATH];

	_stprintf(pathfile,_T("%s\\%s"),path,batchname);
	ifstream input(pathfile);
	if(input.fail())
	{
		cerr<<_T("\nCan't find Batch find");
		return -1;
	}

	string str;
	vector<string> Filelist;

	Filelist.clear();
	while(getline(input,str))
	{
		istringstream in(str);
		if(str[0]!='#') // avoid comments 
			Filelist.push_back(str);
	}


	////
	//Create log file
	FILE* fp;

	if ( (fp=_tfopen(_T("log.csv"),_T("w")))== NULL){

		cerr<<_T("\n Error Can't Open log file ");
		return -1;
	}

	int size =Filelist.size();

	for(int loop=0 ;loop<size ; loop++){ 

		int Width(0),Height(0);
		_TCHAR st1[1024];
		_TCHAR st2[1024];

		string temp;
		_TCHAR spe[]=_T(" \n");
		temp=_tcstok((LPTSTR)Filelist[loop].c_str(),spe);
		Width=_tstoi(temp.c_str());

		temp=_tcstok( NULL,spe);
		Height=_tstoi(temp.c_str());

		temp=_tcstok(NULL,spe);
		_tcscpy(st1,temp.c_str());
	
		temp=_tcstok(NULL,spe);

		_tcscpy(st2,temp.c_str());

		pTest.SetFrameInfo(Width,Height,Width,8,FOURCC_YV12);

	//	pTest.SetMode((AL_MODE)MS_SSIM);
		pTest.SetMode((AL_MODE)iMode);
		double m_avScore(0.0);
		float m_fTime(0.0f);


	//	_DPRINTF((_T(" SSIM set Width %d Height %d F1:%s F2:%s "),Width,Height,st1,st2));

		pTest.ProcessFile((LPCTSTR)st1,(LPCTSTR)st2,m_avScore);

		_ftprintf(fp,_T("%s,%s,%f\n"),st1,st2,m_avScore);
		pTest.getTime(m_fTime);
		_DPRINTF((_T(" QA:%f ,time%f F1:%s F2:%s "),m_avScore,m_fTime,st1,st2));


	}
	fclose(fp);

	_tprintf(_T("\n Done, the log file is under %s "),path);
	return true;
}
bool ProcessImageBatch(int iMode, _TCHAR* batchname){
	using namespace std;
	//Create 
	CSSIM pTest;

	_TCHAR path[MAX_PATH];

	_TCHAR spe[]=_T(" \n");

	GetCurrentDirectory(sizeof(path), path);

	_DPRINTF(("Batch file should be placed under %s ",path));

	_TCHAR pathfile[MAX_PATH];

	_stprintf(pathfile,_T("%s\\%s"),path,batchname);
	ifstream input(pathfile);
	if(input.fail())
	{
		cerr<<_T("\nCan't find Batch find");
		return -1;
	}

	string str;
	vector<string> Filelist;

	Filelist.clear();
	while(getline(input,str))
	{
		istringstream in(str);
		if(str[0]!='#') // avoid comments 
			Filelist.push_back(str);
	}


	_TCHAR buffer[MAX_PATH];
	_stprintf(buffer,_T("%s\\log_%d_%s.csv"),path,iMode,batchname);
	ofstream logfile(buffer);

	int size =Filelist.size();

    clock_t start = clock();

	
	 _TCHAR maxst[1024];
	 double maxScore(0.0);
	for(int loop=0 ;loop<size ; loop++){ 

		int Width(0),Height(0);
		_TCHAR st1[1024];
		_TCHAR st2[1024];

		string temp;
		_TCHAR spe[]=_T(" \n");
		temp=_tcstok((LPTSTR)Filelist[loop].c_str(),spe);
		Width=_tstoi(temp.c_str());

		temp=_tcstok( NULL,spe);
		Height=_tstoi(temp.c_str());

		temp=_tcstok(NULL,spe);
		_tcscpy(st1,temp.c_str());
	
		temp=_tcstok(NULL,spe);

		_tcscpy(st2,temp.c_str());

		pTest.SetFrameInfo(Width,Height,Width,8,FOURCC_YV12);

	//	pTest.SetMode((AL_MODE)MS_SSIM);
		pTest.SetMode((AL_MODE)iMode);
		double m_avScore(0.0);
		float m_fTime(0.0f);


	//	_DPRINTF((_T(" SSIM set Width %d Height %d F1:%s F2:%s "),Width,Height,st1,st2));
		// Read file using OpenCV 
		 IplImage *imgR(NULL), *imgT(NULL) ;

		imgR = cvLoadImage(st1);
		if(imgR==NULL)
			return 0;
		IplImage *imgRG = cvCreateImage( cvSize( imgR->width, imgR->height ), IPL_DEPTH_8U, 1 );
		if(imgR->nChannels==3) //assuming RGB color image
			cvCvtColor( imgR, imgRG, CV_RGB2GRAY );
		

		imgT = cvLoadImage(st2);
		if(imgT==NULL)
			return 0;
		IplImage *imgTG = cvCreateImage( cvSize( imgT->width, imgT->height ), IPL_DEPTH_8U, 1 );
		if(imgT->nChannels==3) //assuming RGB color image
			cvCvtColor( imgT, imgTG, CV_RGB2GRAY );

		//Resize 
		// declare a destination IplImage object with correct size, depth and channels

		IplImage *imgRGs = cvCreateImage( cvSize((int)((imgRG->width*gResizePercent)/100) , (int)((imgRG->height*gResizePercent)/100) ),
			imgRG->depth, imgRG->nChannels );
		//use cvResize to resize source to a destination image
		cvResize(imgRG, imgRGs);

		IplImage *imgTGs = cvCreateImage( cvSize((int)((imgTG->width*gResizePercent)/100) , (int)((imgTG->height*gResizePercent)/100) ),
			imgTG->depth, imgTG->nChannels );
		//use cvResize to resize source to a destination image
		cvResize(imgTG, imgTGs);

		//reset frame info
		pTest.SetFrameInfo(int(Width*gResizePercent/100),int(Height*gResizePercent/100),int(Width*gResizePercent/100),8,FOURCC_YV12);

		BYTE* pDataR,*pDataT;
		int step;
		CvSize cv_size;

		cvGetRawData( imgRGs, (uchar**)&pDataR, &step, &cv_size );
		cvGetRawData( imgTGs, (uchar**)&pDataT, &step, &cv_size );

		//pTest.ProcessFile((LPCTSTR)st1,(LPCTSTR)st2,m_avScore);
		pTest.ProcessOneFrame(pDataR,pDataT,m_avScore);

		if(m_avScore > maxScore){
			maxScore = m_avScore;
			_tcscpy(maxst,st2);
		}
		char buff[256] = {'\0'};
		sprintf(buff, "%s,%s,%f\n",st1,st2,m_avScore);
		logfile<<buff;
	//	_DPRINTF((_T("%s,%s,%f\n"),st1,st2,m_avScore));

		pTest.getTime(m_fTime);
		//_DPRINTF((_T(" cur: %d end %d SSIM S:%f ,time%f F2:%s "),loop, size, m_avScore,m_fTime,st2));
		_tprintf(_T("cur: %d end %d,%s, %f\n"),loop, size,st2,m_avScore);
		cvReleaseImage(&imgR);
		cvReleaseImage(&imgT);
		cvReleaseImage(&imgRGs);
		cvReleaseImage(&imgTGs);
		cvReleaseImage(&imgRG);
		cvReleaseImage(&imgTG);
	}
	//fclose(fp);
	logfile.close();
	clock_t tends = clock();
	double t1=(double) (tends - start) / CLOCKS_PER_SEC;
	_tprintf(_T("\n Done taken %f sec, the log file is under %s "),t1, path);
	_tprintf(_T("\n Best %s %f\n"),maxst,maxScore);
	return 1;
}

bool ProcessImage(int iMode,  _TCHAR* pInput,_TCHAR* pOutput){
	using namespace std;
	//Create 
	CSSIM pTest;

	_TCHAR path[MAX_PATH];

	GetCurrentDirectory(sizeof(path), path);

	_DPRINTF(("Batch file should be placed under %s ",path));

	_TCHAR pathfile[MAX_PATH];

	_stprintf(pathfile,_T("%s\\%s"),path,pOutput);

	_TCHAR spe[]=_T(".\\\n");

	_TCHAR buffer[MAX_PATH];
	_stprintf(buffer,_T("%slog_%d.txt"),pOutput,iMode);
	ofstream logfile(buffer);

    clock_t start = clock();

	int Width(0),Height(0);
	_TCHAR st1[1024];
	_TCHAR st2[1024];

	_tcscpy(st1,pInput);
	_tcscpy(st2,pOutput);



	pTest.SetMode((AL_MODE)iMode);
	double m_avScore(0.0);
	float m_fTime(0.0f);


	//	_DPRINTF((_T(" SSIM set Width %d Height %d F1:%s F2:%s "),Width,Height,st1,st2));
	// Read file using OpenCV 
	IplImage *imgR(NULL), *imgT(NULL) ;

	imgR = cvLoadImage(st1);
	if(imgR==NULL)
		return 0;

	IplImage *imgRG = cvCreateImage( cvSize( imgR->width, imgR->height ), IPL_DEPTH_8U, 1 );
	if(imgR->nChannels==3) //assuming RGB color image
		cvCvtColor( imgR, imgRG, CV_RGB2GRAY );

	imgT = cvLoadImage(st2);
	if(imgT==NULL)
		return 0;
	IplImage *imgTG = cvCreateImage( cvSize( imgT->width, imgT->height ), IPL_DEPTH_8U, 1 );
	if(imgT->nChannels==3) //assuming RGB color image
		cvCvtColor( imgT, imgTG, CV_RGB2GRAY );

	//Resize 
	// declare a destination IplImage object with correct size, depth and channels

	IplImage *imgRGs = cvCreateImage( cvSize((int)((imgRG->width*gResizePercent)/100) , (int)((imgRG->height*gResizePercent)/100) ),
		imgRG->depth, imgRG->nChannels );
	//use cvResize to resize source to a destination image
	cvResize(imgRG, imgRGs);

	IplImage *imgTGs = cvCreateImage( cvSize((int)((imgTG->width*gResizePercent)/100) , (int)((imgTG->height*gResizePercent)/100) ),
		imgTG->depth, imgTG->nChannels );
	//use cvResize to resize source to a destination image
	cvResize(imgTG, imgTGs);

	//reset frame info
	pTest.SetFrameInfo(int(imgTG->width*gResizePercent/100),int(imgTG->height*gResizePercent/100),int(imgTG->width*gResizePercent/100),8,FOURCC_YV12);

	BYTE* pDataR,*pDataT;
	int step;
	CvSize cv_size;

	cvGetRawData( imgRGs, (uchar**)&pDataR, &step, &cv_size );
	cvGetRawData( imgTGs, (uchar**)&pDataT, &step, &cv_size );


	pTest.ProcessOneFrame(pDataR,pDataT,m_avScore);


	char buff[256] = {'\0'};
	sprintf(buff, "%s,%s,%f\n",st1,st2,m_avScore);
	std::cout << buff;
	logfile<<buff;
	//	_DPRINTF((_T("%s,%s,%f\n"),st1,st2,m_avScore));

	pTest.getTime(m_fTime);
	//_DPRINTF((_T(" cur: %d end %d SSIM S:%f ,time%f F2:%s "),loop, size, m_avScore,m_fTime,st2));

	cvReleaseImage(&imgR);
	cvReleaseImage(&imgT);
	cvReleaseImage(&imgRGs);
	cvReleaseImage(&imgTGs);
	cvReleaseImage(&imgRG);
	cvReleaseImage(&imgTG);

	//fclose(fp);
	logfile.close();
	clock_t tends = clock();
	double t1=(double) (tends - start) / CLOCKS_PER_SEC;
	_tprintf(_T("\n Done taken %f sec, the log file is under %s "),t1, path);

	return 1;
}
int _tmain(int argc, _TCHAR* argv[])
{
	_TCHAR batchname[MAX_PATH], st_ref[MAX_PATH], st_test[MAX_PATH];
	int imode(0); // default 0-ssim 1-msssim 2-PSNR
	int iW(0),iH(0);
	int iProcess(0) ;// 0 -batch images 1- image file mode
	if (argc < 2) { // Check the value of argc. If not enough parameters have been passed, inform user and exit.
		std::cout << _T("Image file mode -m <mode> -r <ref.jpg> -t <test.jpg> \n");
		std::cout << _T("Image Batch mode -m <mode> -bim <batchfile> \n");
		std::cout << _T("Video Batch mode -m <mode> -bv <batchfile>  \n");
		exit(0);
	} else { 
		int i=0;
		for (i = 1; i < argc; i++) {
			_DPRINTF((_T(" cur:%s "),argv[i]));
			if (i  != argc) // Check that we haven't finished parsing already
				if (strcmp(argv[i],_T("-m"))==0 ) { //read mode 0-SSIM 1- MSSSIM 2- PSNR             
					imode = _tstoi(argv[i+1]);
				} 
				else if (strcmp(argv[i],_T("-bim"))==0 ) {
					_tcscpy(batchname,argv[i+1]);
					iProcess=0;
				} 
				else if (strcmp(argv[i],_T("-r"))==0 ) {
					_tcscpy(st_ref,argv[i+1]);
					iProcess=1;
				} 
				else if (strcmp(argv[i],_T("-t"))==0 ) {
					_tcscpy(st_test,argv[i+1]);
					iProcess=1;
				} 
				else if (strcmp(argv[i],_T("-bv"))==0 ) {
					_tcscpy(batchname,argv[i+1]);
					iProcess=2;
				} 
				else if (strcmp(argv[i],_T("-h"))==0 ) {
					std::cout << _T("Image Batch mode -m <mode> -bim <batchfile> \n");
					std::cout << _T("Image file mode -m <mode> -r<ref> -t<test> \n");
					std::cout << _T("Video Batch mode -m <mode> -bv <batchfile>  \n");
					iProcess=3;
				} 

		}
	}

	switch (iProcess){
		case 0:
			if(ProcessImageBatch(imode, batchname)==-1)
				return -1;
			return 1;
			break;
		case 1:
			if(ProcessImage(imode, st_ref,st_test)==-1)
				return -1;
			return 1;
			break;
		case 2:
			if(ProcessVideoBatch(imode, batchname)==-1)
				return -1;
			return 1;
			break;
		case 3:
			return 1;
			break;			
		default:
			_tprintf(_T("\Invalidated input argument\n"));			
			break;
	}

	return 0;
}
