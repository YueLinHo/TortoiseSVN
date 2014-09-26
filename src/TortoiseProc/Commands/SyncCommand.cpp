// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2014 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "SyncCommand.h"
#include "registry.h"
#include "TSVNPath.h"
#include "SimpleIni.h"
#include "SmartHandle.h"
#include "StringUtils.h"
#include "UnicodeUtils.h"


// registry entries
std::vector<CString> regUseArray = {
    L"TortoiseSVN\\*",
    L"TortoiseSVN\\RevisionGraph\\*",
    L"TortoiseSVN\\Colors\\*",
    L"TortoiseSVN\\History\\**",
    L"TortoiseSVN\\History\\repoURLS\\**",
    L"TortoiseSVN\\History\\repoPaths\\**",
    L"TortoiseSVN\\LogCache\\*",
    L"TortoiseSVN\\Servers\\global\\**",
    L"TortoiseSVN\\StatusColumns\\*",
    L"TortoiseMerge\\*",
};

std::vector<CString> regBlockArray = {
    L"*debug*",
    L"syncpath",
    L"syncpw",
    L"checknewerweek",
    L"currentversion",
    L"diff",
    L"hooks",
    L"merge",
    L"tblamepos",
    L"tblamesize",
    L"tblamestate",
    L"*windowpos",
    L"nocontextpaths",
    L"configdir",
    L"defaultcheckoutpath",
    L"lastcheckoutpath",
    L"erroroccurred",
    L"newversion",
    L"newversiontext",
    L"newversionlink",
    L"historyhintshown",
    L"mergewcurl",
    L"scintilladirect2d",
    L"udiffpagesetup*",
};


