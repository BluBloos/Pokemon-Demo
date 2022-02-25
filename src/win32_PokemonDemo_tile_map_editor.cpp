#include <Windows.h>

#include "PokemonDemo.h"

#include "win32_PikaBlue.h"
#include "win32_PikaBlue_Functions.cpp"
//#include "PikaBlue_Strings.cpp"

#define ID_Save 1
#define ID_Load 2
#define ID_Exit 3
#define ID_Undo 4
#define ID_Redo 5
#define ID_Clear 6
#define ID_NextBrush 7
#define ID_Texture 8
#define ID_Walkable 9
#define ID_Layer1 10
#define ID_Layer2 11
#define ID_Eraser 12
#define ID_Transparency 13
#define ID_AbovePlayer 14
#define ID_VWS 15 

enum ViewMode
{
	TRANSPARENCY = 1, WALKABLE = 2, SWIMMABLE = 4, WILDERNESS = 8, AREA = 16, LAYER1 = 32, LAYER2 = 64, ABOVEPLAYER = 128
};

global_variable bool globalRunning = false;
global_variable win32_offscreen_buffer globalBackBuffer;
global_variable LONGLONG PerfCountFrequency64;
global_variable tile_chunk GlobalTileChunk = {};
global_variable loaded_bitmap TileSet[TILESETAMOUNT];
global_variable int GlobalBrush = 1;
global_variable int ViewingMode = LAYER1;
global_variable bool Eraser = false;
global_variable bool TextureOverride = true;
global_variable memory_arena MemoryArena;

global_variable HMENU hMenubar = CreateMenu();
global_variable HMENU hFile = CreateMenu();
global_variable HMENU hEdit = CreateMenu();
global_variable HMENU hView = CreateMenu();
global_variable HMENU hHelp = CreateMenu();

global_variable int GlobalOffsetX = 0;
global_variable int GlobalOffsetY = 0;

internal void DrawSelector(game_offscreen_buffer *Buffer, float MinX, float MaxX, float MinY, float MaxY,
                           float R, float G, float B, float size)
{
	float dx = MaxX - MinX;
	float dy = MinY - MaxY;
    
	DrawRect(Buffer, MinX, MinX + size, MaxY, MinY, R, G, B);
	DrawRect(Buffer, MaxX - size, MaxX, MaxY, MinY, R, G, B);
    
	DrawRect(Buffer, MinX, MaxX, MaxY, MaxY + size, R, G, B);
	DrawRect(Buffer, MinX, MaxX, MinY - size, MinY, R, G, B);
}

internal void SaveTileMap(char *FileName, tile_chunk Chunk)
{
	DEBUGPlatformWriteEntireFile(FileName, sizeof(int) * 256, Chunk.Tiles);
}

inline void SetEditorTile(tile_chunk *Chunk, unsigned int TileX, unsigned int TileY, unpacked_tile Value)
{
	unsigned int PackedTile = PackTile(Value);
	Chunk->Tiles[TileY * 16 + TileX] = PackedTile;
}

inline unpacked_tile GetEditorTile(tile_chunk Chunk, unsigned int TileX, unsigned int TileY)
{
	unsigned int Result = Chunk.Tiles[TileY * 16 + TileX];
	return UnPackTile(Result);
}

internal unpacked_tile GetTileAtMouseChecked(int MouseX, int MouseY, int *TileX, int *TileY, int TileSizeInPixels)
{
	unpacked_tile Result = {};
    
	MouseY = TileSizeInPixels * 16 - MouseY;
    
	float X = (float)MouseX / (float)TileSizeInPixels;
	float Y = (float)MouseY / (float)TileSizeInPixels;
    
	if( (X >= 0.0f) || (X < 16.0f) || (Y >= 0.0f) || (Y < 16.0f) )
	{
		Result = GetEditorTile(GlobalTileChunk, *TileX, *TileY);
		*TileX = FloorFloatToInt(X);
		*TileY = FloorFloatToInt(Y);
	}
    
	return Result;
}

inline void MouseToBufferSpace(HWND WindowHandle, int *MouseX, int *MouseY)
{
	win32_window_dimension dimensions = Win32GetWindowDimension(WindowHandle);
	*MouseX = *MouseX - (SafeSubtractInline(dimensions.width, globalBackBuffer.width)) / 2;
	*MouseY = *MouseY - (SafeSubtractInline(dimensions.height, globalBackBuffer.height)) / 2;
}

