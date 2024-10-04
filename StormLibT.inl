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
// UNICODE versions of StormLib API

inline HANDLE SFileFindFirstFileW(HANDLE hMpq, LPCWSTR szMask, SFILE_FIND_DATA * lpFindFileData, LPCTSTR szListFile)
{
    return SFileFindFirstFile(hMpq, TWideToUTF8(szMask), lpFindFileData, szListFile);
}

inline bool SFileOpenFileExW(HANDLE hMpq, LPCWSTR szFileName, DWORD dwSearchScope, HANDLE * phFile)
{
    return SFileOpenFileEx(hMpq, TWideToUTF8(szFileName), dwSearchScope, phFile);
}

inline bool SFileAddFileExW(HANDLE hMpq, LPCTSTR szFileName, LPCWSTR szArchivedName, DWORD dwFlags, DWORD dwCompression, DWORD dwCompressionNext)
{
    return SFileAddFileEx(hMpq, szFileName, TWideToUTF8(szArchivedName), dwFlags, dwCompression, dwCompressionNext);
}

inline bool SFileRenameFileW(HANDLE hMpq, LPCWSTR szOldFileName, LPCWSTR szNewFileName)
{
    return SFileRenameFile(hMpq, TWideToUTF8(szOldFileName), TWideToUTF8(szNewFileName));
}

inline bool SFileRemoveFileW(HANDLE hMpq, LPCWSTR szFileName, DWORD dwSearchScope)
{
    return SFileRemoveFile(hMpq, TWideToUTF8(szFileName), dwSearchScope);
}

//-----------------------------------------------------------------------------
// Ansi/Wide detours

#if defined(UNICODE) || defined(_UNICODE)

#define SFileFindFirstFileT        SFileFindFirstFileW
#define SFileOpenFileExT           SFileOpenFileExW
#define SFileAddFileExT            SFileAddFileExW
#define SFileRenameFileT           SFileRenameFileW
#define SFileRemoveFileT           SFileRemoveFileW

#else

#define SFileFindFirstFileT        SFileFindFirstFile
#define SFileOpenFileExT           SFileOpenFileEx
#define SFileAddFileExT            SFileAddFileEx
#define SFileRenameFileT           SFileRenameFile
#define SFileRemoveFileT           SFileRemoveFile

#endif

