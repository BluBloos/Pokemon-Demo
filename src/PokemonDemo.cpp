/*

TODO:

Misc:
- Player 2 cannot start the game (win32 version)
- Player 2 has no thud sound
-          thud logic is bad and can use improvement anyway
- Game feel w.r.t to the player
- Shaders could be better
-       LUT that exits MAINbattle

Must do:


- Questions: What's happens if I kill arceus?



- Why are the animations loaded on battle -> leak memory?










MESSAGE AND FONT STUFFS
-----------------

- Update the kerning for exclamation mark w/ i (looks gross). Potentially space out the exclamation mark well for all character pairs.
---------------------

SHADERS / TRANSITIONS
----------------------

- Camera fade effect (diff type of "shader")
-------------------

NEW FEATURES
------------------
- Entity path planning / moving around.
- Walk on events.
-        specifically, camera scene for arceus.
- audio fades
- better audio mixing
- Get camera to ensure that both players are on the screen at the same time
-------------------

MOVEMENT SYSTEM / PLAYER SYSTEM
-----------------------
- Collision w/ NPC's is not to my liking!
- Player 2 doesn't do proper layering with other NPC, only player 1.

-----------------------



:

- Don't like the fact that the character can take a step without actually having an animation happen.
-         fixed but game still feels odd
- Game controls could prob be better (game feel)




--------------------------

*/

#include "PokemonDemo.h"

#if POKEMON_DEMO_DEBUG
#include "stdio.h"
#endif

typedef struct 
{
	loaded_bitmap Bitmap;
	float MinX; 
	float MaxX;
	float MinY; 
	float MaxY; 
	signed int Flip;
	int ScaleFlag;
	int Scalar;
} render_item;

typedef struct  
{
	int Count;
	render_item RenderItems[256];
} render_queue;

internal void PopFunction(void *Data, unsigned int QueueIndex)
{
	function_queue *FunctionQueue = (function_queue *)Data;
    
	if (FunctionQueue->Functions[QueueIndex].Function)
	{
        // if there is a function, call the function
		FunctionQueue->Functions[QueueIndex].Function(FunctionQueue->Functions[QueueIndex].Data, FunctionQueue->Functions[QueueIndex].Param);
        
        //clear the function 
		//FunctionQueue->Functions[QueueIndex] = {}; 
        PokeZeroMem(FunctionQueue->Functions[QueueIndex]);
        
		if ((FunctionQueue->Count > 0) && !(QueueIndex > FunctionQueue->Count - 1))
		{
			for (unsigned int x = 0; x < FunctionQueue->Count - 1 - QueueIndex; x++)
			{
				FunctionQueue->Functions[QueueIndex + x] = FunctionQueue->Functions[QueueIndex + x + 1];
			}
            
            //clear the last one too because it doesn't belong.
			//FunctionQueue->Functions[FunctionQueue->Count - 1] = {};
            PokeZeroMem(FunctionQueue->Functions[FunctionQueue->Count - 1]);
		}
        
		FunctionQueue->Count--; //if there was actually a function there, then when we clear it, we are subtracting from the count.
	}
} 

//NOTE: These render items need to be considered
INLINE void PostponeRenderItem(render_item RenderItem, render_queue *RenderQueue)
{
	RenderQueue->RenderItems[RenderQueue->Count] = RenderItem; 
	RenderQueue->Count += 1;
}

internal void DrawRenderQueueItems(game_offscreen_buffer *Buffer, render_queue *RenderQueue)
{
	for (int x = 0; x < RenderQueue->Count; x++)
	{
		render_item Item = RenderQueue->RenderItems[x];
		DrawBitmapScaled(Buffer, Item.Bitmap, Item.MinX, Item.MaxX, 
                         Item.MinY, Item.MaxY, Item.Flip, 2);
	}
}

//NOTE: We should rename this function
internal void SetPauseState(game_state *GameState, float PauseLength)
{
	GameState->StateFlags = GameState->StateFlags | PAUSE;
	GameState->PauseLength = PauseLength;
	GameState->InitialTimer = GameState->GameTimer;
}

//OKay belong is some animation stuff and we need to consider it because we will implement some dope animation system
internal void SetAnimation(animation_player *AnimPlayer, unsigned int Base)
{
	//so before we go to another animation we want to ensure that we actually have that animation
	if (AnimPlayer->MaxIter[Base + AnimPlayer->Offset])
	{
		AnimPlayer->SelectedAnim = Base;
	}
	AnimPlayer->Iter = 0;
}

internal loaded_bitmap AdvanceAnimation(animation_player *AnimPlayer)
{
	animation Animation = AnimPlayer->Animations[AnimPlayer->SelectedAnim + AnimPlayer->Offset];
	loaded_bitmap Result = Animation.Frames[Animation.Counter % Animation.MaxFrames];
	return Result;
}

internal loaded_bitmap GetAnimationBitmap(animation_player *AnimPlayer)
{
	
	if (AnimPlayer->Iter > AnimPlayer->MaxIter[AnimPlayer->SelectedAnim])
	{
		SetAnimation(AnimPlayer, !AnimPlayer->SelectedAnim);
	}
	
	return AdvanceAnimation(AnimPlayer); 
}
//////////////

//NOTE: Should we push load and save game into different files?

internal void SaveGame(debug_platform_write_entire_file *WriteEntireFile, game_state *GameState, char *FileName)
{
    save_game SaveGame = {}; 
    
    // TODO(Noah): Fix this.
    //SaveGame.Player = *GameState->Player;
    //SaveGame.PlayerEntity = *GameState->Player; //derefence the entity to save
    
    WriteEntireFile(FileName, sizeof(save_game), &SaveGame);
}

internal void LoadSaveGame(debug_platform_read_entire_file *ReadEntireFile, game_state *GameState, char *FileName)
{
    save_game SaveGame = {};
    LoadRawFile(ReadEntireFile, FileName, &SaveGame);
    
    // TODO(Noah): Also fix this, please.
    //GameState->Player.MoveDirection = SaveGame.PlayerEntity.MoveDirection;
    //Assert(GameState->Player.Entity); //just in case we assert here!
    //*GameState->Player.Entity = SaveGame.PlayerEntity; //we assume we already have the entity pointer set. 
}

//////////////

internal void CloneBuffer(game_offscreen_buffer *source, game_offscreen_buffer *dest)
{
    memcpy(dest->memory, source->memory, source->width * source->height * source->BytesPerPixel);
    /*
    unsigned int *SourcePixel = (unsigned int *)source->memory;
    unsigned int *DestPixel = (unsigned int *)dest->memory; 
    unsigned int PixelCount = source->width * source->height;
    for (unsigned int x = 0; x < PixelCount; x++)
    {
        *DestPixel++ = *SourcePixel++;
    }
    */
}

//Hm?
internal void InstantiateShader(game_state *GameState, game_offscreen_buffer *source, loaded_bitmap LUT,
                                float Length)
{
    shader Shader = {}; Shader.source = source; Shader.LUT = LUT; Shader.Active = true;
    Shader.Length = Length;
    GameState->Shader = Shader;
}

internal void PapyrusFunction(void *Data, unsigned int Param)
{
    game_state *GameState = (game_state *)Data;
    
    
}

internal void AllocBackBufferIntoGameState(game_state *GameState)
{
    memory_arena *Arena = &GameState->WorldArena;
    GameState->BackBuffer.memory = AtomicPushSize(
        Arena, GameState->GameBuffer->width * GameState->GameBuffer->height * sizeof(unsigned int));
    GameState->BackBuffer.width = GameState->GameBuffer->width;
    GameState->BackBuffer.height = GameState->GameBuffer->height;
    GameState->BackBuffer.pitch = GameState->GameBuffer->pitch;
}

// NOTE: It's alright to have the arceus event coded like this. However I think its important that we restructure 
// how posponed functions work. I think we need to remodel the Param as a generic linked list.
internal void ArceusEvent(void *Data, unsigned int Param)
{
    game_state *GameState = (game_state *)Data;
    SeedRandom(GameState);
    PlaySoundEffect(GameState, GameState->SoundEffects[7], true, 1.0f); //start playing the areus theme
    AllocBackBufferIntoGameState(GameState);
    CloneBuffer(GameState->GameBuffer, &GameState->BackBuffer);
    InstantiateShader(GameState, &GameState->BackBuffer, GameState->LUT[0], 1.5f);
    SetPauseState(GameState, 2.0f);
    
    GameState->GameState = BEGINBATTLE;
    
    //generate the arceus
    CloneString("Arceus", GameState->BufferPokemon[0].Nickname, 10);
    GameState->BufferPokemon[0].Experience = 0;
    UpdatePokemonLevel(&GameState->BufferPokemon[0], 512000);
    GameState->BufferPokemon[0].PokemonID = 493;
    GameState->BufferPokemon[0].Ability = OVERGROW;
    GameState->BufferPokemon[0].Moves[0] = FillPokemonMove(GameState->MoveDatabase, JUDGMENT);
    GameState->BufferPokemon[0].Moves[1] = FillPokemonMove(GameState->MoveDatabase, PSYCHIC_MOVE);
    GameState->BufferPokemon[0].Moves[2] = FillPokemonMove(GameState->MoveDatabase, ICEBEAM);
    GameState->BufferPokemon[0].Moves[3] = FillPokemonMove(GameState->MoveDatabase, RECOVER);
    GameState->BufferPokemon[0].Gender = 1;
    GameState->BufferPokemon[0].HP = GetPokemonStatHP(GameState, &GameState->BufferPokemon[0]);
    
    GameState->BufferPokemon[0].overworldEntity = &GameState->AllEntities[3];
}

//NOTE: This function is very specialized. We need to remove this into some other file.
internal void BlackOut(void *Data, unsigned int Param)
{
    game_state *GameState = (game_state *)Data;
    
    // Heal all the pokemon in the party
    for (unsigned int x = 0; x < 6; x++)
    {
        pokemon *Pokemon = &GameState->PokemonP[x];
        Pokemon->HP = GetPokemonStatHP(GameState, Pokemon);
        for (unsigned int x = 0; x < 4; x++)
        {
            pokemon_move *PokemonMove = &Pokemon->Moves[x];
            PokemonMove->PP = PokemonMove->MaxPP;
        }
    }
    
    
    PlaySoundEffect(GameState, GameState->SoundEffects[2], false, 1.0f);
    SetPauseState(GameState, 2.0f);
    
    // move player back to the origin
    GameState->Player->Entity->TileMapPos.AbsTileX = 0;
    GameState->Player->Entity->TileMapPos.AbsTileY = 0;
    GameState->Player->Entity->TileMapPos.X = 0.7f;
    GameState->Player->Entity->TileMapPos.Y = 0.7f;
    GameState->Player->MoveDirection = UP;

    // Move the camera with the player
    GameState->CameraPos.X = 0;
    GameState->CameraPos.Y = 0;
    
    GameState->GameState = ENTERINGAREA;
}

// TODO: I put this here because I was lazy!
char *PapyrusMessageChain[7]; 

