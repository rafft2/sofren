@echo off

IF NOT EXIST bin/ mkdir bin

set warning_opts=-DWIN32_LEAN_AND_MEAN -DDEBUG_BUILD -D_CRT_SECURE_NO_WARNINGS -Wall -WX -wd4100 -wd4820 -wd4189 -wd5045 -wd4005 -wd4668 -wd4702 -wd4201 -wd5246 -wd4191 -wd4514

cl -nologo -Zi -GR- -EHa- -Odi %warning_opts% /I thirdparty\stb src/sf_main.cpp -Fe:"bin/sf.exe" -Fo:"bin/sf.obj" -Fd:"bin/sf.pdb"

set warning_opts=