inline void BufferToDrawSpace(int TileSizeInPixels, int *MouseX, int *MouseY)
{	
	int OffsetX = (globalBackBuffer.width - 16 * TileSizeInPixels) / 2;
	int OffsetY = (globalBackBuffer.height - 16 * TileSizeInPixels) / 2;
    
	*MouseX = *MouseX - OffsetX;
	*MouseY = *MouseY - OffsetY;
}

internal void FillChunk(tile_chunk *Chunk, unsigned int Type)
{
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			unpacked_tile Tile = {};
			Tile.TileType = Type;
			SetEditorTile(Chunk, x, y, Tile);
		}
	}
}

internal void GenerateBlankChunk(char *FileName)
{
	tile_chunk Chunk = {};
	unsigned int Tiles[256];
	Chunk.Tiles = Tiles;
	FillChunk(&Chunk, GRASS);
	DEBUGPlatformWriteEntireFile(FileName, sizeof(int) * 256, Chunk.Tiles);
}

internal void DrawBrush(game_offscreen_buffer *Buffer, int Brush)
{
	loaded_bitmap Bitmap = GrabTileBitmap(TileSet, Brush);
	DrawBitmapScaled(Buffer, Bitmap, 886, 950, 10, 74, FLIPFALSE, 2);
}

internal void IterateBrush()
{
	GlobalBrush = GlobalBrush + 1;
	if ( GlobalBrush > TILESETAMOUNT)
	{
		GlobalBrush = 1;
	}
}

internal void SwitchViewingMode(short ViewingModeID)
{
	switch(ViewingModeID)
	{
		case ID_Texture:
		{
			if (TextureOverride)
			{
				TextureOverride = false;
				ModifyMenu(hEdit, ID_Texture, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_Texture, "Texture Override");
			}
			else
			{
				TextureOverride = true;
				ModifyMenu(hEdit, ID_Texture, MF_BYCOMMAND | MF_CHECKED | MF_STRING, ID_Texture, "Texture Override");
			}
		}break;
		case ID_Walkable:
		{
			if (ViewingMode & WALKABLE)
			{
				ViewingMode = ViewingMode - WALKABLE;
				ModifyMenu(hView, ID_Walkable, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_Walkable, "Walkable");
			}
			else
			{
				ViewingMode = ViewingMode | WALKABLE;
				ModifyMenu(hView, ID_Walkable, MF_BYCOMMAND | MF_CHECKED | MF_STRING, ID_Walkable, "Walkable");
				if (ViewingMode & TRANSPARENCY)
				{
					ViewingMode = ViewingMode - TRANSPARENCY;
					ModifyMenu(hView, ID_Transparency, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_Transparency, "Transparency");
				}
				if (ViewingMode & ABOVEPLAYER)
				{
					ViewingMode = ViewingMode - ABOVEPLAYER;
					ModifyMenu(hView, ID_AbovePlayer, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_AbovePlayer, "Above Player");
				}
			}
		}break;
		case ID_Transparency:
		{
			if (ViewingMode & TRANSPARENCY)
			{
				ViewingMode = ViewingMode - TRANSPARENCY;
				ModifyMenu(hView, ID_Transparency, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_Transparency, "Transparency");
			}
			else
			{
				ViewingMode = ViewingMode | TRANSPARENCY;
				ModifyMenu(hView, ID_Transparency, MF_BYCOMMAND | MF_CHECKED | MF_STRING, ID_Transparency, "Transparency");
				if (ViewingMode & WALKABLE)
				{
					ViewingMode = ViewingMode - WALKABLE;
					ModifyMenu(hView, ID_Walkable, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_Walkable, "Walkable");
				}
				if (ViewingMode & ABOVEPLAYER)
				{
					ViewingMode = ViewingMode - ABOVEPLAYER;
					ModifyMenu(hView, ID_AbovePlayer, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_AbovePlayer, "Above Player");
				}
			}
		}break;
		case ID_AbovePlayer:
		{
			if (ViewingMode & ABOVEPLAYER)
			{
				ViewingMode = ViewingMode - ABOVEPLAYER;
				ModifyMenu(hView, ID_AbovePlayer, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_AbovePlayer, "Above Player");
			}
			else
			{
				ViewingMode = ViewingMode | ABOVEPLAYER;
				ModifyMenu(hView, ID_AbovePlayer, MF_BYCOMMAND | MF_CHECKED | MF_STRING, ID_AbovePlayer, "Above Player");
				if (ViewingMode & WALKABLE)
				{
					ViewingMode = ViewingMode - WALKABLE;
					ModifyMenu(hView, ID_Walkable, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_Walkable, "Walkable");
				}
				if (ViewingMode & TRANSPARENCY)
				{
					ViewingMode = ViewingMode - TRANSPARENCY;
					ModifyMenu(hView, ID_Transparency, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_Transparency, "Transparency");
				}
			}
		}break;
		case ID_Layer1:
		{
			ViewingMode = ViewingMode | LAYER1;
			if (ViewingMode & LAYER2)
			{
				ViewingMode = ViewingMode - LAYER2;
				ModifyMenu(hView, ID_Layer2, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_Layer2, "Layer 2");
			}
			ModifyMenu(hView, ID_Layer1, MF_BYCOMMAND | MF_CHECKED | MF_STRING, ID_Layer1, "Layer 1");
		}break;
		case ID_Layer2:
		{
			ViewingMode = ViewingMode | LAYER2;
			if (ViewingMode & LAYER1)
			{
				ViewingMode = ViewingMode - LAYER1;
				ModifyMenu(hView, ID_Layer1, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_Layer1, "Layer 1");
			}
			ModifyMenu(hView, ID_Layer2, MF_BYCOMMAND | MF_CHECKED | MF_STRING, ID_Layer2, "Layer 2");
		}break;
		case ID_Eraser:
		{
			if(Eraser)
			{
				Eraser = false;
				ModifyMenu(hEdit, ID_Eraser, MF_BYCOMMAND | MF_UNCHECKED | MF_STRING, ID_Eraser, "Eraser");
			}
			else
			{
				Eraser = true;
				ModifyMenu(hEdit, ID_Eraser, MF_BYCOMMAND | MF_CHECKED | MF_STRING, ID_Eraser, "Eraser");
			}
		}break;
		default:
		{
            
		}break;
	}
}

