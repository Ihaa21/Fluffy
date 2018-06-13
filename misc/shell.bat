@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2018.2.185\windows\bin\ipsxe-comp-vars.bat" intel64 vs2013

set path=W:\towerdef_rosemary\misc;%path%

set _NO_DEBUG_HEAP=1