bool SyncCommand::Execute()
{
    CRegString rSyncPath(L"Software\\TortoiseSVN\\SyncPath");
    CTSVNPath syncPath = CTSVNPath(CString(rSyncPath));

    if (syncPath.IsEmpty())
    {
        // show the settings dialog so the user can enter a sync path
        //return false;
    }

    CSimpleIni iniFile;
    iniFile.SetMultiLine(true);

    CAutoRegKey hMainKey;
    RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\TortoiseSVN", 0, KEY_READ, hMainKey.GetPointer());
    FILETIME filetime = { 0 };
    RegQueryInfoKey(hMainKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &filetime);

    //{
    //    CRegString regPW(L"Software\\TortoiseSVN\\SyncPW");
    //    auto encrypted = CStringUtils::Encrypt(L"testpassword");
    //    regPW = encrypted;
    //}

    bool bCloudIsNewer = false;
    {
        // open the file in read mode
        CAutoFile hFile = CreateFile(syncPath.GetWinPathString() + L"\\tsvnsync.ini", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile.IsValid())
        {
            // compare the file times
            FILETIME ftCreate, ftAccess, ftWrite;
            if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
            {
                if (CompareFileTime(&filetime, &ftWrite) < 0)
                    bCloudIsNewer = true;
            }
            // load the file
            LARGE_INTEGER fsize = { 0 };
            if (GetFileSizeEx(hFile, &fsize))
            {
                auto filebuf = std::make_unique<char[]>(fsize.QuadPart);
                DWORD bytesread = 0;
                if (ReadFile(hFile, filebuf.get(), DWORD(fsize.QuadPart), &bytesread, NULL))
                {
                    // decrypt the file contents
                    std::string encrypted = std::string(filebuf.get(), bytesread);
                    CRegString regPW(L"Software\\TortoiseSVN\\SyncPW");
                    auto password = CStringUtils::Decrypt(CString(regPW));
                    if (password.get())
                    {
                        std::string passworda = CUnicodeUtils::StdGetUTF8(password.get());
                        std::string decrypted = CStringUtils::Decrypt(encrypted, passworda);
                        // pass the decrypted data to the ini file
                        iniFile.LoadFile(decrypted.c_str(), decrypted.size());
                    }
                }
            }
        }
    }

    bool bHaveChanges = false;
    CString sValue;

    // go through all registry values and update either the registry
    // or the ini file, depending on which is newer
    for (const auto& regname : regUseArray)
    {
        CAutoRegKey hKey;
        CAutoRegKey hKeyKey;
        DWORD regtype = 0;
        DWORD regsize = 0;
        CString sKeyPath = L"Software";
        CString sValuePath = regname;
        CString sIniKeyName = regname;
        CString sRegname = regname;
        if (regname.Find('\\') >= 0)
        {
            // handle values in sub-keys
            sKeyPath = L"Software\\" + regname.Left(regname.ReverseFind('\\'));
            sValuePath = regname.Mid(regname.ReverseFind('\\') + 1);
        }
        if (RegOpenKeyEx(HKEY_CURRENT_USER, sKeyPath, 0, KEY_READ, hKey.GetPointer()) == ERROR_SUCCESS)
        {
            bool bEnum = false;
            bool bEnumKeys = false;
            int index = 0;
            int keyindex = 0;
            // an asterisk means: use all values inside the specified key
            if (sValuePath == L"*")
                bEnum = true;
            if (sValuePath == L"**")
            {
                bEnumKeys = true;
                bEnum = true;
                RegOpenKeyEx(HKEY_CURRENT_USER, sKeyPath, 0, KEY_READ, hKeyKey.GetPointer());
            }
            do
            {
                if (bEnumKeys)
                {
                    bEnum = true;
                    index = 0;
                    wchar_t cKeyName[MAX_PATH] = { 0 };
                    DWORD cLen = _countof(cKeyName);
                    if (RegEnumKeyEx(hKeyKey, keyindex, cKeyName, &cLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                    {
                        bEnumKeys = false;
                        break;
                    }
                    ++keyindex;
                    sKeyPath = L"Software\\" + regname.Left(regname.ReverseFind('\\')) + L"\\" + cKeyName + L"\\";
                    sRegname = regname.Left(regname.ReverseFind('\\')) + L"\\" + cKeyName + L"\\";
                    hKey.CloseHandle();
                    if (RegOpenKeyEx(HKEY_CURRENT_USER, sKeyPath, 0, KEY_READ, hKey.GetPointer()) != ERROR_SUCCESS)
                    {
                        bEnumKeys = false;
                        break;
                    }
                }
                do
                {
                    if (bEnum)
                    {
                        // start enumerating all values
                        wchar_t cValueName[MAX_PATH] = { 0 };
                        DWORD cLen = _countof(cValueName);
                        if (RegEnumValue(hKey, index, cValueName, &cLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                        {
                            bEnum = false;
                            break;
                        }
                        ++index;
                        sValuePath = cValueName;
                        CString sValueLower = sValuePath;
                        sValueLower.MakeLower();
                        bool bIgnore = false;
                        for (const auto& ignore : regBlockArray)
                        {
                            if (wcswildcmp(ignore, sValueLower))
                            {
                                bIgnore = true;
                                break;
                            }
                        }
                        if (bIgnore)
                            continue;
                        sIniKeyName = sRegname.Left(sRegname.ReverseFind('\\'));
                        if (sIniKeyName.IsEmpty())
                            sIniKeyName = sValuePath;
                        else
                            sIniKeyName += L"\\" + sValuePath;
                    }
                    if (RegQueryValueEx(hKey, sValuePath, NULL, &regtype, NULL, &regsize) == ERROR_SUCCESS)
                    {
                        if (regtype != 0)
                        {
                            auto regbuf = std::make_unique<BYTE[]>(regsize);
                            if (RegQueryValueEx(hKey, sValuePath, NULL, &regtype, regbuf.get(), &regsize) == ERROR_SUCCESS)
                            {
                                switch (regtype)
                                {
                                    case REG_DWORD:
                                    {
                                        DWORD value = *(DWORD*)regbuf.get();
                                        sValue = iniFile.GetValue(L"registry_dword", sIniKeyName);
                                        DWORD nValue = DWORD(_wtol(sValue));
                                        if (nValue != value)
                                        {
                                            if (bCloudIsNewer)
                                            {
                                                RegSetValueEx(hKey, sValuePath, NULL, regtype, (BYTE *)&nValue, sizeof(nValue));
                                            }
                                            else
                                            {
                                                bHaveChanges = true;
                                                sValue.Format(L"%ld", value);
                                                iniFile.SetValue(L"registry_dword", sIniKeyName, sValue);
                                            }
                                        }
                                        if (bCloudIsNewer)
                                            iniFile.Delete(L"registry_dword", sIniKeyName);
                                    }
                                        break;
                                    case REG_QWORD:
                                    {
                                        QWORD value = *(QWORD*)regbuf.get();
                                        sValue = iniFile.GetValue(L"registry_qword", sIniKeyName);
                                        QWORD nValue = QWORD(_wtoi64(sValue));
                                        if (nValue != value)
                                        {
                                            if (bCloudIsNewer)
                                            {
                                                RegSetValueEx(hKey, sValuePath, NULL, regtype, (BYTE *)&nValue, sizeof(nValue));
                                            }
                                            else
                                            {
                                                bHaveChanges = true;
                                                sValue.Format(L"%I64d", value);
                                                iniFile.SetValue(L"registry_qword", sIniKeyName, sValue);
                                            }
                                        }
                                        if (bCloudIsNewer)
                                            iniFile.Delete(L"registry_qword", sIniKeyName);
                                    }
                                        break;
                                    case REG_EXPAND_SZ:
                                    case REG_MULTI_SZ:
                                    case REG_SZ:
                                    {
                                        sValue = (LPCWSTR)regbuf.get();
                                        CString iniValue = iniFile.GetValue(L"registry_string", sIniKeyName);
                                        if (iniValue != sValue)
                                        {
                                            if (bCloudIsNewer)
                                            {
                                                RegSetValueEx(hKey, sValuePath, NULL, regtype, (BYTE *)(LPCWSTR)iniValue, (iniValue.GetLength() + 1)*sizeof(WCHAR));
                                            }
                                            else
                                            {
                                                bHaveChanges = true;
                                                iniFile.SetValue(L"registry_string", sIniKeyName, sValue);
                                            }
                                        }
                                        if (bCloudIsNewer)
                                            iniFile.Delete(L"registry_string", sIniKeyName);
                                    }
                                        break;
                                }
                            }
                        }
                    }
                } while (bEnum);
            } while (bEnumKeys);
        }
    }

    if (bCloudIsNewer)
    {
        CString regpath = L"Software\\";

        CSimpleIni::TNamesDepend keys;
        iniFile.GetAllKeys(L"registry_dword", keys);
        for (const auto& k : keys)
        {
            CRegDWORD reg(regpath + k);
            reg = _wtol(iniFile.GetValue(L"registry_dword", k, L""));
        }

        keys.clear();
        iniFile.GetAllKeys(L"registry_qword", keys);
        for (const auto& k : keys)
        {
            CRegQWORD reg(regpath + k);
            reg = _wtoi64(iniFile.GetValue(L"registry_qword", k, L""));
        }

        keys.clear();
        iniFile.GetAllKeys(L"registry_string", keys);
        for (const auto& k : keys)
        {
            CRegString reg(regpath + k);
            reg = CString(iniFile.GetValue(L"registry_string", k, L""));
        }
    }

    if (bHaveChanges)
    {
        // save the ini file
        std::string iniData;
        iniFile.SaveString(iniData);
        // encrypt the string

        CRegString regPW(L"Software\\TortoiseSVN\\SyncPW");
        auto password = CStringUtils::Decrypt(CString(regPW));
        std::string passworda = CUnicodeUtils::StdGetUTF8(password.get());

        std::string encrypted = CStringUtils::Encrypt(iniData, passworda);

        CAutoFile hFile = CreateFile(syncPath.GetWinPathString() + L"\\tsvnsync.ini", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile.IsValid())
        {
            DWORD written = 0;
            WriteFile(hFile, encrypted.c_str(), DWORD(encrypted.size()), &written, NULL);
        }

        return true;
    }
    return false;
}
