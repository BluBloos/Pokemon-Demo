// play with whether or not to inline functions
#if 0
#define INLINE inline
#else
#define INLINE
#endif

#ifdef __cplusplus
#define PokeZeroMem(var) var = {}
#else
#include <string.h>
// sizeof the var works, it uses the type of the var.
// keep in mind that if you pass a pointer, then it will get the size of the pointer, not the thing it pointss to
#define PokeZeroMem(var) memset(&var, 0, sizeof(var))
#endif

#if POKEMON_DEMO_DEBUG
#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define MAX_SIGNED_SHORT 32767
#define MIN_SIGNED_SHORT -32768
#define Pi 3.1415f

#define SafeSubtract(Value, Subtractor) Value = (Value >= Subtractor)? Value - Subtractor: 0
#define SafeSubtractInline(Value, Subtractor) ((Value >= Subtractor)? Value - Subtractor: 0)
#define ArrayCount(Array) (int)( sizeof(Array) / sizeof(Array[0]) )

#define internal static
#define local_persist static
#define global_variable static

typedef float real32;
typedef double real64;

#define PushStruct(Arena, type) (type *)PushSize( Arena, sizeof(type) )
#define PushArray(Arena, Count, type) (type *)PushSize( Arena, (Count) * (sizeof(type)) )

#define NULL 0

// NOTE: that in reading the enumerations,
// left -> right corresponds to low order -> high order
typedef enum
{
    RGB, // RGBA
    BGR // BGRA
} ColorMode;

ColorMode GLOBAL_COLOR_MODE;

typedef struct 
{
	float X;
	float Y;	
} vector2f;

typedef struct 
{
	float x;
	float y;
	float z;
} vector3f;

typedef struct 
{
	unsigned int x;
	unsigned int y;
	unsigned int z;	
} vector3;

#include "PokemonDemo_tile.h"
#include "PokemonDemo_intrinsics.h"

#define POKEMON_DATABASE_LENGTH 150
#define TILESETAMOUNT 1708
#define BITMAPAMOUNT 3
#define TESTIFICATE_AMOUNT 10
#define ENTITY_AMOUNT 10
#define UI_AMOUNT 2

#define MAX_ENTITIES 256

typedef enum 
{
	UP, DOWN , LEFT, RIGHT
} Direction;

typedef enum 
{	
	FLIPTRUE = -1, FLIPFALSE = 1, FRONT = 2, BACK = 3
} SpriteRenderDirection;

typedef enum 
{
	OVERWORLD, BEGINBATTLE, BATTLE, EXITBATTLE, MAINMENU, OPENING, ENTERINGAREA, BLACKEDOUT, CHILLING
} GameState;

typedef enum 
{
	ATTACKMENU = 1, 
	MENU = 2, 
	MESSAGEBLIT = 4, 
	PAUSE = 8, 
	USERFIRST = 16, 
	TURN1 = 32, 
	TURN2 = 64,
	FAINT = 128,
	BATTLEWON = 256,
	BATTLELOST = 512, 
	WAITING = 1024,
	LEVELING = 2048,
	DIM = 4096
} StateFlags;

typedef enum 
{
	HOLD = 1, KILLMESSAGE = 2, OVERRIDEWAIT = 4
} MessageFlags;

typedef enum 
{
	TACKLE, SEEDBOMB, LEECHSEED, GROWL, 
	IRONTAIL, ELECTROBALL, THUNDERBOLT, QUICKATTACK,
	JUDGMENT, PSYCHIC_MOVE, ICEBEAM, RECOVER
} Move;

typedef enum 
{
	PHYSICAL = 1, 
	PROTECT = 2,
	MAGICCOAT = 4,
	SNATCH = 8,
	MIRRORMOVE = 16,
	KINGSROCK = 32, 
	CONTACT = 64 
} MoveFlags;

typedef enum 
{
	NORMAL, FIGHTING, FLYING, POISON, GROUND, ROCK, BUG, GHOST, STEEL,
	FIRE, WATER, GRASS, ELECTRIC, PSYCHIC, ICE, DRAGON, DARK, FAIRY, UKNOWN
} Type;

