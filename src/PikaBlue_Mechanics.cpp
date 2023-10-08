/*
Hi thank you for using the PikaBlue engine!
Use this file to override key game mechanics.
See the docs for more information!
*/

//this function could be functional but I choose not to return some dumb structure 
internal void CalculateNewEV(unsigned int *Value, unsigned int *Source, unsigned int *Dest)
{
	Dest[SPEED] = Source[SPEED] + Value[SPEED];
	Dest[SPDEFENSE] = Source[SPDEFENSE] + Value[SPDEFENSE];
	Dest[SPATTACK] = Source[SPATTACK] + Value[SPATTACK];
	Dest[DEFENSE] = Source[DEFENSE] + Value[DEFENSE];
	Dest[ATTACK] = Source[ATTACK] + Value[ATTACK];
	Dest[HP] = Source[HP] + Value[HP];
}

//NOTE: Yay! Another fully functional function. Awesome.
internal float CalculateEXPGain(float BaseExperience, float TargetLevel, float UserLevel, float TotalPokemonWhoGainEXP)
{
	float TrainerBattle = 1.0f;
	float ExpShare = 1.0f;
    
	float A = TargetLevel * 2.0f + 10.0f;
	float C = TargetLevel + UserLevel + 10.0f;
	float B = (BaseExperience * TargetLevel / 5.0f) * TrainerBattle * ExpShare / TotalPokemonWhoGainEXP;
	
	float LuckyEgg = 1.0f;
	float DiffGame = 1.0f;
	float MaxLevel = (UserLevel >= 100)? 0.0f: 1.0f; //we use this modifier to ensure that pokemon who are level 100 do not gain any more exp
	float Modifiers = LuckyEgg * DiffGame *MaxLevel;
	float EXP = (FloorFloat(FloorFloat(Pow(A, 0.5f) * A * A) * B / FloorFloat(Pow(C, 0.5f) * C * C)) + 1.0f) * Modifiers;
	return EXP;
}

INLINE unsigned int CalcPokemonStatHP(unsigned int B, unsigned int I, unsigned int E, unsigned int L)
{
	unsigned int Result = FloorFloatToInt((2.0F * B + I + E) * L / 100.0f + L + 10.0f);
	return Result;
}

//for now this function is a basic function who determines if we will have a random encounter
//but later I can implement that this function determines which pokemon spawns
internal unsigned int TryForRandomEncounter(game_state *GameState)
{
	unsigned int Result = false;
	float RandomNum = GetRandom(GameState) / 255.0f;
	if (RandomNum <= (1.0f / 8.0f) )
	{
		Result = true;
	}
	return Result;
}

//haha, I have transitioned this function to become functional!
//Moreover, it can be called form more than one place!
internal pokemon_database_data GetPokemonDataFromID(pokemon_database_data *PokemonDatabase, unsigned int ID)
{
	pokemon_database_data Result = {};
	for (int x = 0; x < POKEMON_DATABASE_LENGTH; x++)
	{
		pokemon_database_data Data = PokemonDatabase[x];
		if (Data.NationalDexID == ID)
		{
			Result = Data;
		}
	}
	return Result;
}

//NOTE: The name of this function is ambigous 
internal float ApplyStatStage(float Stat, signed char StatStage)
{
	float Result = Stat;
	switch(StatStage)
	{
		case -6:
		Result = Result * (1.0f / 4.0f);
		break;
		case -5:
		Result = Result * (2.0f / 7.0f);
		break;
		case -4:
		Result = Result * (1.0f / 3.0f);
		break;
		case -3:
		Result = Result * (2.0f / 5.0f);
		break;
		case -2:
		Result = Result * (1.0f / 2.0f);
		break;
		case -1:
		Result = Result * (2.0f / 3.0f);
		break;
		case 0:
		//Nothing happens here
		break;
		case 1:
		Result = Result * (3.0f / 2.0f);
		break;
		case 2:
		Result = Result * 2.0f;
		break;
		case 3:
		Result = Result * (5.0f / 2.0f);
		break;
		case 4:
		Result = Result * 3.0f;
		break;
		case 5:
		Result = Result * (7.0f / 2.0f);
		break;
		case 6:
		Result = Result * 8.0f;
		break;
		default:
		//nothing happens here
		break;
	}
	return Result;
}

