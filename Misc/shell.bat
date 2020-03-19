@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
set path=C:\PokemonDemo\Misc;C:\PokemonDemo\Code;C:\PokemonDemo\build;%path%
cd C:\PokemonDemo\build