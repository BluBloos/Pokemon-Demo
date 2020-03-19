@echo off

set CodeLocation=C:\dev\PokemonDemo\Code

set CommonCompilerFlags=-MT -nologo -Gm- -EHa- -GR- -Oi -WX -W4 -wd4293 -wd4456 -wd4100 -wd4189 -wd4505 -wd4701 -DPOKEMON_DEMO_DEBUG=1 -FC -Zi -RTCs
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib dinput8.lib dxguid.lib
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% -Fmwin32_PokemonDemo.map %CodeLocation%\win32_PokemonDemo.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%
cl %CommonCompilerFlags% -FmPokemonDemo.map %CodeLocation%\PokemonDemo.cpp -LD /link -subsystem:windows,5.1 -PDB:PokemonDemo_%random%.pdb -incremental:no -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateRender
popd