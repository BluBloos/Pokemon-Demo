//I love this function just make sure to do a pass
internal void PosponeSuperFunction(game_state *GameState, float WaitLength, game_posponed_function PosponedFunction)
{
	PosponedFunction.Length = WaitLength;
	PosponedFunction.Timer = GameState->GameTimer;
	GameState->SuperFunctionQueue.Functions[GameState->SuperFunctionQueue.Count++] = PosponedFunction;
}
//NOTE: The function above is only in this file by necesity.

internal void PopMessage(void *Data, unsigned int QueueIndex)
{
	message_queue *MessageQueue = (message_queue *)Data;
	unsigned int LastIndex = MessageQueue->Count - 1;
	if(MessageQueue->Count > 0)
	{
		//if the message has a function execute it!
		if (MessageQueue->Messages[QueueIndex].Function.Function)
		{
			MessageQueue->Messages[QueueIndex].Function.Function(MessageQueue->Messages[QueueIndex].Function.Data, MessageQueue->Messages[QueueIndex].Function.Param);
		}
        
		//clear the message
        //MessageQueue->Messages[QueueIndex] = {};
        PokeZeroMem(MessageQueue->Messages[QueueIndex]);
        
        for (unsigned int x = 0; x < LastIndex - QueueIndex; x++)
		{
			MessageQueue->Messages[QueueIndex + x] = MessageQueue->Messages[QueueIndex + x + 1];
		}
		MessageQueue->Count--;
	}
}

//NOTE: This function is just wierd, I don't like it
internal loaded_bitmap GetSpriteFromSpriteMapIgnoreVert(memory_arena *WorldArena,
                                                        loaded_bitmap *SpriteMap, unsigned int *PixelPointer, unsigned int TileSize)
{
	unsigned int MinX = TileSize;
	unsigned int MaxX = 0;
    
	unsigned int *Row = PixelPointer;
	unsigned int BackgroundColor = *Row;
    
	for (unsigned int Y = 0; Y < TileSize;++Y)
	{
		unsigned int *Pixel = (unsigned int *)Row;
		for (unsigned int X = 0; X < TileSize; ++X)
		{
			if (*Pixel != BackgroundColor)
			{
				//so I found something that actually has valid data.
				if (X < MinX)
				{
					MinX = X;
				}
				if (X > MaxX)
				{
					MaxX = X;
				}
			}
			Pixel++;
		}
		Row -= SpriteMap->Width;
	}
    
	unsigned int *NewRow = PixelPointer + MinX;
    
	return GetSpriteByRect(WorldArena, SpriteMap, NewRow, 1 + MaxX - MinX, TileSize, BackgroundColor);
}

//NOTE: This function is super dumb what the hell? Please, we must find a way to improve this.
internal animation MapNPCSpritesToAnimation(loaded_bitmap TemporyBitmaps[9])
{
	animation Result = {};
	Result.Frames[0] = TemporyBitmaps[1];
	Result.Frames[1] = TemporyBitmaps[3];
	Result.Frames[2] = TemporyBitmaps[1];
	Result.Frames[3] = TemporyBitmaps[4];
	Result.Frames[4] = TemporyBitmaps[0];
	Result.Frames[5] = TemporyBitmaps[5];
	Result.Frames[6] = TemporyBitmaps[0];
	Result.Frames[7] = TemporyBitmaps[6];
	Result.Frames[8] = TemporyBitmaps[2];
	Result.Frames[9] = TemporyBitmaps[7];
	Result.Frames[10] = TemporyBitmaps[2];
	Result.Frames[11] = TemporyBitmaps[8];
	return Result;
}

//NOTE: I don't think function belongs inside this file
internal animation_player LoadNPC(memory_arena *WorldArena, loaded_bitmap *SpriteMap, int CharacterRow, int CharacterIndex)
{
	loaded_bitmap TemporyBitmaps[9];
	animation_player Result = {};
    
	if (CharacterIndex > 17)
	{
		CharacterIndex = 17;
	}
	if (CharacterIndex < 1)
	{
		CharacterIndex = 1;
	}
	if (CharacterRow > 6)
	{
		CharacterRow = 6;
	}
	if (CharacterRow < 1)
	{
		CharacterRow = 1;
	}
    
	unsigned int *CharacterPointer = SpriteMap->PixelPointer + SpriteMap->Width * (SpriteMap->Height - 1);
	CharacterPointer = CharacterPointer - (CharacterRow - 1) * 156672 + (CharacterIndex - 1) * 32;
	for (int SpriteIndex = 0; SpriteIndex < 9; SpriteIndex++)
	{
		TemporyBitmaps[SpriteIndex] = GetSpriteFromSpriteMap(WorldArena, SpriteMap, CharacterPointer, 32);
		CharacterPointer -= 32 * SpriteMap->Width;
	}
    
	Result.Animations[0] = MapNPCSpritesToAnimation(TemporyBitmaps);
	return Result;
}

