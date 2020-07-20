Name "Seamly2D installer"

OutFile "seamly2d-installer.exe"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

Unicode True

InstallDir $PROGRAMFILES\Seamly2D

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\NSIS_Seamly2D" "Install_Dir"

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Seamly2D"

  SectionIn RO
  
  SetOutPath $INSTDIR

  File "VC_redist.x86.exe"
  File "VC_redist.x64.exe"

  !include "x64.nsh"
  ${If} ${RunningX64}
    ExecWait '"$INSTDIR\VC_redist.x64.exe"  /norestart'
  ${Else}
    ExecWait '"$INSTDIR\VC_redist.x86.exe"  /norestart'
  ${EndIf}

  ; relative to the location of this .nsi file, copy all the files/directories recursively
  File /r *
  
  WriteRegStr HKLM SOFTWARE\NSIS_Seamly2D "Install_Dir" "$INSTDIR"
  
  StrCpy $R1 " - Seamly2D"
  Call check_for_window
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Seamly2D" "DisplayName" "Seamly2D"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Seamly2D" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Seamly2D" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Seamly2D" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
SectionEnd

Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Seamly2D"
  CreateShortcut "$SMPROGRAMS\Seamly2D\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\Seamly2D\Seamly2D.lnk" "$INSTDIR\seamly2d.exe" "" "$INSTDIR\seamly2d.exe" 0
  CreateShortcut "$SMPROGRAMS\Seamly2D\SeamlyMe.lnk" "$INSTDIR\seamlyme.exe" "" "$INSTDIR\seamlyme.exe" 0
  
SectionEnd


Section "Uninstall"
  
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Seamly2D"
  DeleteRegKey HKLM SOFTWARE\NSIS_Seamly2D

  RMDir /r $INSTDIR

  Delete "$SMPROGRAMS\Seamly2D\*.*"
  RMDir "$SMPROGRAMS\Seamly2D"

SectionEnd

Function check_for_window

Push  0   # the starting offset of the push below
Push  $R1   # the known part of the class name
Call  EnhancedFindWindow
Pop  $0   # will contain the full wcn
Pop  $1   # will contain the window's handle both will contain "failed", if no matching class name was found
IsWindow $1 0 not_running
MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "Please close any running Seamly2D processes" IDRETRY -6 IDABORT +1
Quit
not_running:
FunctionEnd

Function  EnhancedFindWindow
  ; input, save variables
  Exch  $0   # part of the class name to search for
  Exch
  Exch  $1   # starting offset
  Push  $2   # length of $0
  Push  $3   # window handle
  Push  $4   # class name
  Push  $5   # temp
 
  ; set up the variables
  StrCpy  $4  0
  StrLen  $2  $0
 
 ; loop to search for open windows
 search_loop:
  FindWindow  $3  ""  ""  0  $3
   IntCmp  $3  0  search_failed
    IsWindow  $3  0  search_loop
     System::Call 'user32.dll::GetWindowText(i r3, t .r4, i ${NSIS_MAX_STRLEN}) i .n'
	 Push $4
	 Push $R1
	 Call StrContains
	 Pop $R9
     StrCmp $R9 ""  search_loop search_end
 
 ; no matching class-name found, return "failed"
 search_failed:
  StrCpy  $3  "failed"
  StrCpy  $4  "failed"
 
 ; search ended, output and restore variables
 search_end:
  StrCpy  $1  $3
  StrCpy  $0  $4
  Pop  $5
  Pop  $4
  Pop  $3
  Pop  $2
  Exch  $1
  Exch
  Exch  $0
FunctionEnd

; StrContains
; This function does a case sensitive searches for an occurrence of a substring in a string. 
; It returns the substring if it is found. 
; Otherwise it returns null(""). 
; Written by kenglish_hi
; Adapted from StrReplace written by dandaman32
 
 
Var STR_HAYSTACK
Var STR_NEEDLE
Var STR_CONTAINS_VAR_1
Var STR_CONTAINS_VAR_2
Var STR_CONTAINS_VAR_3
Var STR_CONTAINS_VAR_4
Var STR_RETURN_VAR
 
Function StrContains
  Exch $STR_NEEDLE
  Exch 1
  Exch $STR_HAYSTACK
  ; Uncomment to debug
  ;MessageBox MB_OK 'STR_NEEDLE = $STR_NEEDLE STR_HAYSTACK = $STR_HAYSTACK '
    StrCpy $STR_RETURN_VAR ""
    StrCpy $STR_CONTAINS_VAR_1 -1
    StrLen $STR_CONTAINS_VAR_2 $STR_NEEDLE
    StrLen $STR_CONTAINS_VAR_4 $STR_HAYSTACK
    loop:
      IntOp $STR_CONTAINS_VAR_1 $STR_CONTAINS_VAR_1 + 1
      StrCpy $STR_CONTAINS_VAR_3 $STR_HAYSTACK $STR_CONTAINS_VAR_2 $STR_CONTAINS_VAR_1
      StrCmp $STR_CONTAINS_VAR_3 $STR_NEEDLE found
      StrCmp $STR_CONTAINS_VAR_1 $STR_CONTAINS_VAR_4 done
      Goto loop
    found:
      StrCpy $STR_RETURN_VAR $STR_NEEDLE
      Goto done
    done:
   Pop $STR_NEEDLE ;Prevent "invalid opcode" errors and keep the
   Exch $STR_RETURN_VAR  
FunctionEnd
 
!macro _StrContainsConstructor OUT NEEDLE HAYSTACK
  Push `${HAYSTACK}`
  Push `${NEEDLE}`
  Call StrContains
  Pop `${OUT}`
!macroend
 
!define StrContains '!insertmacro "_StrContainsConstructor"'
