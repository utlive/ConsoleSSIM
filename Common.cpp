


#pragma once
#include "Common.h"

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

int _FileExist(LPCTSTR fname)
{
    if(fname == NULL || !_tcscmp(fname, "") || _tcslen(fname) == 0)
        return 0;
    FILE *fp = _tfopen(fname, "rb");
    if(fp == NULL)
        return 0;
    fclose(fp);
    return 1;
}

int _GetFileSize(LPCTSTR fname)
{
    FILE *fp = _tfopen(fname, "rb");
    if(fp == NULL)
        return -1;

    fseek(fp, 0L, SEEK_END);
    int fsz = (int) ftell(fp);
    fclose(fp);
    return fsz;
}

int _dprintf(LPCTSTR fmt, ...)
{
    _TCHAR buffer[4096];
    _TCHAR buffer2[4096];
    va_list marker;
    va_start(marker, fmt);
    int nArgs = _vstprintf(buffer, fmt, marker);
    _stprintf(buffer2, "%s %s", _DBG_MSG_PREFIX_TAG, buffer);
    if(buffer2[_tcslen(buffer2) - 1] != '\n')
        _tcscat(buffer2, "\n");
    OutputDebugString(buffer2);
    va_end (marker);		
    return nArgs;
}

LPTSTR _TimeFormat(LONGLONG tmCode, LPTSTR szTimeString)
{
	int nSec = (int)(tmCode / 10000000);
	int nMin = nSec / 60;
	nSec = nSec % 60;
	int nHr = nMin / 60;
	nMin = nMin % 60;
	_stprintf(szTimeString, "%02d:%02d:%02d", nHr, nMin, nSec);
	return szTimeString;
}

void _GetNowTimeStr(LPTSTR strTime)
{
	time_t tm_now;
	time(&tm_now);
	struct tm *currtime = localtime(&tm_now);
	_stprintf(strTime, "%04d%02d%02d_%02d%02d%02d", 
		currtime->tm_year + 1900, currtime->tm_mon + 1, currtime->tm_mday, 
		currtime->tm_hour, currtime->tm_min, currtime->tm_sec);
}

void _GetTempFileName(LPTSTR tmp_name)
{
    LPTSTR _path = _tgetenv("TEMP");
    if(_path == NULL)
        _path = _tgetenv("TMP");
    _stprintf(tmp_name, "%s\\%d", _path, time(NULL));
}

int _CopyFile(LPCTSTR src_name, LPCTSTR dest_name)
{
    FILE *fp = _tfopen(src_name, "rb");
    if(fp == NULL)
        return -1;
    int fsz = _GetFileSize(src_name);
    BYTE *pBuf = new BYTE[fsz];
    fread(pBuf, sizeof(BYTE), fsz, fp);
    fclose(fp);
    fp = _tfopen(dest_name, "wb");
    if(fp != NULL) {
        fwrite(pBuf, sizeof(BYTE), fsz, fp);
        fclose(fp);
    }
    delete [] pBuf;
    return 0;
}