//NOTE: This function has some oddities in it. For example, when we blit a message we do checking to determine
//if we are in the overworld? I don't know if that's okay. I think that the function needs to be much more gerneral.
//Moreover, the function is not engine specific, so it doesn't belong in the file.
internal void UpdateMessage(game_offscreen_buffer *buffer, game_message *Message, game_state *GameState)
{
	if (GameState->GameState == OVERWORLD)
	{
		float Width = GameState->DEBUGAttackBackground.Width * 2.0f;
		float Height = GameState->DEBUGAttackBackground.Height * 2.0f;
        
		float MinX = Message->BoxX - Width / 2.0f;
		float MaxX = MinX + Width;
		float MinY = Message->BoxY;
		float MaxY = MinY + Height;
        
		DrawBitmapScaled(buffer, GameState->DEBUGAttackBackground, MinX, MaxX,
                         MinY, MaxY, FLIPFALSE, 2);
	}
    
	BlitStringBounds(buffer, GameState->PokemonDemoFont, Message->MinX, Message->MaxX,
                     Message->MinY, Message->MaxY, Message->String, Message->CurrentLength, 0);
    
	if(!(Message->Flags & HOLD))
	{
		if ( !(Message->CurrentLength + 1 > GetStringLength(Message->String)) )
		{
			Message->CurrentLength++;
		}
		else
		{
			Message->Flags = Message->Flags | HOLD;
			if ( !(Message->Flags & OVERRIDEWAIT) )
			{
				game_posponed_function PosponedFunction = {};
				PosponedFunction.Function = PopMessage;
				PosponedFunction.Data = (void *)&GameState->MessageQueue;
				PosponedFunction.Param = 0;
				PosponeSuperFunction(GameState, 1.0f, PosponedFunction);
			}
		}
	}
    
	if(Message->Flags & KILLMESSAGE)
	{
		//Now the message will kill itself
		//do you know whats interesting about a message who is being updated?
		//they are automatically the 0th index in the list. No need to store indices!
		PopMessage(&GameState->MessageQueue, 0);
	}
}

//Okay this function is very very dumb and should die.
internal void DEBUGCreateMoveDataBase(memory_arena *WorldArena, debug_platform_write_entire_file *WriteEntireFile, char *FileName)
{
	pokemon_move Moves[12];
	pokemon_move RunningMove = {};
    
	RunningMove.Type = NORMAL;
	RunningMove.BasePower = 40;
	RunningMove.Accuracy = 100;
	RunningMove.PP = 35;
	RunningMove.Flags = PHYSICAL | CONTACT;
	CloneString("No Description.", RunningMove.Description, 256);
	CloneString("TACKLE", RunningMove.Name, 15);
	Moves[0] = RunningMove;
    //RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = GRASS;
	RunningMove.BasePower = 80;
	RunningMove.Accuracy = 100;
	RunningMove.PP = 15;
	RunningMove.Flags = PHYSICAL;
	CloneString("SEED BOMB", RunningMove.Name, 15);
	Moves[1] = RunningMove;
	//RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = GRASS;
	RunningMove.Accuracy = 90;
	RunningMove.PP = 10;
	CloneString("LEECH SEED", RunningMove.Name, 15);
	Moves[2] = RunningMove;
	//RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = NORMAL;
	RunningMove.Accuracy = 100;
	RunningMove.PP = 40;
	RunningMove.Effect = 1;
	CloneString("GROWL", RunningMove.Name, 5);
	Moves[3] = RunningMove;
	//RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = STEEL;
	RunningMove.Accuracy = 75;
	RunningMove.BasePower = 100;
	RunningMove.PP = 15;
	RunningMove.Flags = PHYSICAL | CONTACT;
	RunningMove.Effect = 2; //30 percent change of lowering opponent defense stage by 1
	CloneString("IRON TAIL", RunningMove.Name, 15);
	Moves[4] = RunningMove;
	//RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = ELECTRIC;
	RunningMove.Accuracy = 100;
	RunningMove.BasePower = 40; //this varies
	RunningMove.PP = 10;
	CloneString("ELECTRO BALL", RunningMove.Name, 15);
	Moves[5] = RunningMove;
	//RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = ELECTRIC;
	RunningMove.Accuracy = 100;
	RunningMove.BasePower = 90;
	RunningMove.PP = 15;
	RunningMove.Effect = 3; //10 percent change of paralyzing target
	CloneString("THUNDERBOLT", RunningMove.Name, 15);
	Moves[6] = RunningMove;
    //RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = NORMAL;
	RunningMove.Accuracy = 100;
	RunningMove.BasePower = 40;
	RunningMove.PP = 30;
	RunningMove.Priority = 1;
	RunningMove.Flags = PHYSICAL | CONTACT;
	CloneString("QUICK ATTACK", RunningMove.Name, 15);
	Moves[7] = RunningMove;
	//RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = NORMAL;
	RunningMove.Accuracy = 100;
	RunningMove.BasePower = 100;
	RunningMove.PP = 10;
	CloneString("JUDGMENT", RunningMove.Name, 15);
	Moves[8] = RunningMove;
	//RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = PSYCHIC;
	RunningMove.Accuracy = 100;
	RunningMove.BasePower = 90;
	RunningMove.PP = 10;
	CloneString("PSYCHIC", RunningMove.Name, 15);
	Moves[9] = RunningMove;
	//RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = ICE;
	RunningMove.Accuracy = 100;
	RunningMove.BasePower = 90;
	RunningMove.PP = 10;
	CloneString("ICE BEAM", RunningMove.Name, 15);
	Moves[10] = RunningMove;
	//RunningMove = {};
    PokeZeroMem(RunningMove);
    
	RunningMove.Type = NORMAL;
	RunningMove.Accuracy = 100;
	RunningMove.BasePower = 0;
	RunningMove.PP = 10;
	RunningMove.Effect = 4; //effect of recover
	CloneString("RECOVER", RunningMove.Name, 15);
	Moves[11] = RunningMove;
	//RunningMove = {};
    PokeZeroMem(RunningMove);
    
	WriteEntireFile(FileName, sizeof(pokemon_move) * 12, Moves);
}

