#define BUTTON_AMOUNT 8

struct game_persistent_data
{
	memory_arena MemoryArena;
	loaded_bitmap PikaBlueFont[70];
	loaded_bitmap Bitmaps[BITMAPAMOUNT];
	
	game_ui_button Buttons[BUTTON_AMOUNT];
	game_ui_element CurrentElement;
	game_ui_scene CurrentScene;
	unsigned int ElementSelected; //controls disply of current element
	unsigned int MovingScene; //whether or not moving scene or current element
    
	float Timer; 
	unsigned int Timing;
    
	char StringBuffer[256];
};

internal game_ui_button MakeButton(float MinX, float MaxX, float MinY, float MaxY, float R, float G, float B, char *Text)
{
	game_ui_button Result = {};
	rect Rect = {}; Rect.MinX = MinX; Rect.MaxX = MaxX;
	Rect.MinY = MinY; Rect.MaxY = MaxY; Result.Rect = Rect;
	color Color = {}; Color.R = R; Color.G = G; Color.B = B;
	Result.Color = Color; CloneString(Text,Result.Text,256);
	return Result;
}

internal void DrawButton(game_offscreen_buffer *buffer, loaded_bitmap *PikaBlueFont, game_ui_button Button)
{
	DrawRect(buffer, Button.Rect.MinX, Button.Rect.MaxX, Button.Rect.MinY, Button.Rect.MaxY, Button.Color.R, Button.Color.G, Button.Color.B);
	BlitStringBoundless(buffer, PikaBlueFont, Button.Rect.MinX, Button.Rect.MinY + 20.0f, Button.Text, 0);
}

internal void DimButton(game_offscreen_buffer *buffer, game_ui_button Button)
{
	DrawRectWithOpacity(buffer, Button.Rect.MinX, Button.Rect.MaxX, Button.Rect.MinY, Button.Rect.MaxY, 0.0f, 0.0f, 0.0f, 100.0f);
}

internal void FinalizeElement(game_persistent_data *GameData, game_ui_element Element)
{
	GameData->CurrentScene.Elements[GameData->CurrentScene.Count++] = Element;
}

internal void CreateNewElement(game_persistent_data *GameData, unsigned int BitmapIndex)
{
	if (GameData->ElementSelected)
	{
		FinalizeElement(GameData, GameData->CurrentElement);		 
	}
	else
	{
		GameData->ElementSelected = true;
	}
    
	GameData->CurrentElement = {}; //reset position
	GameData->CurrentElement.BitmapIndex = BitmapIndex;
	GameData->CurrentElement.Flip = FLIPFALSE;
}

internal void CreateNewText(game_persistent_data *GameData, unsigned int Flip, unsigned int Invert)
{
	if (GameData->ElementSelected)
	{
		FinalizeElement(GameData, GameData->CurrentElement);
	}
	else
	{
		GameData->ElementSelected = true;
	}
    
	GameData->CurrentElement = {};
	GameData->CurrentElement.Flip = Flip;
	GameData->CurrentElement.Invert = Invert;
	GameData->CurrentElement.IsString = true;
	GameData->CurrentElement.RelativePosition.Y = 20.0f;
	CloneString("New String", GameData->CurrentElement.String, 256);
}

internal void SaveUIScene(game_persistent_data *GameData)
{
	PlatformOpenSaveDialog(GameData->StringBuffer);
}

internal void LoadUIScene(game_persistent_data *GameData)
{
	PlatformOpenLoadDialog(GameData->StringBuffer);
}

//NOTE: Beware, when we load a scene we absolutely throw away the last scene
//so we just want to be aware of that, dude.
internal void GameGotOpenFileName(game_memory *Memory)
{
	game_persistent_data *GameData = (game_persistent_data *)Memory->Storage;
	GameData->CurrentScene = *(game_ui_scene *)DEBUGPlatformReadEntireFile(GameData->StringBuffer).Contents;
}

