; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "TortoiseSVN"
!define PRODUCT_VERSION "1.0"
!define PRODUCT_PUBLISHER "Stefan K�ng, Tim Kemp, L�bbe Onken"
!define PRODUCT_WEB_SITE "http://tortoisesvn.tigris.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\TortoiseMerge.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define SYSTEMPATH "C:\Windows\System32"
SetCompressor lzma

; MUI 1.67 compatible ------
!include "MUI.nsh"
; Upgrading DLL's in use
!include "UpgradeDLL.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "..\gpl.txt"
; Components page
!insertmacro MUI_PAGE_COMPONENTS
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\Changelog.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "TortoiseSVN-${PRODUCT_VERSION}-UNICODE.exe"
InstallDir "$PROGRAMFILES\TortoiseSVN"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "core" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "..\..\bin\Release\TortoiseMerge.exe"
  CreateDirectory "$SMPROGRAMS\Tortoise"
  CreateShortCut "$SMPROGRAMS\Tortoise\TortoiseSVN.lnk" "$INSTDIR\TortoiseMerge.exe"
  CreateShortCut "$DESKTOP\TortoiseSVN.lnk" "$INSTDIR\TortoiseMerge.exe"
  File "..\..\bin\Release\TortoisePlink.exe"
  File "..\..\bin\Release\TortoiseProc.exe"
  
  ; The shell dll will be in use, so do an upgrade with reboot
  !insertmacro UpgradeDLL "..\..\bin\Release\TortoiseSVN.dll" "$INSTDIR\TortoiseSVN.dll" "$SYSDIR"
  File "..\..\bin\Release\CrashRpt.dll"
  File "dbghelp.dll"
  File "..\..\bin\Release\libdb42.dll"
  File "..\..\bin\Release\libeay32.dll"
  File "..\..\bin\Release\ssleay32.dll"
  File "${SYSTEMPATH}\MFC71.dll"
  File "${SYSTEMPATH}\MFC71u.dll"
  File "${SYSTEMPATH}\msvcp71.dll"
  File "${SYSTEMPATH}\msvcr71.dll"
  SetOutPath "$INSTDIR\iconv"
  File "..\..\bin\Release\iconv\*.so"
  SetOutPath "$INSTDIR\icons"
  File /oname=TortoiseAdded.ico "..\Icons\AddedIcon.ico"
  File /oname=TortoiseConflict.ico "..\Icons\ConflictIcon.ico"
  File /oname=TortoiseDeleted.ico "..\Icons\DeletedIcon.ico"
  File /oname=TortoiseInSubversion.ico "..\Icons\InSubVersionIcon.ico"
  File /oname=TortoiseModified.ico "..\Icons\ModifiedIcon.ico"
  SetOutPath "$INSTDIR"
  SetOverwrite on
  File "..\Changelog.txt"
  File /oname=TortoiseSVN_license.txt "..\gpl.txt"
  File /oname=Subversion_license.txt "Subversion license.txt"
  WriteRegStr HKCR "*\shellex\ContextMenuHandlers\TortoiseSVN" "" "{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKCR "*\shellex\PropertySheetHandlers\TortoiseSVN" "" "{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"

  WriteRegStr HKCR "CLSID\{C0351346-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKCR "CLSID\{C0351346-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKCR "CLSID\{C0351346-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKCR "CLSID\{C0351347-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKCR "CLSID\{C0351347-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKCR "CLSID\{C0351347-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKCR "CLSID\{C0351348-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKCR "CLSID\{C0351348-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKCR "CLSID\{C0351348-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKCR "CLSID\{C0351349-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKCR "CLSID\{C0351349-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKCR "CLSID\{C0351349-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKCR "CLSID\{C035134A-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKCR "CLSID\{C035134A-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKCR "CLSID\{C035134A-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKCR "CLSID\{C035134B-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKCR "CLSID\{C035134B-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKCR "CLSID\{C035134B-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKCR "CLSID\{C035134C-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKCR "CLSID\{C035134C-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKCR "CLSID\{C035134C-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKCR "Directory\Background\shellex\ContextMenuHandlers\TortoiseSVN" "" "{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKCR "Directory\shellex\ContextMenuHandlers\TortoiseSVN" "" "{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKCR "Directory\shellex\DragDropHandlers\TortoiseSVN" "" "{C035134A-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKCR "Directory\shellex\PropertySheetHandlers\TortoiseSVN" "" "{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  
  WriteRegStr HKCR "Drive\shellex\ContextMenuHandlers\TortoiseSVN" "" "{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKCR "Folder\shellex\ColumnHandlers\{C0351349-7B7D-4fcc-81B4-1E394CA267EB}" "" ""
  WriteRegStr HKCR "Folder\shellex\ContextMenuHandlers\TortoiseSVN" "" "{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKCR "Folder\shellex\DragDropHandlers\TortoiseSVN" "" "{C035134A-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKCR "InternetShortcut\shellex\ContextMenuHandlers\TortoiseSVN" "" "{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKCR "lnkfile\shellex\ContextMenuHandlers\TortoiseSVN" "" "{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  
  WriteRegDWORD HKCU "Software\${PRODUCT_NAME}" "DriveMaskFixed" 2
  WriteRegStr HKCU "Software\${PRODUCT_NAME}" "SSH" "$INSTDIR\TortoisePlink.exe"
  WriteRegStr HKCU "Software\${PRODUCT_NAME}" "TempFileExtensions" "thumbs.db;*.exe;*.dll;*.obj;*.ncb;*.lib;*.idb;*.pdb;*.ilk;*.msi;*.res;*.pch;*.suo;*.aps;*.*~;*.~*;~*.*;cvs\;release\;debug\"
  
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "AddedIcon" "$INSTDIR\icons\TortoiseAdded.ico"
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "ConflictIcon" "$INSTDIR\icons\TortoiseConflict.ico"
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "DeletedIcon" "$INSTDIR\icons\TortoiseDeleted.ico"
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "InSubversionIcon" "$INSTDIR\icons\TortoiseInSubversion.ico"
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "ModifiedIcon" "$INSTDIR\icons\TortoiseModified.ico"
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "Directory" "$INSTDIR"
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "ProcPath" "$INSTDIR\TortoiseProc.exe"
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "TMergePath" "$INSTDIR\TortoiseMerge.exe"
  
  WriteRegStr HKLM "Software\Classes\CLSID\{C0351346-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKLM "Software\Classes\CLSID\{C0351346-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKLM "Software\Classes\CLSID\{C0351346-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKLM "Software\Classes\CLSID\{C0351347-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKLM "Software\Classes\CLSID\{C0351347-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKLM "Software\Classes\CLSID\{C0351347-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKLM "Software\Classes\CLSID\{C0351348-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKLM "Software\Classes\CLSID\{C0351348-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKLM "Software\Classes\CLSID\{C0351348-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKLM "Software\Classes\CLSID\{C0351349-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKLM "Software\Classes\CLSID\{C0351349-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKLM "Software\Classes\CLSID\{C0351349-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKLM "Software\Classes\CLSID\{C035134A-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKLM "Software\Classes\CLSID\{C035134A-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKLM "Software\Classes\CLSID\{C035134A-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKLM "Software\Classes\CLSID\{C035134B-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKLM "Software\Classes\CLSID\{C035134B-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKLM "Software\Classes\CLSID\{C035134B-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"

  WriteRegStr HKLM "Software\Classes\CLSID\{C035134C-7B7D-4fcc-81B4-1E394CA267EB}" "" "TortoiseSVN"
  WriteRegStr HKLM "Software\Classes\CLSID\{C035134C-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "" "$INSTDIR\TortoiseSVN.dll"
  WriteRegStr HKLM "Software\Classes\CLSID\{C035134C-7B7D-4fcc-81B4-1E394CA267EB}\InProcServer32" "ThreadingModel" "Apartment"
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\TortoiseSVN1" "" "{C0351346-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\TortoiseSVN2" "" "{C0351347-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\TortoiseSVN3" "" "{C0351348-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\TortoiseSVN4" "" "{C035134B-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\TortoiseSVN5" "" "{C035134C-7B7D-4fcc-81B4-1E394CA267EB}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{C0351346-7B7D-4fcc-81B4-1E394CA267EB}" "TortoiseSVN"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{C0351347-7B7D-4fcc-81B4-1E394CA267EB}" "TortoiseSVN"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{C0351348-7B7D-4fcc-81B4-1E394CA267EB}" "TortoiseSVN"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{C0351349-7B7D-4fcc-81B4-1E394CA267EB}" "TortoiseSVN"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{C035134A-7B7D-4fcc-81B4-1E394CA267EB}" "TortoiseSVN"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{C035134B-7B7D-4fcc-81B4-1E394CA267EB}" "TortoiseSVN"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{C035134C-7B7D-4fcc-81B4-1E394CA267EB}" "TortoiseSVN"
SectionEnd

Section "help" SEC02
  File "..\..\doc\output\*.chm"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\Tortoise\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\Tortoise\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\TortoiseMerge.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\TortoiseMerge.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} ""
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} ""
!insertmacro MUI_FUNCTION_DESCRIPTION_END


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\*.chm"
  Delete "$INSTDIR\Subversion license.txt"
  Delete "$INSTDIR\gpl.txt"
  Delete "$INSTDIR\Changelog.txt"
  Delete "$INSTDIR\iconv\*.so"
  Delete "$INSTDIR\icons\*.*"
  Delete "$INSTDIR\Languages\*.*"
  Delete "$INSTDIR\ssleay32.dll"
  Delete "$INSTDIR\libeay32.dll"
  Delete "$INSTDIR\libdb42.dll"
  Delete "$INSTDIR\dbghelp.dll"
  Delete "$INSTDIR\CrashRpt.dll"
  Delete "$INSTDIR\TortoiseSVN.dll"
  Delete "$INSTDIR\TortoiseProc.exe"
  Delete "$INSTDIR\TortoisePlink.exe"
  Delete "$INSTDIR\TortoiseMerge.exe"

  Delete "$SMPROGRAMS\Tortoise\Uninstall.lnk"
  Delete "$SMPROGRAMS\Tortoise\Website.lnk"
  Delete "$DESKTOP\TortoiseSVN.lnk"
  Delete "$SMPROGRAMS\Tortoise\TortoiseSVN.lnk"

  RMDir "$SMPROGRAMS\Tortoise"
  RMDir "$INSTDIR\iconv"
  RMDir "$INSTDIR\icons"
  RMDir "$INSTDIR\Languages"
  RMDir "$INSTDIR"


  DeleteRegKey HKCR "*\shellex\ContextMenuHandlers\TortoiseSVN" 
  DeleteRegKey HKCR "*\shellex\PropertySheetHandlers\TortoiseSVN"
  DeleteRegKey HKCR "CLSID\{C0351346-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKCR "CLSID\{C0351347-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKCR "CLSID\{C0351348-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKCR "CLSID\{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKCR "CLSID\{C035134A-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKCR "CLSID\{C035134B-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKCR "CLSID\{C035134C-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKCR "Directory\Background\shellex\ContextMenuHandlers\TortoiseSVN"
  DeleteRegKey HKCR "Directory\shellex\ContextMenuHandlers\TortoiseSVN"
  DeleteRegKey HKCR "Directory\shellex\DragDropHandlers\TortoiseSVN"
  DeleteRegKey HKCR "Directory\shellex\PropertySheetHandlers\TortoiseSVN"
  DeleteRegKey HKCR "Drive\shellex\ContextMenuHandlers\TortoiseSVN"
  DeleteRegKey HKCR "Folder\shellex\ColumnHandlers\{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKCR "Folder\shellex\ContextMenuHandlers\TortoiseSVN"
  DeleteRegKey HKCR "Folder\shellex\DragDropHandlers\TortoiseSVN"
  DeleteRegKey HKCR "InternetShortcut\shellex\ContextMenuHandlers\TortoiseSVN"
  DeleteRegKey HKCR "lnkfile\shellex\ContextMenuHandlers\TortoiseSVN"

  DeleteRegKey HKCU "Software\${PRODUCT_NAME}"

  DeleteRegKey HKLM "Software\${PRODUCT_NAME}"
  DeleteRegKey HKLM "Software\Classes\CLSID\{C0351346-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKLM "Software\Classes\CLSID\{C0351347-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKLM "Software\Classes\CLSID\{C0351348-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKLM "Software\Classes\CLSID\{C0351349-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKLM "Software\Classes\CLSID\{C035134A-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKLM "Software\Classes\CLSID\{C035134B-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKLM "Software\Classes\CLSID\{C035134C-7B7D-4fcc-81B4-1E394CA267EB}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\TortoiseSVN1"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\TortoiseSVN2"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\TortoiseSVN3"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\TortoiseSVN4"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\TortoiseSVN5"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
