pushd emsdk
./emsdk install latest
./emsdk activate latest
source "./emsdk_env.sh"
EMSDK_PATH=$EMSDK
popd

pushd raylib
pushd src
rm *.o
rm libraylib.a
make PLATFORM=PLATFORM_WEB -B
popd
popd

mkdir build
mkdir build/raylib
mkdir build/raylib/surge/

rm build/raylib/surge/*.wasm
rm build/raylib/surge/*.html
rm build/raylib/surge/*.js
rm build/raylib/surge/*.data

emcc -o build/raylib/surge/index.html src/raylib_PokemonDemo.c -Os -Wall raylib/src/libraylib.a  \
    -I raylib/src  -Lraylib/src/libraylib.a -s USE_GLFW=3 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 \
    --shell-file src/shell_minimal.html --emrun --preload-file Data -DPLATFORM_WEB