//this function by definition is not very functional. Unavoidably so, I believe.
//By that I mean it writes a message to a destination and returns true and false as to whether or not to blit message.
internal unsigned int ApplyEffect(pokemon_database_data *PokemonDatabase, battle_pokemon *User, battle_pokemon *Target, pokemon_move PokemonMove, char *Dest)
{
	unsigned int Result = 0;
	if (PokemonMove.Effect == 1) //lower attack stage
	{
		CloneString("Nothing happened!", Dest, 256);
		Target->AttackStage--;
		if (Target->AttackStage > 6)
		{
			Target->AttackStage = 6;
		}
		else if (Target->AttackStage < -6)
		{
			Target->AttackStage = -6;
		}
		else
		{
			//If we were in bound overwrite.
			GameCatStrings(Target->Pokemon->Nickname, "'s Attack fell!", Dest);
		}		
		Result = 1;
	}
	else if (PokemonMove.Effect == 4) //recover
	{
		pokemon *Pokemon = User->Pokemon;
		pokemon_database_data Data = GetPokemonDataFromID(PokemonDatabase, Pokemon->PokemonID);
		unsigned int MaxHP = CalcPokemonStatHP(Data.BaseStats[HP], Pokemon->IV[HP], Pokemon->IV[HP], Pokemon->Level);
		Pokemon->HP += MaxHP / 2;
		if (Pokemon->HP > MaxHP)
		{
			Pokemon->HP = MaxHP;
		}
	}
	return Result;
}

internal float GetCritical(game_state *GameState)
{
	unsigned int C = 0;
	float Result = 1.0f;
	/*do a bunch of stuff
 //for 0 1/16 chance
 for 1 1/8
 for 2 1/4
 for 3 1/3
 for 4+ 1/2	
 */
	float RandomNum = GetRandom(GameState) / 255.0f;
	switch(C)
	{
		case 0:
		{
			if (RandomNum <= 1.0f / 16.0f)
			{
				Result = 1.5f;
			}
		}break;
		case 1:
		{
			if (RandomNum <= 1.0f / 8.0f)
			{
				Result = 1.5f;
			}
		}break;
		case 2:
		{
			if (RandomNum <= 1.0f / 4.0f)
			{
				Result = 1.5f;
			}
		}break;
		case 3:
		{
			if (RandomNum <= 1.0f / 3.0f)
			{
				Result = 1.5f;
			}
		}break;
		case 4:
		{
			if (RandomNum <= 1.0f / 2.0f)
			{
				Result = 1.5f;
			}
		}break;
		default:
		{	
			if (RandomNum <= 1.0f / 2.0f)
			{
				Result = 1.5f;
			}
		}break;
	}
    
	return Result;
}

//this function will return the flag 
internal unsigned int DetermineUserFirst(game_state *GameState, battle_pokemon User, battle_pokemon Target)
{
	unsigned int Result = 0;
	pokemon_move UserMove = User.Pokemon->Moves[User.SelectedMove];
	pokemon_move TargetMove = Target.Pokemon->Moves[Target.SelectedMove]; 
	/*
 1.Whoever is switching goes first.
 2.Anyone with a higher priority move goes first.
 3.Anyone with a quick claw that is activated or someone who has consumed a custap berry goes first.
 4.If they have a full incense or lagging tail then they move later.
 5.If they have stall and the opponent does not have a full incense or lagging tail they move later.
 6.Otherwise, pokemon move in descending order of speed stat, unless trick room is in effect which will make pokemon move in ascending order.
 7.If you got this far, and the pokemon have the same speed stat, break the tie at random
 */
	if (UserMove.Priority > TargetMove.Priority)
	{
		Result = USERFIRST;
	}
	else if (User.Speed > Target.Speed)
	{
		Result = USERFIRST;
	}
	else if (User.Speed == Target.Speed)
	{
		float Random = GetRandom(GameState) / 255.0f;
		if (Random <= 0.5f)
		{
			Result = USERFIRST;
		}
	}
	return Result;
}

internal unsigned int AccuracyCheck(game_state *GameState, pokemon_move PokemonMove)
{
	unsigned int Result = 0;
	float Random = GetRandom(GameState) / 255.0f;
	float CalculatedAccuracy = PokemonMove.Accuracy / 100.0f;
	if (Random <= CalculatedAccuracy)
	{
		Result = true;
	}
	return Result;
}

