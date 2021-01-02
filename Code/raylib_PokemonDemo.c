/* TODO: 

Bugs:
- talking to mom, second message does not work!

*/

/*

First thing - need a buffer for the screen to write to!

- GetWindowHandle looks interesting

Use Module: Textures
- Render textures, or something like this.

Worst case scenario:
- make a buffer, pass it to the game, use the DrawPixel function
- (we basically do the blit ourselves) - too slow





MemAlloc is good for the memory we need

LoadFileData will be nice

bunch of key functions, so we will use those

gamepad support by raylib, will use this

for audio thinking AudioStream and Sound look good

*/

/*******************************************************************************************
*
*   raylib [core] example - 3d camera first person
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_COLUMNS 20
//#define PLATFORM_WEB

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#include <stddef.h> // for size_t

//#include "PokemonDemo.h"
// NOTE: Diff from win32 version, no hot reloading
#include "PokemonDemo.cpp"

// Initialization
//--------------------------------------------------------------------------------------
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
const int screenWidth = SCREEN_WIDTH;
const int screenHeight = SCREEN_HEIGHT;

game_memory gameMemory = {};
game_offscreen_buffer gameOffscreenBuffer = {};
game_user_input gameUserInput = {};
game_user_input PREV_gameUserInput = {};
game_sound_output_buffer gameSoundOutputBuffer = {};

//-------------------- audio things
// buffer to play audio from
short *GameSoundSamples;
// pointer to current pos in buffer
int readCursor = 0;
// audio stream (raylib managed)
AudioStream stream;

// Set the size of each buffer in the double buffer to be 1920 samples. This gives us 40ms delay (w/ 48kHz) between the write cursor and the play cursor.
// At 30 FPS, 33ms / frame, this should be feasible with no audio breaks.
#define SOUND_BUFFER_SAMPLES 1920
//-------------------------

//------------------
Texture2D backbuffer = {};
//-----------------

#define FRAME_RATE 60

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame


//----------------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------------
debug_read_file_result DebugReadEntireFile(char *fileName)
{
    debug_read_file_result result = {};
    
    unsigned int bytesRead;
    unsigned char *buffer = LoadFileData(fileName, &bytesRead);
    
    result.Contents = buffer;
    result.ContentSize = bytesRead;
    
    return result;
}

int DebugWriteEntireFile(char *fileName, int memorySize, void *memory)
{
    return SaveFileData(fileName, memory, memorySize);
}

void DebugFreeFileMemory(void *memory)
{
    UnloadFileData((unsigned char *)memory);
}

TOGGLE_FULLSCREEN_CALLBACK(ToggleFullscreenCallback)
{
	ToggleFullscreen();
}
//----------------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------------


int main(void)
{
    //printf("Hello, World!\n");
    
    InitWindow(screenWidth, screenHeight, "Pokemon Demo");
    
    // Initialize audio things
    {
        InitAudioDevice();       
        
        SetMasterVolume(1.0f); 
        
        SetAudioStreamBufferSizeDefault(SOUND_BUFFER_SAMPLES);  
        
        // 48 khz, 16bit sample size, channels: 2
        stream = InitAudioStream(48000, 16, 1);
        
        gameSoundOutputBuffer.SamplesPerSecond = 48000;
        GameSoundSamples = MemAlloc(SOUND_BUFFER_SAMPLES * sizeof(short));
        
        // Start processing stream buffer (no data loaded currently)
        PlayAudioStream(stream);        
    }
    
    // Initialize the gameMemory structure
    {
        gameMemory.IsInitialized = false;
        gameMemory.StorageSize = MegaBytes(64);
        gameMemory.Storage = MemAlloc(gameMemory.StorageSize); 
        gameMemory.TransientStorageSize = GigaBytes(1);
        gameMemory.TransientStorage = MemAlloc(gameMemory.TransientStorageSize);
        
        if (gameMemory.Storage != NULL && gameMemory.TransientStorage != NULL)
            gameMemory.Valid = true;
        
        gameMemory.DEBUGPlatformReadEntireFile = DebugReadEntireFile;
        gameMemory.DEBUGPlatformFreeFileMemory = DebugFreeFileMemory;
        gameMemory.DEBUGPlatformWriteEntireFile = DebugWriteEntireFile;
        gameMemory.ToggleFullscreen = ToggleFullscreenCallback;
    }
    
    // Initialize the gameOffscreenBuffer structure and the backbuffer through raylib opengl layer
    {
        int width = screenWidth;
        int height = screenHeight;
        // format is R low order, G middle order, B top order, A is the top most order
        int format = UNCOMPRESSED_R8G8B8A8;
        int mipmaps = 1;
        
        // NOTE: Make damn sure the image matches the gameOffscreenBuffer!
        gameOffscreenBuffer.width = width;
        gameOffscreenBuffer.height = height;
        gameOffscreenBuffer.BytesPerPixel = sizeof(unsigned int);
        gameOffscreenBuffer.pitch = width * gameOffscreenBuffer.BytesPerPixel;
        gameOffscreenBuffer.memory = MemAlloc(width * height * gameOffscreenBuffer.BytesPerPixel);
        
        backbuffer.id = rlLoadTexture(gameOffscreenBuffer.memory, width, height, format, mipmaps);
        backbuffer.width = screenWidth;
        backbuffer.height = screenHeight;
        backbuffer.format = format;
        backbuffer.mipmaps = mipmaps;
    }
    
    // Initialize the game user input
    {
        // Open everything from the local dir ?
        
        // pokemondemo.cpp opens things like
        //  "Data\\..." 
        
#ifdef PLATFORM_DESKTOP
        gameUserInput.BaseFilePath = "C:\\dev\\pokemondemo\\Data\\";
#elif PLATFORM_WEB
        gameUserInput.BaseFilePath = "";
#endif
    }
    
#ifdef PLATFORM_WEB
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    // Set our game to run at 60 frames-per-second
    SetTargetFPS(FRAME_RATE);
    
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif
    
    //--------------------------------------------------------------------------------------
    
    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // clean gameMemory stucture
    {
        if (gameMemory.Storage)
            MemFree(gameMemory.Storage);
        if (gameMemory.TransientStorage)
            MemFree(gameMemory.TransientStorage);
        if (GameSoundSamples)
            MemFree(GameSoundSamples);
    }
    
    // Unload the backbuffer
    UnloadTexture(backbuffer);
    // Close raw audio stream and delete buffers from RAM
    CloseAudioStream(stream);   
    // Close audio device (music streaming is automatically stopped)
    CloseAudioDevice();         
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}

void RaylibProcessInput(game_user_gamepad_button_state *PrevState, game_user_gamepad_button_state *CurrentState, bool IsDown)
{
    CurrentState->EndedDown = IsDown;
    // TODO: I hard coded this because we only poll once.
    if (PrevState->EndedDown != CurrentState->EndedDown)
        CurrentState->HalfTransitionCount = 1;
    else
        CurrentState->HalfTransitionCount = 0;
}

void UpdateDrawFrame(void)
{
    
    // Update input stuff
    {
        gameUserInput.DeltaTime = 1.0f / FRAME_RATE;
        
        // old input becomes current input
        {
            for (int i = 0; i < 5; i++)
            {
                PREV_gameUserInput.MouseButtons[i] = gameUserInput.MouseButtons[i];
            }
            
            PREV_gameUserInput.GamepadInput[0] = gameUserInput.GamepadInput[0];
            PREV_gameUserInput.GamepadInput[1] = gameUserInput.GamepadInput[1];
        }
        
        // update the current input
        // TODO: Only polling input once, this surely could lead to errors?
        {
            // TODO: Not sure if these coordinates are in the proper space
            gameUserInput.MouseX = GetMouseX();
            gameUserInput.MouseY = GetMouseY();
            gameUserInput.MouseZ = 0;
            
            // Process the mouse buttons
            for (int i = 0; i < 5; i++)
            {
                RaylibProcessInput(&PREV_gameUserInput.MouseButtons[i],&gameUserInput.MouseButtons[i],IsMouseButtonDown(i));
            }
            
            // Process the keyboard "controller"
            {
                RaylibProcessInput(&PREV_gameUserInput.GamepadInput[1].Up,&gameUserInput.GamepadInput[1].Up,IsKeyDown(KEY_W));
                
                RaylibProcessInput(&PREV_gameUserInput.GamepadInput[1].Left,&gameUserInput.GamepadInput[1].Left,IsKeyDown(KEY_A));
                
                RaylibProcessInput(&PREV_gameUserInput.GamepadInput[1].Down,&gameUserInput.GamepadInput[1].Down,IsKeyDown(KEY_S));
                
                RaylibProcessInput(&PREV_gameUserInput.GamepadInput[1].Right,&gameUserInput.GamepadInput[1].Right,IsKeyDown(KEY_D));
                
                RaylibProcessInput(&PREV_gameUserInput.GamepadInput[1].DebugButtons[0],&gameUserInput.GamepadInput[1].DebugButtons[0],IsKeyDown(KEY_F));
                
                RaylibProcessInput(&PREV_gameUserInput.GamepadInput[1].DebugButtons[1],&gameUserInput.GamepadInput[1].DebugButtons[1],IsKeyDown(KEY_Z));
            }
            
            // Process the controller
            /*
            if (IsGamepadAvailable(GAMEPAD_PLAYER1))
            {
                game_user_gamepad_input *prevc = &PREV_gameUserInput.GamepadInput[0];
                game_user_gamepad_input *c = &gameUserInput.GamepadInput[0];
                
                // Any button
                bool any_button = false;
                
                // NOTE: 17 is a nice number for max buttons
                for (int i = 0; i < 17; i++)
                {
                    bool result = IsGamepadButtonDown(GAMEPAD_PLAYER1, 
                                                      i);
                    if (result)
                        any_button = true;
                }
                
                // NOTE: This works such that a half transition count of 1 means that ALL buttons were not pressed, then at least 1 button was pressed.
                RaylibProcessInput(&prevc->DebugButtons[1],&c->DebugButtons[1], any_button);
                
                // the gamepad has at least 1 axis
                if (GetGamepadAxisCount(GAMEPAD_PLAYER1))
                {
                    float axisRightX = GetGamepadAxisMovement(GAMEPAD_PLAYER1, 0);
                    
                    if (axisRightX > 0.2f) // deadzone
                    {
                        RaylibProcessInput(&prevc->Right, &c->Right, true);
                        RaylibProcessInput(&prevc->Left, &c->Left, false);
                    }
                    else if (axisRightX < 0.2f)
                    {
                        RaylibProcessInput(&prevc->Left, &c->Left, true);
                        RaylibProcessInput(&prevc->Right, &c->Right, false);
                    } else
                    {
                        RaylibProcessInput(&prevc->Right, &c->Right, false);
                        RaylibProcessInput(&prevc->Left, &c->Left, false);
                    }
                }
            }
            */
        }
    } // done updating input
    
    // Run the game
    GameUpdateRender(&gameMemory, &gameOffscreenBuffer, &gameUserInput, RGB);
    
    UpdateTexture(backbuffer, gameOffscreenBuffer.memory); 
    
    // audio stuffs
    {
        // Refill audio stream if required
        if (IsAudioStreamProcessed(stream))
        {
            gameSoundOutputBuffer.SampleAmount = SOUND_BUFFER_SAMPLES;
            gameSoundOutputBuffer.SampleOut = GameSoundSamples;
            
            // get the audio
            // NOTE: The game expects one contiguous chunk of mem to write to. It doesn't know that there is a loop buff in the background.
            GameGetSoundSamples(&gameMemory, &gameSoundOutputBuffer, &gameUserInput, 
                                true);
            
            // Copy finished frame to audio stream
            UpdateAudioStream(stream, GameSoundSamples, SOUND_BUFFER_SAMPLES);
        }
    }
    
    // Blit the screen to the raylib window
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    
    ClearBackground(BLACK);
    
    // Pixel blit:
    // NOTE: This is painstakingly slow
    /*unsigned char *Pixel = (unsigned char *)gameOffscreenBuffer.memory;
    
    for (int y = 0; y < screenHeight; y++)
    {
        unsigned int *Row = (unsigned int *)Pixel;
        for (int x = 0; x < screenWidth; x++)
        {
            DrawPixel(x, y, *(Color *)Row);
            
            // Go to the next pixel
            Row++;
        }
        // go to the next row
        Pixel += gameOffscreenBuffer.pitch;
    }*/
    
    // tint of white (no tint)
    DrawTexture(backbuffer, 0, 0, WHITE);
    
    DrawFPS(10, 10);
    
    if (IsGamepadAvailable(GAMEPAD_PLAYER1))
    {
        DrawText(TextFormat("GP1: %s", GetGamepadName(GAMEPAD_PLAYER1)), screenWidth - 300, 10, 10, BLACK);
        
        DrawText(TextFormat("DETECTED AXIS [%i]:", GetGamepadAxisCount(GAMEPAD_PLAYER1)), 10, 50, 10, MAROON);
        
        for (int i = 0; i < GetGamepadAxisCount(GAMEPAD_PLAYER1); i++)
        {
            DrawText(TextFormat("AXIS %i: %.02f", i, GetGamepadAxisMovement(GAMEPAD_PLAYER1, i)), 20, 70 + 20*i, 10, DARKGRAY);
        }
        
        if (GetGamepadButtonPressed() != -1) DrawText(TextFormat("DETECTED BUTTON: %i", GetGamepadButtonPressed()), 10, 430, 10, RED);
        else DrawText("DETECTED BUTTON: NONE", 10, 430, 10, GRAY);
    }
    
    EndDrawing();
    //----------------------------------------------------------------------------------
}