// Another dumb function
internal void LoadPokemonDatabase(game_state *GameState, debug_platform_read_entire_file *ReadEntireFile, char *FileName)
{
	debug_read_file_result Result = ReadEntireFile(FileName);
	pokemon_move *RunningMove = (pokemon_move *)Result.Contents;
    
	for (unsigned int x = 0; x < Result.ContentSize / sizeof(pokemon_move); x++)
	{
		GameState->MoveDatabase[x] = *RunningMove++;
	}
}

//This is a dumb function. It exists because we decided it was not nescessary to store a pokemons maxHP.
// How is the maxHP different from every other stat? It must be stored. It's kinda like how we don't need to store
// the level but we do.
INLINE unsigned int GetPokemonStatHP(game_state *GameState, pokemon *Pokemon)
{
	pokemon_database_data PokemonData = GetPokemonDataFromID(GameState->PokemonDatabase, Pokemon->PokemonID);
	return CalcPokemonStatHP(PokemonData.BaseStats[HP], Pokemon->IV[HP],
                             Pokemon->EV[HP], Pokemon->Level);
}

// This function does not belong in this file
internal void CanonocalizeBattlePokemonStats(game_state *GameState, battle_pokemon *BattlePokemon)
{
	pokemon_database_data PokemonData = GetPokemonDataFromID(GameState->PokemonDatabase, BattlePokemon->Pokemon->PokemonID);
	pokemon *Pokemon = BattlePokemon->Pokemon;
	BattlePokemon->Speed = CalcPokemonStat(PokemonData.BaseStats[SPEED], Pokemon->IV[SPEED],
                                           Pokemon->EV[SPEED], Pokemon->Level, GameState->NatureDatabase[Pokemon->Nature][SPEED]);
	BattlePokemon->SpDefense = CalcPokemonStat(PokemonData.BaseStats[SPDEFENSE], Pokemon->IV[SPDEFENSE],
                                               Pokemon->EV[SPDEFENSE], Pokemon->Level, GameState->NatureDatabase[Pokemon->Nature][SPEED]);
	BattlePokemon->SpAttack = CalcPokemonStat(PokemonData.BaseStats[SPATTACK], Pokemon->IV[SPATTACK],
                                              Pokemon->EV[SPATTACK], Pokemon->Level, GameState->NatureDatabase[Pokemon->Nature][SPEED]);
	BattlePokemon->Attack = CalcPokemonStat(PokemonData.BaseStats[ATTACK], Pokemon->IV[ATTACK],
                                            Pokemon->EV[ATTACK], Pokemon->Level, GameState->NatureDatabase[Pokemon->Nature][SPEED]);
	BattlePokemon->Defense = CalcPokemonStat(PokemonData.BaseStats[DEFENSE], Pokemon->IV[DEFENSE],
                                             Pokemon->EV[DEFENSE], Pokemon->Level, GameState->NatureDatabase[Pokemon->Nature][SPEED]);
    
}

