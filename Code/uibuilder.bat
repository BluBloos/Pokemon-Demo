@echo off
set CommonCompilerFlags=-MT -nologo -Gm- -EHa- -GR- -Oi -WX -W4 -wd4293 -wd4456 -wd4100 -wd4189 -wd4505 -wd4701 -DPIKABLUE_UI=1 -DPOKEMON_DEMO_DEBUG=1 -FC -Zi 
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib dinput8.lib dxguid.lib Comdlg32.lib
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% C:\PokemonDemo\Code\win32_PikaBlue_main.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%
popd