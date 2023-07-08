/*****************************************************************************/
/* wcx_mpq.h                              Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* Main header file for the wcx_mpq plugin                                   */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* xx.xx.xx  0.00  Pol  Created by Konstantin Polyakov                       */
/* 13.07.03  1.00  Lad  Derived from wcxapi.h                                */
/*****************************************************************************/

#ifndef __WCX_MPQ_H__
#define __WCX_MPQ_H__

#pragma warning (disable: 4091)         // 4091: 'typedef ': ignored on left of 'tagDTI_ADTIWUI' when no variable is declared
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <strsafe.h>

// Include StormLib
#include "StormLib.h"
#include "Utils.h"                          // Utils functions

//-----------------------------------------------------------------------------
// Defines

/* Error codes returned to calling application */
#define E_END_ARCHIVE     10                // No more files in archive
#define E_NO_MEMORY       11                // Not enough memory
#define E_BAD_DATA        12                // Data is bad
#define E_BAD_ARCHIVE     13                // CRC error in archive data
#define E_UNKNOWN_FORMAT  14                // Archive format unknown
#define E_EOPEN           15                // Cannot open existing file
#define E_ECREATE         16                // Cannot create file
#define E_ECLOSE          17                // Error closing file
#define E_EREAD           18                // Error reading from file
#define E_EWRITE          19                // Error writing to file
#define E_SMALL_BUF       20                // Buffer too small
#define E_EABORTED        21                // Function aborted by user
#define E_NO_FILES        22                // No files found
#define E_TOO_MANY_FILES  23                // Too many files to pack
#define E_NOT_SUPPORTED   24                // Function not supported

/* flags for ProcessFile */
#define PK_SKIP             0               // Skip this file
#define PK_TEST             1               // Test file integrity
#define PK_EXTRACT          2               // Extract to disk

/* Flags passed through PFN_CHANGE_VOL */
#define PK_VOL_ASK          0               // Ask user for location of next volume
#define PK_VOL_NOTIFY       1               // Notify app that next volume will be unpacked

/* Flags for packing */

/* For PackFiles */
#define PK_PACK_MOVE_FILES  1               // Delete original after packing
#define PK_PACK_SAVE_PATHS  2               // Save path names of files

/* Returned by GetPackCaps */
#define PK_CAPS_NEW         1               // Can create new archives
#define PK_CAPS_MODIFY      2               // Can modify existing archives
#define PK_CAPS_MULTIPLE    4               // Archive can contain multiple files
#define PK_CAPS_DELETE      8               // Can delete files
#define PK_CAPS_OPTIONS    16               // Has options dialog
#define PK_CAPS_MEMPACK    32               // Supports packing in memory
#define PK_CAPS_BY_CONTENT 64               // Detect archive type by content
#define PK_CAPS_SEARCHTEXT 128              // Allow searching for text in archives
                                            // created with this plugin}
#define PK_CAPS_HIDE       256              // Show as normal files (hide packer
                                            // icon), open with Ctrl+PgDn, not Enter
/* Flags for packing in memory */
#define MEM_OPTIONS_WANTHEADERS 1           // Return archive headers with packed data

#define MEMPACK_OK          0               // Function call finished OK, but there is more data
#define MEMPACK_DONE        1               // Function call finished OK, there is no more data

#define FILE_COMPRESSION_NONE     0
#define FILE_COMPRESSION_INFLATE  1
#define FILE_COMPRESSION_ZLIB     2
#define FILE_COMPRESSION_BZIP2    3
#define FILE_COMPRESSION_LZMA     4

#define INVALID_SIZE_T          (size_t)(-1)

//-----------------------------------------------------------------------------
// Definitions of callback functions

// Ask to swap disk for multi-volume archive
typedef int (WINAPI * PFN_CHANGE_VOLUMEA)(LPCSTR szArcName, int nMode);
typedef int (WINAPI * PFN_CHANGE_VOLUMEW)(LPCWSTR szArcName, int nMode);

// Notify that data is processed - used for progress dialog
typedef int (WINAPI * PFN_PROCESS_DATAA)(LPCSTR szFileName, int nSize);
typedef int (WINAPI * PFN_PROCESS_DATAW)(LPCWSTR szFileName, int nSize);

//-----------------------------------------------------------------------------
// Structures

struct DOS_FTIME
{
    unsigned ft_tsec  : 5;                  // Two second interval
    unsigned ft_min   : 6;                  // Minutes
    unsigned ft_hour  : 5;                  // Hours
    unsigned ft_day   : 5;                  // Days
    unsigned ft_month : 4;                  // Months
    unsigned ft_year  : 7;                  // Year
};

struct THeaderData
{
    char      ArcName[260];
    char      FileName[260];
    DWORD     Flags;
    DWORD     PackSize;
    DWORD     UnpSize;
    DWORD     HostOS;
    DWORD     FileCRC;
    DOS_FTIME FileTime;
    DWORD     UnpVer;
    DWORD     Method;
    DWORD     FileAttr;
    char    * CmtBuf;
    DWORD     CmtBufSize;
    DWORD     CmtSize;
    DWORD     CmtState;
};