typedef enum 
{
	OVERGROW, CHLOROPHYL, STATIC
} Ability;

typedef enum 
{
	SPEED, SPDEFENSE, SPATTACK, DEFENSE, ATTACK, HP
} Stat;

typedef enum 
{
	SUPEREFFECTIVE = 1, CRITICAL = 2, NOTVERYEFFECTIVE = 4, NOTEFFECTIVE = 8
} MoveResult;

typedef struct 
{
	char Name[16];
	char Description[256];
	unsigned char Effect;
	unsigned char Type;
	unsigned char BasePower;
	unsigned char Accuracy;
	unsigned char PP;
	unsigned char MaxPP;
	unsigned char EffectAccuracy;
	unsigned char AffectsWhom;
	unsigned char Priority;
	unsigned char Flags;
} pokemon_move;

//NOTE: We need to reconsider this entire struct 
typedef struct 
{
	char Nickname[13];
	unsigned int Experience;
	char Level; //we should remove this
	short PokemonID;
	short Ability;
	unsigned int Nature;
	unsigned int HP;
	unsigned int IV[6];
	unsigned int EV[6];
	pokemon_move Moves[4];
	char Gender; //the reason we have so much precision here is 
	//because obviously there are more than 2 genders, clearly. 
} pokemon;

typedef struct 
{
	pokemon *Pokemon;
	int Experience;
	short Ability;
	signed char SpeedStage;
	signed char SpDefenseStage;
	signed char SpAttackStage;
	signed char DefenseStage;
	signed char AttackStage;
	unsigned int Speed;
	unsigned int SpDefense;
	unsigned int SpAttack;
	unsigned int Defense;
	unsigned int Attack;
	unsigned int SelectedMove;
} battle_pokemon;

typedef struct 
{
	unsigned int *PixelPointer;
	unsigned int Width;
	unsigned int Height;
	unsigned int Scale;
} loaded_bitmap;

//Animation things
typedef struct 
{
	loaded_bitmap Frames[128]; //the frames of animation
	unsigned int MaxFrames; //how many frames the animation has
	unsigned int Counter; //the current frame of animation
	unsigned int Playing; //NOTE: I think we can remove this item
} animation;

typedef struct 
{
	unsigned int Iter; //so this is how many times the selected animation has been played
	unsigned int SelectedAnim; //specifies the selected animation
	unsigned int Offset; //offsets the selected animation
	unsigned int MaxIter[4]; //used to specify how many times any selected animation should play 
	animation Animations[4]; 
} animation_player;
////////

typedef void game_general_function(void *Data, unsigned int Param); 
typedef struct 
{
	game_general_function *Function;
	float Length;
	float Timer;
	void *Data;
	unsigned int Param;
} game_posponed_function;


game_posponed_function _null_game_func = {};
#define NULL_GAME_FUNCTION _null_game_func 


typedef struct 
{
	float X; float Y;
} game_screen_position;

////////////////// ENTITY STUFFS

struct entity; // force decl for circular dependency

typedef struct 
{
	struct entity *Entity; // super class
    
    // NPC Functionality is such that the entity can move
	
    //a boolean as to whether entity is moving
    unsigned int Walking; 
    //which direction the entity is moving
	unsigned int MoveDirection;
    
    /////////////////////
} entity_npc;

typedef struct entity
{
	//vector2f Pos; // relative to the camera!
	tile_map_position TileMapPos;
    
    // Anim player you would think would be better suited for npc subclass, but there is a case where you might want something that has animation, but doesn't move.
    animation_player AnimationPlayer;
    
    //////////////// INHERITANCE CODE
    entity_npc *npc; // subclass
    ////////////////
    
    //////////////////// EVENT STUFFS
    // bool for if walk on event
    unsigned int WalkOnEvent;
    
    // first character is 0 when the entity does not blit messages.
    char Message[256]; 
	unsigned int MessageFlags;
	
    // will be done after the entity blits any messages, if any at all
	game_posponed_function Function; 
    //////////////////
} entity;
////////////////////////////////////////