//This function does not belong in this file
INLINE battle_pokemon BattlePokemonFromPokemon(game_state *GameState, pokemon *Pokemon)
{
	battle_pokemon Result = {};
	Result.Pokemon = Pokemon;
	Result.Experience = Pokemon->Experience;
	Result.Ability = Pokemon->Ability;
    
	CanonocalizeBattlePokemonStats(GameState, &Result);
    
	return Result;
}

//This function really does not belong in this file
INLINE color GetHealthColor(game_state *GameState, float Value)
{
	color Result = {};
	loaded_bitmap LUT = GameState->HealthLUT;
	unsigned int *Pixel = LUT.PixelPointer + LUT.Width * (LUT.Height - 1);
	if (Value > 0.5f)
	{
		//we should display green
		Result.R = ((*Pixel >> 16) & 0xFF) / 255.0f;
		Result.G = ((*Pixel >> 8) & 0xFF) / 255.0f;
		Result.B = (*Pixel & 0xFF) / 255.0f;
        
	}
	else if ( (Value <= 0.5f) && (Value > 0.2f) )
	{
		//we should display yellow
		Pixel += 5;
		Result.R = ((*Pixel >> 16) & 0xFF) / 255.0f;
		Result.G = ((*Pixel >> 8) & 0xFF) / 255.0f;
		Result.B = (*Pixel & 0xFF) / 255.0f;
	}
	else if (Value <= 0.2f)
	{
		//we should display red
		Pixel += 10;
		Result.R = ((*Pixel >> 16) & 0xFF) / 255.0f;
		Result.G = ((*Pixel >> 8) & 0xFF) / 255.0f;
		Result.B = (*Pixel & 0xFF) / 255.0f;
	}
	return Result;
}

// This function needs to be reconsidered because surely there is a better way to program than to just write out fuc**ng constants.
internal void UpdatePokemonUI(game_offscreen_buffer *buffer, game_state *GameState, battle_pokemon *Pokemon, float MinX, float MaxX, float MinY, float MaxY)
{
	pokemon_database_data PokemonData = GetPokemonDataFromID(GameState->PokemonDatabase, Pokemon->Pokemon->PokemonID);
    
	//NICKNAME
	BlitStringBoundless(buffer, GameState->PokemonDemoFont, MinX + 48.0f, MinY + 34.0f, Pokemon->Pokemon->Nickname, 1);
	//NICKNAME
    
	unsigned int Health = Pokemon->Pokemon->HP;
	unsigned int MaxHealth = CalcPokemonStatHP(PokemonData.BaseStats[HP], Pokemon->Pokemon->IV[HP],
                                               Pokemon->Pokemon->EV[HP], Pokemon->Pokemon->Level);
	char Level = Pokemon->Pokemon->Level;
	int Experience = Pokemon->Pokemon->Experience;
    
	//HEALTHSLIDER
	float HealthSliderMinX = MinX + 144.0f;
	float HealthSliderMaxX = MaxX - 16.0f;
	float HealthSliderMinY = MinY + 48.0f;
	float HealthSliderMaxY = MaxY - 32.0f;
	float HealthSliderValue = (float)Health / (float)MaxHealth;
	float HealthSliderFinalMaxX = (HealthSliderValue) * (HealthSliderMaxX - HealthSliderMinX) + HealthSliderMinX;
	HealthSliderFinalMaxX = (HealthSliderFinalMaxX > HealthSliderMaxX)? HealthSliderMaxX: HealthSliderFinalMaxX;
	color HealthColor = GetHealthColor(GameState, HealthSliderValue);
	DrawRect(buffer, HealthSliderMinX, HealthSliderFinalMaxX,
             HealthSliderMinY, HealthSliderMaxY, HealthColor.R, HealthColor.G, HealthColor.B);
    
	//EXPERIENCE
	float CurrentLevelExperience = (float)GetPokemonExperience(Level);
	float DeltaExperience = (float)Experience - CurrentLevelExperience;
	float ToNextLevelExperience = GetPokemonExperience(Level + 1) - CurrentLevelExperience;
	float ExpSliderMinX = MinX + 48.0f;
	float ExpSliderMaxX = MaxX - 16.0f;
	float ExpSliderMinY = MaxY - 6.0f;
	float ExpSliderMaxY = MaxY - 4.0f;
	float ExpSliderValue = DeltaExperience / ToNextLevelExperience;
	//In the future sample this color value from a color pallete.
	DrawRect(buffer, ExpSliderMinX, (ExpSliderValue) * (ExpSliderMaxX - ExpSliderMinX) + ExpSliderMinX,
             ExpSliderMinY, ExpSliderMaxY, 0.377f, 0.627f, 0.784f);
    
	//MAXHEALTH
	float MaxHealthMinX = MaxX - 64.0f;
	char MaxHealthBuffer[256];
	NumberToASCII(MaxHealth, MaxHealthBuffer);
	BlitStringBoundless(buffer, GameState->PokemonDemoFont, MaxHealthMinX, MinY + 77.0f, MaxHealthBuffer, 1);
    
	//HEALTH
	float MaxHealthMaxX = MaxX - 78.0f;
	char HealthBuffer[256];
	NumberToASCII(Health, HealthBuffer);
	BlitStringReverseBoundless(buffer, GameState->PokemonDemoFont, MaxHealthMaxX, MinY + 77.0f, HealthBuffer, 1);
    
	//GENDER
	loaded_bitmap Gender = GameState->Genders[Pokemon->Pokemon->Gender];
	float GenderMinX = MinX + 176.0f;
	float GenderMaxX = GenderMinX + Gender.Width * 2;
	float GenderMinY = MinY + 16.0f;
	float GenderMaxY = GenderMinY + Gender.Height * 2;
	DrawBitmapScaled(buffer, Gender, GenderMinX, GenderMaxX, GenderMinY, GenderMaxY, FLIPFALSE, 2);
    
	//LEVEL
	char LevelBuffer[256];
	NumberToASCII((int)Level, LevelBuffer);
	float LevelMinX = MinX + 208.0f;
	float LevelMinY = MinY + 34.0f;
	BlitStringBoundless(buffer, GameState->PokemonDemoFont, LevelMinX, LevelMinY, LevelBuffer, 1);
}

