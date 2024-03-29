// PikaBlueCmd.cpp : Defines the entry point for the console application.
#define internal static

#include <Windows.h>
#include <stdio.h>
#include "PokemonDemo.h"
#include "win32_PikaBlue.h"

#define ColorHighlight "\033[0;36m"
#define ColorNormal "\033[0m"

struct parsed_file
{
	char **Lines;
	unsigned int AmountOfLines;
};

static pokemon_move MoveDatabase[256]; //NOTE: I think we should consider this  
static pokemon_database_data PokemonDatabase[256];

internal void LoadMoveDatabase(debug_platform_read_entire_file *ReadEntireFile, char *FileName)
{
	debug_read_file_result Result = ReadEntireFile(FileName);
	pokemon_move *RunningMove = (pokemon_move *)Result.Contents;

	for (unsigned int x = 0; x < Result.ContentSize / sizeof(pokemon_move); x++)
	{
		MoveDatabase[x] = *RunningMove++;
	}	
}

internal pokemon PokemonFromFile(parsed_file File)
{
	pokemon Result = {};
	//so go through all the lines in the file
	for (unsigned int x=0; x< File.AmountOfLines;x++)
	{
		char *Line = File.Lines[x]; 
		if (x == 0){Result.PokemonID = (short)ASCIIToNumber(Line);}
		else if (x == 1){CloneString(Line, Result.Nickname, 16);}
		else if(x == 2)
		{
			unsigned int Experience = GetPokemonExperience((char)ASCIIToNumber(Line));
			UpdatePokemonLevel(&Result, Experience);
		}
		else if(x == 3){Result.Ability = (short)ASCIIToNumber(Line);}
		else if(x == 4){Result.Nature = ASCIIToNumber(Line);}
		else if(x == 5){ SplitLineToInt(Line, Result.IV); }
		else if(x == 6){SplitLineToInt(Line, Result.EV); }
		else if(x == 7){Result.Moves[0] = FillPokemonMove(MoveDatabase, ASCIIToNumber(Line));}
		else if(x == 8){Result.Moves[1] = FillPokemonMove(MoveDatabase, ASCIIToNumber(Line));}
		else if(x == 9){Result.Moves[2] = FillPokemonMove(MoveDatabase, ASCIIToNumber(Line));}
		else if(x == 10){Result.Moves[3] = FillPokemonMove(MoveDatabase, ASCIIToNumber(Line));}
		else if(x == 11){Result.Gender = (char)ASCIIToNumber(Line);}
	}

	pokemon_database_data PokemonData = GetPokemonDataFromID(PokemonDatabase, Result.PokemonID);						
	Result.HP = CalcPokemonStatHP(PokemonData.BaseStats[HP], Result.IV[HP], Result.EV[HP], (unsigned int)Result.Level);

	return Result;
}

internal void BuildPokemonDatabase(memory_arena *Arena, char *BaseFilePath, char *FileName, char **Lines, unsigned int AmountOfLines)
{
	unsigned int Count = 0;
	pokemon_database_data Data[151]; //for now we can have 151 pokemon. 
	//length of struct in lines: 28 
	unsigned int x = 0;
	while (x < AmountOfLines)
	{
		unsigned int LocalLine = x % 28;
		char *Line = Lines[x]; 
		if (LocalLine == 0){Data[Count].NationalDexID = ASCIIToNumber(Line);}
		else if (LocalLine == 1){CloneString(Line, Data[Count].Name, 16);}
		else if(LocalLine == 2){Data[Count].Types[0] = ASCIIToNumber(Line);}
		else if(LocalLine == 3){Data[Count].Types[1] = ASCIIToNumber(Line);}
		else if(LocalLine == 4){Data[Count].Abilities[0] = ASCIIToNumber(Line);}
		else if(LocalLine == 5){Data[Count].Abilities[1] = ASCIIToNumber(Line);}
		else if(LocalLine == 6){Data[Count].GenderRatio = ASCIIToNumber(Line);}
		else if(LocalLine == 7){Data[Count].CatchRate = ASCIIToNumber(Line);}
		else if(LocalLine == 8){Data[Count].EggGroups[0] = ASCIIToNumber(Line);}
		else if(LocalLine == 9){Data[Count].EggGroups[1] = ASCIIToNumber(Line);}
		else if(LocalLine == 10){Data[Count].HatchTime = ASCIIToNumber(Line);}
		else if(LocalLine == 11){Data[Count].Height = ASCIIToFloat(Line);}
		else if(LocalLine == 12){Data[Count].Weight = ASCIIToFloat(Line);}
		else if(LocalLine == 13){Data[Count].BaseExperienceYield = ASCIIToNumber(Line);}
		else if(LocalLine == 14){Data[Count].LevelingRate = ASCIIToNumber(Line);}
		else if(LocalLine == 15){Data[Count].EvYeilds[0] = ASCIIToNumber(Line);}
		else if(LocalLine == 16){Data[Count].EvYeilds[1] = ASCIIToNumber(Line);}
		else if(LocalLine == 17){Data[Count].EvYeilds[2] = ASCIIToNumber(Line);}
		else if(LocalLine == 18){Data[Count].EvYeilds[3] = ASCIIToNumber(Line);}
		else if(LocalLine == 19){Data[Count].EvYeilds[4] = ASCIIToNumber(Line);}
		else if(LocalLine == 20){Data[Count].EvYeilds[5] = ASCIIToNumber(Line);}
		else if(LocalLine == 21){Data[Count].BaseStats[0] = ASCIIToNumber(Line);}
		else if(LocalLine == 22){Data[Count].BaseStats[1] = ASCIIToNumber(Line);}
		else if(LocalLine == 23){Data[Count].BaseStats[2] = ASCIIToNumber(Line);}
		else if(LocalLine == 24){Data[Count].BaseStats[3] = ASCIIToNumber(Line);}
		else if(LocalLine == 25){Data[Count].BaseStats[4] = ASCIIToNumber(Line);}
		else if(LocalLine == 26){Data[Count].BaseStats[5] = ASCIIToNumber(Line);}
		else if(LocalLine == 27){Data[Count].BaseFriendship = ASCIIToNumber(Line);}
		if (++x % 28 == 0)
		{
			Count++;
		}
	}

	BuildAsset(Arena, Data, sizeof(pokemon_database_data), Count, BaseFilePath, FileName);
}

