/*****************************************************************************/
/* Config.cpp                             Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* Functions for loading, editing and saving configuration of wcx_mpq        */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 28.07.03  1.00  Lad  The first version of Config.cpp                      */
/*****************************************************************************/

#include "wcx_mpq.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// Global variables

TConfiguration g_cfg;
TCHAR g_szIniFile[MAX_PATH] = _T("");

//-----------------------------------------------------------------------------
// Local variables

static LPCTSTR szCfgSection = _T("wcx_mpq");
static LPCTSTR szDefExcludedTypes = _T("smk;bik;w3m;w3n;w3x;mpq;mp3");

//-----------------------------------------------------------------------------
// Public functions

static BOOL WritePrivateProfileInt(LPCTSTR szSection, LPCTSTR szKeyName, UINT uValue, LPCTSTR szIniFile)
{
    TCHAR szIntValue[32];

    StringCchPrintf(szIntValue, _countof(szIntValue), _T("%u"), uValue);
    return WritePrivateProfileString(szSection, szKeyName, szIntValue, szIniFile);
}

int SetDefaultConfiguration()
{
    // By default, all is zero or false
    ZeroMemory(&g_cfg, sizeof(TConfiguration));

    g_cfg.bCreateMpqV2        = FALSE;
    g_cfg.bAddLocaleToName    = TRUE;
    g_cfg.bEncryptAddedFiles  = FALSE;
    g_cfg.bCompactAfterDelete = TRUE;

    g_cfg.dwMaxFileCount      = 0x1000;
    g_cfg.dwCompression       = FILE_COMPRESSION_ZLIB;

    GetModuleFileName(g_hInst, g_cfg.szListFile, _countof(g_cfg.szListFile));
    ReplaceFileName(g_cfg.szListFile, _T("ListFile.txt"));
    if(GetFileAttributes(g_cfg.szListFile) & FILE_ATTRIBUTE_DIRECTORY)
        g_cfg.szListFile[0] = 0;

    StringCchCopy(g_cfg.szExcludedTypes, _countof(g_cfg.szExcludedTypes), szDefExcludedTypes);
    return ERROR_SUCCESS;
}

int LoadConfiguration()
{
    TCHAR szDefListFile[MAX_PATH];

    // Try to find the "Listfile.txt" in the same directory where the plugin is
    GetModuleFileName(g_hInst, szDefListFile, _countof(szDefListFile));
    ReplaceFileName(szDefListFile, _T("ListFile.txt"));
    if(GetFileAttributes(szDefListFile) & FILE_ATTRIBUTE_DIRECTORY)
        szDefListFile[0] = 0;

    // Load values for the variables
    g_cfg.dwMaxFileCount      = GetPrivateProfileInt(szCfgSection, _T("HashTableSize"),     0x1000, g_szIniFile);
    g_cfg.dwCompression       = GetPrivateProfileInt(szCfgSection, _T("FileCompression"),   FILE_COMPRESSION_ZLIB, g_szIniFile);
    g_cfg.lcFileLocale        = GetPrivateProfileInt(szCfgSection, _T("FileLocale"),        0, g_szIniFile);
    g_cfg.bCreateMpqV2        = GetPrivateProfileInt(szCfgSection, _T("CreateMpqV2"),       0, g_szIniFile);
    g_cfg.bAddLocaleToName    = GetPrivateProfileInt(szCfgSection, _T("AddLocaleToName"),   1, g_szIniFile);
    g_cfg.bEncryptAddedFiles  = GetPrivateProfileInt(szCfgSection, _T("EncryptAddedFiles"), 0, g_szIniFile);
    g_cfg.bCompactAfterDelete = GetPrivateProfileInt(szCfgSection, _T("CompactAfterDel"),   1, g_szIniFile);

    GetPrivateProfileString(szCfgSection, _T("ListFile"), szDefListFile, g_cfg.szListFile, _countof(g_cfg.szListFile), g_szIniFile);

    GetPrivateProfileString(szCfgSection, _T("ExcludedTypes"), szDefExcludedTypes, g_cfg.szExcludedTypes, _countof(g_cfg.szExcludedTypes), g_szIniFile);
    return ERROR_SUCCESS;
}

int SaveConfiguration()
{
    WritePrivateProfileInt(szCfgSection, _T("HashTableSize"),     g_cfg.dwMaxFileCount,      g_szIniFile);
    WritePrivateProfileInt(szCfgSection, _T("FileCompression"),   g_cfg.dwCompression,       g_szIniFile);
    WritePrivateProfileInt(szCfgSection, _T("CreateMpqV2"),       g_cfg.bCreateMpqV2,        g_szIniFile);
    WritePrivateProfileInt(szCfgSection, _T("AddLocaleToName"),   g_cfg.bAddLocaleToName,    g_szIniFile);
    WritePrivateProfileInt(szCfgSection, _T("EncryptAddedFiles"), g_cfg.bEncryptAddedFiles,  g_szIniFile);
    WritePrivateProfileInt(szCfgSection, _T("CompactAfterDel"),   g_cfg.bCompactAfterDelete, g_szIniFile);

    WritePrivateProfileString(szCfgSection, _T("ListFile"), g_cfg.szListFile, g_szIniFile);
    WritePrivateProfileString(szCfgSection, _T("ExcludedTypes"), g_cfg.szExcludedTypes, g_szIniFile);
    return ERROR_SUCCESS;
}