typedef struct 
{
	entity_npc NPC; game_screen_position ScreenPos;
} npc_render_item;
//

#include "PokemonDemo_io.h"

//UI THINGS
typedef struct 
{
	float R;
	float G;
	float B;
	float A;
} color;

typedef struct 
{
	float MinX; float MaxX; float MinY; float MaxY;
} rect;

typedef struct 
{
	//Bitmap properties
	unsigned int BitmapIndex; //Index into gamestate bitmap array
	//shared properties
	game_screen_position RelativePosition; //position relative to position of the UI Scene
	unsigned int IsString; //whether or not the element is a string
	unsigned int Flip; //whether or not to flip the bitmap or in the case of a string to draw it reversed.
	//string things
	char String[256];
	unsigned int Invert; //used for string to invert color.	
    
} game_ui_element;

typedef struct 
{
	unsigned int Count; //the amount of elements
	game_ui_element Elements[256];
	unsigned int Active;
	game_screen_position Position;
} game_ui_scene;

typedef struct 
{
	color Color;
	rect Rect; //the bounds of the button
	char Text[256]; //if there is no text the first string is null.
} game_ui_button;
///////////

typedef struct 
{
	size_t Size;
	unsigned char *Base;
	size_t Used;
} memory_arena;

internal void InitializeArena(memory_arena *Arena, unsigned char *StartOfMemory, size_t SizeOfMemory)
{
	//NOTE: Always write the usage code first, as a wise Casey once said
	Arena->Size = SizeOfMemory;
	Arena->Base = StartOfMemory;
	Arena->Used = 0;
}

internal void *PushSize(memory_arena *Arena, size_t SizeOfStruct)
{
	Assert( (Arena->Used + SizeOfStruct) <= Arena->Size );
	void *Result = Arena->Base + Arena->Used;
	Arena->Used += SizeOfStruct;
	return Result;
}

//NOTE(Noah): the #define is a macro and when the new 
//definition is found in our code the old definition is literally
//pasted in no joke fam. And we can even have very useful things like 
//arguments for the macros which makes them so cool. Gotta love C.

//NOTE(Noah):POKEMON_DEMO_DEBUG is whether or not we are debugging
//and POKEMON_DEMO_INTERNAL is whether or not we are shipping this version.

#define KiloBytes(Value) (Value * 1024)
#define MegaBytes(Value) (KiloBytes(Value) * 1024)
#define GigaBytes(Value) (MegaBytes(Value) * 1024)

#define TOGGLE_FULLSCREEN_CALLBACK(name) void name()
typedef TOGGLE_FULLSCREEN_CALLBACK(toggle_fullscreen_callback);

typedef struct 
{
	loaded_wav Sound;
	int MaxVolume;
	int SoundIndex;
	int QueueIndex;
	int Loop;
} sound_item;

//NOTE: We have a lot of queue's
//maybe it is considerable to implement 
//some sort of list system? Where we can make lists?
//where the lists support generic things?
typedef struct 
{
	int Count;
	sound_item SoundItems[256];
} sound_queue;

typedef struct 
{
	float BoxX; float BoxY; //x is center of text box and y is top of text box
	char String[256]; //the message to display
	unsigned int CurrentLength;
	float MaxY; float MinY; float MaxX; float MinX; //the bounds
	int Flags; //any message flags
	game_posponed_function Function;
} game_message;

typedef struct 
{
	int Count;
	game_message Messages[256];
} message_queue;

typedef struct 
{
	bool IsInitialized;
	bool Valid;
	long int StorageSize;
	void *Storage; //REQUIRED to be cleared when created. If Santa claus
	//is real than everything will be good.
	long int TransientStorageSize;
	void *TransientStorage;
    
	debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
	debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
	debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
	toggle_fullscreen_callback *ToggleFullscreen;
} game_memory;

typedef struct 
{
	tile_map *TileMap;
} world;