internal void GameGotSaveFileName(game_memory *Memory)
{
	//actually save the scene
	game_persistent_data *GameData = (game_persistent_data *)Memory->Storage;
	Memory->DEBUGPlatformWriteEntireFile(GameData->StringBuffer, sizeof(game_ui_scene), &GameData->CurrentScene); 
} 

GAME_UPDATE_RENDER(PikeBlueUIUpdate)
{
	char StringBuffer[256];
	game_user_gamepad_input *Controller = &Input->GamepadInput[0]; //get controller
	game_user_gamepad_input *Keyboard = &Input->GamepadInput[1]; //get keyboard
	game_persistent_data *GameData = (game_persistent_data *)Memory->Storage;
	if (Memory->Valid)
	{
		if (!Memory->IsInitialized)
		{
			InitializeArena(&GameData->MemoryArena, ((unsigned char *)Memory->Storage) + sizeof(game_persistent_data),
                            Memory->StorageSize - sizeof(game_persistent_data));
			//here we are going to load in all the required bitmaps into our bitmap array
			GameData->Bitmaps[0] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data\\AttackBackground.bmp", StringBuffer));
			GameData->Bitmaps[1] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data\\EnemyHealth.bmp", StringBuffer));
			GameData->Bitmaps[2] = DEBUGLoadBMP(Memory->DEBUGPlatformReadEntireFile, GameCatStrings(Input->BaseFilePath, "Data\\LargeDialouge.bmp", StringBuffer));
			GenerateByTiles(&GameData->MemoryArena, DEBUGLoadBMP( Memory->DEBUGPlatformReadEntireFile,
                                                                 GameCatStrings(Input->BaseFilePath, "Data\\PokemonDemoFont.bmp", StringBuffer)), 16, 16, GameData->PikaBlueFont);
            
			//intialize buttons!
			GameData->Buttons[0] = MakeButton(725.0f, 935.0f, 25.0f, 65.0f, 0.5f, 0.5f, 0.5f,"Construct Element");
			GameData->Buttons[1] = MakeButton(725.0f, 935.0f, 85.0f, 125.0f, 0.5f, 0.5f, 0.5f,"Construct Text");
			GameData->Buttons[2] = MakeButton(725.0f, 800.0f, 145.0f, 185.0f, 0.5f, 0.5f, 0.5f,"Save");
			GameData->Buttons[3] = MakeButton(820.0f, 935.0f, 145.0f, 185.0f, 0.5f, 0.5f, 0.5f,"Load");
			GameData->Buttons[4] = MakeButton(880.0f, 935.0f, 265.0f, 305.0f, 0.2f, 0.2f, 0.2f,"Next");
			GameData->Buttons[5] = MakeButton(880.0f, 935.0f, 335.0f, 375.0f, 0.2f, 0.2f, 0.2f,"Invert");
			GameData->Buttons[6] = MakeButton(880.0f, 935.0f, 395.0f, 425.0f, 0.2f, 0.2f, 0.2f,"Flip");
			GameData->Buttons[7] = MakeButton(880.0f, 935.0f, 445.0f, 485.0f, 0.2f, 0.2f, 0.2f,"Pos");
            
			Memory->IsInitialized = true;
		}
        
		//below is a ton of default drawing routines for the application
		DrawRect(buffer, 0.0f, 960.0f, 0.0f, 540.0f, 0.2f, 0.2f, 0.2f); //draw background
		DrawRect(buffer, 20.0f, 520.0f, 20.0f, 520.0f, 0.3f, 0.3f, 0.3f); //draw drawing bounds
		DrawRect(buffer, 700.0f, 960.0f, 0.0f, 540.0f, 1.0f, 1.0f, 1.0f); //draw UI interface backdrop
		DrawRect(buffer, 725.0f, 935.0f, 205.0f, 515.0f, 0.5f, 0.5f, 0.5f); //draw current element
		game_screen_position MouseScreenPos = {}; MouseScreenPos.X = (float)Input->MouseX; MouseScreenPos.Y = (float)Input->MouseY;
		for (int x = 0; x < BUTTON_AMOUNT; x++)
		{
			game_ui_button Button = GameData->Buttons[x];
			DrawButton(buffer, GameData->PikaBlueFont, Button); 
			if ( ScreenPositionInRect(MouseScreenPos, Button.Rect) )
			{
				DimButton(buffer, Button);
			}
		}  
        
		//draw all elements that have been created thus far
		game_screen_position ScreenPos = {};
		DrawUIScene(buffer, GameData->Bitmaps, GameData->PikaBlueFont, GameData->CurrentScene, GameData->CurrentScene.Position);
        
		//update and draw current element
		if (GameData->ElementSelected)
		{
			if ( (Keyboard->Left.EndedDown) && (Keyboard->Left.HalfTransitionCount > 0) )
			{
				//on button down
				if (!GameData->MovingScene)
				{
					GameData->CurrentElement.RelativePosition.X--;
				}
				else
				{
					GameData->CurrentScene.Position.X--; 
				}
			}
			else if ( (Keyboard->Left.EndedDown) && (Keyboard->Left.HalfTransitionCount == 0) )
			{
				//holding the button
				if(!GameData->Timing)
				{
					GameData->Timing = true;
				}
				else
				{
					if(GameData->Timer > 0.5f)
					{
						if (!GameData->MovingScene)
						{
							GameData->CurrentElement.RelativePosition.X -= 2;
						}
						else
						{
							GameData->CurrentScene.Position.X -= 2; 
						}
					}	
				}
			}
			else if ( !(Keyboard->Left.EndedDown) && (Keyboard->Left.HalfTransitionCount > 0) )
			{
				//on button up
				GameData->Timing = false;
				GameData->Timer = 0.0f;
			}
			
			if ( (Keyboard->Right.EndedDown) && (Keyboard->Right.HalfTransitionCount > 0) )
			{
				if (!GameData->MovingScene)
				{
					GameData->CurrentElement.RelativePosition.X++;
				}
				else
				{
					GameData->CurrentScene.Position.X++; 
				}
			}
			else if ( (Keyboard->Right.EndedDown) && (Keyboard->Right.HalfTransitionCount == 0) )
			{
				if(!GameData->Timing)
				{
					GameData->Timing = true;
				}
				else
				{
					if(GameData->Timer > 0.5f)
					{
						if (!GameData->MovingScene)
						{
							GameData->CurrentElement.RelativePosition.X += 2;
						}
						else
						{
							GameData->CurrentScene.Position.X += 2; 
						}
					}
				}
			}
			else if ( !(Keyboard->Right.EndedDown) && (Keyboard->Right.HalfTransitionCount > 0) )
			{
				//on button up
				GameData->Timing = false;
				GameData->Timer = 0.0f;
			}
            
			if ( (Keyboard->Up.EndedDown) && (Keyboard->Up.HalfTransitionCount > 0) )
			{
				if (!GameData->MovingScene)
				{
					GameData->CurrentElement.RelativePosition.Y--;
				}
				else
				{
					GameData->CurrentScene.Position.Y--; 
				}
			}
			else if ( (Keyboard->Up.EndedDown) && (Keyboard->Up.HalfTransitionCount == 0) )
			{
				if(!GameData->Timing)
				{
					GameData->Timing = true;
				}
				else
				{
					if(GameData->Timer > 0.5f)
					{
						if (!GameData->MovingScene)
						{
							GameData->CurrentElement.RelativePosition.Y-=2;
						}
						else
						{
							GameData->CurrentScene.Position.Y-=2; 
						}
					}	
				}
			}
			else if ( !(Keyboard->Up.EndedDown) && (Keyboard->Up.HalfTransitionCount > 0) )
			{
				//on button up
				GameData->Timing = false;
				GameData->Timer = 0.0f;
			}
            
			if ( (Keyboard->Down.EndedDown) && (Keyboard->Down.HalfTransitionCount > 0) )
			{
				if (!GameData->MovingScene)
				{
					GameData->CurrentElement.RelativePosition.Y++;
				}
				else
				{
					GameData->CurrentScene.Position.Y++; 
				}
			}
			else if ( (Keyboard->Down.EndedDown) && (Keyboard->Down.HalfTransitionCount == 0) )
			{
				if(!GameData->Timing)
				{
					GameData->Timing = true;
				}
				else
				{
					if(GameData->Timer > 0.5f)
					{
						if (!GameData->MovingScene)
						{
							GameData->CurrentElement.RelativePosition.Y+=2;
						}
						else
						{
							GameData->CurrentScene.Position.Y+=2; 
						}
					}	
				}
			}
			else if ( !(Keyboard->Down.EndedDown) && (Keyboard->Down.HalfTransitionCount > 0) )
			{
				//on button up
				GameData->Timing = false;
				GameData->Timer = 0.0f;
			}
            
			//draw element
			game_ui_element Element = GameData->CurrentElement;
			DrawUIElement(buffer, GameData->Bitmaps, GameData->PikaBlueFont, GameData->CurrentElement, GameData->CurrentScene.Position);
            
			//draw information about the current element on the right hand panel
			unsigned int PosX = (GameData->MovingScene)?(unsigned int)GameData->CurrentScene.Position.X:
            (unsigned int)GameData->CurrentElement.RelativePosition.X;
			unsigned int PosY = (GameData->MovingScene)?(unsigned int)GameData->CurrentScene.Position.Y:
            (unsigned int)GameData->CurrentElement.RelativePosition.Y;
			BlitStringBoundless(buffer, GameData->PikaBlueFont, 725.0f, 225.0f, 
                                NumberToASCII(PosX,StringBuffer), 0);
			BlitStringBoundless(buffer, GameData->PikaBlueFont, 725.0f, 245.0f, 
                                NumberToASCII(PosY,StringBuffer), 0);
		}
        
		//below will handle when the user clicks the mouse
		if ( (Input->MouseButtons[0].EndedDown) && (Input->MouseButtons[0].HalfTransitionCount > 0) )
		{
			if (  ScreenPositionInRect(MouseScreenPos, GameData->Buttons[0].Rect) )
			{
				CreateNewElement(GameData, 0);
			}
			else if ( ScreenPositionInRect(MouseScreenPos, GameData->Buttons[1].Rect) )
			{
				CreateNewText(GameData, false, false);
			}
			else if ( ScreenPositionInRect(MouseScreenPos, GameData->Buttons[2].Rect) )
			{
				SaveUIScene(GameData);
			}
			else if (ScreenPositionInRect(MouseScreenPos, GameData->Buttons[3].Rect))
			{
				LoadUIScene(GameData);
			}
			else if (ScreenPositionInRect(MouseScreenPos, GameData->Buttons[4].Rect))
			{
				//Next bitmap
				GameData->CurrentElement.BitmapIndex++;
				if (GameData->CurrentElement.BitmapIndex + 1 > BITMAPAMOUNT)
				{
					GameData->CurrentElement.BitmapIndex = 0;
				}	
			}
			else if (ScreenPositionInRect(MouseScreenPos, GameData->Buttons[5].Rect))
			{
				//invert string
				if (GameData->CurrentElement.IsString)
				{
					GameData->CurrentElement.Invert = !GameData->CurrentElement.Invert; 
				}
			}
			else if (ScreenPositionInRect(MouseScreenPos, GameData->Buttons[6].Rect))
			{
				//flip string
				if (GameData->CurrentElement.IsString)
				{
					GameData->CurrentElement.Flip = !GameData->CurrentElement.Flip; 
				}
			}
			else if (ScreenPositionInRect(MouseScreenPos, GameData->Buttons[7].Rect))
			{
				GameData->MovingScene = !GameData->MovingScene;
			}
		}
        
		//if we are supposed to time, then time 
		if (GameData->Timing)
		{
			GameData->Timer += Input->DeltaTime;
		}
	}
}