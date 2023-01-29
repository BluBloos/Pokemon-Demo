// this file permits us to compile PokemonDemo with automata engine
// but without actually changing PokemonDemo's code :D

#include "PokemonDemo.cpp"

#include <automata_engine.h>

// NOTE: It seems that PokemonDemo does init within game update and render
// but only if memory has the dirty bit or something like this.

static void ToggleFullscreen() {}
static debug_read_file_result ReadEntireFile(char *FileName)
{
    debug_read_file_result result = {};
    // the conversion here to const char is quite crude.
    // but it is OK.
    // the reason AE does this is to store the fileName.
    // but here, we never access it again.
    ae::loaded_file loadedFile = ae::platform::readEntireFile((const char*)FileName);
    result.Contents = loadedFile.contents;
    result.ContentSize = loadedFile.contentSize;
    return result;
}

// TODO: Currently, automata engine does not offer this sort of thing.
// returns byes written.
static int WriteEntireFile(char *FileName, int MemorySize, void *Memory)
{
    int result = MemorySize;
    return result;
}

static void FreeFileMemory(void *Memory)
{
    ae::platform::free(Memory);
}

static game_memory Memory = {};

static const char *getRuntimeExeDirPathPath[256];// TODO: make this better.

static void UpdateAndRenderProxy(ae::game_memory_t *gameMemory)
{    
    Memory.StorageSize = gameMemory->dataBytes;
    Memory.Storage = gameMemory->data;
    if (Memory.Storage != NULL)
    {
        Memory.Valid = true;
    }

    Memory.DEBUGPlatformReadEntireFile = ReadEntireFile;
    Memory.DEBUGPlatformWriteEntireFile = WriteEntireFile;
    Memory.DEBUGPlatformFreeFileMemory = FreeFileMemory;

    // NOTE: doesn't seem PokemonDemo uses the transient storage.
    //Memory.TransientStorageSize = 0;
    //Memory.TransientStorage = NULL;

    // TODO:
    Memory.ToggleFullscreen = ToggleFullscreen;

    game_offscreen_buffer buffer = {};
    buffer.memory = gameMemory->backbufferPixels;
    buffer.width = gameMemory->backbufferWidth;
    buffer.height = gameMemory->backbufferHeight;
    buffer.BytesPerPixel = sizeof(uint32_t);
    buffer.pitch = buffer.width * buffer.BytesPerPixel;

    ae::user_input_t userInput;
    ae::platform::getUserInput(&userInput);

    game_user_input Input = {};

    // TODO:
    Input.MouseButtons[0].EndedDown = userInput.mouseLBttnDown;
    Input.MouseButtons[1].EndedDown = userInput.mouseRBttnDown;

    Input.MouseX = userInput.mouseX;
    Input.MouseY = userInput.mouseY;
    // NOTE: Don't need Input.MouseZ;

    Input.DeltaTime = ae::platform::lastFrameTimeTotal;
    Input.BaseFilePath = (char*)getRuntimeExeDirPathPath;
    assert(Input.BaseFilePath != NULL);

    // TODO:
    Input.GamepadInput;

    ColorMode gameColorMode = BGR; // is this right?
    GameUpdateRender(&Memory, &buffer, &Input, gameColorMode);
}

void ae::PreInit(ae::game_memory_t *gameMemory)
{
    ae::defaultWindowName = "Pokemon Demo";
}

void ae::Init(ae::game_memory_t *gameMemory)
{
//    ae::game_state_t *gameState = automata_engine::getGameState(gameMemory);

    ae::platform::getRuntimeExeDirPath((char*)getRuntimeExeDirPathPath, 256);
    Memory.IsInitialized = false;
    Memory.Valid = false;

    // TODO:
    // Register a voice that is merely a proxy
    // to get OnVoiceBufferProcess call,
    // then we insert our own samples.

    // register the game's main function
    automata_engine::bifrost::registerApp("pokemon demo", UpdateAndRenderProxy);
}

void ae::Close(ae::game_memory_t *gameMemory)
{
    // PokemonDemo has no close state.
    // it never allocates memory and allocs within its own arenas
    // plus it has game_memory_t.
}

void ae::HandleWindowResize(ae::game_memory *gameMemory, int newWdith, int newHeight)
{
    // pokemon demo has no special behaviour for window resize.
    // the backbuffer has whatever width/height it has and that is that.
    // up to platform layer to control how this works.

}

void ae::OnVoiceBufferProcess(ae::game_memory_t* gameMemory, intptr_t voiceHandle, void* dst, void* src,
    uint32_t samplesToWrite, int channels, int bytesPerSample)
{
    // TODO:
    //GameGetSoundSamples

}

void ae::OnVoiceBufferEnd(ae::game_memory_t* gameMemory, intptr_t voiceHandle)
{

}