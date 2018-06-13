@echo off

set CodeDir=..\code
set OutputDir=..\build_win32

set CommonCompilerFlags= -arch:AVX2 -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4127 -wd4201 -wd4100 -wd4189 -wd4505 -wd4324 -wd4310 -Z7 -FC
set CommonCompilerFlags=-DFLUFFY_DEBUG=1 -DFLUFFY_WIN32=1 %CommonCompilerFlags%
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib Winmm.lib opengl32.lib

IF NOT EXIST %OutputDir% mkdir %OutputDir%

pushd %OutputDir%

del *.pdb > NUL 2> NUL

REM 64-bit build
echo WAITING FOR PDB > lock.tmp
cl -O2 %CommonCompilerFlags% %CodeDir%\fluffy.cpp -Fmfluffy.map -LD /link /Profile %CommonLinkerFlags% -incremental:no -opt:ref -PDB:fluffy_%random%.pdb -EXPORT:GameInit -EXPORT:GameUpdateAndRender
del lock.tmp
cl -Od %CommonCompilerFlags% %CodeDir%\win32_fluffy.cpp -Fmwin32_fluffy.map /link /Profile %CommonLinkerFlags%

popd
