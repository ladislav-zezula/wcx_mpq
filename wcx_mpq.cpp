/*****************************************************************************/
/* wcx_mpq.cpp                            Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* Main file for Total Commander MPQ plugin                                  */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 28.07.03  1.00  Lad  The first version of wcx_mpq.cpp                     */
/*****************************************************************************/

#include "wcx_mpq.h"                    // Our functions
#include "resource.h"                   // Resource symbols

#include "StormLibT.inl"                // UTF-8 and UNICODE support for StormLib

//-----------------------------------------------------------------------------
// Local variables

PFN_PROCESS_DATAA PfnProcessDataA;      // Process data procedure (ANSI)
PFN_PROCESS_DATAW PfnProcessDataW;      // Process data procedure (UNICODE)
PFN_CHANGE_VOLUMEA PfnChangeVolA;       // Change volume procedure (ANSI)
PFN_CHANGE_VOLUMEW PfnChangeVolW;       // Change volume procedure (UNICODE)

//-----------------------------------------------------------------------------
// CanYouHandleThisFile(W) allows the plugin to handle files with different
// extensions than the one defined in Total Commander
// https://www.ghisler.ch/wiki/index.php?title=CanYouHandleThisFile

static DWORD GetStreamFlagsFromFileName(LPCTSTR szFileName, bool bCheckReadOnly = false)
{
    DWORD dwBaseFlags = 0;

    // If the file has read-only attribute, then all read only
    if(bCheckReadOnly && (GetFileAttributes(szFileName) & FILE_ATTRIBUTE_READONLY))
        dwBaseFlags |= STREAM_FLAG_READ_ONLY;

    // Add flags based on file name
    while(szFileName[0] != 0)
    {
        // Did we find a possible extension?
        if(szFileName[0] == '.')
        {
            if(!_tcsicmp(szFileName, _T(".mpq.part")))
                return dwBaseFlags | STREAM_PROVIDER_PARTIAL | BASE_PROVIDER_FILE;

            if(!_tcsicmp(szFileName, _T(".MPQE")))
                return dwBaseFlags | STREAM_PROVIDER_MPQE | BASE_PROVIDER_FILE;

            if(!_tcsicmp(szFileName, _T(".MPQ.0")))
                return dwBaseFlags | STREAM_PROVIDER_BLOCK4 | BASE_PROVIDER_FILE;
        }

        // Move to the next character
        szFileName++;
    }

    return dwBaseFlags | STREAM_PROVIDER_FLAT | BASE_PROVIDER_FILE;
}

// This function is called when Total Commander tries to process an archive with the plugin
BOOL WINAPI CanYouHandleThisFileW(LPCWSTR szFileName)
{
    HANDLE hMpq = NULL;
    DWORD dwFlags = GetStreamFlagsFromFileName(szFileName) | MPQ_OPEN_NO_ATTRIBUTES | MPQ_OPEN_NO_LISTFILE | STREAM_FLAG_READ_ONLY;

    // Try to open the archive
    if(SFileOpenArchive(szFileName, 0, dwFlags, &hMpq))
    {
        SFileCloseArchive(hMpq);
        return TRUE;
    }
    return FALSE;
}

BOOL WINAPI CanYouHandleThisFile(LPCSTR szFileName)
{
    return CanYouHandleThisFileW(TAnsiToWide(szFileName));
}

//-----------------------------------------------------------------------------
// OpenArchive(W) should perform all necessary operations when an archive is
// to be opened
// https://www.ghisler.ch/wiki/index.php?title=OpenArchive

static void FileTimeToDosFTime(DOS_FTIME & DosTime, DWORD dwFileTimeHi, DWORD dwFileTimeLo)
{
    SYSTEMTIME stUTC;                   // Local file time
    SYSTEMTIME st;                      // Local file time
    FILETIME ft;                        // System file time

    ft.dwHighDateTime = dwFileTimeHi;
    ft.dwLowDateTime = dwFileTimeLo;

    FileTimeToSystemTime(&ft, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &st);

    DosTime.ft_year = st.wYear - 60;
    DosTime.ft_month = st.wMonth;
    DosTime.ft_day = st.wDay;
    DosTime.ft_hour = st.wHour;
    DosTime.ft_min = st.wMinute;
    DosTime.ft_tsec = (st.wSecond / 2);
}

static void GetArchiveTime(HANDLE hMPQ, DOS_FTIME & Time)
{
    TMPQArchive * ha = (TMPQArchive *)hMPQ;
    FILETIME ft;

    FileStream_GetTime(ha->pStream, (ULONGLONG *)&ft);
    FileTimeToDosFTime(Time, ft.dwHighDateTime, ft.dwLowDateTime);
}