internal float CalcDamage(game_state *GameState, battle_pokemon Attacker, pokemon_move PokemonMove, battle_pokemon Target, unsigned int *Flags)
{
	float Level = Attacker.Pokemon->Level;
	float A = 0.0f;
	float D = 0.0f;
    
	if (PokemonMove.Flags & PHYSICAL)
	{
		A = (float)Attacker.Attack;
		A = ApplyStatStage(A, Attacker.AttackStage);
		D = (float)Target.Defense;
		D = ApplyStatStage(D, Target.DefenseStage); 
	}
	else
	{
		A = (float)Attacker.SpAttack;
		A = ApplyStatStage(A, Attacker.SpAttackStage);
		D = (float)Target.SpDefense;
		D = ApplyStatStage(D, Target.SpDefenseStage); 
	}
    
	//float Modifier = targets * weather * badge * critical * random * STAB * type * burn * other;
	//NOTE: We should implement a random class so that we can use randomness for important things.
	//Very nescessary things.
	float Critical = GetCritical(GameState);
	if (Critical == 1.5f)
	{
		*Flags = *Flags | CRITICAL;
	}
	float Random = GetRandom(GameState) / 255.0f;
	if (Random < 0.8f)
	{
		Random = 0.8f;
	}
    
	pokemon_database_data AttackerData = GetPokemonDataFromID(GameState->PokemonDatabase, Attacker.Pokemon->PokemonID);
	pokemon_database_data TargetData = GetPokemonDataFromID(GameState->PokemonDatabase, Target.Pokemon->PokemonID);
	float STAB = ( (AttackerData.Types[0] == PokemonMove.Type) | (AttackerData.Types[1] == PokemonMove.Type) )?2.0f: 1.0f; //same type of move as user type bonus
	
	// TODO: so what we are doing here is just not the play. we can fix this by changing the value type of the type matchup database.
	// we can just have this encode some integer which directly gives the code of NOTEFFECTIVE, NOTVERYEFFECTIVE, etc.
	
	unsigned int TypeBonus1 = GameState->TypeMatchupDatabase[PokemonMove.Type][TargetData.Types[0]];
	unsigned int TypeBonus2 = GameState->TypeMatchupDatabase[PokemonMove.Type][TargetData.Types[1]];

	unsigned int FlagsTable[4][4] = {
		{ NOTEFFECTIVE, NOTEFFECTIVE,     NOTEFFECTIVE,     NOTEFFECTIVE },   // 0x
		{ NOTEFFECTIVE, NOTVERYEFFECTIVE, NOTVERYEFFECTIVE, 0 },              // 0.5x
		{ NOTEFFECTIVE, NOTVERYEFFECTIVE, 0,                SUPEREFFECTIVE },  // 1x
		{ NOTEFFECTIVE, 0,                SUPEREFFECTIVE,   SUPEREFFECTIVE }, // 2x
	};

	float DamageModifierTable[4][4] = {
		{ 0.f, 0.f,   0.f,  0.f }, // 0x
		{ 0.f, 0.25f, 0.5f, 1.f }, // 0.5x
		{ 0.f, 0.5f,  1.f,  2.f },  // 1x
		{ 0.f, 1.f,   2.f,  4.f },  // 2x
	};

	*Flags |= FlagsTable[TypeBonus1][TypeBonus2];
    
	float Modifier = Critical * Random * STAB * DamageModifierTable[TypeBonus1][TypeBonus2];  
	float Damage = (FloorFloat(FloorFloat(FloorFloat(2.0f * Level / 5.0f + 2.0f) * PokemonMove.BasePower * A / D) / 50.0f) + 2.0f) * Modifier;
	return Damage;
}

INLINE unsigned int GetPokemonExperience(char Level)
{
	//right now I only implement the medium fast function
	return (unsigned int)FloorFloatToInt(Pow((float)Level,3.0f));
}

INLINE char CalcPokemonLevel(unsigned int Experience)
{
	//here we implement the medium fast function
	char Level = (char)FloorFloatToInt(Pow((float)Experience, 1.0f / 3.0f));
	if (Level > 100) {Level = 100;} //here we cap the level to 100. So no pokemon have a level greater than 100
	return Level;
}

INLINE unsigned int CalcPokemonStat(unsigned int B, unsigned int I, unsigned int E, unsigned int L, float N)
{
	unsigned int Result = FloorFloatToInt(FloorFloat((2.0f * B + I + E) * L / 100.0f + 5) * N);
	return Result;
}

internal void UpdatePokemonLevel(pokemon *Pokemon, unsigned int Experience)
{
	Pokemon->Experience += Experience;
	Pokemon->Level = CalcPokemonLevel(Pokemon->Experience);
	if(!Pokemon->Level) //If the pokemon is at level 0
	{
		Pokemon->Level = 1;
	}
}

