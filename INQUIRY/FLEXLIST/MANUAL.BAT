@echo off
if not exist hypertxt.exe goto :then
hypertxt flexlist.htx
goto :endif
:then
echo Change the default drive/path to the drive/path containing the
echo FlexList Demo/Manual distribution diskette files and then
echo at the DOS command prompt type:   manual
:endif
