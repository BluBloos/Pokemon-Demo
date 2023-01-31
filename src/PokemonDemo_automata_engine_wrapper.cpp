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
    ae::loaded_file_t loadedFile = ae::platform::readEntireFile((const char*)FileName);
    result.Contents = loadedFile.contents;
    result.ContentSize = loadedFile.contentSize;
    return result;
}

static int WriteEntireFile(char *FileName, int MemorySize, void *Memory)
{
    // crude conversion to const char*.
    // but it is OK.
    return ae::platform::writeEntireFile(
        (const char*)FileName, Memory, MemorySize);
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

    game_offscreen_buffer buffer = {};
    buffer.memory = gameMemory->backbufferPixels;
    buffer.width = gameMemory->backbufferWidth;
    buffer.height = gameMemory->backbufferHeight;
    buffer.BytesPerPixel = sizeof(uint32_t);
    buffer.pitch = buffer.width * buffer.BytesPerPixel;

    ae::user_input_t userInput;
    ae::platform::getUserInput(&userInput);

    game_user_input Input = {};

    // NOTE: PokemonDemo does not use the mouse.
    //Input.MouseButtons[0].EndedDown = userInput.mouseLBttnDown;
    //Input.MouseButtons[1].EndedDown = userInput.mouseRBttnDown;

    Input.MouseX = userInput.mouseX;
    Input.MouseY = userInput.mouseY;
    // NOTE: Don't need Input.MouseZ;

    Input.DeltaTime = ae::platform::lastFrameTimeTotal;
    Input.BaseFilePath = (char*)getRuntimeExeDirPathPath;
    assert(Input.BaseFilePath != NULL);

    auto &keyboard = Input.GamepadInput[1]; // Has 2. So 2 controllers??
    // 0 = controller.
    // 1 = keyboard.
    // In PokemonDemo seems like the keyboard is virtualized as a gamepad.

    // DebugButton[0] = toggle fullscreen button.
    // DebugButton[1] = the Z key.
    // the Right and Left map to in battle.
    // Up and Down map to movement (along with left and right).

    // NOTE: the below stuff is sort of a hack.
    // we are likely to make the automata engine input system better.
    // but for now, this is OK.

    static bool zBefore = false;
    static bool rBefore = false;
    static bool lBefore = false;
    static bool uBefore = false;
    static bool dBefore = false;

    keyboard.DebugButtons[1].EndedDown = userInput.keyDown[ae::GAME_KEY_Z];
    keyboard.Right.EndedDown = userInput.keyDown[ae::GAME_KEY_D];
    keyboard.Left.EndedDown = userInput.keyDown[ae::GAME_KEY_A];
    keyboard.Up.EndedDown = userInput.keyDown[ae::GAME_KEY_W];
    keyboard.Down.EndedDown = userInput.keyDown[ae::GAME_KEY_S];

    keyboard.DebugButtons[1].HalfTransitionCount = !!(zBefore != keyboard.DebugButtons[1].EndedDown);
    keyboard.Right.HalfTransitionCount = !!(rBefore != keyboard.Right.EndedDown);
    keyboard.Left.HalfTransitionCount = !!(lBefore != keyboard.Left.EndedDown);
    keyboard.Up.HalfTransitionCount = !!(uBefore != keyboard.Up.EndedDown);
    keyboard.Down.HalfTransitionCount = !!(dBefore != keyboard.Down.EndedDown);

    zBefore = keyboard.DebugButtons[1].EndedDown;
    rBefore = keyboard.Right.EndedDown;
    lBefore = keyboard.Left.EndedDown;
    uBefore = keyboard.Up.EndedDown;
    dBefore = keyboard.Down.EndedDown;

    ColorMode gameColorMode = BGR; // is this right?
    GameUpdateRender(&Memory, &buffer, &Input, gameColorMode);
}

void ae::PreInit(ae::game_memory_t *gameMemory)
{
    ae::defaultWindowName = "Pokemon Demo";
    
    // Pokemon demo assumes a backbuffer of this precise size.
    // Also that the backbuffer size never changes.
    ae::defaultWidth = 1280;
    ae::defaultHeight = 720;
}

