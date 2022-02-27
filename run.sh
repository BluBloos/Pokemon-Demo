pushd emsdk
./emsdk install latest
./emsdk activate latest
source "./emsdk_env.sh"
EMSDK_PATH=$EMSDK
popd

emrun build/raylib/surge/index.html