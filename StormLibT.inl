/*****************************************************************************/
/* StormLibT.inl                          Copyright (c) Ladislav Zezula 2023 */
/*---------------------------------------------------------------------------*/
/* Unicode layer for StormLib functions                                      */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 04.07.23  1.00  Lad  Created                                              */
/*****************************************************************************/

//-----------------------------------------------------------------------------
// Classes for conversions UNICODE <--> UTF-8

typedef TConvertString<WCHAR, char, CP_UTF8> TWideToUTF8;
typedef TConvertString<char, WCHAR, CP_UTF8> TUTF8ToWide;
typedef TConvertString<char, WCHAR, CP_ACP>  TAnsiToWide;
typedef TConvertString<WCHAR, char, CP_ACP>  TWideToAnsi;

//-----------------------------------------------------------------------------
// UNICODE versions of StormLib API

inline HANDLE SFileFindFirstFileW(HANDLE hMpq, LPCWSTR szMask, SFILE_FIND_DATA * lpFindFileData, LPCTSTR szListFile)
{
    return SFileFindFirstFile(hMpq, TWideToMPQ8(szMask), lpFindFileData, szListFile);
}

inline bool SFileOpenFileExW(HANDLE hMpq, LPCWSTR szFileName, DWORD dwSearchScope, HANDLE * phFile)
{
    return SFileOpenFileEx(hMpq, TWideToMPQ8(szFileName), dwSearchScope, phFile);
}

inline bool SFileAddFileExW(HANDLE hMpq, LPCTSTR szFileName, LPCWSTR szArchivedName, DWORD dwFlags, DWORD dwCompression, DWORD dwCompressionNext)
{
    return SFileAddFileEx(hMpq, szFileName, TWideToMPQ8(szArchivedName), dwFlags, dwCompression, dwCompressionNext);
}

inline bool SFileRenameFileW(HANDLE hMpq, LPCWSTR szOldFileName, LPCWSTR szNewFileName)
{
    return SFileRenameFile(hMpq, TWideToMPQ8(szOldFileName), TWideToMPQ8(szNewFileName));
}

//-----------------------------------------------------------------------------
// Ansi/Wide detours

#if defined(UNICODE) || defined(_UNICODE)

#define SFileFindFirstFileT        SFileFindFirstFileW
#define SFileOpenFileExT           SFileOpenFileExW
#define SFileAddFileExT            SFileAddFileExW
#define SFileRenameFileT           SFileRenameFileW

#else

#define SFileFindFirstFileT        SFileFindFirstFile
#define SFileOpenFileExT           SFileOpenFileEx
#define SFileAddFileExT            SFileAddFileEx
#define SFileRenameFileT           SFileRenameFile

#endif