typedef struct 
{
	unsigned int Count;
	game_posponed_function Functions[10];
} function_queue;

typedef struct 
{
	void *memory;
	int width;
	int height;
	int pitch;
	int BytesPerPixel;
} game_offscreen_buffer;

typedef struct 
{
	game_offscreen_buffer *source;
	game_offscreen_buffer *source2;
	loaded_bitmap LUT;
	unsigned int Active;
	float ActiveTime;
	float Length;
} shader;


typedef struct 
{
	loaded_wav Wave;
	unsigned int ID;
} pokemon_cry;

#define LUT_AMOUNT 4

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

typedef struct 
{
	memory_arena WorldArena;
	world *World;
	int GameState;
	int LastState;
	int StateFlags;
	save_world LoadedWorld;
	vector2f CameraPos;
    
	//NOTE: The field below should be provided by the game
	unsigned int BackBufferPixels[SCREEN_WIDTH][SCREEN_HEIGHT];
	game_offscreen_buffer *GameBuffer;
	game_offscreen_buffer BackBuffer2;
	game_offscreen_buffer BackBuffer;
	shader Shader;
    
	//here are a ton of database files that we use at runtime
	//I think I like these so I want to keep them
	pokemon_database_data PokemonDatabase[POKEMON_DATABASE_LENGTH];
	float NatureDatabase[25][6];
	float TypeMatchupDatabase[18][18];
	game_ui_scene UserInterfaces[256]; //typically we dafault using 256 elements
	////////
	
	entity_npc *Player;
    entity LastPlayer;
    //_lp_npc is just for storage, should not be accessed.
    entity_npc _lp_npc;
    
    // this is the thing:
    // we want one unified array for the entities.
    // then we want proper inheritance, such that
    // BOTH npc and base class entities are in this array.
    
    /* Ideas:
    
sruct (base class): all the pointers to the subclasses
each subclass has a pointer BACK to the superclass. Like a double linked list.

*/
    
    unsigned int EntityCount; // start at 0
    unsigned int NPC_Count; // start at 0
    
    // NOTE: 256 is the MAX amount of entities
	entity AllEntities[MAX_ENTITIES];
    
    // note that this var should never be accessed from the outside. It is merely storage for use by an entity.
    entity_npc _npc_storage[MAX_ENTITIES];
    
    
    
    float InitialTimer; //consider this element
	float AnimationTimer; //consider this element
	float GameTimer;
	float PauseLength; //consider this element
    
	game_posponed_function PosponedFunction;
	function_queue FunctionQueue;
	function_queue SuperFunctionQueue;
    
	//below are things for randomness
	unsigned int LastRandom;
	unsigned int RandomState;
	
	float FloatBuffer[256];
	battle_pokemon *PokemonBackBuffer[6];
	pokemon BufferPokemon[2]; //consider this element
	pokemon PokemonP[6]; //consider this element
    
	battle_pokemon PokemonB1; //consider this element
	battle_pokemon PokemonB2; //consider this element
    
	animation_player PokemonA1; //consider this element 
	animation_player PokemonA2; //consider this element
    
	loaded_bitmap LUT[LUT_AMOUNT];
	loaded_bitmap SpriteMap; //consider this element
	loaded_bitmap PlayerSprites[12]; //consider this element
	loaded_bitmap TileSet[TILESETAMOUNT]; //consider this element
	loaded_bitmap PokemonDemoFont[70]; //consider this element
	loaded_bitmap MessageBox; //remove this element
	loaded_bitmap BattleBackground; //remove this element
	loaded_bitmap EnemyHealth; //remove this element
	loaded_bitmap PlayerHealth; //remove this element
	loaded_bitmap Genders[2]; //remove this element
	loaded_bitmap HealthLUT; //remove this element
	loaded_bitmap DEBUGAttackBackground; //remove this element
	loaded_bitmap AttackBoxes[19]; //remove this element
	loaded_bitmap MoveSelector; //remove this element
	loaded_bitmap DEBUGSheet; //remove this element
	loaded_bitmap BattleButton; //remove this element
	loaded_bitmap Bitmaps[BITMAPAMOUNT]; //consider this element
    
	pokemon_move MoveDatabase[12]; //consider this element
	unsigned int MenuSelection; //consider this element
    
    message_queue MessageQueue;
	sound_queue SoundQueue;
	unsigned int ThudLastFrame;
	vector3 LastThudTile;
    
	loaded_wav Hit2; //remove this element
	loaded_wav Music; //remove this element
	loaded_wav SoundEffects[256];
	pokemon_cry Cries[POKEMON_DATABASE_LENGTH];
	unsigned int CurrentSineIndex;
	unsigned int GainingExp;
} game_state;