static HANDLE OpenArchiveAW(TOpenArchiveData * pArchiveData, LPCWSTR szArchiveName)
{
    TOpenMpqInfo * pInfo = NULL;
    HANDLE hMPQ = NULL;
    size_t nSize;
    DWORD dwFileCount = 0;

    // Check the valid parameters
    if(pArchiveData && szArchiveName && szArchiveName[0])
    {
        // Check the valid archive access
        if(pArchiveData->OpenMode == PK_OM_LIST || pArchiveData->OpenMode == PK_OM_EXTRACT)
        {
            // Try to open the archive
            if(SFileOpenArchive(szArchiveName, 0, GetStreamFlagsFromFileName(szArchiveName, true), &hMPQ))
            {
                // Add the external listfile for the archive
                if(g_cfg.szListFile[0] != 0)
                    SFileAddListFile(hMPQ, g_cfg.szListFile);

                // Create the handle for Total Commander
                SFileGetFileInfo(hMPQ, SFileMpqBlockTableSize, &dwFileCount, sizeof(DWORD), NULL);
                nSize = sizeof(TOpenMpqInfo) + (dwFileCount * sizeof(BYTE));

                // Allocate the buffer
                pInfo = (TOpenMpqInfo *)HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, nSize);
                if(pInfo != NULL)
                {
                    // Fill the archive info
                    pInfo->dwMaxFileIndex = dwFileCount;
                    pInfo->dwSignature = ID_MPQ;
                    pInfo->nOpenMode = pArchiveData->OpenMode;
                    GetArchiveTime(hMPQ, pInfo->ArchiveTime);
                    pInfo->hMPQ = hMPQ;

                    // Return the open result
                    pArchiveData->OpenResult = ERROR_SUCCESS;
                    return (HANDLE)(pInfo);
                }
                else
                {
                    pArchiveData->OpenResult = E_NO_MEMORY;
                }
            }
            else
            {
                pArchiveData->OpenResult = E_UNKNOWN_FORMAT;
            }
        }
        else
        {
            pArchiveData->OpenResult = E_NOT_SUPPORTED;
        }
    }
    else
    {
        pArchiveData->OpenResult = E_NOT_SUPPORTED;
    }
    return NULL;
}

HANDLE WINAPI OpenArchiveW(TOpenArchiveData * pArchiveData)
{
    return OpenArchiveAW(pArchiveData, pArchiveData->szArchiveNameW);
}

HANDLE WINAPI OpenArchive(TOpenArchiveData * pArchiveData)
{
    return OpenArchiveAW(pArchiveData, TAnsiToWide(pArchiveData->szArchiveNameA));
}

//-----------------------------------------------------------------------------
// CloseArchive should perform all necessary operations when an archive
// is about to be closed. 
// https://www.ghisler.ch/wiki/index.php?title=CloseArchive

static TOpenMpqInfo * IsValidArchiveHandle(HANDLE hArchive)
{
    if(hArchive != NULL && hArchive != INVALID_HANDLE_VALUE)
    {
        if((static_cast<TOpenMpqInfo *>(hArchive))->dwSignature == ID_MPQ)
        {
            return static_cast<TOpenMpqInfo *>(hArchive);
        }
    }
    return NULL;
}

