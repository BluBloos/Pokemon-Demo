pushd emsdk
./emsdk install latest
./emsdk activate latest
source "./emsdk_env.sh"
EMSDK_PATH=$EMSDK
popd

pushd raylib
pushd src
if [ -f "libraylib.a" ]; then echo "No need to build raylib"; else make PLATFORM=PLATFORM_WEB -B; fi
popd
popd

pushd build
pushd raylib

rm pokemondemo.wasm
rm pokemondemo.html

popd
popd

emcc -o build/raylib/index.html src/raylib_PokemonDemo.c -Os -Wall raylib/src/libraylib.a  \
    -I raylib/src  -Lraylib/src/libraylib.a -s USE_GLFW=3 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file Data -DPLATFORM_WEB

pushd build
pushd raylib

npm install
npm start

popd
popd

#pushd src
#del *.o
#popd

#pushd build
#pushd raylib

#del pokemondemo.wasm
#del pokemondemo.html

#make BUILD_MODE=DEBUG

#popd
#popd