LRESULT CALLBACK Win32WindowProc(HWND window,
                                 UINT message,
                                 WPARAM wParam,
                                 LPARAM lParam)
{
	LRESULT result = 0;
	switch(message)
	{
		case WM_COMMAND:
		{
			switch( LOWORD(wParam) )
			{
				case ID_Exit:
				{
					globalRunning = false;
				}break;
				case ID_Clear:
				{
					Assert(GlobalTileChunk.Tiles);
					FillChunk(&GlobalTileChunk,1);
				}break;
				case ID_NextBrush:
				{
					IterateBrush();
				}break;
				default:
				{
					SwitchViewingMode( LOWORD(wParam) );
				}break;
			}
		}break;
		case WM_DESTROY:
		{
			//TODO(Noah): Hanle as error
			globalRunning = false;
		}break;
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC DeviceContext = BeginPaint(window,&paint);
			win32_window_dimension dimensions = Win32GetWindowDimension(window);
			/*
   int x = paint.rcPaint.left;
   int y = paint.rcPaint.top;
   LONG height = paint.rcPaint.bottom - paint.rcPaint.top;
   LONG width = paint.rcPaint.right - paint.rcPaint.left;
   */
			Win32DisplayBufferWindow(DeviceContext,&globalBackBuffer,dimensions.width,dimensions.height);
			EndPaint(window,&paint);
		}break;
		case WM_CLOSE:
		{
			//TODO(Noah): Handle as message to user
			globalRunning = false;
		}break;
		default:
		{
			result = DefWindowProc(window,message,wParam,lParam);
		}break;
	}
    
	return result;
}



