/*****************************************************************************/
/* DlgSettings.cpp                        Copyright (c) Ladislav Zezula 2007 */
/*---------------------------------------------------------------------------*/
/* Description:                                                              */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 19.06.05  1.00  Lad  The first version of DlgSettings.cpp                 */
/*****************************************************************************/

#include "wcx_mpq.h"                    // Our functions
#include "resource.h"                   // Resource symbols

#pragma warning(disable: 4996)          // 4996: 'GetVersionExW': was declared deprecated

//-----------------------------------------------------------------------------
// Local functions

// Fills a combobox with locales and selects the right item
static HWND hWndLocales = NULL;

static void ComboBox_InsertLocale(HWND hWndCombo, LCID lcLocaleId, UINT nIDString)
{
    TCHAR szLocaleText[128];
    TCHAR szBuffer[128];
    int nIndex;

    // Retrieve the locale short name
    if(nIDString != 0)
        LoadString(g_hInst, nIDString, szLocaleText, _countof(szLocaleText));
    else
        GetLocaleInfo(lcLocaleId, LOCALE_SLANGUAGE, szLocaleText, _countof(szLocaleText));

    // Insert the string to the combo box
    StringCchPrintf(szBuffer, _countof(szBuffer), _T("%s (%04X)"), szLocaleText, lcLocaleId);
    nIndex = ComboBox_AddString(hWndCombo, szBuffer);
    ComboBox_SetItemData(hWndCombo, nIndex, lcLocaleId);
}

static BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString)
{
    LCID lcLocaleId;

    if(hWndLocales != NULL)
    {
        // Retrieve the numeric value
        lcLocaleId = _tcstoul(lpLocaleString, &lpLocaleString, 16);

        // Insert the locale. Retrieve the string from the system
        ComboBox_InsertLocale(hWndLocales, lcLocaleId, 0);
    }
    return TRUE;
}

void FillLocalesAndSelect(HWND hWndCombo, LCID lcID)
{
    int nItems;

    // Delete all items in the combo box
    ComboBox_ResetContent(hWndCombo);

    // Insert the "Neutral" string to the begin of the list
    ComboBox_InsertLocale(hWndCombo, 0, IDS_NEUTRAL);

    // Fill the combo box with system locales
    hWndLocales = hWndCombo;
    EnumSystemLocales(EnumLocalesProc, LCID_SUPPORTED);
    hWndLocales = NULL;

    // Select the item that contains the locale
    if(lcID != (LCID)-1)
    {
        nItems = ComboBox_GetCount(hWndCombo);
        for(int i = 0; i < nItems; i++)
        {
            if(ComboBox_GetItemData(hWndCombo, i) == lcID)
            {
                ComboBox_SetCurSel(hWndCombo, i);
                return;
            }
        }
    }

    // If failed, select the first item
    ComboBox_SetCurSel(hWndCombo, 0);
}

//-----------------------------------------------------------------------------
// Dialog handlers

static int OnInitDialog(HWND hDlg)
{
    TCHAR szText[64];
    HWND hWndChild;
    int nCheck;

    // Initialize the combo box
    InitDialogControls(hDlg, MAKEINTRESOURCE(IDD_WCX_MPQ_SETTINGS));

    // MPQ version 2.0
    nCheck = g_cfg.bCreateMpqV2 ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(hDlg, IDC_CREATE_MPQ_V2, nCheck);

    // Show locale ID after file name
    nCheck = g_cfg.bAddLocaleToName ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(hDlg, IDC_LOCALES_WITH_NAME, nCheck);

    // Encrypt newly added files
    nCheck = g_cfg.bEncryptAddedFiles ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(hDlg, IDC_ENCRYPT_ADDED_FILES, nCheck);

    // Compact archive after delete
    nCheck = g_cfg.bCompactAfterDelete ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(hDlg, IDC_COMPACT_AFTER_DELETE, nCheck);

    // Hash table size
    StringCchPrintf(szText, _countof(szText), _T("0x%lX"), g_cfg.dwMaxFileCount);
    SetDlgItemText(hDlg, IDC_MAX_FILE_COUNT, szText);

    // File compression
    if((hWndChild = GetDlgItem(hDlg, IDC_COMPRESSION)) != NULL)
        ComboBox_SetCurSel(hWndChild, g_cfg.dwCompression);

    if((hWndChild = GetDlgItem(hDlg, IDC_FILE_LOCALE)) != NULL)
        FillLocalesAndSelect(hWndChild, g_cfg.lcFileLocale);

    // Listfile
    SetDlgItemText(hDlg, IDC_LISTFILE, g_cfg.szListFile);

    // Excluded file types
    SetDlgItemText(hDlg, IDC_EXCLUDED_TYPES, g_cfg.szExcludedTypes);
    return TRUE;
}

static int OnBrowseListFile(HWND hDlg)
{
    OSVERSIONINFO osvi = {sizeof(OSVERSIONINFO)};
    OPENFILENAME * pOfn;
    TCHAR szOpenFileName[sizeof(OPENFILENAME) + 0x80] = _T("");
    TCHAR szListFile[MAX_PATH] = _T("");
    TCHAR szInitialDir[MAX_PATH] = _T("");

    // Read the listfile
    GetDlgItemText(hDlg, IDC_LISTFILE, szListFile, _countof(szListFile));
    StringCchCopy(szInitialDir, _countof(szInitialDir), szListFile);
    *GetPlainName(szInitialDir) = 0;

    // Get version of the operating system. If greater or equal,
    // we will include the support for OPENFILENAME extension under Win2000+
    GetVersionEx(&osvi);

    // Call the browse dialog
    pOfn = (OPENFILENAME *)szOpenFileName;
    pOfn->lStructSize = sizeof(OPENFILENAME);
    if(osvi.dwMajorVersion >= 5)
        pOfn->lStructSize += sizeof(void *) + sizeof(DWORD) + sizeof(DWORD);
    pOfn->hwndOwner   = hDlg;
    pOfn->hInstance   = g_hInst;
    pOfn->lpstrFilter = _T("Listfiles (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0\0");
    pOfn->lpstrFile   = szListFile;
    pOfn->nMaxFile    = _countof(szListFile) - 1;
    pOfn->lpstrInitialDir = szInitialDir;
    pOfn->lpstrTitle  = _T("Choose the external listfile for opening MPQ archives");
    pOfn->Flags       = (OFN_ENABLESIZING | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_SHAREAWARE);
    if(GetOpenFileName(pOfn))
        SetDlgItemText(hDlg, IDC_LISTFILE, szListFile);
    return TRUE;
}