//THis function needs to be reconsidered because surely there is a better way to program than to just write out fucking constants.
internal void UpdatePokemonUI2(game_offscreen_buffer *buffer, game_state *GameState, battle_pokemon *Pokemon, float MinX, float MaxX, float MinY, float MaxY)
{
	pokemon_database_data PokemonData = GetPokemonDataFromID(GameState->PokemonDatabase, Pokemon->Pokemon->PokemonID);
    
	//NICKNAME
	BlitStringBoundless(buffer, GameState->PokemonDemoFont, MinX + 2.0f, MinY + 32.0f, Pokemon->Pokemon->Nickname, 1);
	//NICKNAME
    
	unsigned int Health = Pokemon->Pokemon->HP;
	unsigned int MaxHealth = CalcPokemonStatHP(PokemonData.BaseStats[HP], Pokemon->Pokemon->IV[HP],
                                               Pokemon->Pokemon->EV[HP], Pokemon->Pokemon->Level);
    
	float HealthSliderMinX = MinX + 100.0f;
	float HealthSliderMaxX = MaxX - 48.0f;
	float HealthSliderMinY = MinY + 46.0f;
	float HealthSliderMaxY = MaxY - 16.0f;
    
	//HEALTHSLIDER
	float HealthSliderValue = (float)Health / (float)MaxHealth;
	float HealthSliderFinalMaxX = (HealthSliderValue) * (HealthSliderMaxX - HealthSliderMinX) + HealthSliderMinX;
	HealthSliderFinalMaxX = (HealthSliderFinalMaxX > HealthSliderMaxX)? HealthSliderMaxX: HealthSliderFinalMaxX;
	color HealthColor = GetHealthColor(GameState, HealthSliderValue);
	DrawRect(buffer, HealthSliderMinX, HealthSliderFinalMaxX,
             HealthSliderMinY, HealthSliderMaxY, HealthColor.R, HealthColor.G, HealthColor.B);
	//HEALTHSLIDER
    
	//GENDER
	loaded_bitmap Gender = GameState->Genders[Pokemon->Pokemon->Gender];
	float GenderMinX = MinX + 132.0f;
	float GenderMaxX = GenderMinX + Gender.Width * 2;
	float GenderMaxY = MaxY - 34.0f;
	float GenderMinY = GenderMaxY - Gender.Height * 2;
	DrawBitmapScaled(buffer, Gender, GenderMinX, GenderMaxX, GenderMinY, GenderMaxY, FLIPFALSE, 2);
    
	//LEVEL
	char Level = Pokemon->Pokemon->Level;
	char LevelBuffer[256];
	NumberToASCII((int)Level, LevelBuffer);
	float LevelMinX = MaxX - 80.0f;
	float LevelMinY = MinY + 32.0f;
	BlitStringBoundless(buffer, GameState->PokemonDemoFont, LevelMinX, LevelMinY, LevelBuffer, 1);
}

//Why are there structs in this file?
typedef struct
{
	unsigned int RegularCount;
	unsigned int SpeicalCount;
	unsigned int HeaderCount;
} pokemon_master_animation_header;

