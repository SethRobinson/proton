;NSIS Modern User Interface version 1.70
;Header Bitmap Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General
  SetCompressor lzma
  !define _TITLE_ "Dink Smallwood HD"
  !define _VERSION_ "$%C_TEXT_VERSION%"
  !define _COMPILE_DATE_ "${__DATE__}"

  ;Name and file
  Name "${_TITLE_}"
 ; Icon
!define MUI_ICON "dink.ico" 
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\classic-uninstall.ico"

RequestExecutionLevel user

OutFile "..\$%C_FILENAME%"

!define MUI_WELCOMEFINISHPAGE_BITMAP "welcome_side.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "welcome_side.bmp"

  ;Default installation folder
  InstallDir "$LOCALAPPDATA\DinkSmallwoodHD"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\RTSOFT\DINK" "path"

BrandingText " "
;InitPluginsDir
;  File /oname=$PLUGINSDIR\splash.bmp "path\to\your\bitmap.bmp"
;  advsplash::show 1000 600 400 -1 $PLUGINSDIR\splash
;  Pop $0

;--------------------------------
;Interface Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "header.bmp"
  !define MUI_HEADERIMAGE_UNBITMAP "header.bmp"
  !define MUI_ABORTWARNING
 
;--------------------------------
;Pages

  
  !define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${_TITLE_}. (${_VERSION_} released on ${_COMPILE_DATE_})\r\n\r\nClick Next to continue."
  !define MUI_WELCOMEPAGE_TITLE "${_TITLE_} ${_VERSION_} Installer"
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "readme.txt"
  
  
  ;Customize component texts
  
  !define MUI_COMPONENTSPAGE_TEXT_COMPLIST "Select extra components to install."
  !define MUI_COMPONENTSPAGE_TEXT_TOP "Here, you can choose whether or not you want a desktop icon in addition to the normal start menu options."
  !insertmacro MUI_PAGE_COMPONENTS
  
  
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
 
!define MUI_FINISHPAGE_LINK "Click here to visit the RTsoft website"
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.rtsoft.com/"
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
 
 !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
 
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

;Remove the - part in from of Main Game if you don't want it to be hidden...
Section "-Main Game" SecMain
SectionIn RO
  SetOutPath "$INSTDIR"
  File "..\..\bin\dink.exe"
  ;dink.pdf is optional, it's like 11 MB but it allows auto logged crash stacks to contain useful info
  File "..\..\bin\dink.pdb"
  File "..\..\bin\fmod.dll"
  File "..\..\bin\zlib1.dll"
  SetOutPath "$INSTDIR\audio\"
  File /r "..\..\bin\audio\"
 SetOutPath "$INSTDIR"
   SetOutPath "$INSTDIR\dink\"

 File /r /x TimGM6mbTiny.dls "..\..\bin\dink\"
SetOutPath "$INSTDIR\interface\"

File /r "..\..\bin\interface\ipad"
File /r "..\..\bin\interface\large"
File /r "..\..\bin\interface\particle"
File /r "..\..\bin\interface\win"
File "..\..\bin\interface\*.*"
 
  ;to create the dir
SetOutPath "$INSTDIR\dmods\"
File "..\..\bin\dmods\info.txt"
SetOutPath "$INSTDIR"

;File /r "..\bin\base"
  

;AccessControl::GrantOnFile "$INSTDIR" "(S-1-5-32-545)" "GenericRead + GenericWrite + DeleteChild"

CreateDirectory "$SMPROGRAMS\${_TITLE_}"
;ok, this will create a Folder in your Start menue

  ;Store installation folder
  WriteRegStr HKCU "Software\RTSOFT\DINK" "path" $INSTDIR

  ;write uninstall strings
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_TITLE_}" "DisplayName" "${_TITLE_} (remove only)"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_TITLE_}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateShortCut "$SMPROGRAMS\${_TITLE_}\${_TITLE_}.lnk" "$INSTDIR\dink.exe" "" "$INSTDIR\dink.exe"
  WriteIniStr "$SMPROGRAMS\${_TITLE_}\Report a bug or make a comment.url" "InternetShortcut" "URL" "http://www.rtsoft.com/pages/feedback_app.htm?game=${_TITLE_}&version=${_VERSION_}"
  WriteIniStr "$SMPROGRAMS\${_TITLE_}\Robinson Technologies Website.url" "InternetShortcut" "URL" "http://www.rtsoft.com"
  WriteIniStr "$SMPROGRAMS\${_TITLE_}\Visit the Dink Network.url" "InternetShortcut" "URL" "http://www.dinknetwork.com"
  ;CreateShortCut "$SMPROGRAMS\${_TITLE_}\Quick Help.lnk" "$INSTDIR\help\documentation.htm" ; use defaults for parameters, icon, etc.
  CreateShortCut "$SMPROGRAMS\${_TITLE_}\Uninstall ${_TITLE_}.lnk" "$INSTDIR\Uninstall.exe" ; use defaults for parameters, icon, etc.

