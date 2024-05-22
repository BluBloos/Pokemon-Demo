@echo off

set CommonCompilerFlags=-nologo -Gm- -EHa- -GR- -Oi -W4 -wd4293 -wd4456 -wd4100 -wd4189 -wd4505 -wd4701 ^
    -DPOKEMON_DEMO_DEBUG=1 -FC -Zi -RTCs -I ..\Code

set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib dinput8.lib dxguid.lib

mkdir build
mkdir build\windows

pushd build\windows

del *.pdb > NUL 2> NUL

cl %CommonCompilerFlags% -Fmwin32_PokemonDemo.map ..\..\src\win32_PokemonDemo.cpp /link -subsystem:windows,5.2 %CommonLinkerFlags%

cl %CommonCompilerFlags% -FmPokemonDemo.map ..\..\src\PokemonDemo.cpp -LD /link -subsystem:windows,5.2 -PDB:PokemonDemo_%random%.pdb ^
    -incremental:no -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateRender

cl %CommonCompilerFlags% -FmPokemonDemoTileEditor.map ..\..\src\win32_PokemonDemo_tile_map_editor.cpp /link ^
    -subsystem:windows,5.2 %CommonLinkerFlags%

cl %CommonCompilerFlags% ..\..\src\win32_PikaBlue_Cmd.cpp /link -subsystem:console,5.2 %CommonLinkerFlags%

cl %CommonCompilerFlags% -DPIKABLUE_UI=1 ..\..\src\win32_PikaBlue_main.cpp /link -subsystem:windows,5.2 %CommonLinkerFlags% Comdlg32.lib

popd
