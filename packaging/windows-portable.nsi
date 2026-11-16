Unicode true
Name "Biblioteca 3D"
OutFile "dist\Biblioteca-Windows.exe"
RequestExecutionLevel user
SilentInstall silent
AutoCloseWindow true

Section
  InitPluginsDir
  SetOutPath "$PLUGINSDIR"
  File /r "windows-stage\*"
  ExecWait '"$PLUGINSDIR\biblioteca.exe"' $0
  SetErrorLevel $0
SectionEnd