#ifdef __cplusplus
extern "C"
#endif
GAME_UPDATE_RENDER(GameUpdateRender)
{
    char StringBuffer[256];
    game_user_gamepad_input *Controller = &Input->GamepadInput[0]; //get controller
    game_user_gamepad_input *Keyboard = &Input->GamepadInput[1]; //get keyboard
    game_state *GameState = (game_state *)Memory->Storage;
    
    GameState->GameBuffer = buffer;
    GameState->LastState = GameState->GameState;
    
    if (Memory->Valid)
    {	
        if (!Memory->IsInitialized)
        {
            // set the global color mode
            GLOBAL_COLOR_MODE = gameColorMode;
            //initialize the random generator
            SeedRandom(GameState); 
            //set the game state
            GameState->GameState = OPENING; 
            
            //load in the pokemon database.
            loaded_asset Asset = LoadAsset(Memory->DEBUGPlatformReadEntireFile, Input->BaseFilePath, "Data//PokemonDatabase.dat");
            ParseAsset(Asset, GameState->PokemonDatabase, pokemon_database_data);
            
            // LOAD IN ALL THE POKEMON CRIES
            LoadCries(GameState, Memory->DEBUGPlatformReadEntireFile, Input->BaseFilePath);
            
            //initialize the nature database
            //NatureDatabase[25][6]
            GameState->NatureDatabase[0][0] = 1.0f; GameState->NatureDatabase[0][1] = 1.0f; GameState->NatureDatabase[0][2] = 1.0f;
            GameState->NatureDatabase[0][3] = 1.0f; GameState->NatureDatabase[0][4] = 1.0f; GameState->NatureDatabase[0][5] = 1.0f;
            
            //load in the UserInterfaces
            loaded_asset UserInterfaceAsset = LoadAsset(Memory->DEBUGPlatformReadEntireFile, Input->BaseFilePath, "Data//UserInterfaces.dat");
            ParseAsset(UserInterfaceAsset, GameState->UserInterfaces, game_ui_scene); 
            
            //load in the music
            GameState->Music = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//Battle.wav", StringBuffer));
            GameState->SoundEffects[0] = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//LevelUp.wav", StringBuffer));
            GameState->SoundEffects[1] = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//AdvanceMessage.wav", StringBuffer));
            GameState->SoundEffects[2] = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//Heal.wav", StringBuffer));
            GameState->SoundEffects[3] = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//Pikachu.wav", StringBuffer));
            GameState->SoundEffects[4] = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//PokemonLeagueDay.wav", StringBuffer));
            GameState->SoundEffects[5] = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//Thud.wav", StringBuffer));
            GameState->SoundEffects[6] = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//ArceusCry.wav", StringBuffer));
            GameState->SoundEffects[7] = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//ArceusTheme.wav", StringBuffer));
            GameState->SoundEffects[8] = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//Victory.wav", StringBuffer));
            GameState->SoundEffects[9] = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//BuildExp.wav", StringBuffer));
            
            //So we initialize the arena by sending a pointer to the arena, saying where in memory to begin allocating, 
            //then we pass how much memory for the arena. Very trivial.
            InitializeArena(&GameState->WorldArena, ((unsigned char *)Memory->Storage) + sizeof(game_state),
                            Memory->StorageSize - sizeof(game_state)  ) ;
            
            //load in the type matchup database
            char *Lines[256];
            unsigned int LineAmount = ReadLines(Memory->DEBUGPlatformReadEntireFile, &GameState->WorldArena, Input->BaseFilePath, "Data//TypeMatchupDatabase.txt", Lines);
            LoadIntegerMatrix(&GameState->TypeMatchupDatabase[0][0], Lines, LineAmount);
            
            //load in LUTS for shader
            GameState->LUT[0] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//LUT1.bmp", StringBuffer));
            GameState->LUT[1] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//LUT2.bmp", StringBuffer));
            GameState->LUT[2] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//LUT3.bmp", StringBuffer));
            GameState->LUT[3] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//LUT4.bmp", StringBuffer));
            
            //load in bitmaps for UI
            GameState->Bitmaps[0] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//AttackBackground.bmp", StringBuffer));
            GameState->Bitmaps[1] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//EnemyHealth.bmp", StringBuffer));
            GameState->Bitmaps[2] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//LargeDialouge.bmp", StringBuffer));
            
            //load message box bitmap
            //GameState->MessageBox = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data\\MessageBox.bmp", StringBuffer));
            
            //WRITE REQUIRED DATABASE FILES
            //DEBUGCreatePokemonAnimations(&GameState->WorldArena, Memory->DEBUGPlatformWriteEntireFile, Input->BaseFilePath);
            DEBUGCreateMoveDataBase(&GameState->WorldArena, Memory->DEBUGPlatformWriteEntireFile, 
                                    GameCatStrings(Input->BaseFilePath, "Data//MoveDatabase.dat", StringBuffer));
            ///////////////////////
            
            //Load font
            GenerateByTiles(&GameState->WorldArena, DEBUGLoadBMP( Memory->DEBUGPlatformReadEntireFile,
                                                                 GameCatStrings(Input->BaseFilePath, "Data//PokemonDemoFont.bmp", StringBuffer)), 16, 16, GameState->PokemonDemoFont);
            
            //load player sprites and initialize player
            GameState->SpriteMap = DEBUGLoadBMP( Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//PlatinumSpriteSheet.bmp", StringBuffer));
            
            // NOTE: In later code player 2 is the first entity in the array. This is odd, sure, but it is simply the state of things, so deal with it.
            // create player two
            {
                entity_npc *npc = CreateNPC(GameState);
                
                npc->Entity->AnimationPlayer = LoadNPC(&GameState->WorldArena, &GameState->SpriteMap, 1, 4);
                
                CloneString("Hi! I'm player two. Connect a controller to move me around!", npc->Entity->Message, 256); 
                
                npc->Entity->TileMapPos.AbsTileX = 25; npc->Entity->TileMapPos.X = 0.7f;
                npc->Entity->TileMapPos.AbsTileY = 8;
            }
            
            // Make the primary player
            { 
                GameState->Player = CreateNPC(GameState);
                //GameState->LastPlayer = CreateNPC(GameState);
                
                // Set up custom last player since it cannot be a standard entity.
                GameState->LastPlayer.npc = &GameState->_lp_npc;
                GameState->_lp_npc.Entity = &GameState->LastPlayer;
                
                //LoadSaveGame(Memory->DEBUGPlatformReadEntireFile, GameState, GameCatStrings(Input->BaseFilePath, "Data\\SaveData.sav", StringBuffer));
                
                GameState->Player->Entity->AnimationPlayer = LoadNPC(&GameState->WorldArena, &GameState->SpriteMap, 1, 6);
                
                CloneString("You found the first easter egg! Try to find the other...if you can!", GameState->Player->Entity->Message, 256); 
            }
            
            
            //create mom.
            {
                entity_npc *mom_npc = CreateNPC(GameState);
                
                mom_npc->Entity->AnimationPlayer = LoadNPC(&GameState->WorldArena, &GameState->SpriteMap, 4, 12);
                CloneString("Honey, let me heal your pokemon for you!", mom_npc->Entity->Message, 256); 
                mom_npc->Entity->TileMapPos.AbsTileX = 7; mom_npc->Entity->TileMapPos.X = 0.7f;
                mom_npc->Entity->TileMapPos.AbsTileY = 1; mom_npc->Entity->TileMapPos.Y = 0.7f;
                game_posponed_function MomFunction = {}; MomFunction.Data = GameState; MomFunction.Function = HealPokemon;
                mom_npc->Entity->Function = MomFunction;
            }
            
            //create arceus NPC
            {
                entity_npc *arceus_npc = CreateNPC(GameState);
                
                CloneString("Dodogyuuun!", arceus_npc->Entity->Message, 256); 
                arceus_npc->Entity->TileMapPos.AbsTileX = 24; arceus_npc->Entity->TileMapPos.X = 0.7f;
                arceus_npc->Entity->TileMapPos.AbsTileY = 56; arceus_npc->Entity->TileMapPos.Y = 0.7f;
                //default arceus to facing down
                arceus_npc->MoveDirection = DOWN; 
                game_posponed_function ArceusFunction = {}; ArceusFunction.Data = GameState; ArceusFunction.Function = ArceusEvent;
                arceus_npc->Entity->Function = ArceusFunction;
                
                //set arceus frames
                arceus_npc->Entity->AnimationPlayer.Animations[0].Frames[UP * 4] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, 
                                                                                                GameCatStrings(Input->BaseFilePath, "Data//ArceusOverworldUp.bmp", StringBuffer));
                arceus_npc->Entity->AnimationPlayer.Animations[0].Frames[DOWN * 4] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, 
                                                                                                  GameCatStrings(Input->BaseFilePath, "Data//ArceusOverworldDown.bmp", StringBuffer)) ;
                arceus_npc->Entity->AnimationPlayer.Animations[0].Frames[LEFT * 4] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, 
                                                                                                  GameCatStrings(Input->BaseFilePath, "Data//ArceusOverworldLeft.bmp", StringBuffer)) ;
                arceus_npc->Entity->AnimationPlayer.Animations[0].Frames[RIGHT * 4] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, 
                                                                                                   GameCatStrings(Input->BaseFilePath, "Data//ArceusOverworldRight.bmp", StringBuffer)) ;
            }
            
            //create the sign entity
            {
                entity *Sign = CreateEntity(GameState);
                Sign->TileMapPos.AbsTileX = 1; Sign->TileMapPos.AbsTileY = 8;
                Sign->TileMapPos.X = 0.7f; Sign->TileMapPos.Y = 0.7f;
                CloneString("It appears that you do not know how to read.", Sign->Message, 256);
            }
            
            //create the first door
            {
                entity *Door = CreateEntity(GameState);
                Door->TileMapPos.AbsTileX = 3; Door->TileMapPos.AbsTileY = 8;
                Door->TileMapPos.X = 0.7f; Door->TileMapPos.Y = 0.7f;
                CloneString("The door is locked from the inside.", Door->Message, 256);
            }
            
            //create the pokeball
            {
                entity *Pokeball = CreateEntity(GameState);
                Pokeball->TileMapPos.AbsTileX = 19; Pokeball->TileMapPos.AbsTileY = 29;
                Pokeball->TileMapPos.X = 0.7f; Pokeball->TileMapPos.Y = 0.7f;
                CloneString("The pokeball is empty.", Pokeball->Message, 256);
            }
            
            //create the second door
            {
                entity *Door2 = CreateEntity(GameState);
                Door2->TileMapPos.AbsTileX = 19; Door2->TileMapPos.AbsTileY = 40;
                Door2->TileMapPos.X = 0.7f; Door2->TileMapPos.Y = 0.7f;
                CloneString("Professor Maple's house!", Door2->Message, 256);
            }
            
            // create the NPC that is hiding (second easter egg)
            {
                entity_npc *egg = CreateNPC(GameState);
                
                egg->Entity->AnimationPlayer = LoadNPC(&GameState->WorldArena, &GameState->SpriteMap, 0, 0);
                
                // BIG Message chain for my boy papyrus...
                
                PapyrusMessageChain[0] = "...";
                PapyrusMessageChain[1] = "I...";
                PapyrusMessageChain[2] = "I'VE BEEN FOUND!";
                PapyrusMessageChain[3] = "...";
                PapyrusMessageChain[4] = "NO...";
                PapyrusMessageChain[5] = "IT IS I WHO HAS FOUND YOU! HA!";
                PapyrusMessageChain[6] = "...";
                
                egg->Entity->MessageChain = PapyrusMessageChain;
                egg->Entity->MessageFlags = CHAIN ;
                egg->Entity->MessageChainCount = 7;
                
                game_posponed_function _PapyrusFunction;
                _PapyrusFunction.Function = PapyrusFunction;
                _PapyrusFunction.Data = GameState;
                _PapyrusFunction.Param = 0;
                
                egg->Entity->Function = _PapyrusFunction;
                
                egg->Entity->TileMapPos.AbsTileX = 3; egg->Entity->TileMapPos.X = 0.7f;
                egg->Entity->TileMapPos.AbsTileY = 10; egg->Entity->TileMapPos.Y = 0.7f;
                
            }
            
            
            //LOAD GENDERS
            UnPackBitmapTiles(&GameState->WorldArena, DEBUGLoadBMP( Memory->DEBUGPlatformReadEntireFile,
                                                                   GameCatStrings(Input->BaseFilePath, "Data//Genders.bmp", StringBuffer)), 6, 9, 0, GameState->Genders);
            
            //LOAD ATTACk BOXES
            UnPackBitmapTiles(&GameState->WorldArena, DEBUGLoadBMP( Memory->DEBUGPlatformReadEntireFile,
                                                                   GameCatStrings(Input->BaseFilePath, "Data//AttackBoxes.bmp", StringBuffer)), 124, 55, 0, GameState->AttackBoxes);
            //LOAD TILES
            UnPackBitmapTiles(&GameState->WorldArena, DEBUGLoadBMP( Memory->DEBUGPlatformReadEntireFile,
                                                                   GameCatStrings(Input->BaseFilePath, "Data//PikaBlueTileSet.bmp", StringBuffer)), 16, 16, 1, GameState->TileSet);
            //////////////////////
            
            GameState->Hit2 = DEBUGLoadWav(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//Hit2.wav", StringBuffer));
            // The spritemap I use for the battle backgrounds are thanks to Proffesor Valley who ripped them. 
            GameState->BattleBackground = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//GrassBattle.bmp", StringBuffer));
            //The HP bars and other essential battle UI were ripped by Haru Raindose so big thanks to him.
            GameState->EnemyHealth = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//EnemyHealth.bmp", StringBuffer));
            GameState->PlayerHealth = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//PlayerHealth.bmp", StringBuffer));
            GameState->HealthLUT = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//HealthLUT.bmp", StringBuffer));
            GameState->DEBUGAttackBackground = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, 
                                                            GameCatStrings(Input->BaseFilePath, "Data//AttackBackground.bmp", StringBuffer));
            GameState->MoveSelector = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//MoveSelector.bmp", StringBuffer));
            GameState->BattleButton = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//BattleButton.bmp", StringBuffer));
            
            //push struct function will actually get the memory out. So we get what we want essentially.
            //In fact, in programming we always get what we want.
            GameState->World = PushStruct(&GameState->WorldArena, world);
            world *World = GameState->World;
            World->TileMap = PushStruct(&GameState->WorldArena, tile_map);
            
            tile_map *TileMap = World->TileMap;
            TileMap->ChunkShift = 4;
            TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;
            TileMap->ChunkSize = (1 << TileMap->ChunkShift);
            
            TileMap->WorldCountY = 128;
            TileMap->WorldCountX = 128;
            TileMap->WorldCountZ = 2;
            
            TileMap->TileChunks = PushArray(&GameState->WorldArena,
                                            TileMap->WorldCountX * TileMap->WorldCountY * TileMap->WorldCountZ, tile_chunk);
            
            // below is the code to load in the world save (which has all the chunks in it)
            LoadRawFile(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data//World3.world", StringBuffer), &GameState->LoadedWorld);
            for (unsigned int x = 0; x < GameState->LoadedWorld.ChunkCount; x++)
            {
                SetChunk(TileMap, GameState->LoadedWorld.ChunkPos[x].x, GameState->LoadedWorld.ChunkPos[x].y, 
                         GameState->LoadedWorld.ChunkPos[x].z, &GameState->LoadedWorld.Chunks[x][0][0]);
            }
            
            LoadPokemonDatabase(GameState, Memory->DEBUGPlatformReadEntireFile, 
                                GameCatStrings(Input->BaseFilePath, "Data//MoveDatabase.dat", StringBuffer));
            
            //below is the code to initialize the demo pokemon
            loaded_asset PartyPokemonAsset = LoadAsset(Memory->DEBUGPlatformReadEntireFile, Input->BaseFilePath, "Data//Party.dat");
            ParseAsset(PartyPokemonAsset, GameState->PokemonP, pokemon); 
            
            //initialze the continue interface
            CloneString("CONTINUE", GameState->UserInterfaces[1].Elements[1].String, 256);
            CloneString("PLAYER", GameState->UserInterfaces[1].Elements[2].String, 256);
            CloneString("Red", GameState->UserInterfaces[1].Elements[3].String, 256);
            CloneString("BADGES", GameState->UserInterfaces[1].Elements[4].String, 256);
            CloneString("8", GameState->UserInterfaces[1].Elements[5].String, 256);
            CloneString("POKEDEX", GameState->UserInterfaces[1].Elements[6].String, 256);
            CloneString("151", GameState->UserInterfaces[1].Elements[7].String, 256);
            
            Memory->IsInitialized = true;
        } // done memory initialization
        
        
        world *World = GameState->World;
        float CenterX = buffer->width / 2.0f;
        float CenterY = buffer->height / 2.0f;
        
        // toggle fullscreen functionality.
        if(Keyboard->DebugButtons[0].EndedDown && Keyboard->DebugButtons[0].HalfTransitionCount > 0)
        {
            toggle_fullscreen_callback * ToggleFullscreen = Memory->ToggleFullscreen;
            ToggleFullscreen();
            //SaveGame(Memory->DEBUGPlatformWriteEntireFile, GameState, GameCatStrings(Input->BaseFilePath, "Data//SaveData.sav", StringBuffer));
        }
        
        GameState->GameTimer += Input->DeltaTime;
        if (GameState->StateFlags & PAUSE)
        {   
            if (GameState->GameTimer - GameState->InitialTimer > GameState->PauseLength)
            {
                GameState->StateFlags -= PAUSE; //take away pause state
            }
        }
        else // game is not paused
        {
            DrawRect(buffer, 0.0f, (float)buffer->width, 0.0f,(float)buffer->height, 0.0f, 0.0f, 0.0f);
            switch(GameState->GameState)
            {	
                case OPENING:
                {
                    GameState->GameState = MAINMENU;
                }break;
                case MAINMENU:
                {
                    // draw blue backdrop
                    DrawRect(buffer, 0.0f, (float)buffer->width, 0.0f, (float)buffer->height, 0.388f, 0.816f, 0.949f);
                    GameState->UserInterfaces[1].Active = true;
                    
                    //the user pressed z
                    if( Keyboard->DebugButtons[1].EndedDown || Controller->DebugButtons[1].EndedDown )
                    {
                        PlaySoundEffect(GameState, GameState->SoundEffects[1], false, 1.0f);
                        PlaySoundEffect(GameState, GameState->SoundEffects[3], false, 1.0f);
                        SetPauseState(GameState, 1.2f);
                        GameState->UserInterfaces[1].Active = false;
                        DrawRect(buffer, 0.0f, (float)buffer->width, 0.0f, (float)buffer->height, 0.0f, 0.0f, 0.0f);
                        GameState->GameState = ENTERINGAREA;
                    }
                }break;
                case ENTERINGAREA:
                {
                    //play the cool area animation.
                    //start playing the awesome music.
                    PlaySoundEffect(GameState, GameState->SoundEffects[4], true, 0.1f);
                    SetPauseState(GameState, 1.2f);
                    // clear csreen to black
                    DrawRect(buffer, 0.0f, (float)buffer->width, 0.0f, (float)buffer->height, 0.0f, 0.0f, 0.0f);
                    GameState->GameState = OVERWORLD;
                }break;
                case BEGINBATTLE:
                {	
                    GameState->PokemonB1 = BattlePokemonFromPokemon(GameState, &GameState->PokemonP[0]);
                    GameState->PokemonB2 = BattlePokemonFromPokemon(GameState, &GameState->BufferPokemon[0]);
                    
                    //initialize the animation frames! Nice!
                    
                    // TODO: Extended to infinity, this code will likely crash the game as the memory arena is going to be full of animation data.
                    LoadPokemonBattleAnimation(Memory->DEBUGPlatformReadEntireFile, &GameState->WorldArena, Input->BaseFilePath, 
                                               &GameState->PokemonA1, GetPokemonDataFromID(GameState->PokemonDatabase, GameState->PokemonB1.Pokemon->PokemonID), 
                                               true);
                    
                    LoadPokemonBattleAnimation(Memory->DEBUGPlatformReadEntireFile, &GameState->WorldArena, Input->BaseFilePath, 
                                               &GameState->PokemonA2, GetPokemonDataFromID(GameState->PokemonDatabase, GameState->PokemonB2.Pokemon->PokemonID), 
                                               false);
                    
                    //below is the code to blit the intial battle message
                    float ExtensionHeight = GameState->DEBUGAttackBackground.Height * 2.0f;
                    GameCatStrings("Go! ", GameState->PokemonB1.Pokemon->Nickname, StringBuffer);
                    CreateNewMessage(GameState, CenterX, CenterY + GameState->BattleBackground.Height - ExtensionHeight / 2.0f,
                                     GameCatStrings(StringBuffer, "!", StringBuffer), 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
                    
                    GameState->GameState = BATTLE;
                    GameState->StateFlags = ATTACKMENU;
                }break;
                case BATTLE:
                {
                    GameState->GainingExp = false;
                    
                    DrawRect(buffer, 0.0f, (float)buffer->width, 0.0f,(float)buffer->height, 0.2f, 0.2f, 0.2f);
                    //DRAW BACKGROUND
                    float CanvasOffset = -1.0f * GameState->DEBUGAttackBackground.Height;
                    float CanvasMinX = CenterX - GameState->BattleBackground.Width;
                    float CanvasMaxX = CenterX + GameState->BattleBackground.Width;
                    float CanvasMinY = CenterY - GameState->BattleBackground.Height + CanvasOffset;
                    float CanvasMaxY = CenterY + GameState->BattleBackground.Height + CanvasOffset;
                    DrawBitmapScaled(buffer, GameState->BattleBackground, CanvasMinX, CanvasMaxX, CanvasMinY, CanvasMaxY, FLIPFALSE, 2);
                    
                    //DRAW EXT SHADOW
                    float ExtensionHeight = GameState->DEBUGAttackBackground.Height * 2.0f;
                    float OutlineWidth = 2.0f;
                    float MessageShadowMinX = CanvasMinX;
                    float MessageShadowMaxX = CanvasMaxX;
                    float MessageShadowMinY = CanvasMaxY;
                    float MessageShadowMaxY = CanvasMaxY + ExtensionHeight + OutlineWidth;
                    DrawRect(buffer, MessageShadowMinX, MessageShadowMaxX, MessageShadowMinY, MessageShadowMaxY, 0.0f, 0.0f, 0.0f);
                    
                    if (!(GameState->StateFlags & BATTLEWON))
                    { 
                        //DRAW ENEMY
                        int EnemyScale = 2;
                        float EnemyCenterX = CanvasMaxX - 136.0f;
                        float EnemyBottomY = CanvasMinY + 178.0f;
                        loaded_bitmap Enemy = GetAnimationBitmap(&GameState->PokemonA2);
                        float EnemyRadius = Enemy.Width * EnemyScale / 2.0f;
                        DrawBitmapScaled(buffer, Enemy, EnemyCenterX - EnemyRadius, EnemyCenterX + EnemyRadius,
                                         EnemyBottomY - Enemy.Height * EnemyScale, EnemyBottomY, FLIPFALSE, EnemyScale);
                    }
                    
                    if (!(GameState->StateFlags & BATTLELOST))
                    { 
                        //DRAW PLAYER
                        int PlayerScale = 4;
                        float PlayerCenterX = CanvasMinX + 132.0f;
                        float PlayerBottomY = CanvasMaxY + 20.0f;
                        loaded_bitmap Player = GetAnimationBitmap(&GameState->PokemonA1);
                        float PlayerRadius = Player.Width * PlayerScale / 2.0f;
                        DrawBitmapScaled(buffer, Player, PlayerCenterX - PlayerRadius, PlayerCenterX + PlayerRadius, 
                                         PlayerBottomY - Player.Height * PlayerScale, CanvasMaxY, FLIPFALSE, PlayerScale);
                    }
                    
                    //DRAW ENEMY HEALTH
                    float HealthOneMinX = CanvasMinX + 2.0f;
                    float HealthOneMaxX = HealthOneMinX + 2.0f * GameState->EnemyHealth.Width;
                    float HealthOneMinY = CanvasMinY + 40.0f;
                    float HealthOneMaxY = HealthOneMinY + 2.0f * GameState->EnemyHealth.Height;
                    DrawBitmapScaled(buffer, GameState->EnemyHealth, HealthOneMinX, HealthOneMaxX, HealthOneMinY, HealthOneMaxY, FLIPFALSE, 2);
                    
                    //DRAW PLAYER HEALTH
                    float HealthTwoMaxX = CanvasMaxX - 2.0f;
                    float HealthTwoMaxY = CanvasMaxY - 14.0f;
                    float HealthTwoMinX = HealthTwoMaxX - 2.0f * GameState->PlayerHealth.Width;
                    float HealthTwoMinY = HealthTwoMaxY - 2.0f * GameState->PlayerHealth.Height;
                    DrawBitmapScaled(buffer, GameState->PlayerHealth, HealthTwoMinX, HealthTwoMaxX, HealthTwoMinY, HealthTwoMaxY, FLIPFALSE, 2);
                    
                    //DRAW ATTACK BACKGROUND
                    float AttackBackWidth = GameState->DEBUGAttackBackground.Width * 2.0f;
                    float AttackBackHeight = GameState->DEBUGAttackBackground.Height * 2.0f;
                    
                    float AttackBackMinX = CanvasMinX;
                    float AttackBackMaxX = CanvasMinX + AttackBackWidth;
                    float AttackBackMinY = CanvasMaxY;
                    float AttackBackMaxY = AttackBackMinY + AttackBackHeight;
                    DrawBitmapScaled(buffer, GameState->DEBUGAttackBackground, AttackBackMinX, AttackBackMaxX, AttackBackMinY, AttackBackMaxY, FLIPFALSE, 2);
                    
                    //the regular pokemon UI call is referencing the players UI
                    UpdatePokemonUI(buffer, GameState, &GameState->PokemonB1, HealthTwoMinX, HealthTwoMaxX, HealthTwoMinY, HealthTwoMaxY);
                    //the secondary UI is the enemy UI
                    UpdatePokemonUI2(buffer, GameState, &GameState->PokemonB2, HealthOneMinX, HealthOneMaxX, HealthOneMinY, HealthOneMaxY);
                    
                    if ( (GameState->MessageQueue.Count > 0) | (GameState->StateFlags & DIM) )
                    {
                        //so here we do nothing because we are blitting a message and because we are dimming
                        if(Keyboard->DebugButtons[1].EndedDown && Keyboard->DebugButtons[1].HalfTransitionCount > 0)
                        {
                            if (GameState->StateFlags & LEVELING)
                            {
                                goto Leveling;
                            }
                        }
                    }
                    else if (GameState->StateFlags & LEVELING)
                    {
                        //NOTE: the currently leveling pokemon will always be in the first index of the pokemon back buffer
                        //we need the back buffer because we know we are leveling but we don't know who is leveling.
                        //although I can argue that we will always be leveling Battle Pokemon 1. So lets fix that
                        char LastLevel = GameState->PokemonBackBuffer[0]->Pokemon->Level;
                        float LastExperience = GameState->FloatBuffer[0];
                        SafeSubtract(GameState->FloatBuffer[0], GameState->FloatBuffer[2]); 
                        float DeltaExperience = LastExperience - GameState->FloatBuffer[0];
                        
                        DeltaExperience = DeltaExperience * 8;
                        
                        GameState->GainingExp = true;
                        
                        if (!GameState->UserInterfaces[0].Active)
                        {
                            if (DeltaExperience != 0.0f) 
                            {
                                pokemon_database_data PokemonData = GetPokemonDataFromID(GameState->PokemonDatabase, 
                                                                                         GameState->PokemonBackBuffer[0]->Pokemon->PokemonID);
                                unsigned int LastHealth = CalcPokemonStatHP(PokemonData.BaseStats[HP], 
                                                                            GameState->PokemonBackBuffer[0]->Pokemon->IV[HP],
                                                                            GameState->PokemonBackBuffer[0]->Pokemon->EV[HP], 
                                                                            GameState->PokemonBackBuffer[0]->Pokemon->Level);
                                
                                UpdatePokemonLevel(GameState->PokemonBackBuffer[0]->Pokemon, (unsigned int)DeltaExperience);
                                if (LastLevel != GameState->PokemonBackBuffer[0]->Pokemon->Level)
                                {
                                    GameState->CurrentSineIndex = 0;
                                    battle_pokemon LastBattlePokemon = *GameState->PokemonBackBuffer[0];
                                    CanonocalizeBattlePokemonStats(GameState, GameState->PokemonBackBuffer[0]);
                                    
                                    unsigned int CurrentHealth = CalcPokemonStatHP(PokemonData.BaseStats[HP], 
                                                                                   GameState->PokemonBackBuffer[0]->Pokemon->IV[HP],
                                                                                   GameState->PokemonBackBuffer[0]->Pokemon->EV[HP], 
                                                                                   GameState->PokemonBackBuffer[0]->Pokemon->Level);
                                    unsigned int DeltaHealth = CurrentHealth - LastHealth; 
                                    unsigned int DeltaAttack = GameState->PokemonBackBuffer[0]->Attack - LastBattlePokemon.Attack;
                                    unsigned int DeltaDefense = GameState->PokemonBackBuffer[0]->Defense - LastBattlePokemon.Defense;
                                    unsigned int DeltaSpAttack = GameState->PokemonBackBuffer[0]->SpAttack - LastBattlePokemon.SpAttack;
                                    unsigned int DeltaSpDefense = GameState->PokemonBackBuffer[0]->SpDefense - LastBattlePokemon.SpDefense;
                                    unsigned int DeltaSpeed = GameState->PokemonBackBuffer[0]->Speed - LastBattlePokemon.Speed;
                                    
                                    CreateNewMessage(GameState, CenterX, CanvasMaxY,
                                                     GameCatStrings(GameState->PokemonBackBuffer[0]->Pokemon->Nickname," leveled up!", StringBuffer), 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
                                    
                                    PlaySoundEffect(GameState, GameState->SoundEffects[0], false, 1.0f);
                                    
                                    GameState->UserInterfaces[0].Active = true;
                                    
                                    //draw the descriptors of the UI 
                                    CloneString("Max HP", GameState->UserInterfaces[0].Elements[1].String, 256);
                                    CloneString("Attack", GameState->UserInterfaces[0].Elements[2].String, 256);
                                    CloneString("Defense", GameState->UserInterfaces[0].Elements[3].String, 256);
                                    CloneString("Sp.Attack", GameState->UserInterfaces[0].Elements[4].String, 256);
                                    CloneString("Sp.Defense", GameState->UserInterfaces[0].Elements[5].String, 256);
                                    CloneString("Speed", GameState->UserInterfaces[0].Elements[6].String, 256);
                                    
                                    //draw the deltas
                                    CloneString(NumberToASCII(DeltaHealth, StringBuffer), GameState->UserInterfaces[0].Elements[7].String, 256);
                                    CloneString(NumberToASCII(DeltaAttack, StringBuffer), GameState->UserInterfaces[0].Elements[8].String, 256);
                                    CloneString(NumberToASCII(DeltaDefense, StringBuffer), GameState->UserInterfaces[0].Elements[9].String, 256);
                                    CloneString(NumberToASCII(DeltaSpAttack, StringBuffer), GameState->UserInterfaces[0].Elements[10].String, 256);
                                    CloneString(NumberToASCII(DeltaSpDefense, StringBuffer), GameState->UserInterfaces[0].Elements[11].String, 256);
                                    CloneString(NumberToASCII(DeltaSpeed, StringBuffer), GameState->UserInterfaces[0].Elements[12].String, 256); 
                                }
                            }
                            else
                            {
                                GameState->CurrentSineIndex = 0;
                                GameState->StateFlags -= LEVELING;
                            }
                        }
                        else
                        {
                            //here we want to test for them clicking z, then we can blit the next message to the thing, and test for z again
                            if(Keyboard->DebugButtons[1].EndedDown && Keyboard->DebugButtons[1].HalfTransitionCount > 0)
                            {
                                Leveling:
                                if (GameState->FloatBuffer[1] != 1.0f)
                                {
                                    pokemon_database_data PokemonData = GetPokemonDataFromID(GameState->PokemonDatabase, 
                                                                                             GameState->PokemonBackBuffer[0]->Pokemon->PokemonID);
                                    unsigned int Health = CalcPokemonStatHP(PokemonData.BaseStats[HP], 
                                                                            GameState->PokemonBackBuffer[0]->Pokemon->IV[HP],
                                                                            GameState->PokemonBackBuffer[0]->Pokemon->EV[HP], 
                                                                            GameState->PokemonBackBuffer[0]->Pokemon->Level);
                                    
                                    CloneString(NumberToASCII(Health, StringBuffer),
                                                GameState->UserInterfaces[0].Elements[7].String, 256);
                                    CloneString(NumberToASCII(GameState->PokemonBackBuffer[0]->Attack, StringBuffer), 
                                                GameState->UserInterfaces[0].Elements[8].String, 256);
                                    CloneString(NumberToASCII(GameState->PokemonBackBuffer[0]->SpDefense, StringBuffer), 
                                                GameState->UserInterfaces[0].Elements[9].String, 256);
                                    CloneString(NumberToASCII(GameState->PokemonBackBuffer[0]->SpAttack, StringBuffer), 
                                                GameState->UserInterfaces[0].Elements[10].String, 256);
                                    CloneString(NumberToASCII(GameState->PokemonBackBuffer[0]->SpDefense, StringBuffer), 
                                                GameState->UserInterfaces[0].Elements[11].String, 256);
                                    CloneString(NumberToASCII(GameState->PokemonBackBuffer[0]->Speed, StringBuffer), 
                                                GameState->UserInterfaces[0].Elements[12].String, 256); 
                                    
                                    GameState->FloatBuffer[1] = 1.0f;
                                }
                                else
                                {
                                    GameState->FloatBuffer[1] = 0.0f;
                                    GameState->UserInterfaces[0].Active = false;
                                }
                            }
                        }
                    }
                    else if (GameState->StateFlags & MENU)
                    {
                        //just chilling
                    }
                    else if (GameState->StateFlags & BATTLEWON)
                    {
                        //PopSound(GameState, 0);
                        
                        // stop playing all music
                        while (!PopSound(GameState, 0));
                        
                        GameState->GameState = EXITBATTLE; 
                        
                        AllocBackBufferIntoGameState(GameState);
                        CloneBuffer(buffer, &GameState->BackBuffer);
                        InstantiateShader(GameState, &GameState->BackBuffer, GameState->LUT[2], 0.5f);
                        SetPauseState(GameState, 0.7f);
                    }
                    else if (GameState->StateFlags & BATTLELOST)
                    {
                        // Stop playing all music
                        // while (!PopSound(GameState, 0)); 
                        // PopSound(GameState, 0);
                        while (!PopSound(GameState, 0));
                        
                        GameState->GameState = BLACKEDOUT;
                        
                        AllocBackBufferIntoGameState(GameState);
                        CloneBuffer(buffer, &GameState->BackBuffer);
                        InstantiateShader(GameState, &GameState->BackBuffer, GameState->LUT[2], 0.3f);
                        SetPauseState(GameState, 0.5f);
                    }
                    else if (GameState->StateFlags & FAINT)
                    {
                        if (GameState->StateFlags & TURN2)
                        {
                            battle_pokemon Target = {};
                            battle_pokemon User = {};
                            if (GameState->StateFlags & USERFIRST)
                            {
                                Target = GameState->PokemonB2;
                                User = GameState->PokemonB1;
                                GameState->StateFlags = GameState->StateFlags | BATTLEWON | LEVELING;
                                GameState->PokemonBackBuffer[0] = &GameState->PokemonB1;
                            }
                            else
                            {
                                Target = GameState->PokemonB1;
                                User = GameState->PokemonB2;
                                GameState->StateFlags = GameState->StateFlags | BATTLELOST;
                            }
                            
                            game_screen_position ScreenPos = {}; ScreenPos.X = CenterX; ScreenPos.Y = CanvasMaxY;
                            OnPokemonFainted(GameState, ScreenPos, User, Target, GameState->StateFlags & BATTLEWON);
                        }
                        else
                        {
                            battle_pokemon Target = {};
                            battle_pokemon User = {};
                            if (GameState->StateFlags & USERFIRST)
                            {
                                Target = GameState->PokemonB1;
                                User = GameState->PokemonB2;
                                GameState->StateFlags = GameState->StateFlags | BATTLELOST;
                            }
                            else
                            {
                                Target = GameState->PokemonB2;
                                User = GameState->PokemonB1;
                                GameState->StateFlags = GameState->StateFlags | BATTLEWON | LEVELING;
                                GameState->PokemonBackBuffer[0] = &GameState->PokemonB1;
                            }
                            
                            game_screen_position ScreenPos = {}; ScreenPos.X = CenterX; ScreenPos.Y = CanvasMaxY;
                            OnPokemonFainted(GameState, ScreenPos, User, Target, GameState->StateFlags & BATTLEWON);
                        }
                    }
                    else if (GameState->StateFlags & TURN1)
                    {
                        battle_pokemon *Attacker = NULL;
                        battle_pokemon *Target = NULL;
                        
                        if (GameState->StateFlags & USERFIRST)
                        {
                            Attacker = &GameState->PokemonB1;
                            Target = &GameState->PokemonB2;	
                        }
                        else
                        {
                            Attacker = &GameState->PokemonB2;
                            Target = &GameState->PokemonB1;
                        }
                        
                        game_screen_position ScreenPos = {}; ScreenPos.X = CenterX; ScreenPos.Y = CanvasMaxY;
                        ExecuteTurn(GameState, Attacker, Target, ScreenPos);
                        
                        GameState->StateFlags = (GameState->StateFlags - TURN1) | TURN2;
                    }
                    else if (GameState->StateFlags & TURN2)
                    {
                        battle_pokemon *Attacker = NULL;
                        battle_pokemon *Target = NULL;
                        
                        if (GameState->StateFlags & USERFIRST)
                        {
                            Attacker = &GameState->PokemonB2;
                            Target = &GameState->PokemonB1;	
                        }
                        else
                        {
                            Attacker = &GameState->PokemonB1;
                            Target = &GameState->PokemonB2;
                        }
                        
                        game_screen_position ScreenPos = {}; ScreenPos.X = CenterX; ScreenPos.Y = CanvasMaxY;
                        ExecuteTurn(GameState, Attacker, Target, ScreenPos);
                        
                        GameState->StateFlags = GameState->StateFlags - TURN2;
                    }
                    else if (GameState->StateFlags & ATTACKMENU)
                    {
                        
                        if (Keyboard->Right.EndedDown && Keyboard->Right.HalfTransitionCount > 0)
                        {
                            GameState->MenuSelection++;
                            if (GameState->MenuSelection > 3)
                            {
                                GameState->MenuSelection = 3;
                            }
                        }
                        else if (Keyboard->Left.EndedDown && Keyboard->Left.HalfTransitionCount > 0)
                        {
                            if (GameState->MenuSelection)
                            {
                                GameState->MenuSelection--;
                            }
                        }
                        
                        //DRAW ATTACK BOXES
                        float AttackBoxWidth = 248.0f;
                        float AttackBoxHeight = 110.0f;
                        float AttackBoxOffsetY = 2.0f;
                        float AttackBoxOffsetX = 8.0f;
                        int SelectionOffset = (GameState->MenuSelection > 1)? 2: 0;
                        
                        float Attack1MinX = CanvasMinX + AttackBoxOffsetX;
                        float Attack1MaxX = Attack1MinX + AttackBoxWidth;
                        float Attack1MinY = CanvasMaxY + AttackBoxOffsetY;
                        float Attack1MaxY = Attack1MinY + AttackBoxHeight;
                        UpdatePokemonAttack(buffer, GameState, GameState->PokemonB1.Pokemon->Moves[SelectionOffset], Attack1MinX, Attack1MaxX, Attack1MinY, Attack1MaxY);
                        
                        float Attack2MinX = Attack1MaxX + 2;
                        float Attack2MaxX = Attack2MinX + AttackBoxWidth;
                        float Attack2MinY = CanvasMaxY + AttackBoxOffsetY;
                        float Attack2MaxY = Attack1MinY + AttackBoxHeight;
                        UpdatePokemonAttack(buffer, GameState, GameState->PokemonB1.Pokemon->Moves[SelectionOffset + 1], Attack2MinX, Attack2MaxX, Attack2MinY, Attack2MaxY);
                        ////////////////
                        
                        //DRAW ATTACK SELECTOR
                        int LocalSelection = GameState->MenuSelection % 2;
                        float SelectorMinX = (LocalSelection)? Attack2MinX: Attack1MinX;
                        float SelectorMaxX = (LocalSelection)? Attack2MaxX: Attack1MaxX;
                        float SelectorMinY = (LocalSelection)? Attack2MinY: Attack1MinY;
                        float SelectorMaxY = (LocalSelection)? Attack2MaxY: Attack1MaxY;
                        DrawBitmapScaled(buffer, GameState->MoveSelector, SelectorMinX, SelectorMaxX, SelectorMinY, SelectorMaxY, FLIPFALSE, 2);
                        //////////////////
                        
                        if(Keyboard->DebugButtons[1].EndedDown && Keyboard->DebugButtons[1].HalfTransitionCount > 0)
                        {
                            //Step 1 is to determine what move the opponent will use and to set the selected moves 
                            GameState->PokemonB1.SelectedMove = GameState->MenuSelection;
                            GameState->PokemonB2.SelectedMove = FloatToInt(GetRandom(GameState) / 225.0f * 3.0f); 
                            
                            //step 2 is to determine who goes first
                            GameState->StateFlags = GameState->StateFlags | DetermineUserFirst(GameState, GameState->PokemonB1, GameState->PokemonB2);
                            
                            if (GameState->PokemonB1.Pokemon->Moves[GameState->PokemonB1.SelectedMove].PP != 0)
                            {
                                GameState->StateFlags = GameState->StateFlags | TURN1;
                                PlaySoundEffect(GameState, GameState->SoundEffects[1], false, 1.0f);
                            }
                            else
                            {
                                CreateNewMessage(GameState, CenterX, CanvasMaxY,
                                                 "No PP LEFT! You're in trouble.", 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
                            }
                            
                        }
                    }
                }break;
                case EXITBATTLE:
                {
                    GameState->CurrentSineIndex = 0;
                    GameState->GameState = ENTERINGAREA;
                }break;
                case CHILLING:
                {
                    //we're just chilling fam
                }break;
                case BLACKEDOUT:
                {
                    CreateNewMessage(GameState, CenterX, CenterY, "Luckily, someone found you and brought you to the nearest pokemon center.", 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
                    game_posponed_function PosponedFunction = {}; PosponedFunction.Data = GameState; PosponedFunction.Function = BlackOut;
                    PosponeSuperFunction(GameState, 4.0f, PosponedFunction);
                    GameState->GameState = CHILLING;
                }break;
                case OVERWORLD:
                {
                    // Overworld background
                    DrawRect(buffer, 0.0f, (float)buffer->width, 0.0f,(float)buffer->height, 0.2f, 0.2f, 0.2f);
                    
                    tile_map *TileMap = World->TileMap;
                    
                    TileMap->TileSizeInMeters = 1.4f;
                    int TileSizeInPixels = 32;
                    float MetersToPixels = (float)TileSizeInPixels / TileMap->TileSizeInMeters;
                    
                    float dPlayerX = 0.0f;
                    float dPlayerY = 0.0f;
                    float PlayerSpeed = 5.0f;
                    
                    float dPlayer2X = 0.0f;
                    float dPlayer2Y = 0.0f;
                    
                    // TODO: Should we handle the invertedness of our controller input in the win32 layer or in 
                    //the game layer. I don't know we shall revisit that sometime later.
                    
                    //below will handle events, so when the user interacts with something
                    if ( GameState->MessageQueue.Count == 0 )
                    {
                        entity *EntityAhead = GetEntityAhead(TileMap, GameState, GameState->Player);
                        
                        entity *EntityAhead_p2 = GetEntityAhead(TileMap, GameState, GameState->AllEntities[0].npc);
                        
                        if (EntityAhead_p2 != NULL && !EntityAhead_p2->WalkOnEvent)
                        {
                            // Player 2 any button down
                            // TODO: Abstract this code, i.e. the code that checks for user input.
                            if (Controller->DebugButtons[1].EndedDown && Controller->DebugButtons[1].HalfTransitionCount > 0)
                            {
                                // check if the entity blits messages
                                if (EntityAhead_p2->Message[0])
                                {
                                    game_posponed_function entityFunction = {}; entityFunction.Data = GameState; entityFunction.Function = EntityAhead_p2->Function.Function;
                                    entityFunction.Param = 0; // index of initiator for subsequent use in Function made by message
                                    
                                    CreateNewMessage(GameState, CenterX, CenterY + 100.0f, EntityAhead_p2->Message, OVERRIDEWAIT, entityFunction, GameState->AllEntities[0].npc, EntityAhead_p2->npc);
                                }
                                
                                // Check if the entity has a super function?
                                // Check for not arceus but there is still an event 
                                
                                /*if (EntityAhead_p2->Function.Function && !StringEquals(EntityAhead_p2->Message, "Dodogyuuun!"))
                                {
                                    GameState->FunctionQueue.Functions[GameState->FunctionQueue.Count++] = EntityAhead_p2->Function;
                                }
                                */
                                
                                // Is entity an NPC?
                                // Make the NPC face player 2 when they interact with player 2.
                                if (EntityAhead_p2->npc != NULL)
                                {
                                    entity_npc *EntityAheadNPC = EntityAhead_p2->npc;
                                    entity_npc *Player2 = GameState->AllEntities[0].npc;
                                    
                                    if (Player2->MoveDirection == UP)
                                    {
                                        EntityAheadNPC->MoveDirection = DOWN; 
                                    }
                                    else if (Player2->MoveDirection == DOWN)
                                    {
                                        EntityAheadNPC->MoveDirection = UP;
                                    }
                                    else if (Player2->MoveDirection == LEFT)
                                    {
                                        EntityAheadNPC->MoveDirection = RIGHT;
                                    }
                                    else if (Player2->MoveDirection == RIGHT)
                                    {
                                        EntityAheadNPC->MoveDirection = LEFT;
                                    }
                                }
                            }
                        }
                        
                        if (EntityAhead != NULL) //so provided there is an entity ahead of us lets do some processing
                        {
                            // TODO(Noah): Actually make a walk on event.
                            if (!EntityAhead->WalkOnEvent)
                            {
                                // get button down (previously up)
                                if (Keyboard->DebugButtons[1].EndedDown && Keyboard->DebugButtons[1].HalfTransitionCount > 0)
                                {
                                    // check if the entity blits messages
                                    if (EntityAhead->Message[0] || (EntityAhead->MessageFlags & CHAIN))
                                    {
                                        game_posponed_function entityFunction = {}; entityFunction.Data = GameState; entityFunction.Function = EntityAhead->Function.Function;
                                        // NOTE: Player index in entity list is 1
                                        entityFunction.Param = 1;
                                        
                                        if (EntityAhead->MessageFlags & CHAIN)
                                        {
                                            CreateNewMessageChain(GameState, CenterX, CenterY + 100.0f,  entityFunction, GameState->Player, EntityAhead->npc, EntityAhead->MessageChain,EntityAhead->MessageChainCount
                                                                  );
                                        }
                                        else
                                        {
                                            CreateNewMessage(GameState, CenterX, CenterY + 100.0f, EntityAhead->Message, OVERRIDEWAIT, entityFunction, GameState->Player, EntityAhead->npc);
                                        }
                                    }
                                    
                                    
                                    // Check for not arceus but there is still an event 
                                    /*if (EntityAhead->Function.Function && !StringEquals(EntityAhead->Message, "Dodogyuuun!"))
                                    {
                                        GameState->FunctionQueue.Functions[GameState->FunctionQueue.Count++] = EntityAhead->Function;
                                    }*/
                                    
                                    // NOTE: The code below is specialized for the arceus entity
                                    if (StringEquals(EntityAhead->Message ,"Dodogyuuun!"))
                                    {
                                        // the entity is arceus
                                        
                                        // stop playing background music
                                        PopSound(GameState, 0);
                                        // play arceus cry
                                        PlaySoundEffect(GameState, GameState->SoundEffects[6], false, 1.0f); 
                                        // load the arceus function into the queue
                                        GameState->FunctionQueue.Functions[GameState->FunctionQueue.Count++] = EntityAhead->Function;
                                    }
                                    
                                    // Is entity an NPC?
                                    // Make the NPC face the player when they interact with the player.
                                    if (EntityAhead->npc != NULL)
                                    {
                                        entity_npc *EntityAheadNPC = EntityAhead->npc;
                                        entity_npc *Player = GameState->Player;
                                        
                                        if (Player->MoveDirection == UP)
                                        {
                                            EntityAheadNPC->MoveDirection = DOWN; 
                                        }
                                        else if (Player->MoveDirection == DOWN)
                                        {
                                            EntityAheadNPC->MoveDirection = UP;
                                        }
                                        else if (Player->MoveDirection == LEFT)
                                        {
                                            EntityAheadNPC->MoveDirection = RIGHT;
                                        }
                                        else if (Player->MoveDirection == RIGHT)
                                        {
                                            EntityAheadNPC->MoveDirection = LEFT;
                                        }
                                    }
                                    
                                }
                            }
                            else // walk on event
                            {
                                //so if the player is on the entity tile
                                if ( (EntityAhead->TileMapPos.AbsTileX == GameState->Player->Entity->TileMapPos.AbsTileX) && 
                                    (EntityAhead->TileMapPos.AbsTileY == GameState->Player->Entity->TileMapPos.AbsTileY) &&
                                    (EntityAhead->TileMapPos.AbsTileZ == GameState->Player->Entity->TileMapPos.AbsTileZ))
                                {
                                    if (EntityAhead->Message[0])
                                    {
                                        CreateNewMessage(GameState, CenterX, CenterY + 100.0f, EntityAhead->Message, OVERRIDEWAIT, NULL_GAME_FUNCTION, GameState->Player, NULL_PLAYER);
                                    }
                                    
                                    if (EntityAhead->Function.Function)
                                    {
                                        GameState->FunctionQueue.Functions[GameState->FunctionQueue.Count++] = EntityAhead->Function;
                                    }									
                                }
                            } // end of if (!walkon event)
                        } // end of processing entity ahead
                    }
                    else // we are blitting a message!
                    {
                        bool user1_act = Keyboard->DebugButtons[1].EndedDown && Keyboard->DebugButtons[1].HalfTransitionCount > 0;
                        
                        bool user2_act = Controller->DebugButtons[1].EndedDown && Controller->DebugButtons[1].HalfTransitionCount > 0;
                        
                        game_message currentMessage = GameState->MessageQueue.Messages[0];
                        
                        // user (correct user) presses button to "skip" the message
                        if ( (currentMessage.initiator == GameState->Player && user1_act) || (currentMessage.initiator == GameState->AllEntities[0].npc && user2_act) )
                        {
                            PlaySoundEffect(GameState, GameState->SoundEffects[1], false, 1.0f);
                            
                            //kill the first message in the queue, because FIFO
                            PopMessage(&GameState->MessageQueue, 0); 
                            game_posponed_function PosponedFunction = {}; PosponedFunction.Data = &GameState->FunctionQueue; 
                            PosponedFunction.Function = PopFunction; 
                            GameState->SuperFunctionQueue.Functions[GameState->SuperFunctionQueue.Count++] = PosponedFunction;							 
                        }
                        
                    }
                    
                    // NOTE: Player movement code and appropriate blocking according to state of current message, if any
                    {
                        bool should_player1_move = true;
                        bool should_player2_move = true;
                        
                        if (GameState->MessageQueue.Count != 0)
                        {
                            game_message currentMessage = GameState->MessageQueue.Messages[0];
                            
                            if (GameState->Player == currentMessage.speaker || GameState->Player == currentMessage.initiator)
                            {
                                should_player1_move = false;
                            }
                            
                            entity_npc *player2 = GameState->AllEntities[0].npc;
                            
                            if (player2 == currentMessage.speaker || player2 == currentMessage.initiator)
                            {
                                should_player2_move = false;
                            }
                        }
                        
                        // the first player
                        if (should_player1_move)
                        {
                            if (Keyboard->Up.EndedDown)
                            {
                                dPlayerY += PlayerSpeed;
                                GameState->Player->MoveDirection = UP;
                            }
                            else if (Keyboard->Down.EndedDown)
                            {
                                dPlayerY -= PlayerSpeed;
                                GameState->Player->MoveDirection = DOWN;
                            }
                            else if (Keyboard->Left.EndedDown)
                            {
                                dPlayerX -= PlayerSpeed;
                                GameState->Player->MoveDirection = LEFT;
                            }
                            else if (Keyboard->Right.EndedDown)
                            {
                                dPlayerX += PlayerSpeed;
                                GameState->Player->MoveDirection = RIGHT;
                            }
                        }
                        
                        // the second player
                        entity_npc *PlayerTwo = GameState->AllEntities[0].npc;
                        
                        if (should_player2_move)
                        {
                            if (Controller->Down.EndedDown)
                            {
                                dPlayer2Y += PlayerSpeed;
                                PlayerTwo->MoveDirection = UP;
                                //GameState->CameraPos.Y -= Input->DeltaTime * 20.0f; 
                            }
                            else if (Controller->Up.EndedDown)
                            {
                                dPlayer2Y -= PlayerSpeed;
                                PlayerTwo->MoveDirection = DOWN;
                                //GameState->CameraPos.Y += Input->DeltaTime * 20.0f;
                            }
                            else if (Controller->Left.EndedDown)
                            {
                                dPlayer2X -= PlayerSpeed;
                                PlayerTwo->MoveDirection = LEFT;
                                //GameState->CameraPos.X -= Input->DeltaTime * 20.0f;
                            }
                            else if (Controller->Right.EndedDown)
                            {
                                dPlayer2X += PlayerSpeed;
                                PlayerTwo->MoveDirection = RIGHT;
                                //GameState->CameraPos.X += Input->DeltaTime * 20.0f;
                            }
                        }
                    }
                    
                    // If the player is moving and was not moving before, then reset AnimationTimer 
                    if( (dPlayerX != 0.0f) | (dPlayerY != 0.0f) )
                    {
                        if (GameState->Player->Walking == false)
                        {
                            // NOTE: We set the timer such that the player will start a new frame of animation as soon as they start walking
                            GameState->AnimationTimer = 0.126f;
                            //GameState->Player->Walking = true;
                        }
                        GameState->Player->Walking = true;
                    }
                    else //otherwise the player is not walking
                    {
                        GameState->Player->Walking = false;
                    }
                    
                    // the second player animation timer reset - see above for player 1
                    {
                        
                        entity_npc *PlayerTwo = GameState->AllEntities[0].npc;
                        
                        if( (dPlayer2X != 0.0f) | (dPlayer2Y != 0.0f) )
                        {
                            if (PlayerTwo->Walking == false)
                            {
                                GameState->AnimationTimer = 0.0f;
                            }
                            PlayerTwo->Walking = true;
                        }
                        else //otherwise the player is not walking
                        {
                            PlayerTwo->Walking = false;
                        }
                    }
                    
                    //Initialize some specs about the player's bounding box
                    float PlayerHeight = TileMap->TileSizeInMeters;
                    float PlayerWidth = 0.75f * PlayerHeight;
                    
                    // TODO: Abstract this function
                    // unfreeze player
                    
                    /*{
                        entity Entity = *GameState->Player->Entity;
                        unpacked_tile Tile = GetTileValue(&NewTileMap, Entity.TileMapPos.AbsTileX, Entity.TileMapPos.AbsTileY,
                                                          Entity.TileMapPos.AbsTileZ);
                        Tile.Walkable = true;
                        SetTile(&GameState->WorldArena, &NewTileMap, Entity.TileMapPos.AbsTileX, Entity.TileMapPos.AbsTileY,
                                Entity.TileMapPos.AbsTileZ, Tile);
                    }*/
                    ThawEntity(GameState, GameState->Player->Entity);
                    
                    // Freeze all npc so that the player can move
                    for (unsigned int x = 0; x < GameState->EntityCount; x++)
                    {
                        entity Entity = GameState->AllEntities[x];
                        
                        // NOTE: Can walk on all entities unless they are an NPC!
                        if (Entity.npc != NULL && Entity.npc != GameState->Player)
                        {
                            unpacked_tile Tile = GetTileValue(TileMap, Entity.TileMapPos.AbsTileX, Entity.TileMapPos.AbsTileY,
                                                              Entity.TileMapPos.AbsTileZ);
                            Tile.Walkable = false;
                            SetTile(&GameState->WorldArena, TileMap, Entity.TileMapPos.AbsTileX, Entity.TileMapPos.AbsTileY,
                                    Entity.TileMapPos.AbsTileZ, Tile);
                        }
                    }
                    
                    //Below we move the player
                    tile_map_position PlayerP = GameState->Player->Entity->TileMapPos;
                    tile_map_position NewPlayerP = PlayerP;
                    NewPlayerP.X = PlayerP.X + Input->DeltaTime * dPlayerX;
                    NewPlayerP.Y = PlayerP.Y + Input->DeltaTime * dPlayerY;
                    
                    // must recompute tile space since the X and Y values are world space offsets from the tile.  
                    NewPlayerP = ReCalcTileMapPosition(TileMap, NewPlayerP);
                    
                    unsigned int SteppingOntoNewTile = false;
                    if ( (NewPlayerP.X != PlayerP.X) | (NewPlayerP.Y != PlayerP.Y) ) {SteppingOntoNewTile = true;}
                    
                    // Below we actually ask the tile system to calculate our new tile position
                    GameState->Player->Entity->TileMapPos = QueryNewTileMapPos(TileMap, GameState->Player->Entity->TileMapPos,
                                                                               NewPlayerP, GameState->Player->MoveDirection, 0.5f * PlayerWidth);
                    
                    // unfreeze player 2
                    {
                        entity Entity = GameState->AllEntities[0];
                        unpacked_tile Tile = GetTileValue(TileMap, Entity.TileMapPos.AbsTileX, Entity.TileMapPos.AbsTileY,
                                                          Entity.TileMapPos.AbsTileZ);
                        Tile.Walkable = true;
                        SetTile(&GameState->WorldArena, TileMap, Entity.TileMapPos.AbsTileX, Entity.TileMapPos.AbsTileY,
                                Entity.TileMapPos.AbsTileZ, Tile);
                    }
                    
                    // Freeze all npc so that the player2 can move
                    for (unsigned int x = 0; x < GameState->EntityCount; x++)
                    {
                        entity Entity = GameState->AllEntities[x];
                        
                        // NOTE: Can walk on all entities unless they are an NPC!
                        if (Entity.npc != NULL && Entity.npc != GameState->AllEntities[0].npc)
                        {
                            unpacked_tile Tile = GetTileValue(TileMap, Entity.TileMapPos.AbsTileX, Entity.TileMapPos.AbsTileY,
                                                              Entity.TileMapPos.AbsTileZ);
                            Tile.Walkable = false;
                            SetTile(&GameState->WorldArena, TileMap, Entity.TileMapPos.AbsTileX, Entity.TileMapPos.AbsTileY,
                                    Entity.TileMapPos.AbsTileZ, Tile);
                        }
                    }
                    
                    // move player 2
                    {
                        // the second player
                        entity_npc *PlayerTwo = GameState->AllEntities[0].npc;
                        
                        tile_map_position Player2P = PlayerTwo->Entity->TileMapPos;
                        tile_map_position NewPlayer2P = Player2P;
                        NewPlayer2P.X = Player2P.X + Input->DeltaTime * dPlayer2X;
                        NewPlayer2P.Y = Player2P.Y + Input->DeltaTime * dPlayer2Y;		
                        NewPlayer2P = ReCalcTileMapPosition(TileMap, NewPlayer2P);
                        
                        //Below we actually ask the tile system to calculate our new tile position
                        PlayerTwo->Entity->TileMapPos = QueryNewTileMapPos(TileMap, PlayerTwo->Entity->TileMapPos,
                                                                           NewPlayer2P, PlayerTwo->MoveDirection, 0.5f * PlayerWidth);
                    }
                    
                    vector2f PlayerPos = {}; 
                    PlayerPos.X = GameState->Player->Entity->TileMapPos.AbsTileX * TileSizeInPixels + 
                        GameState->Player->Entity->TileMapPos.X * MetersToPixels;
                    PlayerPos.Y = GameState->Player->Entity->TileMapPos.AbsTileY * TileSizeInPixels + 
                        GameState->Player->Entity->TileMapPos.Y * MetersToPixels;
                    
                    
                    
                    // TODO: Is the camera operating in pixel space?
                    GameState->CameraPos = Lerp2f(GameState->CameraPos, PlayerPos, 0.9f * Input->DeltaTime);
                    
                    
                    if ( GameState->ThudLastFrame && ( (NewPlayerP.AbsTileX != GameState->LastThudTile.x) |
                                                      (NewPlayerP.AbsTileY != GameState->LastThudTile.y) | !GameState->Player->Walking ) )
                    {	
                        GameState->ThudLastFrame = false;
                    }
                    
                    // if im trying for a new tile and aftwards Im still not on that tile then I failed
                    if ( SteppingOntoNewTile && ( (NewPlayerP.X != GameState->Player->Entity->TileMapPos.X) | 
                                                 (NewPlayerP.Y != GameState->Player->Entity->TileMapPos.Y) ) && !GameState->ThudLastFrame)
                    {
                        PlaySoundEffect(GameState, GameState->SoundEffects[5], false, 1.0f);
                        GameState->ThudLastFrame = true;
                        GameState->LastThudTile.x = NewPlayerP.AbsTileX; GameState->LastThudTile.y = NewPlayerP.AbsTileY;
                    }
                    
                    render_queue RenderQueue = {};
                    
                    for (int RelRow = -20; RelRow < 20; ++RelRow)
                    {
                        for (int RelColumn = -40; RelColumn < 40; ++RelColumn)
                        {
                            //get the actual tile we are drawing
                            unsigned int Column = GameState->Player->Entity->TileMapPos.AbsTileX + RelColumn;
                            unsigned int Row = GameState->Player->Entity->TileMapPos.AbsTileY + RelRow;
                            unpacked_tile TileInformation = GetTileValue(TileMap, Column, Row, GameState->Player->Entity->TileMapPos.AbsTileZ);
                            
                            //only draw the tile if it actually has a bitmap
                            //NOTE: This still works for transparent tiles
                            if (TileInformation.TileType > 0)
                            { 
                                //Calculate the rectangle to draw
                                float MinX = CenterX - GameState->CameraPos.X + ((float)Column) * TileSizeInPixels;
                                float MaxX = MinX + TileSizeInPixels;
                                float MinY = CenterY + GameState->CameraPos.Y - ((float)Row) * TileSizeInPixels;
                                float MaxY = MinY - TileSizeInPixels;
                                
                                if( TileInformation.Transparent )
                                {
                                    //unpack the transparent tile
                                    unsigned int BackgroundIndex = 0;
                                    unsigned int ForegroundIndex = 0;
                                    UnPackTransparentTile(TileInformation, &BackgroundIndex, &ForegroundIndex);
                                    
                                    //draw the background tile
                                    if (BackgroundIndex)
                                    {
                                        loaded_bitmap TileBitmap = GrabTileBitmap(GameState->TileSet, BackgroundIndex);
                                        DrawBitmapScaled(buffer, TileBitmap, MinX, MaxX, MaxY, MinY, FLIPFALSE, 2);
                                    }
                                    
                                    //if there is no foreground tile we can't draw anything
                                    if (ForegroundIndex)
                                    {
                                        loaded_bitmap ForegroundBitamp = GrabTileBitmap(GameState->TileSet, ForegroundIndex);
                                        
                                        if (!TileInformation.AbovePlayer)
                                        {
                                            DrawBitmapScaled(buffer, ForegroundBitamp, MinX, MaxX, MaxY, MinY, FLIPFALSE, 2);
                                        }
                                        else
                                        {
                                            render_item RenderItem = {};
                                            RenderItem.Bitmap = ForegroundBitamp;
                                            RenderItem.MinX = MinX; RenderItem.MaxX = MaxX;
                                            RenderItem.MinY = MaxY; RenderItem.MaxY = MinY; 
                                            RenderItem.Flip = FLIPFALSE;
                                            RenderItem.ScaleFlag = 0; RenderItem.Scalar = 0;
                                            PostponeRenderItem(RenderItem, &RenderQueue);
                                        }
                                        
                                        //check if the tile is a grass tile and the player is on it
                                        if ( (ForegroundIndex == 112) && (GameState->Player->Entity->TileMapPos.AbsTileX == Column) &&
                                            (GameState->Player->Entity->TileMapPos.AbsTileY == Row))
                                        {
                                            loaded_bitmap Bitmap = GrabTileBitmap(GameState->TileSet, 47);
                                            render_item RenderItem = {};
                                            RenderItem.MinX = MinX; RenderItem.MaxX = MaxX;
                                            RenderItem.MinY = MaxY; RenderItem.MaxY = MinY; 
                                            RenderItem.Flip = FLIPFALSE;
                                            RenderItem.ScaleFlag = 0; RenderItem.Scalar = 0;
                                            RenderItem.Bitmap = ForegroundBitamp;
                                            PostponeRenderItem(RenderItem, &RenderQueue);
                                            if (GameState->Player->Walking)
                                            {
                                                RenderItem.Bitmap = Bitmap;
                                                PostponeRenderItem(RenderItem, &RenderQueue);
                                            }
                                        }
                                        
                                    }
                                }
                                else //the tile is not transparent
                                {
                                    loaded_bitmap TileBitmap = GrabTileBitmap(GameState->TileSet, TileInformation.TileType);
                                    
                                    if (!TileInformation.AbovePlayer)
                                    {
                                        DrawBitmapScaled(buffer, TileBitmap, MinX, MaxX, MaxY, MinY, FLIPFALSE, 2);
                                    }
                                    else
                                    {
                                        render_item RenderItem = {}; RenderItem.Bitmap = TileBitmap;
                                        RenderItem.MinX = MinX; RenderItem.MaxX = MaxX;
                                        RenderItem.MinY = MaxY; RenderItem.MaxY = MinY; 
                                        RenderItem.Flip = FLIPFALSE;
                                        RenderItem.ScaleFlag = 0; RenderItem.Scalar = 0;
                                        PostponeRenderItem(RenderItem, &RenderQueue);
                                    }
                                    
                                    //check if its a grass tile and the player is on that tile 
                                    if ( (TileInformation.TileType == 112) && (GameState->Player->Entity->TileMapPos.AbsTileX == Column) &&
                                        (GameState->Player->Entity->TileMapPos.AbsTileY == Row))
                                    {
                                        loaded_bitmap Bitmap = GrabTileBitmap(GameState->TileSet, 47);
                                        render_item RenderItem = {};
                                        RenderItem.MinX = MinX; RenderItem.MaxX = MaxX;
                                        RenderItem.MinY = MaxY; RenderItem.MaxY = MinY; 
                                        RenderItem.Flip = FLIPFALSE;
                                        RenderItem.ScaleFlag = 0; RenderItem.Scalar = 0;
                                        RenderItem.Bitmap = TileBitmap;
                                        PostponeRenderItem(RenderItem, &RenderQueue);
                                        if (GameState->Player->Walking)
                                        {
                                            RenderItem.Bitmap = Bitmap;
                                            PostponeRenderItem(RenderItem, &RenderQueue);
                                        }
                                    }
                                    
                                }
                            }
                        }
                    }
                    
                    /* 
                    Smart system for drawing the NPC's
                    and the player with proper layering.
                    */
                    {
                        unsigned int delay_NPC_Count = 0;
                        entity_npc entity_npc_buf[MAX_ENTITIES];
                        
                        // Draw the NPC's
                        for (unsigned int x = 0; x < GameState->EntityCount; x++)
                        {
                            entity Entity = GameState->AllEntities[x];
                            
                            // is an npc and not the player
                            if (Entity.npc != NULL && Entity.npc != GameState->Player)
                            {
                                entity_npc *currentNPC = Entity.npc;
                                
                                // Check if rendering should NOT be postponed (render above player)
                                if (Entity.TileMapPos.AbsTileY >= GameState->Player->Entity->TileMapPos.AbsTileY)
                                {
                                    // if the player is below the NPC (in world space), render the NPC
                                    
                                    float X = Entity.TileMapPos.AbsTileX * TileSizeInPixels + 
                                        Entity.TileMapPos.X * MetersToPixels + CenterX - GameState->CameraPos.X;
                                    
                                    float Y = -1.0f * Entity.TileMapPos.AbsTileY * TileSizeInPixels - 
                                        Entity.TileMapPos.Y * MetersToPixels + CenterY + GameState->CameraPos.Y;
                                    
                                    vector2f ScreenPos2 = {}; ScreenPos2.X = X; ScreenPos2.Y = Y;
                                    
                                    DrawNPC(buffer, currentNPC, ScreenPos2);
                                }
                                else
                                {
                                    // postpone the rendering until after the player
                                    entity_npc_buf[delay_NPC_Count++] = *currentNPC;
                                }
                            }
                        }
                        
                        // render the player
                        float X = GameState->Player->Entity->TileMapPos.AbsTileX * TileSizeInPixels + 
                            GameState->Player->Entity->TileMapPos.X * MetersToPixels + CenterX - GameState->CameraPos.X;
                        
                        float Y = -1.0f * GameState->Player->Entity->TileMapPos.AbsTileY * TileSizeInPixels - 
                            GameState->Player->Entity->TileMapPos.Y * MetersToPixels + CenterY + GameState->CameraPos.Y;
                        
                        vector2f ScreenPos2 = {}; ScreenPos2.X = X; ScreenPos2.Y = Y;
                        
                        DrawNPC(buffer, GameState->Player, ScreenPos2);
                        
                        // render the postponed NPC's
                        for (unsigned int i = 0; i < delay_NPC_Count; i++)
                        {
                            entity Entity = *entity_npc_buf[i].Entity;
                            entity_npc *currentNPC = Entity.npc;
                            
                            float X = Entity.TileMapPos.AbsTileX * TileSizeInPixels + 
                                Entity.TileMapPos.X * MetersToPixels + CenterX - GameState->CameraPos.X;
                            
                            float Y = -1.0f * Entity.TileMapPos.AbsTileY * TileSizeInPixels - 
                                Entity.TileMapPos.Y * MetersToPixels + CenterY + GameState->CameraPos.Y;
                            
                            vector2f ScreenPos2 = {}; ScreenPos2.X = X; ScreenPos2.Y = Y;
                            
                            DrawNPC(buffer, currentNPC, ScreenPos2);
                        }
                    }
                    
                    
                    //draw any tiles that were posponed!
                    DrawRenderQueueItems(buffer, &RenderQueue);
                    
                    //if the player stepped onto a new tile!
                    if ( (GameState->Player->Entity->TileMapPos.AbsTileX != GameState->LastPlayer.TileMapPos.AbsTileX) |
                        (GameState->Player->Entity->TileMapPos.AbsTileY != GameState->LastPlayer.TileMapPos.AbsTileY))
                    {
                        //get the information about the tile we are on
                        unpacked_tile TileInformation = GetTileValue(TileMap, GameState->Player->Entity->TileMapPos.AbsTileX,
                                                                     GameState->Player->Entity->TileMapPos.AbsTileY, GameState->Player->Entity->TileMapPos.AbsTileZ);
                        
                        unsigned int PlayerOnGrassTile = false;
                        if (TileInformation.Transparent)
                        {
                            unsigned int BackgroundIndex = 0;
                            unsigned int ForegroundIndex = 0;
                            UnPackTransparentTile(TileInformation, &BackgroundIndex, &ForegroundIndex);
                            if (ForegroundIndex == 112)
                            {
                                PlayerOnGrassTile = true;
                            }
                        }	
                        else
                        {
                            if (TileInformation.TileType == 112)
                            {
                                PlayerOnGrassTile = true;
                            }
                        }
                        
                        if( PlayerOnGrassTile & TryForRandomEncounter(GameState))
                        {
                            // stop playing background music
                            PopSound(GameState, 0); 
                            
                            // start plaing battle music
                            PlaySoundEffect(GameState, GameState->Music, true, 1.0f); 
                            
                            // unfreeze the player;
                            ThawEntity(GameState, GameState->Player->Entity);
                            
                            SeedRandom(GameState);
                            
                            //generate the wild pokemon!
                            CloneString("Bulbasaur", GameState->BufferPokemon[0].Nickname, 10);
                            GameState->BufferPokemon[0].Experience = 0;
                            UpdatePokemonLevel(&GameState->BufferPokemon[0], 226000);
                            GameState->BufferPokemon[0].PokemonID = 1;
                            GameState->BufferPokemon[0].Ability = OVERGROW;
                            GameState->BufferPokemon[0].Moves[0] = FillPokemonMove(GameState->MoveDatabase, TACKLE);
                            GameState->BufferPokemon[0].Moves[1] = FillPokemonMove(GameState->MoveDatabase, SEEDBOMB);
                            GameState->BufferPokemon[0].Moves[2] = FillPokemonMove(GameState->MoveDatabase, LEECHSEED);
                            GameState->BufferPokemon[0].Moves[3] = FillPokemonMove(GameState->MoveDatabase, GROWL);
                            GameState->BufferPokemon[0].Gender = 1;
                            GameState->BufferPokemon[0].HP = GetPokemonStatHP(GameState, &GameState->BufferPokemon[0]);
                            
                            AllocBackBufferIntoGameState(GameState);
                            CloneBuffer(buffer, &GameState->BackBuffer);
                            InstantiateShader(GameState, &GameState->BackBuffer, GameState->LUT[0], 1.5f);
                            SetPauseState(GameState, 2.0f);
                            GameState->GameState = BEGINBATTLE;
                        }
                    }
                    
                }break;
            } // end of switch
            
            float AnimationDeltaTime = 0.125f;
            
            GameState->AnimationTimer += Input->DeltaTime;
            if (GameState->AnimationTimer > AnimationDeltaTime)
            {
                /*
                if (GameState->Player->Walking)
                {
                //increment the counter of the first animation on the animation player
                GameState->Player->Entity->AnimationPlayer.Animations[0].Counter++;
                }
                */
                
                for (unsigned int x = 0; x < GameState->EntityCount; x++)
                {
                    entity_npc *NPC = GameState->AllEntities[x].npc;
                    if (NPC != NULL)
                    {
                        if (NPC->Walking)
                        {
                            NPC->Entity->AnimationPlayer.Animations[0].Counter++;
                        }
                    }
                }
                
                if (GameState->GameState == BATTLE)
                {
                    for (int x = 0; x < 4; x++)
                    {
                        if(GameState->PokemonA1.Animations[x].Playing)
                        {
                            animation *Animation = &GameState->PokemonA1.Animations[x];
                            if (++Animation->Counter % Animation->MaxFrames == 0)
                            {
                                GameState->PokemonA1.Iter++;
                            }	
                        }
                        if(GameState->PokemonA2.Animations[x].Playing)
                        {
                            animation *Animation = &GameState->PokemonA2.Animations[x];
                            if (++Animation->Counter % Animation->MaxFrames == 0)
                            {
                                GameState->PokemonA2.Iter++;
                            }	
                        }
                    }
                }
                
                GameState->AnimationTimer = 0.0f;
            }
            
            if (GameState->MessageQueue.Count)
            {
                UpdateMessage(buffer, &GameState->MessageQueue.Messages[0], GameState);	
            }
            
            //below is the code for drawing all the UI scenes
            //currently we will draw all UI at the center of the screen. although their top left thing will be at the center
            for (unsigned int x = 0; x < UI_AMOUNT; x++) 
            {
                game_ui_scene CurrentScene = GameState->UserInterfaces[x];
                if (CurrentScene.Active)
                {
                    game_screen_position ScreenPos = CurrentScene.Position;
                    ScreenPos.X += buffer->width / 2.0f; ScreenPos.Y += buffer->height / 2.0f;
                    DrawUIScene(buffer, GameState->Bitmaps, GameState->PokemonDemoFont, CurrentScene, ScreenPos);
                }
            }
            
            // deep copy of time components
            // GameState->LastPlayer = GameState->Player;
            GameState->LastPlayer.npc->Walking = GameState->Player->Walking;
            GameState->LastPlayer.npc->MoveDirection = GameState->Player->MoveDirection;
            
            GameState->LastPlayer.TileMapPos = GameState->Player->Entity->TileMapPos;
            
        } // end of game pause state
        
        
        
        // Note there are 10 total functions that might be in the function queue. For all these functions, update their timers.
        for (unsigned int x = 0; x < 10; x++)
        {
            game_posponed_function PosponedFunction = GameState->SuperFunctionQueue.Functions[x];
            if (PosponedFunction.Length)
            {
                if (GameState->GameTimer - PosponedFunction.Timer > PosponedFunction.Length)
                { 
                    PopFunction(&GameState->SuperFunctionQueue, x);
                }
            }
            else
            {
                PopFunction(&GameState->SuperFunctionQueue, x);				
            }
        }
        
        
        // here, right below the pause we can do some shader code because I allow shader code to execute outside of the pause
        if (GameState->Shader.Active)
        {
#if 0
            // old code block.
            if (GameState->Shader.source2)
            {
                ExecuteShader2(buffer, &GameState->Shader, Input->DeltaTime);
            }
            else
#endif
            {
                // TODO: Shouldn't the shader store a ref to the allocator
                // for the source that it owns?
                ExecuteShader(GameState, buffer, &GameState->Shader, Input->DeltaTime);
            }
        }
    } // end of test for if the game memory exists
}

#ifdef __cplusplus
extern "C"
#endif
GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    //GameUpdateSound(SoundBuffer,256);
    
    game_state *GameState = (game_state *)Memory->Storage;
    
    if (Memory->Valid)
    {
        short *SampleOut = SoundBuffer->SampleOut;
        for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleAmount; ++SampleIndex)
        {
            int OverlaySound = 0;  
            
            for (int x = 0; x < GameState->SoundQueue.Count; x++)
            {
                sound_item *SoundItem = &GameState->SoundQueue.SoundItems[x];
                
                // TODO: we check here for SampleCount == 0 because it may be that there was some error loading
                // the sound. Of course, this means there was a bug...
                if ( (SoundItem->Sound.SampleCount == 0) || 
                     (SoundItem->SoundIndex == (SoundItem->Sound.SampleCount-1)) )
                {
                    if (SoundItem->Sound.SampleCount == 0)
                    {
#ifdef _WIN32
                        // TODO: add this warning.
                        //OutputDebugStringA("WARNING! a sound may have not loaded proper");
#endif
                    }
                    //kill the sound because its done playing!
                    if (!SoundItem->Loop)
                    {
                        PopSound(GameState, SoundItem->QueueIndex);
                    }
                }
                else
                {
                    OverlaySound += (int)SoundItem->Sound.SampleData[0][
                        SoundItem->SoundIndex++ % SoundItem->Sound.SampleCount];
                }                
            }

#if 0
            if (GameState->GainingExp)
            {
                //OverlaySound += PlayRisingSine(Input->DeltaTime, GameState->CurrentSineIndex++, SoundBuffer->SamplesPerSecond);
            }
#endif

            // TODO: this is crude. What happened to mixing and mastering ????
            int Value = OverlaySound;
            
            if (Value > MAX_SIGNED_SHORT)
            {
                Value = MAX_SIGNED_SHORT;
            }
            else if (Value < MIN_SIGNED_SHORT)
            {
                Value = MIN_SIGNED_SHORT;
            }
            
            // 2 channel audio buffer, write same audio to both channels!
            
            *SampleOut++ = (short)Value;
            if (!mono)
                *SampleOut++ = (short)Value;
        }
    }
}