static intptr_t g_proxyVoice;

void ae::Init(ae::game_memory_t *gameMemory)
{
//    ae::game_state_t *gameState = automata_engine::getGameState(gameMemory);

    ae::platform::getRuntimeExeDirPath((char*)getRuntimeExeDirPathPath, 256);
    Memory.IsInitialized = false;
    Memory.Valid = false;

    g_proxyVoice = ae::platform::createVoice();
    if (g_proxyVoice == ae::platform::INVALID_VOICE)
    {
        PlatformLoggerError("Failed to create proxy voice for PokemonDemo.\n");
    }
    //bool voiceSubmitBuffer(intptr_t voiceHandle, void *data, uint32_t size, bool shoudLoop = false);
    constexpr size_t audioBytesCount = ae::io::ENGINE_DESIRED_SAMPLES_PER_SECOND * 2;
    static constexpr char audioBytes[audioBytesCount] = {}; // dummy data
    ae::platform::voiceSubmitBuffer(
        g_proxyVoice, (void*)audioBytes, audioBytesCount, true);
    ae::platform::voicePlayBuffer(g_proxyVoice);

    Memory.DEBUGPlatformReadEntireFile = ReadEntireFile;
    Memory.DEBUGPlatformWriteEntireFile = WriteEntireFile;
    Memory.DEBUGPlatformFreeFileMemory = FreeFileMemory;

    // NOTE: doesn't seem PokemonDemo uses the transient storage.
    //Memory.TransientStorageSize = 0;
    //Memory.TransientStorage = NULL;

    // TODO:
    Memory.ToggleFullscreen = ToggleFullscreen;

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

void ae::HandleWindowResize(ae::game_memory_t *gameMemory, int newWdith, int newHeight)
{
    // pokemon demo has no special behaviour for window resize.
    // the backbuffer has whatever width/height it has and that is that.
    // up to platform layer to control how this works.

}

void ae::OnVoiceBufferProcess(ae::game_memory_t* gameMemory, intptr_t voiceHandle, void* dst, void* src,
    uint32_t samplesToWrite, int channels, int bytesPerSample)
{
    Memory.StorageSize = gameMemory->dataBytes;
    Memory.Storage = gameMemory->data;
    if (Memory.Storage != NULL)
    {
        Memory.Valid = true;
    }

    game_sound_output_buffer SoundBuffer;
    SoundBuffer.SamplesPerSecond = ae::io::ENGINE_DESIRED_SAMPLES_PER_SECOND;
    SoundBuffer.SampleAmount = samplesToWrite;

    // Pokemon demo expects 16-bit samples.
    // but OnVoiceBufferProcess works with float.
    // alloc temp buffer.
    static constexpr size_t tempBufferSize = ae::io::ENGINE_DESIRED_SAMPLES_PER_SECOND * 2;
    static short tempBuffer[tempBufferSize] = {};
    SoundBuffer.SampleOut = tempBuffer;

    // void GameGetSoundSamples(
    //     game_memory *Memory, game_sound_output_buffer *SoundBuffer, game_user_input *Input,bool mono)
    GameGetSoundSamples(&Memory, &SoundBuffer, NULL, false);

    // copy tempBuffer to dst.
    float *dstPtr = (float *)dst;
    for (size_t i = 0; i < samplesToWrite; ++i)
    {
        *dstPtr++ = (float)tempBuffer[i*2] / 32768.0f;
        *dstPtr++ = (float)tempBuffer[i*2+1] / 32768.0f;
    }

    // NOTE: currently, PokemonDemo does not use Input in the sound callback.
    // So we are good to go for just not setting this.
}

void ae::OnVoiceBufferEnd(ae::game_memory_t* gameMemory, intptr_t voiceHandle)
{
    

}

// TODO: after not looking at Pokemon Demo in a while, looks like the docs
// for this proj can be improved.
// for example, the layout of buttons and mouse in the game_user_input struct
// is not well-defined at all.