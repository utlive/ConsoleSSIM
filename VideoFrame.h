#pragma once


#ifndef FOURCC
#define FOURCC DWORD
#endif

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | \
    ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif

#define FOURCC_YV12     MAKEFOURCC('Y', 'V', '1', '2')
#define FOURCC_YUY2	    MAKEFOURCC('Y', 'U', 'Y', '2')
#define FOURCC_RGB24    MAKEFOURCC('R', 'G', '2', '4')
#define FOURCC_RGB32    MAKEFOURCC('R', 'G', '3', '2')

class VFrameInfo
{
public:
    int nWidth;
    int nHeight;
    int nStride;
    int nBitCount;

    FOURCC dwFourCC;

    VFrameInfo() 
    { 
        nWidth = nHeight = nStride = 0;
        nBitCount = dwFourCC = 0;
    }
    ~VFrameInfo() { }

	VFrameInfo& operator=(const VFrameInfo& that){
		if(this==&that){
			return *this;
		}
		nWidth=that.nWidth;
		nHeight=that.nHeight;
		nStride=that.nStride;
		nBitCount=that.nBitCount;
		dwFourCC=that.dwFourCC;
		return *this;
	}
};

class VideoFrame
{
public:
    VFrameInfo m_FrmInfo;       

    LONGLONG   m_tmStart;
    LONGLONG   m_tmEnd;

    VideoFrame();
    ~VideoFrame();

    void SetFrameInfo(DWORD fourCC, int bitCount, int width, int height, int stride = -1);

	//reserver
	//Pointer link to data
};