internal void ExecuteTurn(game_state *GameState, battle_pokemon *Attacker, battle_pokemon *Target, game_screen_position ScreenPos)
{
	char StringBuffer[256];
	pokemon_move *PokemonMove = &Attacker->Pokemon->Moves[Attacker->SelectedMove];
	if (AccuracyCheck(GameState, *PokemonMove))
	{
		if (PokemonMove->BasePower)
		{
			unsigned int Flags = 0;
			float Damage = CalcDamage(GameState, *Attacker, *PokemonMove, *Target, &Flags);
			SafeSubtract(Target->Pokemon->HP, (unsigned int)Damage);
			if (Target->Pokemon->HP == 0)
			{
				GameState->StateFlags = GameState->StateFlags | FAINT; 	
			}
            
			PlaySoundEffect(GameState, GameState->Hit2, false, 1.0f);
            
			GameCatStrings(Attacker->Pokemon->Nickname," used ", StringBuffer);
			GameCatStrings(StringBuffer, PokemonMove->Name, StringBuffer); 
			CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y,
                             GameCatStrings(StringBuffer, "!", StringBuffer), 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
            
			if ( Flags & SUPEREFFECTIVE )
			{
				CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y, "It was super effective!", 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
			}
			else if (Flags & NOTVERYEFFECTIVE)
			{
				CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y, "It was not very effective!",0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
			}
			else if (Flags & NOTEFFECTIVE)
			{
				CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y, "It had no effect!", 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
			}
            
			if ( (Flags & CRITICAL) && !(Flags & NOTEFFECTIVE) )
			{
				CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y, "It was a critical hit!", 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
			}
		}
		else
		{
			GameCatStrings(Attacker->Pokemon->Nickname," used ", StringBuffer);
			GameCatStrings(StringBuffer, PokemonMove->Name, StringBuffer); 
			CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y, GameCatStrings(StringBuffer, "!", StringBuffer), 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
            
			//the move is a status move so do status things!
			if ( ApplyEffect(GameState->PokemonDatabase, Attacker, Target, *PokemonMove, StringBuffer) )
			{
				CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y, StringBuffer, 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
			}					
		}
		SafeSubtract(PokemonMove->PP,1);					
	}
	else
	{
		CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y, 
                         GameCatStrings(Attacker->Pokemon->Nickname, " missed!", StringBuffer), 0, NULL_GAME_FUNCTION, NULL_PLAYER,NULL_PLAYER);
	}
}

internal loaded_wav GetPokemonCry(game_state *GameState, unsigned int ID)
{
	loaded_wav Result = {};
	//NOTE: We harcoded the amount of pokemon we have created
	for (unsigned int x = 0; x < 4; x++)
	{
		pokemon_cry *Cry = &GameState->Cries[x];
		if (Cry->ID == ID)
		{
			Result = Cry->Wave;
		}
	}
	return Result;
}

//sorry this function is not functional, sucks.
internal void OnPokemonFainted(game_state *GameState, game_screen_position ScreenPos, battle_pokemon User, battle_pokemon Target, unsigned int Won)
{
	char StringBuffer[256];
	
	pokemon *UserPokemon = User.Pokemon;
	pokemon *TargetPokemon = Target.Pokemon;
    
	PlaySoundEffect(GameState, GetPokemonCry(GameState, TargetPokemon->PokemonID), false, 1.0f);
	if (Won)
	{
		game_posponed_function Function = {}; Function.Function = KillSoundThenPlay; Function.Data = GameState; 
		Function.Param = 8;
		CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y,
                         GameCatStrings(TargetPokemon->Nickname," fainted!", StringBuffer), 0, Function, NULL_PLAYER, NULL_PLAYER);
        
        if (TargetPokemon->overworldEntity != NULL) {
            // delete the entity!
            DeleteEntity(GameState, TargetPokemon->overworldEntity);
        }
	}
	else
	{
		CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y,
                         GameCatStrings(TargetPokemon->Nickname," fainted!", StringBuffer), 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
	}
	
	if (Won)
	{	
		pokemon_database_data EnemyData = GetPokemonDataFromID(GameState->PokemonDatabase, TargetPokemon->PokemonID);
		CalculateNewEV(EnemyData.EvYeilds, UserPokemon->EV, UserPokemon->EV); //update the EV of our pokemon
        
		float EXPGain = CalculateEXPGain((float)EnemyData.BaseExperienceYield, (float)TargetPokemon->Level, (float)UserPokemon->Level, 1.0f);
		GameState->FloatBuffer[0] = EXPGain;
		GameState->FloatBuffer[2] = EXPGain / (0.5f * 30.0f);
        
		char NewMessage[256];
		GameCatStrings(UserPokemon->Nickname, " gained ", NewMessage);
		GameCatStrings(NewMessage, NumberToASCII((unsigned int)EXPGain, StringBuffer), NewMessage);
		game_posponed_function Function = {}; Function.Function = PlaySoundEffectBare; Function.Data = GameState; 
		Function.Param = 9;
		CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y, 
                         GameCatStrings(NewMessage, " experience points!", NewMessage), 0, Function, NULL_PLAYER, NULL_PLAYER);
	} 
    
	if (!Won)
	{
		CreateNewMessage(GameState, ScreenPos.X, ScreenPos.Y, "Red blacked out!", 0, NULL_GAME_FUNCTION, NULL_PLAYER, NULL_PLAYER);
	}
}