int main()
{
	//LoadMoveDatabase(DEBUGPlatformReadEntireFile, "C:\\dev\\pokemondemo\\Data\\MoveDatabase.dat");
	char StringBuffer[256];

	//below is the code to initialize the memory! Awesome!
	unsigned int StorageSize = MegaBytes(64);
	unsigned char *Memory = (unsigned char *)VirtualAlloc(0,StorageSize,MEM_COMMIT,PAGE_READWRITE);
	memory_arena MemoryArena = {};
	InitializeArena(&MemoryArena, Memory, StorageSize);
	
	//below we initialize the working directory and blit a few welcome messages. Even more awesome!
	char LastString[256];
	char WorkingDirectory[256] = {};
	printf("Hello! Thank you for choosing PikaBlue engine!\nType 'help' for help.\n");
	CloneString("C:\\", WorkingDirectory, 256);
	printf("Working directory set to %s\nType 'cd' to change directories.\n\n", WorkingDirectory);

	//below is the code to load in the pokemon database
	loaded_asset Asset = LoadAsset(DEBUGPlatformReadEntireFile, WorkingDirectory, "PokemonDatabase.dat");
	ParseAsset(Asset, PokemonDatabase, pokemon_database_data);
	
	unsigned int GlobalRunning = true;
	while(GlobalRunning)
	{
		scanf_s("%s", &LastString, 256);
		if ( StringEquals(LastString, "help") )
		{
			printf("\n=== Commands ===\n");
			printf("cd <absoluteDirectory>  - Change directory.\n");
			printf("makeDatabase            - Create pokemon database asset from file.\n");
			printf("makeWorld               - Create world asset from many tilechunk files.\n");
			printf("makeUI                  - Create ui scene asset from many .ui files.\n");
			printf("makePokemon             - Legacy command. Not in use.\n");
			printf("printPok0               - Load in pokemon database asset and print the first pokemon.\n");
			printf("exit                    - Exit the application.\n\n");
		}
		else if ( StringEquals(LastString, "cd")  )
		{
			// printf("Please enter an absolute directory, i.e. not one that would be relative to C:\\\n");
			scanf_s("%s", &WorkingDirectory, 256);
			printf("Working directory set to %s\n", WorkingDirectory);
		}
		else if ( StringEquals(LastString,"makeDatabase") )
		{
			printf("Please enter the file name to read from.\n");
			char FileName[256]; 
			scanf_s("%s", FileName, 256);
			//make save data from file
			char *Dest[PIKABLUE_DATABASE_MAX_LINE_COUNT];
			unsigned int AmountOfLines = 0;
			AmountOfLines = ReadLines(DEBUGPlatformReadEntireFile, &MemoryArena, WorkingDirectory, FileName, Dest);
			printf("Please enter the file name to save to.\n");
			scanf_s("%s", FileName, 256);
			BuildPokemonDatabase(&MemoryArena, WorkingDirectory, FileName, Dest, AmountOfLines);
			
		}
		else if ( StringEquals(LastString, "makeWorld") )
		{
			printf("Please enter the amount of files to read from!\n");
			char FileName[256];
			scanf_s("%s", FileName, 256);

			save_world WorldSave = {};
			unsigned int ChunkCount = ASCIIToNumber(FileName);
			WorldSave.ChunkCount = ChunkCount;

			for (unsigned int x = 0; x < ChunkCount; x++)
			{
				printf("Please enter a file name!\n");
				scanf_s("%s", FileName, 256);

				debug_read_file_result File = DEBUGPlatformReadEntireFile(GameCatStrings(WorkingDirectory, FileName, StringBuffer));
				unsigned int *value = (unsigned int *)File.Contents;
				for (unsigned int j = 0; j < 16; j++)
				{
					for (unsigned int i = 0; i < 16; i++)
					{
						WorldSave.Chunks[x][j][i] = *value++;
					}
				}

				printf("Please enter the x coord!\n");
				scanf_s("%s", FileName, 256);
				WorldSave.ChunkPos[x].x = ASCIIToNumber(FileName);
				printf("Please enter the y coord!\n");
				scanf_s("%s", FileName, 256);
				WorldSave.ChunkPos[x].y = ASCIIToNumber(FileName);
				printf("Please enter the z coord!\n");
				scanf_s("%s", FileName, 256);
				WorldSave.ChunkPos[x].z = ASCIIToNumber(FileName);
			}

			printf("Please enter the file name to save to!\n");
			scanf_s("%s", FileName, 256);
			char *outFileName = GameCatStrings(WorkingDirectory, FileName, StringBuffer);
			DEBUGPlatformWriteEntireFile( outFileName, sizeof(save_world), &WorldSave);
			printf("Wrote world to %s\n", outFileName);

		}
		else if ( StringEquals(LastString, "makeUI") )
		{
			printf("Please enter the amount of files to read from!\n");
			
			char FileName[256];
			game_ui_scene Scenes[10] = {};
			unsigned int LoadedSceneCount = 0;
			scanf_s("%s", FileName, 256);

			unsigned int AssetChunks = ASCIIToNumber(FileName);
			for (unsigned int x = 0; x < AssetChunks; x++)
			{
				printf("Please enter a file name!\n");
				scanf_s("%s", FileName, 256);
				Scenes[LoadedSceneCount++] = *(game_ui_scene *)DEBUGPlatformReadEntireFile(GameCatStrings(WorkingDirectory, FileName, StringBuffer)).Contents;
			}

			printf("Please enter the file name to save to!\n");
			scanf_s("%s", FileName, 256);

			BuildAsset(&MemoryArena, Scenes, sizeof(game_ui_scene), AssetChunks, WorkingDirectory, FileName);
		}
		else if ( StringEquals(LastString, "makePokemon") )
		{
			printf("Please enter the amount of pokemon to build");
			char FileName[256];
			scanf_s("%s", FileName, 256);

			pokemon Pokemons[10] = {};
			unsigned int LoadedPokemonCount = 0;

			unsigned int AssetChunks = ASCIIToNumber(FileName);
			for (unsigned int x = 0; x < AssetChunks; x++)
			{
				printf("Please enter the file to read from!");
				scanf_s("%s", FileName, 256);

				char *Dest[PIKABLUE_DATABASE_MAX_LINE_COUNT];
				unsigned int AmountOfLines = ReadLines(DEBUGPlatformReadEntireFile, &MemoryArena, WorkingDirectory, FileName, Dest);
				parsed_file File = {}; File.AmountOfLines = AmountOfLines; File.Lines = Dest;
				Pokemons[LoadedPokemonCount++] = PokemonFromFile(File);
			}

			printf("Please enter the file name to save to!\n");
			scanf_s("%s", FileName, 256);

			BuildAsset(&MemoryArena, Pokemons, sizeof(pokemon), AssetChunks, WorkingDirectory, FileName);
		}
		else if ( StringEquals(LastString, "printPok0")  )
		{
			printf("Please enter the file name.\n");
			char FileName[256]; 
			scanf_s("%s", FileName, 256);
			loaded_asset Asset = LoadAsset(DEBUGPlatformReadEntireFile, WorkingDirectory, FileName);
			
			// NOTE(Noah): We must ensure that the database array has enough space.
			pokemon_database_data Database[4] = {};
			ParseAsset(Asset, Database, pokemon_database_data);
			
			pokemon_database_data Thing = Database[0];
			printf("NationalDexID: %d\n",Thing.NationalDexID);
			printf("Name: %s\n",&Thing.Name);
			printf("Type: %d\n",Thing.Types[0]);
			printf("Ability 1: %d\n",Thing.Abilities[0]);
			printf("GenderRatio: %d\n",Thing.GenderRatio);
			printf("CatchRate: %d\n",Thing.CatchRate);
			printf("EggGroup 1: %d\n",Thing.EggGroups[0]);
			printf("EggGroup 2: %d\n",Thing.EggGroups[1]);
			printf("HatchTime: %d\n",Thing.HatchTime);
			printf("Height: %f\n",Thing.Height);
			printf("Weight: %f\n",Thing.Weight);
			printf("Base Experience Yield: %d\n",Thing.BaseExperienceYield);
			printf("Leveling Rate: %d\n",Thing.BaseExperienceYield);
			printf("Base Stat 1: %d\n",Thing.BaseStats[0]);
			printf("Base Stat 2: %d\n",Thing.BaseStats[1]);
			printf("Base Stat 3: %d\n",Thing.BaseStats[2]);
			printf("Base Stat 4: %d\n",Thing.BaseStats[3]);
			printf("Base Stat 5: %d\n",Thing.BaseStats[4]);
			printf("Base Stat 6: %d\n",Thing.BaseStats[5]);
			printf("BaseFriendship: %d\n",Thing.BaseFriendship);
		}
		else if ( StringEquals(LastString,"exit") )
		{
			GlobalRunning = false;;
		}
		else
		{
			printf("Invalid Command!\n");
		}
	}

    return 0;

}