typedef struct
{
	unsigned int AnimationType;
	unsigned int TileX;
	unsigned int TileY;
	unsigned int TileCount;
} pokemon_animation_header;

typedef enum
{
	FRONTNORMAL = RIFF_CODE('f','n','0','0'),
	FRONTSPECIAL = RIFF_CODE('f','s','0','0'),
	BACKNORMAL = RIFF_CODE('b','n','0','0'),
	BACKSPECIAL = RIFF_CODE('b','s','0','0')
} AnimHeaderCodes;
////////////////

//This function is dumb
internal void DEBUGCreatePokemonAnimations(memory_arena *WorldArena, debug_platform_write_entire_file *WriteEntireFile, char *BaseFilePath)
{
	char StringBuffer[256];
	//for now im going to have to push the data to my memory, however later im going to make sure that I
	//can rip memory off of my virtual page system. So essentially im going to build a very robust
	//virtul pageing system.
	char *Memory = (char *)PushSize(WorldArena, sizeof(pokemon_master_animation_header) + 4 * sizeof(pokemon_animation_header));
	char *Pointer = Memory;
    
	pokemon_master_animation_header MasterHeader = {};
	MasterHeader.RegularCount = 3;
	MasterHeader.SpeicalCount = 1;
	MasterHeader.HeaderCount = 4;
    
	pokemon_animation_header Headers[4];
	pokemon_animation_header RunningHeader = {};
	RunningHeader.TileX = 40;
	RunningHeader.TileY = 40;
    
	RunningHeader.AnimationType = RIFF_CODE('f','n','0','0');
	RunningHeader.TileCount = 11;
	Headers[0] = RunningHeader;
    
	RunningHeader.AnimationType = RIFF_CODE('f','s','0','0');
	RunningHeader.TileCount = 10;
	Headers[1] = RunningHeader;
    
	RunningHeader.AnimationType = RIFF_CODE('b','n','0','0');
	RunningHeader.TileCount = 12;
	Headers[2] = RunningHeader;
    
	RunningHeader.AnimationType = RIFF_CODE('b','s','0','0');
	RunningHeader.TileCount = 12;
	Headers[3] = RunningHeader;
    
	*(pokemon_master_animation_header *)Pointer = MasterHeader;
	Pointer += sizeof(pokemon_master_animation_header);
	for (int x = 0; x < 4; x++)
	{
		*(pokemon_animation_header *)Pointer = Headers[x];
		Pointer += sizeof(pokemon_animation_header);
	}
    
	WriteEntireFile(GameCatStrings(BaseFilePath, "Data//Bulbasaur.amdt",StringBuffer),
                    sizeof(pokemon_master_animation_header) + sizeof(pokemon_animation_header) * 4, Memory);
    
	Memory = (char *)PushSize(WorldArena, sizeof(pokemon_master_animation_header) + 2 * sizeof(pokemon_animation_header));
	Pointer = Memory;
    
	MasterHeader.RegularCount = 1;
	MasterHeader.SpeicalCount = 0;
	MasterHeader.HeaderCount = 2;
    
	RunningHeader.TileX = 65;
	RunningHeader.TileY = 65;
    
	RunningHeader.AnimationType = RIFF_CODE('f','n','0','0');
	RunningHeader.TileCount = 62;
	Headers[0] = RunningHeader;
    
	RunningHeader.AnimationType = RIFF_CODE('b','n','0','0');
	RunningHeader.TileCount = 62;
	Headers[1] = RunningHeader;
    
	*(pokemon_master_animation_header *)Pointer = MasterHeader;
	Pointer += sizeof(pokemon_master_animation_header);
	for (int x = 0; x < 2; x++)
	{
		*(pokemon_animation_header *)Pointer = Headers[x];
		Pointer += sizeof(pokemon_animation_header);
	}
    
	WriteEntireFile(GameCatStrings(BaseFilePath, "Data//Pikachu.amdt",StringBuffer),
                    sizeof(pokemon_master_animation_header) + sizeof(pokemon_animation_header) * 2, Memory);
    
	Memory = (char *)PushSize(WorldArena, sizeof(pokemon_master_animation_header) + 2 * sizeof(pokemon_animation_header));
	Pointer = Memory;
    
	MasterHeader.RegularCount = 1;
	MasterHeader.SpeicalCount = 0;
	MasterHeader.HeaderCount = 2;
    
	RunningHeader.TileX = 75;
	RunningHeader.TileY = 75;
    
	RunningHeader.AnimationType = RIFF_CODE('f','n','0','0');
	RunningHeader.TileCount = 12;
	Headers[0] = RunningHeader;
    
	RunningHeader.AnimationType = RIFF_CODE('b','n','0','0');
	RunningHeader.TileCount = 12;
	Headers[1] = RunningHeader;
    
	*(pokemon_master_animation_header *)Pointer = MasterHeader;
	Pointer += sizeof(pokemon_master_animation_header);
	for (int x = 0; x < 2; x++)
	{
		*(pokemon_animation_header *)Pointer = Headers[x];
		Pointer += sizeof(pokemon_animation_header);
	}
    
	WriteEntireFile(GameCatStrings(BaseFilePath, "Data//Arceus.amdt",StringBuffer),
                    sizeof(pokemon_master_animation_header) + sizeof(pokemon_animation_header) * 2, Memory);
}

