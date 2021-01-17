#include "Common.h"
#include "VideoFrame.h"


VideoFrame::VideoFrame()
{
    m_tmStart = 0;
    m_tmEnd = 0;
}

VideoFrame::~VideoFrame()
{
}



void VideoFrame::SetFrameInfo(DWORD fourCC, int bitCount, int width, int height, int stride)
{
    m_FrmInfo.dwFourCC = fourCC;
    m_FrmInfo.nBitCount = bitCount;
    m_FrmInfo.nWidth = width;
    m_FrmInfo.nHeight = height;
    m_FrmInfo.nStride = stride;
}