typedef struct 
{
	int SamplesPerSecond;
	int SampleAmount;
	short *SampleOut;
} game_sound_output_buffer;

typedef struct 
{
	bool EndedDown;
	//TODO(Noah): currently on the keyboard this value does not get reset each frame.
	char HalfTransitionCount;
} game_user_gamepad_button_state;

typedef struct 
{
	game_user_gamepad_button_state DebugButtons[2];
	game_user_gamepad_button_state Dpad_up;
	game_user_gamepad_button_state Dpad_down;
	game_user_gamepad_button_state Dpad_left;
	game_user_gamepad_button_state Dpad_right;
	game_user_gamepad_button_state StartButton;
	game_user_gamepad_button_state AButton;
	game_user_gamepad_button_state BButton;
	game_user_gamepad_button_state XButton;
	game_user_gamepad_button_state YButton;
	game_user_gamepad_button_state LeftShoulder;
	game_user_gamepad_button_state RightShoulder;
	game_user_gamepad_button_state Right;
	game_user_gamepad_button_state Left;
	game_user_gamepad_button_state Up;
	game_user_gamepad_button_state Down;
} game_user_gamepad_input;

typedef struct 
{
	game_user_gamepad_input GamepadInput[2];
	//Mouse debug info
	game_user_gamepad_button_state MouseButtons[5];
	int MouseX, MouseY, MouseZ;
    
	float DeltaTime;
	char *BaseFilePath;
} game_user_input;

#define GAME_UPDATE_RENDER(name) void name(game_memory *Memory, game_offscreen_buffer *buffer, game_user_input *Input, ColorMode gameColorMode)
typedef GAME_UPDATE_RENDER(game_update_render);

#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory *Memory, game_sound_output_buffer *SoundBuffer, game_user_input *Input,bool mono)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

internal void PopSound(game_state *GameState, unsigned int QueueIndex)
{
	unsigned int LastIndex = GameState->SoundQueue.Count - 1;
	Assert(GameState->SoundQueue.Count > 0);
	for (unsigned int x = 0; x < LastIndex - QueueIndex; x++)
	{
		GameState->SoundQueue.SoundItems[QueueIndex + x] = GameState->SoundQueue.SoundItems[QueueIndex + x + 1];
		GameState->SoundQueue.SoundItems[QueueIndex + x].QueueIndex--;
	}
	GameState->SoundQueue.Count--;
}

internal unsigned int PlaySoundEffect(game_state *GameState, loaded_wav Wav, unsigned int Loop, float MaxVolume)
{
	unsigned int Result = GameState->SoundQueue.Count;
	sound_item SoundItem = {};
	SoundItem.MaxVolume = (int)(MaxVolume * MAX_SIGNED_SHORT);
	SoundItem.Sound = Wav;
	SoundItem.QueueIndex = GameState->SoundQueue.Count;
	SoundItem.Loop = Loop;
	GameState->SoundQueue.SoundItems[GameState->SoundQueue.Count++] = SoundItem;
	return Result;
}