//Canocolize sprite map what?
INLINE void CanonicalizeSpriteMapTiles(unsigned int *TileX, unsigned int *TileY, unsigned int MapTileWidth)
{
	int ActTileX = *TileX + *TileY * MapTileWidth;
	int ActTileY = FloorFloatToInt((float)ActTileX / MapTileWidth);
	*TileY = ActTileY;
	*TileX = ActTileX - ActTileY * MapTileWidth;
}

//I don't even know what this function does
internal animation GenerateAnimationFromChunk(memory_arena *WorldArena, loaded_bitmap SpriteSheet,
                                              pokemon_animation_header Header, unsigned int *PixelPointer, unsigned int TileX, unsigned int TileY)
{
	animation Animation = {};
    
	for (unsigned int x = 0; x < Header.TileCount; x++)
	{
		unsigned int *Pointer = PixelPointer - TileY * Header.TileY * SpriteSheet.Width + TileX++ * Header.TileX;
		CanonicalizeSpriteMapTiles(&TileX, &TileY, SpriteSheet.Width / Header.TileX);
        
		//Note, right now we are only supporting square tile sizes, so in the future we need to fix this issue
		//maybe like very, very soon. Hopefully I don't forget about this issue. That's why I put the note afterall.
		Animation.Frames[x] = GetSpriteFromSpriteMapIgnoreVert(WorldArena, &SpriteSheet, Pointer, Header.TileX);
	}
    
	Animation.MaxFrames = Header.TileCount;
    
	return Animation;
}

//this is dumb
internal void LoadAnimtionHeaders(debug_platform_read_entire_file *ReadEntireFile, char *FileName, pokemon_animation_header *Dest, pokemon_master_animation_header *MasterDest)
{
	debug_read_file_result Result = ReadEntireFile(FileName);
	pokemon_master_animation_header *MasterHeader = (pokemon_master_animation_header *)Result.Contents;
	*MasterDest = *MasterHeader++;
	pokemon_animation_header *Header = (pokemon_animation_header *)MasterHeader;
    
	for (unsigned int x = 0; x < MasterDest->HeaderCount; x++)
	{
		Dest[x] = *Header++;
	}
}

//this is dumb
internal animation_player LoadAnimationPlayer(memory_arena *WorldArena, loaded_bitmap SpriteSheet,
                                              pokemon_animation_header *AnimHeader, unsigned int HeaderCount)
{
	animation_player Result = {};
	animation Animation = {};
    
	//initialize the pixel pointer
	unsigned int TileX = 0;
	unsigned int TileY = 0;
	unsigned int *PixelPointer = SpriteSheet.PixelPointer + SpriteSheet.Width * (SpriteSheet.Height - 1);
    
	for (unsigned int x = 0; x < HeaderCount; x++)
	{
		pokemon_animation_header Header = AnimHeader[x];
		Animation = GenerateAnimationFromChunk(WorldArena, SpriteSheet, Header, PixelPointer, TileX, TileY);
        
		if ( Header.AnimationType == FRONTNORMAL )
		{
			Result.Animations[0] = Animation;
		}
		if (Header.AnimationType == FRONTSPECIAL)
		{
			Result.Animations[1] = Animation;
		}
		if (Header.AnimationType == BACKNORMAL)
		{
			Result.Animations[2] = Animation;
		}
		if (Header.AnimationType == BACKSPECIAL)
		{
			Result.Animations[3] = Animation;
		}
        
		//push the tileX
		TileX += Header.TileCount;
		CanonicalizeSpriteMapTiles(&TileX, &TileY, SpriteSheet.Width / Header.TileX);
	}
    
	return Result;
}

