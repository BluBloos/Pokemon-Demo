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
const int screenWidth = 800;
const int screenHeight = 450;

game_memory gameMemory = {};
game_offscreen_buffer gameOffscreenBuffer = {};
game_user_input gameUserInput = {};
game_user_input PREV_gameUserInput = {};
game_sound_output_buffer gameSoundOutputBuffer = {};

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
    
    /* Next Steps: Put data in a data file.
    
    */
    
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
    
    // Initialize the gameOffscreenBuffer structure
    {
        // NOTE: Make damn sure the image matches the gameOffscreenBuffer!
        //backbuffer = LoadTexture("texture");
        
        gameOffscreenBuffer.width = screenWidth;
        gameOffscreenBuffer.height = screenHeight;
        gameOffscreenBuffer.BytesPerPixel = 4;
        gameOffscreenBuffer.pitch = screenWidth * gameOffscreenBuffer.BytesPerPixel;
        gameOffscreenBuffer.memory = MemAlloc(screenWidth * screenHeight * gameOffscreenBuffer.BytesPerPixel);
    }
    
    // Initialize the game user input
    {
        // Open everything from the local dir ?
        
        // pokemondemo.cpp opens things like
        //  "Data\\..." 
        
        gameUserInput.BaseFilePath = ""; 
    }
    
#ifdef PLATFORM_WEB
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    // Set our game to run at 60 frames-per-second
    SetTargetFPS(60);
    
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
    }
    
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
        gameUserInput.DeltaTime = 1.0f / 30.0f;
        
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
            
            /* some debug stuff
            {
                bool z_down = IsKeyDown(KEY_Z);
                if (z_down)
                    printf("Z Down\n");
                else
                    printf("Z not down\n");
            }*/
            
            // Process the controller
            if (IsGamepadAvailable(GAMEPAD_PLAYER1))
            {
                game_user_gamepad_input *prevc = &PREV_gameUserInput.GamepadInput[0];
                game_user_gamepad_input *c = &gameUserInput.GamepadInput[0];
                
                // A button
                RaylibProcessInput(&prevc->DebugButtons[1],&c->DebugButtons[1],IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT));
                
                float axisRightX = GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_RIGHT_X);
                
                if (axisRightX > 0.1f) // deadzone
                {
                    RaylibProcessInput(&prevc->Right, &c->Right, true);
                    RaylibProcessInput(&prevc->Left, &c->Left, false);
                }
                else if (axisRightX < 0.1f)
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
    }
    
    // Run the game
    GameUpdateRender(&gameMemory, &gameOffscreenBuffer, &gameUserInput);
    
    //UpdateTexture(backbuffer, gameOffscreenBuffer.memory); 
    
    
    // get the audio
    //GameGetSoundSamples(game_memory *Memory, game_sound_output_buffer *SoundBuffer, game_user_input *Input);
    
    // play the audio
    
    // Blit the screen to the raylib window
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    
    ClearBackground(BLACK);
    
    // Pixel blit
    
    unsigned char *Pixel = (unsigned char *)gameOffscreenBuffer.memory;
    
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
    }
    
    
    //DrawTexture(backbuffer, 0, 0, WHITE);
    
    EndDrawing();
    //----------------------------------------------------------------------------------
}
