pushd emsdk
./emsdk install latest
./emsdk activate latest
source "./emsdk_env.sh"
EMSDK_PATH=$EMSDK
popd

pushd raylib
pushd src
export MACOSX_DEPLOYMENT_TARGET=10.9
xcode-select --install
rm *.o
rm libraylib.a
make PLATFORM=PLATFORM_DESKTOP -B
popd
popd

mkdir build
mkdir build/macOS
cp -r Data build/macOS/Data

clang -g -framework CoreVideo -framework IOKit -framework Cocoa \
    -framework GLUT -framework OpenGL \
    src/raylib_PokemonDemo.c -o build/macOS/PokemonDemo \
    -I raylib/src -L raylib/src/ -lraylib