/*
internal unsigned int PlaySoundEffect(game_state *GameState, loaded_wav Wav, unsigned int Loop)
{
 unsigned int Result = GameState->SoundQueue.Count;
 sound_item SoundItem = {};
 SoundItem.MaxVolume = MAX_SIGNED_SHORT;
 SoundItem.Sound = Wav;
 SoundItem.QueueIndex = GameState->SoundQueue.Count;
 SoundItem.Loop = Loop;
 GameState->SoundQueue.SoundItems[GameState->SoundQueue.Count++] = SoundItem;
 return Result;
}
*/

internal void PlaySoundEffectBare(void *Data, unsigned int Index)
{
	game_state *GameState = (game_state *)Data;
	sound_item SoundItem = {};
	SoundItem.MaxVolume = MAX_SIGNED_SHORT;
	SoundItem.Sound = GameState->SoundEffects[Index];
	SoundItem.QueueIndex = GameState->SoundQueue.Count;
	SoundItem.Loop = false;
	GameState->SoundQueue.SoundItems[GameState->SoundQueue.Count++] = SoundItem;
}

internal void KillSoundThenPlay(void *Data, unsigned int Index)
{
	game_state *GameState = (game_state *)Data;
	PopSound(GameState,0);
	sound_item SoundItem = {};
	SoundItem.MaxVolume = MAX_SIGNED_SHORT;
	SoundItem.Sound = GameState->SoundEffects[Index];
	SoundItem.QueueIndex = GameState->SoundQueue.Count;
	SoundItem.Loop = true;
	GameState->SoundQueue.SoundItems[GameState->SoundQueue.Count++] = SoundItem;
}

/* Function forward decl for C compilation
*/
//float ASCIIToFloat(char *String);

#include "PikaBlue_Strings.cpp"

/*
internal void CreateNewMessage(game_state *GameState, float PosX, float PosY, char *String, unsigned int OverflowFlags)
{
    game_message *Message = &GameState->MessageQueue.Messages[GameState->MessageQueue.Count++];
    *Message = {};
    
    loaded_bitmap Menu = GameState->DEBUGAttackBackground;
    Message->BoxX = PosX;
    Message->BoxY = PosY;
    float Padding = 20.0f; //NOTE: All default messages have a 40.0 pixel padding.
    Message->MinX = PosX - Menu.Width * Menu.Scale / 2.0f + Padding;
    Message->MaxX = PosX + Menu.Width * Menu.Scale / 2.0f - Padding;
    Message->MinY = PosY + 20.0f + Padding; //we can use 20.0f because there is only one font size.
    Message->MaxY = PosY + Menu.Height * Menu.Scale - Padding;
    
    CloneString(String, Message->String, GetStringLength(String));
    Message->Flags = OverflowFlags;
}
*/

internal void CreateNewMessage(game_state *GameState, float PosX, float PosY, char *String, unsigned int OverflowFlags, 
                               game_posponed_function Function)
{
    game_message *Message = &GameState->MessageQueue.Messages[GameState->MessageQueue.Count++];
	
    //*Message = {}; // zero initialization
    PokeZeroMem(*Message);
	
	loaded_bitmap Menu = GameState->DEBUGAttackBackground;
	Message->BoxX = PosX;
	Message->BoxY = PosY;
	float Padding = 20.0f; //NOTE: All default messages have a 40.0 pixel padding.
	Message->MinX = PosX - Menu.Width * Menu.Scale / 2.0f + Padding;
	Message->MaxX = PosX + Menu.Width * Menu.Scale / 2.0f - Padding;
	Message->MinY = PosY + 20.0f + Padding; //we can use 20.0f because there is only one font size.
	Message->MaxY = PosY + Menu.Height * Menu.Scale - Padding;
	Message->Function = Function;
	
	CloneString(String, Message->String, GetStringLength(String));
	Message->Flags = OverflowFlags;
}

#include "PokemonDemo_io.cpp"
#include "PokemonDemo_tile.cpp"
#include "PikaBlue_Functions.cpp"
#include "PokemonDemo_Renderer.cpp"
#include "PikaBlue_Random.cpp"
#include "PikaBlue_Mechanics.cpp"

//NOTE: The file below is a home for all the functions I hate
#include "PikaBlue_Playground.cpp"