; file associations

/*
; back up old value of .opt
!define Index "Line${__LINE__}"
  ReadRegStr $1 HKCR ".dmod" ""
  StrCmp $1 "" "${Index}-NoBackup"
    StrCmp $1 "DinkAddon" "${Index}-NoBackup"
    WriteRegStr HKCR ".dmod" "backup_val" $1
"${Index}-NoBackup:"
  WriteRegStr HKCR ".dmod" "" "DinkAddon"
  ReadRegStr $0 HKCR "DinkAddon" ""
  StrCmp $0 "" 0 "${Index}-Skip"
	WriteRegStr HKCR "DinkAddon" "" "DinkAddon"
	WriteRegStr HKCR "DinkAddon\shell" "" "open"
	WriteRegStr HKCR "DinkAddon\DefaultIcon" "" "$INSTDIR\dink.exe,0"
"${Index}-Skip:"
  WriteRegStr HKCR "DinkAddon\shell\open\command" "" \
    '"$INSTDIR\dink.exe" "%1"'

  System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
!undef Index
*/

SectionEnd


;Installer Sections

Section "Desktop Icon" SecDesktopIcon
  SetOutPath "$INSTDIR"
  CreateShortCut "$DESKTOP\${_TITLE_}.lnk" "$INSTDIR\dink.exe" 

SectionEnd

Function LaunchLink
  ExecShell "" "$INSTDIR\dink.exe"
FunctionEnd



;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecMain ${LANG_ENGLISH} "The main game files, these are required to play the game."
  LangString DESC_SecDesktopIcon ${LANG_ENGLISH} "This option will throw one of those handy desktop icons on the main menu for easy access to the program."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktopIcon} $(DESC_SecDesktopIcon)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

;default delete

   Delete "$INSTDIR\dink.exe"
   ;dink.pdf is optional, it's like 11 MB but it allows auto logged crash stacks to contain useful info
   Delete "$INSTDIR\dink.pdb"
   Delete "$INSTDIR\Uninstall.exe"
   Delete "$INSTDIR\fmodex.dll"
   Delete "$INSTDIR\zlib1.dll"
   RMDir /r "$INSTDIR\audio"
   RMDir /r "$INSTDIR\interface"
   RMDir /r "$INSTDIR\dink\dink"
   RMDir /r "$INSTDIR\dink\graphics"
   RMDir /r "$INSTDIR\dink\midi"
   RMDir /r "$INSTDIR\dink\sound"
   RMDir /r "$INSTDIR\dink\story"
   RMDir /r "$INSTDIR\dink\tiles"
   RMDir /r "$INSTDIR\dink\tiles"
   Delete "$INSTDIR\dink\dmod.diz"
   Delete "$INSTDIR\dink\dink.ini"
   Delete "$INSTDIR\dink\hard.dat"
   Delete "$INSTDIR\dink\map.dat"
   Delete "$INSTDIR\dink\dink.dat"
   RMDir "$INSTDIR\dink"
  Delete  "$INSTDIR\dmods\info.txt"
 RMDir "$INSTDIR\dmods"

;  RMDir /r "$INSTDIR\base"
  
  DeleteRegKey HKCU "Software\RTSOFT\DINK\path"

  DeleteRegKey /ifempty HKCU "Software\RTSOFT\DINK"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_TITLE_}"
  Delete "$SMPROGRAMS\${_TITLE_}\*.*"
  RMDir "$SMPROGRAMS\${_TITLE_}"
  Delete "$DESKTOP\${_TITLE_}.lnk"

;delete user stuff too?

MessageBox MB_YESNO "Would you like to also delete all saved games and installed DMOD addons?" IDNO skip_it 
;RMDir /r "$INSTDIR\worlds"
RMDir /r "$INSTDIR"
skip_it:

  ;start of restore script
  
  /*
!define Index "Line${__LINE__}"
  ReadRegStr $1 HKCR ".dmod" ""
  StrCmp $1 "DinkAddon" 0 "${Index}-NoOwn" ; only do this if we own it
    ReadRegStr $1 HKCR ".dmod" "backup_val"
    StrCmp $1 "" 0 "${Index}-Restore" ; if backup="" then delete the whole key
      DeleteRegKey HKCR ".dmod"
    Goto "${Index}-NoOwn"
"${Index}-Restore:"
      WriteRegStr HKCR ".dmod" "" $1
      DeleteRegValue HKCR ".dmod" "backup_val"
   
    DeleteRegKey HKCR "DinkAddon" ;Delete key with association settings
 
    System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
"${Index}-NoOwn:"
!undef Index
 */
 
 
  ;rest of script 
  
  
  

SectionEnd