int WINAPI CloseArchive(HANDLE hArchive)
{
    TOpenMpqInfo * pInfo;

    // Check the right parameters
    if((pInfo = IsValidArchiveHandle(hArchive)) != NULL)
    {
        // Close the archive
        if(pInfo->hMPQ != NULL)
            SFileCloseArchive(pInfo->hMPQ);
        pInfo->hMPQ = NULL;

        // Free the structure itself
        HeapFree(g_hHeap, 0, pInfo);
        return ERROR_SUCCESS;
    }
    return E_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
// GetPackerCaps tells Totalcmd what features your packer plugin supports.
// https://www.ghisler.ch/wiki/index.php?title=GetPackerCaps

// GetPackerCaps tells Total Commander what features we support
int WINAPI GetPackerCaps()
{
    return(PK_CAPS_NEW |               // Can create new archives               
           PK_CAPS_MODIFY |            // Can modify existing archives          
           PK_CAPS_MULTIPLE |          // Archive can contain multiple files    
           PK_CAPS_DELETE |            // Can delete files                      
           PK_CAPS_OPTIONS |           // Has options dialog                    
//         PK_CAPS_MEMPACK |           // Supports packing in memory            
           PK_CAPS_BY_CONTENT |        // Detect archive type by content        
           PK_CAPS_SEARCHTEXT          // Allow searching for text in archives created with this plugin
//         PK_CAPS_HIDE                // Show as normal files (hide packer icon, open with Ctrl+PgDn, not Enter)
          );
}

//-----------------------------------------------------------------------------
// ProcessFile should unpack the specified file or test the integrity of the archive.
// https://www.ghisler.ch/wiki/index.php?title=ProcessFile

static int CallProcessDataProc(LPCWSTR szFullPath, int nSize)
{
    // Call UNICODE version of the callback, if needed
    if(PfnProcessDataW != NULL)
    {
        if(!PfnProcessDataW(szFullPath, nSize))
            return FALSE;
    }

    // Call ANSI version of callback, if needed
    if(PfnProcessDataA != NULL)
    {
        if(!PfnProcessDataA(TWideToAnsi(szFullPath), nSize))
            return FALSE;
    }

    // Continue the operation
    return TRUE;
}

static void MergePath(LPWSTR szFullPath, size_t ccFullPath, LPCWSTR szPath, LPCWSTR szName)
{
    // Always the buffer with zero
    if(ccFullPath != 0)
    {
        szFullPath[0] = 0;

        // Append destination path, if exists
        if(szPath && szPath[0])
        {
            StringCchCopy(szFullPath, ccFullPath, szPath);
        }

        // Append the name, if any
        if(szName && szName[0])
        {
            // Append backslash
            AddBackslash(szFullPath, ccFullPath);
            StringCchCat(szFullPath, ccFullPath, szName);
        }
    }
}

int WINAPI ProcessFileW(HANDLE hArchive, int nOperation, LPCWSTR szDestPath, LPCWSTR szDestName)
{
    TOpenMpqInfo * pInfo;
    HANDLE hLocFile = INVALID_HANDLE_VALUE;
    HANDLE hMpqFile = NULL;                     // File handle (Archived file)
    WCHAR szFullPath[MAX_PATH];
    int nResult = E_NOT_SUPPORTED;              // Result reported to Total Commander

    // Check whether it's the valid archive handle
    if((pInfo = IsValidArchiveHandle(hArchive)) != NULL)
    {
        // If verify or skip the file, do nothing
        if(nOperation == PK_TEST || nOperation == PK_SKIP)
            return 0;

        // Do we have to extract the file? If yes, the file must be saved in TOpenMpqInfo
        if(nOperation != PK_EXTRACT || pInfo->sf.cFileName[0] == 0)
            return E_NOT_SUPPORTED;

        // Set the locale that is intended to be open
        SFileSetLocale(pInfo->sf.lcLocale);

        // Open the source file
        if(SFileOpenFileEx(pInfo->hMPQ, pInfo->sf.cFileName, 0, &hMpqFile))
        {
            // Construct the full path name
            MergePath(szFullPath, _countof(szFullPath), szDestPath, szDestName);

            // Open the local file
            hLocFile = CreateFile(szFullPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
            if(hLocFile != INVALID_HANDLE_VALUE)
            {
                DWORD dwTotalBytes = 0;
                bool bEndOfFile = false;

                // Copy the file's content
                while(bEndOfFile == false)
                {
                    DWORD dwBytesWritten = 0;
                    DWORD dwBytesRead = 0;
                    BYTE Buffer[0x1000];

                    // Tell Total Commader what we are doing
                    if(!CallProcessDataProc(szFullPath, dwTotalBytes))
                        break;

                    // Read the source file
                    if(!SFileReadFile(hMpqFile, Buffer, sizeof(Buffer), &dwBytesRead, NULL))
                    {
                        if(GetLastError() != ERROR_HANDLE_EOF)
                        {
                            nResult = E_EREAD;
                            break;
                        }

                        nResult = ERROR_SUCCESS;
                        bEndOfFile = true;
                    }

                    // Write the target file
                    if(!WriteFile(hLocFile, Buffer, dwBytesRead, &dwBytesWritten, NULL))
                    {
                        nResult = E_EWRITE;
                        break;
                    }

                    // Increment the total bytes
                    dwTotalBytes += dwBytesRead;
                }

                // Close the local file
                CloseHandle(hLocFile);
            }
            else
            {
                nResult = E_ECREATE;
            }

            // Close the MPQ file
            SFileCloseFile(hMpqFile);
        }
        else
        {
            nResult = E_EREAD;
        }
    }
    return nResult;
}

int WINAPI ProcessFile(HANDLE hArchive, int nOperation, LPCSTR szDestPath, LPCSTR szDestName)
{
    return ProcessFileW(hArchive, nOperation, TAnsiToWide(szDestPath), TUTF8ToWide(szDestName));
}

//-----------------------------------------------------------------------------
// Totalcmd calls ReadHeader to find out what files are in the archive. 
// https://www.ghisler.ch/wiki/index.php?title=ReadHeader

static void StoreFileName(TOpenMpqInfo * pInfo, void * pvBuffer, size_t ccBuffer, bool bWideName)
{
    if(bWideName == false)
    {
        LPSTR szBuffer = (LPSTR)(pvBuffer);

        StringCchCopyExA(szBuffer, ccBuffer, pInfo->sf.cFileName, &szBuffer, &ccBuffer, 0);

        if(pInfo->sf.lcLocale && g_cfg.bAddLocaleToName)
        {
            StringCchPrintfA(szBuffer, ccBuffer, " (%04lX)", pInfo->sf.lcLocale);
        }
    }
    else
    {
        LPWSTR szBuffer = (LPWSTR)(pvBuffer);

        StringCchCopyExW(szBuffer, ccBuffer, TUTF8ToWide(pInfo->sf.cFileName), &szBuffer, &ccBuffer, 0);

        if(pInfo->sf.lcLocale && g_cfg.bAddLocaleToName)
        {
            StringCchPrintfW(szBuffer, ccBuffer, L" (%04lX)", pInfo->sf.lcLocale);
        }
    }
}

template <typename HDR>
static void StoreFoundFile(TOpenMpqInfo * pInfo, HDR * pHeaderData, bool bWideName)
{
    // Store the file name
    StoreFileName(pInfo, pHeaderData->FileName, _countof(pHeaderData->FileName), bWideName);

    // Store the file time
    pHeaderData->FileTime = pInfo->ArchiveTime;
    if(pInfo->sf.dwFileTimeHi != 0 || pInfo->sf.dwFileTimeLo != 0)
        FileTimeToDosFTime(pHeaderData->FileTime, pInfo->sf.dwFileTimeHi, pInfo->sf.dwFileTimeLo);

    // Store the file sizes
    pHeaderData->UnpSize = pInfo->sf.dwFileSize;
    pHeaderData->PackSize = pInfo->sf.dwCompSize;

    // Store file attributes
    pHeaderData->FileAttr = FILE_ATTRIBUTE_ARCHIVE;
    if(pInfo->sf.dwFileFlags & MPQ_FILE_COMPRESS_MASK)
        pHeaderData->FileAttr |= FILE_ATTRIBUTE_COMPRESSED;
    if(pInfo->sf.dwFileFlags & MPQ_FILE_ENCRYPTED)
        pHeaderData->FileAttr |= FILE_ATTRIBUTE_ENCRYPTED;
    pInfo->FoundFile[pInfo->sf.dwBlockIndex] = TRUE;
}

template <typename HDR>
int ReadHeaderTemplate(HANDLE hArchive, HDR * pHeaderData, bool bWideName = false)
{
    TOpenMpqInfo * pInfo;

    // Check the right parameters
    if((pInfo = IsValidArchiveHandle(hArchive)) != NULL)
    {
        // Split the action 
        for(;;)
        {
            switch(pInfo->LoadState)
            {
                case LOADSTATE_FIND_FIRST:
                    pInfo->hFind = SFileFindFirstFile(pInfo->hMPQ, "*", &pInfo->sf, NULL);
                    pInfo->bResult = TRUE;

                    // If mothing has been found, go to the "FindNext" phase
                    if(pInfo->hFind == NULL)
                    {
                        pInfo->LoadState = LOADSTATE_COMPLETE;
                        break;
                    }

                    StoreFoundFile(pInfo, pHeaderData, bWideName);
                    pInfo->LoadState = LOADSTATE_FIND_NEXT;
                    return 0;

                case LOADSTATE_FIND_NEXT:

                    // Search the next file
                    if(pInfo->hFind != NULL)
                    {
                        pInfo->bResult = SFileFindNextFile(pInfo->hFind, &pInfo->sf);

                        if(pInfo->bResult == FALSE)
                        {
                            pInfo->LoadState = LOADSTATE_COMPLETE;
                            break;
                        }

                        StoreFoundFile(pInfo, pHeaderData, bWideName);
                        return 0;
                    }

                    // Nothing has been found.
                    pInfo->LoadState = LOADSTATE_COMPLETE;
                    break;

                case LOADSTATE_COMPLETE:    // No more files in the archive.
                default:
                    return E_END_ARCHIVE;
            }
        }
    }
    return E_NOT_SUPPORTED;
}

int WINAPI ReadHeader(HANDLE hArchive, THeaderData * pHeaderData)
{
    // Use the common template function
    return ReadHeaderTemplate(hArchive, pHeaderData);
}

int WINAPI ReadHeaderEx(HANDLE hArchive, THeaderDataEx * pHeaderData)
{
    // Use the common template function
    return ReadHeaderTemplate(hArchive, pHeaderData);
}

int WINAPI ReadHeaderExW(HANDLE hArchive, THeaderDataExW * pHeaderData)
{
    // Use the common template function
    return ReadHeaderTemplate(hArchive, pHeaderData, true);
}

//-----------------------------------------------------------------------------
// SetChangeVolProc(W) allows you to notify user about changing a volume when packing files
// https://www.ghisler.ch/wiki/index.php?title=SetChangeVolProc

// This function allows you to notify user 
// about changing a volume when packing files
void WINAPI SetChangeVolProc(HANDLE /* hArchive */, PFN_CHANGE_VOLUMEA PfnChangeVol)
{
    PfnChangeVolA = PfnChangeVol;
}

void WINAPI SetChangeVolProcW(HANDLE /* hArchive */, PFN_CHANGE_VOLUMEW PfnChangeVol)
{
    PfnChangeVolW = PfnChangeVol;
}

//-----------------------------------------------------------------------------
// SetProcessDataProc(W) allows you to notify user about the progress when you un/pack files
// Note that Total Commander may use INVALID_HANDLE_VALUE for the hArchive parameter
// https://www.ghisler.ch/wiki/index.php?title=SetProcessDataProc

void WINAPI SetProcessDataProc(HANDLE /* hArchive */, PFN_PROCESS_DATAA PfnProcessData)
{
    PfnProcessDataA = PfnProcessData;
}

void WINAPI SetProcessDataProcW(HANDLE /* hArchive */, PFN_PROCESS_DATAW PfnProcessData)
{
    PfnProcessDataW = PfnProcessData;
}

//-----------------------------------------------------------------------------
// PackFiles(W) specifies what should happen when a user creates, or adds files to the archive
// https://www.ghisler.ch/wiki/index.php?title=PackFiles

typedef struct _SFILE_CALLBACK_DATA
{
    LPCWSTR szFileName;
    DWORD dwWritten;
} SFILE_CALLBACK_DATA, * PSFILE_CALLBACK_DATA;

static void WINAPI AddFileCallback(void * pvUserData, DWORD dwBytesWritten, DWORD dwTotalBytes, bool bFinalCall)
{
    PSFILE_CALLBACK_DATA pCBData = (PSFILE_CALLBACK_DATA)pvUserData;

    UNREFERENCED_PARAMETER(dwTotalBytes);
    UNREFERENCED_PARAMETER(bFinalCall);

    // Inform Total Commander about what are we doing
    CallProcessDataProc(pCBData->szFileName, dwBytesWritten - pCBData->dwWritten);
    pCBData->dwWritten = dwBytesWritten;
}

static DWORD FixupMaxFileCount(LPCWSTR szAddList, DWORD dwMaxFileCount)
{
    LPCWSTR szAddFile;
    DWORD dwFilesToAdd = 0;

    for(szAddFile = szAddList; *szAddFile != 0; szAddFile += wcslen(szAddFile) + 1)
    {
        if(++dwFilesToAdd > dwMaxFileCount)
        {
            dwMaxFileCount *= 2;
        }
    }
    return dwMaxFileCount;
}

static LPWSTR NewMulStr(LPCSTR szMulStr)
{
    LPCSTR szSaveMulStr = szMulStr;
    LPWSTR szMulStrW = NULL;
    size_t nLength = 0;

    if(szMulStr != NULL)
    {
        // Count the length of the multi string
        while(szMulStr[0] != 0)
        {
            szMulStr = szMulStr + strlen(szMulStr) + 1;
        }

        // Allocate space
        nLength = (szMulStr - szSaveMulStr + 1);
        if((szMulStrW = new WCHAR[nLength]) != NULL)
        {
            MultiByteToWideChar(CP_ACP, 0, szSaveMulStr, (int)nLength, szMulStrW, (int)nLength);
        }
    }

    return szMulStrW;
}

static bool IsFileExcludedFromCompressionEncryption(LPCTSTR szFileName)
{
    LPCTSTR szExcludedTypes = g_cfg.szExcludedTypes;
    LPCTSTR szExt;
    size_t nLength;

    // Get the file extension
    szExt = GetFileExtension(szFileName);
    if(szExt++ != NULL)
    {
        nLength = _tcslen(szExt);
        while(*szExcludedTypes != 0)
        {
            // Skip spaces
            while(0 < *szExcludedTypes && *szExcludedTypes <= 0x20)
                szExcludedTypes++;

            // Compare the extension
            if(!_tcsnicmp(szExcludedTypes, szExt, nLength))
            {
                if(szExcludedTypes[nLength] == ';' || szExcludedTypes[nLength] == 0)
                    return true;
            }

            // Skip the current extension
            while(*szExcludedTypes != 0 && *szExcludedTypes != ';')
                szExcludedTypes++;
            while(*szExcludedTypes == ';')
                szExcludedTypes++;
        }
    }

    // Not in any of the excluded types
    return false;
}

static bool OpenOrCreateArchive(LPCWSTR szPackedFile, DWORD dwCreateFlags, DWORD dwMaxFileCount, PHANDLE PtrMPQ)
{
    // Try to open an existing archive
    if(SFileOpenArchive(szPackedFile, 0, GetStreamFlagsFromFileName(szPackedFile, false), PtrMPQ))
        return true;

    // ERROR_FILE_NOT_FOUND: Create new archive
    // ERROR_BAD_FORMAT: Convert existing file to a MPQ archive
    if(SFileCreateArchive(szPackedFile, dwCreateFlags, dwMaxFileCount, PtrMPQ))
        return true;

    return false;
}

static int PackFileToArchiveW(
    HANDLE hMPQ,
    LPCWSTR szTrgPath,
    LPCWSTR szSrcPath,
    DWORD dwMpqFlags,
    DWORD dwDataCompression,
    int nFlags)
{
    SFILE_CALLBACK_DATA CBData = {0};

    // Prepare the add callback
    CBData.szFileName = szSrcPath;
    CBData.dwWritten = 0;
    SFileSetAddFileCallback(hMPQ, AddFileCallback, &CBData);

    // Check if the file is one of the files excluded from compression and encryption
    if(IsFileExcludedFromCompressionEncryption(szSrcPath))
        dwMpqFlags &= ~(MPQ_FILE_COMPRESS_MASK | MPQ_FILE_ENCRYPTED);

    // Set the locale that the file should have
    SFileSetLocale(g_cfg.lcFileLocale);

    // Add the file to the archive
    if(SFileAddFileExW(hMPQ, szSrcPath, szTrgPath, dwMpqFlags, dwDataCompression, dwDataCompression))
    {
        if(nFlags & PK_PACK_MOVE_FILES)
            DeleteFile(szSrcPath);
        return ERROR_SUCCESS;
    }
    return E_EWRITE;
}

static int PackFilesToArchiveW(
    HANDLE hMPQ,
    LPCWSTR szSubPath,
    LPCWSTR szSrcPath,
    LPCWSTR szAddList,
    DWORD dwDefMpqFlags,
    DWORD dwDefDataCompression,
    int nFlags)
{
    TCHAR szSourcePath[MAX_PATH];
    TCHAR szTargetPath[MAX_PATH];
    int nResult = 0;

    // Now pass the add list
    for(LPCWSTR szAddFile = szAddList; szAddFile[0] != 0 && nResult == 0; szAddFile += wcslen(szAddFile) + 1)
    {
        // Create full path of the source file
        MergePath(szSourcePath, _countof(szSourcePath), szSrcPath, szAddFile);
        MergePath(szTargetPath, _countof(szTargetPath), szSubPath, szAddFile);

        // Check the attributes. When there's a folder, we ignore it, because
        // Total Commander includes all files in the folder after the folder name
        if(GetFileAttributes(szSourcePath) & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        // Tell Total Commander what file we are adding
        if(!CallProcessDataProc(szAddFile, 0))
            break;

        // Add the file to the archive
        nResult = PackFileToArchiveW(hMPQ,
                                     szTargetPath,
                                     szSourcePath,
                                     dwDefMpqFlags,
                                     dwDefDataCompression,
                                     nFlags);
    }

    return nResult;
}

int WINAPI PackFilesW(LPCWSTR szPackedFile, LPCWSTR szSubPath, LPCWSTR szSrcPath, LPCWSTR szAddList, int nFlags)
{
    HANDLE hMPQ = NULL;
    DWORD dwDefDataCompression = 0;
    DWORD dwMaxFileCount = g_cfg.dwMaxFileCount;
    DWORD dwCreateFlags = MPQ_CREATE_ATTRIBUTES;
    DWORD dwDefMpqFlags = MPQ_FILE_REPLACEEXISTING;
    int nResult = E_NOT_SUPPORTED;

    // Test the valid parameters
    if(szPackedFile && szPackedFile[0] && szAddList && szAddList[0])
    {
        // Count the files. If count is bigger than hash table size,
        // increment the hash table size.
        dwMaxFileCount = FixupMaxFileCount(szAddList, g_cfg.dwMaxFileCount);

        // Choose the data compression method
        switch(g_cfg.dwCompression)
        {
            case FILE_COMPRESSION_INFLATE:
                dwDefDataCompression = MPQ_COMPRESSION_PKWARE;
                dwDefMpqFlags |= MPQ_FILE_COMPRESS;
                break;

            case FILE_COMPRESSION_ZLIB:
                dwDefDataCompression = MPQ_COMPRESSION_ZLIB;
                dwDefMpqFlags |= MPQ_FILE_COMPRESS;
                break;

            case FILE_COMPRESSION_BZIP2:
                dwDefDataCompression = MPQ_COMPRESSION_BZIP2;
                dwDefMpqFlags |= MPQ_FILE_COMPRESS;
                break;

            case FILE_COMPRESSION_LZMA:
                dwDefDataCompression = MPQ_COMPRESSION_LZMA;
                dwDefMpqFlags |= MPQ_FILE_COMPRESS;
                break;
        }

        // Choose the encryption
        if(g_cfg.bEncryptAddedFiles)
            dwDefMpqFlags |= MPQ_FILE_ENCRYPTED;

        // Choose archive format
        if(g_cfg.bCreateMpqV2)
            dwCreateFlags |= MPQ_CREATE_ARCHIVE_V2;

        // Check the file name and get the appropriate flags
        if(OpenOrCreateArchive(szPackedFile, dwCreateFlags, dwMaxFileCount, &hMPQ))
        {
            // (Recursively) add all files to that archive
            nResult = PackFilesToArchiveW(hMPQ,
                                          szSubPath,
                                          szSrcPath,
                                          szAddList,
                                          dwDefMpqFlags,
                                          dwDefDataCompression,
                                          nFlags);
            SFileCloseArchive(hMPQ);
        }
        else
        {
            nResult = E_BAD_ARCHIVE;
        }
    }
    return nResult;
}

// PackFiles adds file(s) to an archive
int WINAPI PackFiles(LPCSTR szPackedFile, LPCSTR szSubPath, LPCSTR szSrcPath, LPCSTR szAddList, int nFlags)
{
    LPWSTR szAddListW;
    int nResult = E_OUTOFMEMORY;

    if((szAddListW = NewMulStr(szAddList)) != NULL)
    {
        nResult = PackFilesW(TAnsiToWide(szPackedFile),
                             TAnsiToWide(szSubPath),
                             TAnsiToWide(szSrcPath),
                             szAddListW,
                             nFlags);
        delete [] szAddListW;
    }
    return nResult;
}

//-----------------------------------------------------------------------------
// DeleteFiles(W) should delete the specified files from the archive 
// https://www.ghisler.ch/wiki/index.php?title=DeleteFiles

static LPWSTR ExtractLocaleFromFileName(LPCWSTR szFileNameWithLocale, LCID * PtrLocale)
{
    LCID lcLocale = 0;
    LPWSTR szLocalePtr;
    LPWSTR szLocaleEnd;
    LPWSTR szFileName;
    LPWSTR szTemp;
    size_t nLength;

    // Create the copy of the file name
    if((szFileName = NewStr(szFileNameWithLocale)) != NULL)
    {
        // We expect the format "%s (%u)"
        if((nLength = wcslen(szFileName)) > 0)
        {
            // Does the name end with closing parenthesis?
            if(szFileName[nLength - 1] == ')')
            {
                // Walk back as long as we found a digit
                szLocaleEnd = szFileName + nLength - 1;
                szLocalePtr = szLocaleEnd - 1;
                while(szLocalePtr > szFileName && isxdigit(szLocalePtr[0]))
                    szLocalePtr--;

                // Did we encounter an opening parenthesis?
                if(szLocalePtr > (szFileName + 1) && szLocalePtr[0] == '(' && szLocalePtr[-1] == ' ' && (szLocaleEnd - szLocalePtr) == 5)
                {
                    // Convert the locale to integer
                    lcLocale = wcstol(szLocalePtr + 1, &szTemp, 16);

                    // Cut the locale from the file name
                    szLocalePtr[-1] = 0;
                }
            }
        }

        // Give the locale to the caller
        if(PtrLocale != NULL)
        {
            PtrLocale[0] = lcLocale;
        }
    }

    return szFileName;
}

static int RemoveFiles(HANDLE hMpq, LPCWSTR szMask, LPCWSTR szListFile, LCID lcLocale)
{
    SFILE_FIND_DATA sf;
    HANDLE hFind;
    BOOL bFileFound = TRUE;
    int nResult = ERROR_SUCCESS;

    // We always need to search the archive for the file mask
    hFind = SFileFindFirstFileW(hMpq, szMask, &sf, szListFile);
    if(hFind != NULL)
    {
        // Keep deleting as long as there is something to delete
        while(bFileFound)
        {
            // Only delete files with matching locale
            if(lcLocale == 0 || sf.lcLocale == lcLocale)
            {
                // Set the proper file locale
                SFileSetLocale(lcLocale);

                // Attempt to remove the file
                if(!SFileRemoveFile(hMpq, sf.cFileName, SFILE_OPEN_FROM_MPQ))
                {
                    nResult = E_BAD_DATA;
                    break;
                }
            }

            // Attempt to find the next file
            bFileFound = SFileFindNextFile(hFind, &sf);
        }

        // Close the search handle
        SFileFindClose(hFind);
    }
    return nResult;
}

// Delete files from the archive
int WINAPI DeleteFilesW(LPCWSTR szPackedFile, LPCWSTR szDeleteList)
{
    HANDLE hMpq = NULL;
    LPCTSTR szListFile = (g_cfg.szListFile[0] != 0) ? g_cfg.szListFile : NULL;
    LPCWSTR szFileToDelete = NULL;
    LPCWSTR szDeleteEntry = NULL;
    int nDeleteCount = 0;
    int nResult = ERROR_SUCCESS;

    // Test the valid parameters
    if(szPackedFile && szPackedFile[0] && szDeleteList && szDeleteList[0])
    {
        // Open the archive
        if(SFileOpenArchive(szPackedFile, 0, 0, &hMpq))
        {
            // Delete all files from the archive.
            for(szDeleteEntry = szDeleteList; szDeleteEntry[0] != 0; szDeleteEntry = szDeleteEntry + wcslen(szDeleteEntry) + 1)
            {
                LCID lcLocale = 0;

                // Tell Total Commander what we are doing
                if(!CallProcessDataProc(szDeleteEntry, nDeleteCount++))
                    break;

                // If we allow the locale to be part of file name, we have to extract it from there
                szFileToDelete = szDeleteEntry;
                if(g_cfg.bAddLocaleToName)
                    szFileToDelete = ExtractLocaleFromFileName(szDeleteEntry, &lcLocale);

                // First try internal listfile
                RemoveFiles(hMpq, szFileToDelete, NULL, lcLocale);

                // Then external listfile, if any
                if(g_cfg.szListFile[0] != 0)
                    RemoveFiles(hMpq, szFileToDelete, g_cfg.szListFile, lcLocale);

                // Free allocated memory
                if(szFileToDelete != szDeleteEntry)
                    delete[] szFileToDelete;
            }

            // Compact the archive
            if(nResult == ERROR_SUCCESS && g_cfg.bCompactAfterDelete)
                SFileCompactArchive(hMpq, szListFile, false);

            // Close the archive and exit
            SFileCloseArchive(hMpq);
        }
        else
        {
            nResult = E_BAD_ARCHIVE;
        }
    }
    else
    {
        nResult = E_NOT_SUPPORTED;
    }
    return nResult;
}

int WINAPI DeleteFiles(LPCSTR szPackedFile, LPCSTR szDeleteList)
{
    LPWSTR szDeleteListW;
    int nResult = E_OUTOFMEMORY;

    if((szDeleteListW = NewMulStr(szDeleteList)) != NULL)
    {
        nResult = DeleteFilesW(TAnsiToWide(szPackedFile), szDeleteListW);
        delete [] szDeleteListW;
    }
    return nResult;
}

//-----------------------------------------------------------------------------
// ConfigurePacker gets called when the user clicks the Configure button
// from within "Pack files..." dialog box in Totalcmd. 
// https://www.ghisler.ch/wiki/index.php?title=ConfigurePacker

void WINAPI ConfigurePacker(HWND hParent, HINSTANCE hDllInstance)
{
    UNREFERENCED_PARAMETER(hDllInstance);

    SettingsDialog(hParent);
}

//-----------------------------------------------------------------------------
// PackSetDefaultParams is called immediately after loading the DLL, before
// any other function. This function is new in version 2.1. It requires Total
// Commander >=5.51, but is ignored by older versions. 
// https://www.ghisler.ch/wiki/index.php?title=PackSetDefaultParams

void WINAPI PackSetDefaultParams(TPackDefaultParamStruct * dps)
{
    // Set default configuration.
    SetDefaultConfiguration();
    g_szIniFile[0] = 0;

    // If INI file, load it from it too.
    if(dps != NULL && dps->DefaultIniName[0])
    {
        StringCchCopyX(g_szIniFile, _countof(g_szIniFile), dps->DefaultIniName);
        LoadConfiguration();
    }
}
