@echo off

pushd Code
del *.o
popd

pushd build
pushd raylib

del pokemondemo.wasm
del pokemondemo.html

mingw32-make BUILD_MODE=DEBUG

popd
popd