int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance, 
                     LPSTR cmdLine,
                     int showCode) 
{
    
    // set render mode
    GLOBAL_COLOR_MODE = BGR;
    
    win32_state Win32State;
    Win32GetRelativePaths(&Win32State);
    char StringBuffer[256];
    
    //load a tile map on startup here
    GlobalTileChunk = LoadChunk( DEBUGPlatformReadEntireFile, GameCatStrings(Win32State.BaseFilePath, "Data\\TileChunk.chunk", StringBuffer)); 
    
    //if there is no tile map make a blank tile map
    if(!GlobalTileChunk.Tiles)
	{
		GenerateBlankChunk(StringBuffer);
		GlobalTileChunk = LoadChunk( DEBUGPlatformReadEntireFile,
                                    StringBuffer); 
	}	
    
	//below I make some memory
	//below is the code to initialize that good old memory, we like memory
	game_memory GameMemory = {}; GameMemory.StorageSize = MegaBytes(64);
	GameMemory.Storage = VirtualAlloc(0,GameMemory.StorageSize,MEM_COMMIT,PAGE_READWRITE);
	if (GameMemory.Storage){ GameMemory.Valid = true;}
	GameMemory.TransientStorageSize = GigaBytes((long int)1);
	GameMemory.TransientStorage = VirtualAlloc(0,GameMemory.TransientStorageSize,MEM_COMMIT,PAGE_READWRITE);
    
	//initialize memory arena
	InitializeArena(&MemoryArena, ((unsigned char *)GameMemory.Storage), GameMemory.StorageSize);
    
	//load the tiles
	UnPackBitmapTiles(&MemoryArena, DEBUGLoadBMP(DEBUGPlatformReadEntireFile, GameCatStrings(Win32State.BaseFilePath, "Data\\PikaBlueTileSet.bmp", StringBuffer) 
                                                 ), 16, 16, 1, TileSet);
    
	LARGE_INTEGER PerfCountFrequency;
	QueryPerformanceFrequency(&PerfCountFrequency);
	PerfCountFrequency64 = PerfCountFrequency.QuadPart;
    
	UINT DesiredSchedularGranularity = 1;
	bool SleepGranular = (timeBeginPeriod(DesiredSchedularGranularity) == TIMERR_NOERROR);
    
	Win32ResizeDIBSection(&globalBackBuffer, 960, 540);
    
	WNDCLASS WindowClass = {}; 
	WindowClass.style = CS_VREDRAW|CS_HREDRAW; //ensuring the window will redraw after being resized
	WindowClass.lpfnWndProc = Win32WindowProc; //Setting callback to recieve windows messages
	WindowClass.hInstance = Instance;
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	//WindowClass.hIcon;
	WindowClass.lpszClassName = "PokemonDemoWindowClassTileMapEditor"; //rather pointless line, eh.
    
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hFile, "File");           
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hEdit, "Edit");
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hView, "View");
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hHelp, "Help");
    
	AppendMenu(hFile, MF_STRING, ID_Save, "Save");
	AppendMenu(hFile, MF_STRING, ID_Load, "Load");
	AppendMenu(hFile, MF_STRING, ID_Exit, "Exit");
    
	AppendMenu(hEdit, MF_STRING, ID_Undo, "Undo");
	AppendMenu(hEdit, MF_STRING, ID_Redo, "Redo");
	AppendMenu(hEdit, MF_STRING, ID_Clear, "Clear");
	AppendMenu(hEdit, MF_STRING, ID_NextBrush, "Next Brush");
	AppendMenu(hEdit, MF_STRING | MF_CHECKED, ID_Texture, "Texture Override");
	AppendMenu(hEdit, MF_STRING | MF_UNCHECKED, ID_Eraser, "Eraser");
    
	AppendMenu(hView, MF_UNCHECKED | MF_STRING, ID_Walkable, "Walkable");
	AppendMenu(hView, MF_UNCHECKED | MF_STRING, ID_Transparency, "Transparency");
	AppendMenu(hView, MF_UNCHECKED | MF_STRING, ID_AbovePlayer, "Above Player");
	AppendMenu(hView, MF_CHECKED | MF_STRING, ID_Layer1, "Layer 1");
	AppendMenu(hView, MF_UNCHECKED | MF_STRING, ID_Layer2, "Layer 2");
    
	SetMenuItemBitmaps(hEdit, ID_Texture, MF_BYCOMMAND, 0, 0); //This sets the menu to use the default checkmark
	SetMenuItemBitmaps(hEdit, ID_Eraser, MF_BYCOMMAND, 0, 0);
	SetMenuItemBitmaps(hView, ID_Walkable, MF_BYCOMMAND, 0, 0);
	SetMenuItemBitmaps(hView, ID_Layer1, MF_BYCOMMAND, 0, 0);
	SetMenuItemBitmaps(hView, ID_Layer2, MF_BYCOMMAND, 0, 0);
	SetMenuItemBitmaps(hView, ID_Transparency, MF_BYCOMMAND, 0, 0);
	SetMenuItemBitmaps(hView, ID_AbovePlayer, MF_BYCOMMAND, 0, 0);
    
	AppendMenu(hHelp, MF_STRING, ID_VWS, "Visit website"); 
    
	if(RegisterClass(&WindowClass))
	{
		HWND WindowHandle = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Tile Map Editor",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_MAXIMIZE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			hMenubar,
			Instance,
			0);
		if(WindowHandle)
		{
            
			HDC RefreshDC = GetDC(WindowHandle);
			int MonitorRefreshRateHz = 60;
			int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
			if (Win32RefreshRate > 1)
			{
				MonitorRefreshRateHz = Win32RefreshRate;
			}
			float GameUpdateHz (MonitorRefreshRateHz/2.0f);
			float TargetSecondsElapsedPerFrame = 1.0f / GameUpdateHz;
			ReleaseDC(WindowHandle,RefreshDC);
            
			globalRunning = true;
            
			LARGE_INTEGER LastCounter = Win32GetWallClock();
            
			while(globalRunning)
			{
				MSG message;
				while( PeekMessage(&message,0,0,0,PM_REMOVE))
				{
					switch(message.message)
					{
						case WM_QUIT:
						{	
							globalRunning = false;
						}break;
						default:
						{
							TranslateMessage(&message);
							DispatchMessage(&message);
						}break;
					}
				}
                
				float TileSizeInMeters = 1.4f;
				int TileSizeInPixels = 32;
				float MetersToPixels = (float)TileSizeInPixels / TileSizeInMeters; 
                
				POINT MouseP;
				GetCursorPos(&MouseP);
				ScreenToClient(WindowHandle, &MouseP);
				int MouseX = MouseP.x;
				int MouseY = MouseP.y;
				int MouseZ = 0;
				bool MouseButtons[5];
				MouseToBufferSpace(WindowHandle,&MouseX, &MouseY);
                
				char Buffer[256];
				wsprintf(Buffer, "Mouse posX is %d, posY is %d\n", MouseX, MouseY);
				OutputDebugStringA(Buffer);
				
				MouseButtons[0] = ((GetKeyState(VK_LBUTTON) & (1 << 15)) == 1);
				MouseButtons[1] = ((GetKeyState(VK_RBUTTON) & (1 << 15)) == 1);
				MouseButtons[2] = ((GetKeyState(VK_MBUTTON) & (1 << 15)) == 1);
				MouseButtons[3] = ((GetKeyState(VK_XBUTTON1) & (1 << 15)) == 1);
				MouseButtons[4] = ((GetKeyState(VK_XBUTTON2) & (1 << 15)) == 1);
                
				game_offscreen_buffer GameBuffer = {};
				GameBuffer.memory = globalBackBuffer.memory;
				GameBuffer.width = globalBackBuffer.width;
				GameBuffer.height = globalBackBuffer.height;
				GameBuffer.pitch = globalBackBuffer.pitch;
				GameBuffer.BytesPerPixel = globalBackBuffer.bytesPerPixel;
                
				//draw viewing area
				DrawRect(&GameBuffer, 0.0f, (float)GameBuffer.width, 0.0f,(float)GameBuffer.height, 0.2f, 0.2f, 0.2f);
				DrawRect(&GameBuffer, 800.0f, 960.0f, 0.0f, 540.0f, 1.0f, 1.0f, 1.0f);
                
				//draw pallete, and handle and draw brush
				unsigned int PalleteRow = 0;
				unsigned int CurrentTile = 0;
				while(CurrentTile < TILESETAMOUNT)
				{
					float Padding = 1.0f;
					float MinX = 800.0f + (CurrentTile % 9) * (16.0f + Padding);
					float MinY = 50.0f + PalleteRow * (16.0f + Padding);
					loaded_bitmap Bitmap = GrabTileBitmap(TileSet, CurrentTile + 1);
					DrawBitmap(&GameBuffer, Bitmap, MinX, MinX + 16.0f, MinY, MinY + 16.0f, FLIPFALSE);
					
					//here we update the brush if the mouse is hovering over!
					if(GetKeyState(VK_LBUTTON) & (1 << 15))
					{
						rect Rect = {}; Rect.MinX = MinX; Rect.MaxX = MinX + 16.0f; 
						Rect.MinY = MinY; Rect.MaxY = MinY + 16.0f; game_screen_position ScreenPos = {};
						ScreenPos.X = (float)MouseX; ScreenPos.Y = (float)MouseY;
						if (ScreenPositionInRect(ScreenPos, Rect))
						{
							GlobalBrush = CurrentTile + 1;
						}
					}					
                    
					if ( ++CurrentTile % 9 == 0 )
					{
						PalleteRow++;
					}
				}
				DrawBrush(&GameBuffer, GlobalBrush);
				
				BufferToDrawSpace(TileSizeInPixels, &MouseX, &MouseY);
				GlobalOffsetX = (GameBuffer.width - 16 * TileSizeInPixels) / 2;
				GlobalOffsetY = (GameBuffer.height - 16 * TileSizeInPixels) / 2;
                
				for (int x = 0; x < 16; x++)
				{
					for (int y = 0; y < 16; y++)
					{
						float MinX = (float)(x * TileSizeInPixels) + GlobalOffsetX;
						float MaxX = MinX + TileSizeInPixels;
						float MinY = GameBuffer.height - (float)(y * TileSizeInPixels) - GlobalOffsetY;
						float MaxY = MinY - TileSizeInPixels;
                        
						unpacked_tile Tile = GetEditorTile(GlobalTileChunk, x, y);
                        
						if (Tile.TileType)
						{
							if(Tile.Transparent)
							{
								unsigned int BackgroudIndex = 0;
								unsigned int ForegroundIndex = 0;
								UnPackTransparentTile(Tile, &BackgroudIndex, &ForegroundIndex);
								if (BackgroudIndex)
								{
									loaded_bitmap TileBitmap = GrabTileBitmap(TileSet, BackgroudIndex);
									DrawBitmapScaled(&GameBuffer, TileBitmap, MinX, MaxX, MaxY, MinY, FLIPFALSE, 2);
								}
								if (ForegroundIndex)
								{
									loaded_bitmap ForegroundBitamp = GrabTileBitmap(TileSet, ForegroundIndex);
									DrawBitmapScaled(&GameBuffer, ForegroundBitamp, MinX, MaxX, MaxY, MinY, FLIPFALSE, 2);
								}
                                
							}
							else
							{
								loaded_bitmap TileBitmap = GrabTileBitmap(TileSet, Tile.TileType);
								DrawBitmapScaled(&GameBuffer, TileBitmap, MinX, MaxX, MaxY, MinY, FLIPFALSE, 2);
							}
						}
                        
						if (ViewingMode & WALKABLE)
						{
							if (Tile.Walkable)
							{
								DrawRectWithOpacity(&GameBuffer, MinX, MaxX, MaxY, MinY, 0.0f, 0.7f, 0.0f, 100.0f);
							}
							else
							{
								DrawRectWithOpacity(&GameBuffer, MinX, MaxX, MaxY, MinY, 0.7f, 0.0f, 0.0f, 100.0f);
							}
						}
						else if(ViewingMode & TRANSPARENCY)
						{
							if (Tile.Transparent)
							{
								DrawRectWithOpacity(&GameBuffer, MinX, MaxX, MaxY, MinY, 0.0f, 0.7f, 0.0f, 100.0f);
							}
							else
							{
								DrawRectWithOpacity(&GameBuffer, MinX, MaxX, MaxY, MinY, 0.7f, 0.0f, 0.0f, 100.0f);
							}
						}
						else if(ViewingMode & ABOVEPLAYER)
						{
							if (Tile.AbovePlayer)
							{
								DrawRectWithOpacity(&GameBuffer, MinX, MaxX, MaxY, MinY, 0.0f, 0.7f, 0.0f, 100.0f);
							}
							else
							{
								DrawRectWithOpacity(&GameBuffer, MinX, MaxX, MaxY, MinY, 0.7f, 0.0f, 0.0f, 100.0f);
							}
						}
                        
						int TileX = 0;
						int TileY = 0;
                        
						unpacked_tile MouseTile = GetTileAtMouseChecked(MouseX, MouseY, &TileX, &TileY, TileSizeInPixels);
                        
						if ( (MouseTile.TileType > 0) && (TileX == x) && (TileY == y) )
						{
							if ( GetKeyState(VK_LBUTTON) & (1 << 15) )
							{	
								if (Eraser)
								{
									if (TextureOverride)
									{
										if (ViewingMode & LAYER1)
										{
											if (Tile.Transparent)
											{
												unsigned int BackgroudIndex = 0;
												unsigned int ForegroundIndex = 0;
												UnPackTransparentTile(Tile, &BackgroudIndex, &ForegroundIndex);
												Tile = MakeTransparentTile(Tile, 0, ForegroundIndex);
											}
											else
											{
												Tile.TileType = 0;
											}
										}
										else if (ViewingMode & LAYER2)
										{
											if (Tile.Transparent)
											{
												unsigned int BackgroudIndex = 0;
												unsigned int ForegroundIndex = 0;
												UnPackTransparentTile(Tile, &BackgroudIndex, &ForegroundIndex);
												Tile = MakeTransparentTile(Tile, BackgroudIndex, 0);
											}
											else
											{
												Tile = MakeTransparentTile(Tile, Tile.TileType, 0);
											}
										}
									}
                                    
									if (ViewingMode & WALKABLE)
									{
										Tile.Walkable = 0;
									}
									else if(ViewingMode & TRANSPARENCY)
									{
										if (Tile.Transparent)
										{
											unsigned int BackgroundIndex = 0;
											unsigned int ForegroundIndex = 0;
											UnPackTransparentTile(Tile, &BackgroundIndex, &ForegroundIndex);
											Tile.Transparent = 0;
											Tile.TileType = BackgroundIndex;
										}	
									}
									else if(ViewingMode & ABOVEPLAYER)
									{
										Tile.AbovePlayer = 0;
									}
								}
								else
								{
									if (TextureOverride)
									{
										if (ViewingMode & LAYER1)
										{
											if (Tile.Transparent)
											{
												unsigned int BackgroudIndex = 0;
												unsigned int ForegroundIndex = 0;
												UnPackTransparentTile(Tile, &BackgroudIndex, &ForegroundIndex);
												Tile = MakeTransparentTile(Tile, GlobalBrush, ForegroundIndex);
											}
											else
											{
												Tile.TileType = GlobalBrush;
											}
										}
										else if (ViewingMode & LAYER2)
										{
											if (Tile.Transparent)
											{
												unsigned int BackgroudIndex = 0;
												unsigned int ForegroundIndex = 0;
												UnPackTransparentTile(Tile, &BackgroudIndex, &ForegroundIndex);
												Tile = MakeTransparentTile(Tile, BackgroudIndex, GlobalBrush);
											}
											else
											{
												Tile = MakeTransparentTile(Tile, Tile.TileType, GlobalBrush);
											}
										}
									}
                                    
									if (ViewingMode & WALKABLE)
									{
										Tile.Walkable = 1;
									}
									else if(ViewingMode & ABOVEPLAYER)
									{
										Tile.AbovePlayer = 1;
									}
								}
                                
								SetEditorTile(&GlobalTileChunk, TileX, TileY, Tile);
								SaveTileMap( GameCatStrings(Win32State.BaseFilePath, "Data\\TileChunk.chunk", StringBuffer), GlobalTileChunk);
							}
							DrawSelector(&GameBuffer, MinX, MaxX, MinY, MaxY, 0.0f, 0.0f, 0.0f, 3.0f);
						}
					}
				}
                
				LARGE_INTEGER WorkCounter = Win32GetWallClock();
				float WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter, PerfCountFrequency64);
                
				float SecondsElapsedForFrame = WorkSecondsElapsed;
				if (SecondsElapsedForFrame < TargetSecondsElapsedPerFrame)
				{
					if (SleepGranular)
					{
						//Assert(!"aghhhhhh")
						DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsElapsedPerFrame - WorkSecondsElapsed));
						if (SleepMS > 0)
						{
							Sleep(SleepMS);
						}
					}
					while (SecondsElapsedForFrame < TargetSecondsElapsedPerFrame)
					{
						SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock(), PerfCountFrequency64);
					}
				}
				else
				{
					//TODO(Noah): Logging, WE MISSED THE FRAME RATE!
				}
                
				LARGE_INTEGER EndCounter = Win32GetWallClock();
				LastCounter = EndCounter;
                
				HDC DeviceContext = GetDC(WindowHandle);
				win32_window_dimension dimensions = Win32GetWindowDimension(WindowHandle);
				Win32DisplayBufferWindow(DeviceContext,&globalBackBuffer,dimensions.width,dimensions.height);
				ReleaseDC(WindowHandle,DeviceContext);
			}
		}
	}
}