//NOTE: We should consider this function because we may be able to blit this stuff using the UI system
internal void UpdatePokemonAttack(game_offscreen_buffer *buffer,game_state *GameState, pokemon_move PokemonMove,
                                  float MinX, float MaxX, float MinY, float MaxY)
{
	loaded_bitmap AttackBox = GameState->AttackBoxes[PokemonMove.Type];
	DrawBitmapScaled(buffer, AttackBox, MinX, MaxX, MinY, MaxY, FLIPFALSE, 2);
    
	//NAME
	float NameMinY = MinY + 48.0f;
	float NameMinX = MinX + 26.0f;
	BlitStringBoundless(buffer, GameState->PokemonDemoFont, NameMinX, NameMinY, PokemonMove.Name, false);
	///////////
    
	float PPMinY = MinY + 84.0f;
    
	//MAXPP
	float MaxPPMinX = MinX + 184.0f;
	char MaxPP[256]; NumberToASCII(PokemonMove.MaxPP, MaxPP);
	BlitStringBoundless(buffer, GameState->PokemonDemoFont, MaxPPMinX, PPMinY, MaxPP, false);
	//////
    
	//PP
	float PPMaxX = MinX + 172.0f;
	char PP[256]; NumberToASCII(PokemonMove.PP, PP);
	BlitStringReverseBoundless(buffer, GameState->PokemonDemoFont, PPMaxX, PPMinY, PP, false);
	//////
}

// NOTE: The functions below are super janky
// edit: even more so janky now
// NOTE: Mom NPC at index 2.  
internal void HealPokemon2(void *Data, unsigned int Param)
{
	game_state *GameState = (game_state *)Data;
    
	CreateNewMessage(GameState, 640.0f, 460.0f,
                     "Your pokemon were restored to full health!", OVERRIDEWAIT, NULL_GAME_FUNCTION, GameState->AllEntities[Param].npc, GameState->AllEntities[2].npc);
}

// this doesn't belong here
internal void HealPokemon(void *Data, unsigned int Param)
{
	game_state *GameState = (game_state *)Data;
    
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
    
    // Play the healing sound effect
	PlaySoundEffect(GameState, GameState->SoundEffects[2], false, 1.0f);
    
	game_posponed_function Function = {}; Function.Data = GameState;
	Function.Function = HealPokemon2;
    Function.Param = Param;
	PosponeSuperFunction(GameState, 1.0f, Function);
}

// NOTE: We seriously need to reconsider how animations work
internal void LoadPokemonBattleAnimation(debug_platform_read_entire_file *ReadEntireFile, memory_arena *Arena, char *BaseFilePath,
                                         animation_player *Dest, pokemon_database_data PokemonData, unsigned int User)
{
	char StringBuffer[256];
	char BaseFilePathWithName[256];
	GameCatStrings(BaseFilePath, "Data//", BaseFilePathWithName);
	GameCatStrings(BaseFilePathWithName, PokemonData.Name, BaseFilePathWithName);
    
	pokemon_master_animation_header MasterHeader;
	pokemon_animation_header Headers[12];
	LoadAnimtionHeaders(ReadEntireFile, GameCatStrings(BaseFilePathWithName, ".amdt", StringBuffer), Headers, &MasterHeader);
    
	loaded_bitmap SpriteSheet = DEBUGLoadBMP(ReadEntireFile,
                                             GameCatStrings(BaseFilePathWithName, ".bmp", StringBuffer));
	//GameState->DEBUGSheet = SpriteSheet;
    
	*Dest = LoadAnimationPlayer(Arena, SpriteSheet, Headers, MasterHeader.HeaderCount);
	Dest->MaxIter[0] = MasterHeader.RegularCount;
	Dest->MaxIter[1] = MasterHeader.SpeicalCount;
    
	if (User)
	{
		Dest->Offset = 2;
		Dest->Animations[2].Playing = true;
		if (MasterHeader.SpeicalCount)
		{
			Dest->Animations[3].Playing = true;
		}
	}
	else
	{
		Dest->Animations[0].Playing = true;
		if (MasterHeader.SpeicalCount)
		{
			Dest->Animations[1].Playing = true;
		}
	}
}

// Should this function remain specialized? Maybe it needs to be more functional
internal void LoadCries(game_state *GameState, debug_platform_read_entire_file *ReadEntireFile, char *BaseFilePath)
{
	char StringBuffer[256];
	//NOTE: We harcoded the amount of pokemon we have created
	for (unsigned int x = 0; x < 4; x++)
	{
		char FileName[256];
		pokemon_cry *Cry = &GameState->Cries[x];
		Cry->ID = GameState->PokemonDatabase[x].NationalDexID;
		GameCatStrings(BaseFilePath, "Data//cries//", FileName);
		GameCatStrings(FileName, NumberToASCII(Cry->ID, StringBuffer), FileName);
		Cry->Wave = DEBUGLoadWav(ReadEntireFile, GameCatStrings(FileName, ".wav", FileName));
	}
}
