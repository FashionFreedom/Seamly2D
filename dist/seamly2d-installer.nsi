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

  ; relative to the location of this .nsi file, copy all the files/directories recursively
  File /r *
  
  WriteRegStr HKLM SOFTWARE\NSIS_Seamly2D "Install_Dir" "$INSTDIR"
  
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
