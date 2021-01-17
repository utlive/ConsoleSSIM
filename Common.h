#pragma once

#include <wtypes.h>
#include <time.h>
#include <string>
#include <tchar.h>
#include <malloc.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////
#define iY2RGB   76293 //  1.1641444 * 65536
#define iCr2R   104581 //  1.5957862 * 65536
#define iCb2R     -117 // -0.0017889 * 65536
#define iCr2G   -53312 // -0.8134821 * 65536
#define iCb2G   -25654 // -0.3914428 * 65536
#define iCr2B      -82 // -0.0012458 * 65536 
#define iCb2B   132240 //  2.0178255 * 65536


//////////////////////////////////////////////////////////////////////////

#define IS_WITHIN(x, min, max)  ((x)>=(min)&&(x)<(max))

#define _NEW_PTR(ptr, type) { if(ptr) delete ptr; ptr = new type; }
#define _DELETE_PTR(ptr)    { if(ptr) { delete ptr; ptr = NULL; } }

#define _ALIGNED_MALLOC_PTR(ptr, type, size)            { if(ptr) _aligned_free(ptr); ptr = (type *) _aligned_malloc(sizeof(type) * size, 16); }
#define _ALIGNED_FREE_PTR(ptr)                          { if(ptr) { _aligned_free(ptr); ptr = NULL; } }
#define _ALIGNED_MALLOC_IF_NULL_PTR(ptr, type, size)    { if(ptr == NULL) { ptr = (type *) _aligned_malloc(sizeof(type) * size, 16); } }

#define _NEW_PTRS(ptr, type, size)  { if(ptr) delete [] ptr; ptr = new type[size]; }
#define _DELETE_PTRS(ptr)           { if(ptr) { delete [] ptr; ptr = NULL; } }

#define _NEW_IF_NULL_PTRS(ptr, type, size)  { if(ptr == NULL) { ptr = new type[size]; } }

#define _RELEASE_COMPTR(x) { if(x.p) { x.Release(); x = NULL; } }

#define TO_PERCENT(a, b)	((int)((a) * 100 / (((b) == 0 ? 1: (b)))))

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

#define _DBG_MSG_PREFIX_TAG _T("[Stereo 1.0]")

#ifndef _DPRINTF
#if defined(_DEBUG) || defined(_DBG_MSG) || defined(_DBG_REL)
#if defined(_DBG_MSG) || defined(_DBG_REL)
#pragma message("Warning! _DPRINTF defined in release mode!") 
#endif
#define _DPRINTF(argument)	_dprintf##argument
#else
#define _DPRINTF(argument)
#endif
#endif

#if defined(_UNICODE) || defined(UNICODE)
typedef std::string std_tstring;
#else
typedef std::string std_tstring;
#endif


//////////////////////////////////////////////////////////////////////////

int _FileExist(LPCTSTR fname);
int _CopyFile(LPCTSTR src_name, LPCTSTR dest_name);
int _GetFileSize(LPCTSTR fname);
int _dprintf(LPCTSTR fmt, ...);
LPTSTR _TimeFormat(LONGLONG tmCode, LPTSTR _TimeFormat);
void _GetNowTimeStr(LPTSTR strTime);
void _GetTempFileName(LPTSTR tmp_name);

//////////////////////////////////////////////////////////////////////////
template <typename T>
inline T MJ_Fuzzy(T x, T y, float p){
	return std::max((T)0,pow( ( pow(x,p)+pow(y,p))*0.5f,1/p ));
}
inline std_tstring _GetFilePath(LPCTSTR _path) {
    _TCHAR drive[16];
    _TCHAR dir[256];
    _TCHAR fname[256];
    _TCHAR ext[16];
    _tsplitpath(_path, drive, dir, fname, ext);
    _TCHAR ret_str[256];
    _stprintf(ret_str, "%s%s", drive, dir);
    return std_tstring(ret_str);
}

inline std_tstring _GetFileBaseName(LPCTSTR _path) {
    _TCHAR drive[16];
    _TCHAR dir[256];
    _TCHAR fname[256];
    _TCHAR ext[16];
    _tsplitpath(_path, drive, dir, fname, ext);
    return std_tstring(fname);
}

inline std_tstring _GetFileExtName(LPCTSTR _path) {
    _TCHAR drive[16];
    _TCHAR dir[256];
    _TCHAR fname[256];
    _TCHAR ext[16];
    _tsplitpath(_path, drive, dir, fname, ext);
    return std_tstring(ext);
}

inline std_tstring _MakeFileFullPath(LPCTSTR _path, LPCTSTR _fullname) {
    _TCHAR ret_str[256];
    _stprintf(ret_str, "%s%s%s", _path, 
        (_path[_tcslen(_path) - 1] == '\\' || _path[_tcslen(_path) - 1] == '/') ? "": "\\", 
        _fullname);
    return std_tstring(ret_str);
}

inline std_tstring _GetFileFullName(LPCTSTR _path) {
    _TCHAR drive[16];
    _TCHAR dir[256];
    _TCHAR fname[256];
    _TCHAR ext[16];
    _tsplitpath(_path, drive, dir, fname, ext);
    std_tstring ret_str = fname;
    ret_str += ext;
    return std_tstring(ret_str);
}

inline std_tstring _ExtendFileName(LPCTSTR _path, LPCTSTR _tag, LPCTSTR _ext) {
    _TCHAR drive[16];
    _TCHAR dir[256];
    _TCHAR fname[256];
    _TCHAR ext[16];
    _tsplitpath(_path, drive, dir, fname, ext);
    _TCHAR ext_fn[256];
    _stprintf(ext_fn, "%s%s%s%s%s%s.%s", drive, dir, 
        (dir[_tcslen(dir) - 1] == '\\' || dir[_tcslen(dir) - 1] == '/') ? "": "\\", 
        fname, 
        _tag == NULL ? "": "-",
        _tag == NULL ? "": _tag, 
        _ext == NULL ? ext: _ext);
    return std_tstring(ext_fn);
}
class CTimeCost
{
	clock_t tm_start;
	clock_t tm_end;
public:
	CTimeCost() { }
	~CTimeCost() { }

	void start()
	{
		tm_start = clock();
	}
	void end()
	{
		tm_end = clock();
	}
	clock_t get_clocks()
	{
		return tm_end - tm_start;
	}
	float get_seconds()
	{
		return float(tm_end - tm_start) / CLOCKS_PER_SEC;
	}
};