static int OnSaveDialog(HWND hDlg)
{
    LPTSTR szTemp;
    HANDLE hFile;
    TCHAR szText[MAX_PATH];
    HWND hWndChild;
    int nSelected;

    // MPQ version 2.0
    g_cfg.bCreateMpqV2 = (IsDlgButtonChecked(hDlg, IDC_CREATE_MPQ_V2) == BST_CHECKED);

    // Add locale to the file name
    g_cfg.bAddLocaleToName = (IsDlgButtonChecked(hDlg, IDC_LOCALES_WITH_NAME) == BST_CHECKED);
    
    // Encrypt newly added files
    g_cfg.bEncryptAddedFiles = (IsDlgButtonChecked(hDlg, IDC_ENCRYPT_ADDED_FILES) == BST_CHECKED);
    
    // Compact after delete
    g_cfg.bCompactAfterDelete = (IsDlgButtonChecked(hDlg, IDC_COMPACT_AFTER_DELETE) == BST_CHECKED);

    // Hash table size
    GetDlgItemText(hDlg, IDC_MAX_FILE_COUNT, szText, _countof(szText));
    g_cfg.dwMaxFileCount = _tcstoul(szText, &szTemp, 16);

    // File compression
    if((hWndChild = GetDlgItem(hDlg, IDC_COMPRESSION)) != NULL)
        g_cfg.dwCompression = ComboBox_GetCurSel(hWndChild);

    // Save the locale ID
    if((hWndChild = GetDlgItem(hDlg, IDC_FILE_LOCALE)) != NULL)
    {
        nSelected = ComboBox_GetCurSel(hWndChild);
        g_cfg.lcFileLocale = (LCID)ComboBox_GetItemData(hWndChild, nSelected);
    }

    // Listfile
    GetDlgItemText(hDlg, IDC_LISTFILE, szText, _countof(szText));
    hFile = CreateFile(szText, FILE_READ_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        MessageBoxRc(hDlg, IDS_ERROR, IDS_LISTFILE_DOES_NOT_EXIST);
        return FALSE;
    }
    StringCchCopy(g_cfg.szListFile, _countof(g_cfg.szListFile), szText);
    CloseHandle(hFile);

    // Excluded file types
    GetDlgItemText(hDlg, IDC_EXCLUDED_TYPES, g_cfg.szExcludedTypes, _countof(g_cfg.szExcludedTypes));
    
    // Save the configuration to the file
    SaveConfiguration();
    return TRUE;
}

static int OnDeltaPos(HWND hDlg, NMUPDOWN * pNMUpDown)
{
    TCHAR * szTemp;
    DWORD dwMaxFileCount;
    TCHAR szText[64];

    GetDlgItemText(hDlg, IDC_MAX_FILE_COUNT, szText, _countof(szText));
    dwMaxFileCount = _tcstol(szText, &szTemp, 16);

    if(pNMUpDown->iDelta > 0)
        dwMaxFileCount >>= 1;
    if(pNMUpDown->iDelta < 0)
        dwMaxFileCount <<= 1;
    if(dwMaxFileCount > HASH_TABLE_SIZE_MAX)
        dwMaxFileCount = HASH_TABLE_SIZE_MAX;
    if(dwMaxFileCount < HASH_TABLE_SIZE_MIN)
        dwMaxFileCount = HASH_TABLE_SIZE_MIN;
    
    StringCchPrintf(szText, _countof(szText), _T("0x%lX"), dwMaxFileCount);
    SetDlgItemText(hDlg, IDC_MAX_FILE_COUNT, szText);
    return TRUE;  
}

static int OnCommand(HWND hDlg, int nNotify, UINT_PTR nIDCtrl)
{
    if(nNotify == BN_CLICKED)
    {
        switch(nIDCtrl)
        {
            case IDC_BROWSE:
                return OnBrowseListFile(hDlg);

            case IDOK:
                OnSaveDialog(hDlg);
                // No break here !!

            case IDCANCEL:
                EndDialog(hDlg, nIDCtrl);
                break;
        }
    }

    return FALSE;
}

static int OnNotify(HWND hDlg, NMHDR * pNMHdr)
{
    switch(pNMHdr->code)
    {
        case UDN_DELTAPOS:
            return OnDeltaPos(hDlg, (NMUPDOWN *)pNMHdr);
    }

    return FALSE;
}

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:         // Dialog initialization
            return OnInitDialog(hDlg);

        case WM_COMMAND:
            return OnCommand(hDlg, WMC_NOTIFY(wParam), WMC_CTRLID(wParam));

        case WM_NOTIFY:
            return OnNotify(hDlg, (NMHDR *)lParam);
    }

    return FALSE;
}

INT_PTR SettingsDialog(HWND hParent)
{
    return DialogBox(g_hInst, MAKEINTRESOURCE(IDD_WCX_MPQ_SETTINGS), hParent, DialogProc);
}