#pragma pack(push, 4)
struct THeaderDataEx
{
    char      ArcName[1024];
    char      FileName[1024];
    DWORD     Flags;
    ULONGLONG PackSize;
    ULONGLONG UnpSize;
    DWORD     HostOS;
    DWORD     FileCRC;
    DOS_FTIME FileTime;
    DWORD     UnpVer;
    DWORD     Method;
    DWORD     FileAttr;
    char    * CmtBuf;
    DWORD     CmtBufSize;
    DWORD     CmtSize;
    DWORD     CmtState;
    char Reserved[1024];
};

struct THeaderDataExW
{
    WCHAR ArcName[1024];
    WCHAR FileName[1024];
    DWORD Flags;
    ULONGLONG PackSize;
    ULONGLONG UnpSize;
    DWORD HostOS;
    DWORD FileCRC;
    DOS_FTIME FileTime;
    DWORD UnpVer;
    DWORD Method;
    DWORD FileAttr;
    char* CmtBuf;
    DWORD CmtBufSize;
    DWORD CmtSize;
    DWORD CmtState;
    char Reserved[1024];
};
#pragma pack(pop, 4)

//-----------------------------------------------------------------------------
// Open archive information

#define PK_OM_LIST          0
#define PK_OM_EXTRACT       1

struct TOpenArchiveData
{
    union
    {
        LPCSTR szArchiveNameA;              // Archive name to open
        LPCWSTR szArchiveNameW;             // Archive name to open
    };

    int    OpenMode;                        // Open reason (See PK_OM_XXXX)
    int    OpenResult;
    char * CmtBuf;
    int    CmtBufSize;
    int    CmtSize;
    int    CmtState;
};

typedef struct
{
	int   size;
	DWORD PluginInterfaceVersionLow;
	DWORD PluginInterfaceVersionHi;
	char  DefaultIniName[MAX_PATH];
} TPackDefaultParamStruct;

//-----------------------------------------------------------------------------
// Information about loading archive

typedef enum _LDST
{
    LOADSTATE_FIND_FIRST,                   // Loading files using listfile (first)
    LOADSTATE_FIND_NEXT,                    // Loading files using listfile (first)
    LOADSTATE_COMPLETE,                     // Complete
} LDST, *PLDST;

// Our structure describing open archive archive
struct TOpenMpqInfo
{
    ULONGLONG dwSignature;                  // Handle signature
    //PFN_PROCESS_DATAA PfnProcessDataA;    // Process data procedure (ANSI)
    //PFN_PROCESS_DATAW PfnProcessDataW;    // Process data procedure (UNICODE)
    //PFN_CHANGE_VOLUMEA PfnChangeVolA;     // Change volume procedure (ANSI)
    //PFN_CHANGE_VOLUMEW PfnChangeVolW;     // Change volume procedure (UNICODE)
    SFILE_FIND_DATA sf;                     // Current search record
    DOS_FTIME ArchiveTime;                  // Date and time of the archive
    HANDLE hMPQ;                            // Handle of open archive
    HANDLE hFind;                           // Search handle
    HANDLE hStopEvent;                      // Event saying "stop" to the worker thread(s);
    DWORD dwBytesProcessed;                 // Number of bytes processed since last call of ProcessDataProc
    DWORD dwMaxFileIndex;                   // Size of block table (number of files)
    DWORD dwLastFileIndex;                  // Last file indes that was tried to open
    char * szFullPath;                      // Full path of the file currently being processed
    char * szDestPath;                      // Dest path
    char * szDestName;
    BOOL   bResult;                         // Last result of SFileFindNextFile
    int nOpenMode;                          // The mode which the archive has been open for
    LDST LoadState;                         // Current load state. See LOADSTATE_XXXX definitions
    BYTE FoundFile[1];                      // List of BYTEs indicating that a file has been already found (variable length)
};

//-----------------------------------------------------------------------------
// Configuration structures

typedef struct
{
    DWORD dwMaxFileCount;                   // Maximum file count for newly created files

    DWORD dwCompression;                    // Compression for newly added files

    LCID  lcFileLocale;                     // Locale for newly added files

    BOOL  bCreateMpqV2;                     // If TRUE, plugin will create archives version 2
                                            // (supports archive sizes > 4BG)
    BOOL  bAddLocaleToName;                 // If TRUE, the file name will contain the locale
                                            // ID, e.g. "File.ext (40A)"
    BOOL  bEncryptAddedFiles;               // If TRUE, the newly added files will be encrypted
                                        
    BOOL  bCompactAfterDelete;              // If TRUE, the plugin will compact the MPQ archive
                                            // after a change operation
    TCHAR szListFile[MAX_PATH];             // External listfile

    TCHAR szExcludedTypes[0x400];           // File types that are excluded from compression

} TConfiguration;

//-----------------------------------------------------------------------------
// Global variables

extern TConfiguration g_cfg;                // Plugin configuration
extern HINSTANCE g_hInst;                   // Our DLL instance
extern HANDLE g_hHeap;                      // Process heap
extern TCHAR g_szIniFile[MAX_PATH];         // Packer INI file

//-----------------------------------------------------------------------------
// Global functions

// Configuration functions
int SetDefaultConfiguration();
int LoadConfiguration();
int SaveConfiguration();

//-----------------------------------------------------------------------------
// Dialogs

INT_PTR SettingsDialog(HWND hParent);

#endif // __WCX_MPQ_H__
