#include <Windows.h>
#include "PokemonDemo.h"
#include "win32_PikaBlue.h"

global_variable bool GlobalRunning = false;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LONGLONG GlobalPerfCountFrequency64;
global_variable game_update_render *GameUpdateRender;

global_variable unsigned int TryOpenSaveDialog = false;
global_variable unsigned int TryOpenLoadDialog = false;

global_variable char *FileNameBuffer;
global_variable OPENFILENAME OpenFileName;

internal void PlatformOpenSaveDialog(char *Dest)
{
	FileNameBuffer = Dest;
	TryOpenSaveDialog = true; 
}

internal void PlatformOpenLoadDialog(char *Dest)
{
	FileNameBuffer = Dest;
	TryOpenLoadDialog = true; 
}

#include "PikaBlue_UI_Editor.cpp"

internal unsigned int Win32OpenLoadDialog(HWND WindowHandle)
{
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = WindowHandle;
	OpenFileName.lpstrFile = FileNameBuffer;
	OpenFileName.nMaxFile = 256;
	return GetOpenFileName(&OpenFileName);
}

internal unsigned int Win32OpenSaveDialog(HWND WindowHandle)
{
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = WindowHandle;
	OpenFileName.lpstrFile = FileNameBuffer;
	OpenFileName.nMaxFile = 256;
	return GetSaveFileName(&OpenFileName);
}

inline void MouseToBufferSpace(HWND WindowHandle, int *MouseX, int *MouseY)
{
	win32_window_dimension dimensions = Win32GetWindowDimension(WindowHandle);
	*MouseX = *MouseX - (SafeSubtractInline(dimensions.width, GlobalBackBuffer.width)) / 2;
	*MouseY = *MouseY - (SafeSubtractInline(dimensions.height, GlobalBackBuffer.height)) / 2;
}

internal void Win32ProcessKey(game_user_gamepad_input *KeyboardGamepad, unsigned int VKCode, bool IsDown)
{
	if (VKCode == 'W')
	{
		Win32ProcessKeyboardButton(&KeyboardGamepad->Up, IsDown);
	}
	if (VKCode == 'A')
	{
		Win32ProcessKeyboardButton(&KeyboardGamepad->Left, IsDown);
	}
	if (VKCode == 'S')
	{
		Win32ProcessKeyboardButton(&KeyboardGamepad->Down, IsDown);
	}
	if (VKCode == 'D')
	{
		Win32ProcessKeyboardButton(&KeyboardGamepad->Right, IsDown);
	}
	if(VKCode == 'F')
	{
		Win32ProcessKeyboardButton(&KeyboardGamepad->DebugButtons[0], IsDown);
	}
	if(VKCode == 'Z')
	{
		Win32ProcessKeyboardButton(&KeyboardGamepad->DebugButtons[1], IsDown);
	}
}

internal void Win32ProcessMessages(game_user_gamepad_input *KeyboardGamepad)
{
	MSG message;
	while( PeekMessage(&message,0,0,0,PM_REMOVE))
	{
		if(message.message == WM_QUIT)
		{
			GlobalRunning = false;
		}
		switch(message.message)
		{
			case WM_KEYDOWN:
			{
				unsigned int VKCode = (unsigned int)message.wParam;
				bool WasDown = ((message.lParam & (1 << 30)) != 0);
				bool IsDown = ((message.lParam & (1 << 31)) == 0);
				if (WasDown != IsDown)
				{
					Win32ProcessKey(KeyboardGamepad, VKCode, IsDown);
				}
			}
			break;
			case WM_KEYUP:
			{
				unsigned int VKCode = (unsigned int)message.wParam;
				bool WasDown = ((message.lParam & (1 << 30)) != 0);
				bool IsDown = ((message.lParam & (1 << 31)) == 0);
				if (WasDown != IsDown)
				{
					Win32ProcessKey(KeyboardGamepad, VKCode, IsDown);
				}
			}
			break;
			default:
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			break;
		}
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
		case WM_DESTROY:
		{
			GlobalRunning = false; //TODO: Handle as error
		}break;
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC DeviceContext = BeginPaint(window,&paint);
			win32_window_dimension dimensions = Win32GetWindowDimension(window);
			Win32DisplayBufferWindow(DeviceContext,&GlobalBackBuffer,dimensions.width,dimensions.height);
			EndPaint(window,&paint);
		}break;
		case WM_CLOSE:
		{
			GlobalRunning = false;
		}break;
		default:
		{
			result = DefWindowProc(window,message,wParam,lParam); //execture default windows message handling
		}break;
	}
	return result;
}

int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance, 
                     LPSTR cmdLine,
                     int showCode) 
{
	LARGE_INTEGER PerfCountFrequency;
	QueryPerformanceFrequency(&PerfCountFrequency);
	GlobalPerfCountFrequency64 = PerfCountFrequency.QuadPart;
    
	UINT DesiredSchedularGranularity = 1;
	bool SleepGranular = (timeBeginPeriod(DesiredSchedularGranularity) == TIMERR_NOERROR);
    
	Win32ResizeDIBSection(&GlobalBackBuffer, 960, 540); //Create the global back buffer
    
	WNDCLASS WindowClass = {}; 
	WindowClass.style = CS_VREDRAW|CS_HREDRAW; //Ensuring the window will redraw after being resized
	WindowClass.lpfnWndProc = Win32WindowProc; //Setting callback to recieve windows messages
	WindowClass.hInstance = Instance;
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	WindowClass.lpszClassName = "PikaBlueMain";
    
	//below is the code to initialize that good old memory, we like memory
	game_memory GameMemory = {}; GameMemory.StorageSize = MegaBytes(64);
	GameMemory.Storage = VirtualAlloc(0,GameMemory.StorageSize,MEM_COMMIT,PAGE_READWRITE);
	if (GameMemory.Storage){ GameMemory.Valid = true;}
	GameMemory.TransientStorageSize = GigaBytes((long int)1);
	GameMemory.TransientStorage = VirtualAlloc(0,GameMemory.TransientStorageSize,MEM_COMMIT,PAGE_READWRITE);
	GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
	GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
	GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
    
	//initialize the user input stuff
	game_user_input GameInput[2] = {};
	game_user_input *NewInput = &GameInput[0];
	game_user_input *OldInput = &GameInput[1];
    
#if 1
	GameUpdateRender = PikeBlueUIUpdate;
#elif PIKABLUE_TILE
#endif
    
	if(RegisterClass(&WindowClass))
	{
		HWND WindowHandle = CreateWindowEx(
			0, WindowClass.lpszClassName, "PikeBlue Engine",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_MAXIMIZE, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
			Instance, 0);
        
		if(WindowHandle)
		{
			HDC RefreshDC = GetDC(WindowHandle); int MonitorRefreshRateHz = 60;
			int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
			if (Win32RefreshRate > 1) { MonitorRefreshRateHz = Win32RefreshRate;}
			float GameUpdateHz = MonitorRefreshRateHz / 2.0f;
			float TargetSecondsElapsedPerFrame = 1.0f / GameUpdateHz;
			ReleaseDC(WindowHandle,RefreshDC);
            
			GlobalRunning = true;
			LARGE_INTEGER LastCounter = Win32GetWallClock();
            
			//main loop below, shit is sacred
			while(GlobalRunning)
			{
				//Here we are clearing the half transition counts of the keyboard each frame to ensure that they are actually acurate.
				game_user_gamepad_input *Keyboard = &NewInput->GamepadInput[1];
				for(int ButtonIndex = 0; ButtonIndex < ArrayCount(Keyboard->DebugButtons); ButtonIndex++)
				{
					Keyboard->DebugButtons[ButtonIndex].HalfTransitionCount = 0; 
				}
				Keyboard->Right.HalfTransitionCount = 0; NewInput->MouseButtons[0].HalfTransitionCount = 0;
				Keyboard->Left.HalfTransitionCount = 0; NewInput->MouseButtons[1].HalfTransitionCount = 0;
				Keyboard->Up.HalfTransitionCount = 0; NewInput->MouseButtons[2].HalfTransitionCount = 0;
				Keyboard->Down.HalfTransitionCount = 0; NewInput->MouseButtons[3].HalfTransitionCount = 0;
				NewInput->MouseButtons[4].HalfTransitionCount = 0;
                
				Win32ProcessMessages(&NewInput->GamepadInput[1]); //process keyboard and windows messages
                
				//Below is the code to grab the mouse stuff
				POINT MouseP; GetCursorPos(&MouseP);
				ScreenToClient(WindowHandle, &MouseP);
				NewInput->MouseX = MouseP.x; NewInput->MouseY = MouseP.y; NewInput->MouseZ = 0;
				MouseToBufferSpace(WindowHandle, &NewInput->MouseX, &NewInput->MouseY);
				Win32ProcessDigitalButton(&OldInput->MouseButtons[0], &NewInput->MouseButtons[0], 
                                          GetKeyState(VK_LBUTTON) & (1 << 15));
				Win32ProcessDigitalButton(&OldInput->MouseButtons[1], &NewInput->MouseButtons[1], 
                                          GetKeyState(VK_RBUTTON) & (1 << 15));
				Win32ProcessDigitalButton(&OldInput->MouseButtons[2], &NewInput->MouseButtons[2], 
                                          GetKeyState(VK_MBUTTON) & (1 << 15));
				Win32ProcessDigitalButton(&OldInput->MouseButtons[3], &NewInput->MouseButtons[3], 
                                          GetKeyState(VK_XBUTTON1) & (1 << 15));
				Win32ProcessDigitalButton(&OldInput->MouseButtons[4], &NewInput->MouseButtons[4], 
                                          GetKeyState(VK_XBUTTON2) & (1 << 15));
                
				//below is code to handle some input things
				NewInput->DeltaTime = TargetSecondsElapsedPerFrame;
				NewInput->BaseFilePath = "C:\\PokemonDemo\\";
                
				//make the game buffer
				game_offscreen_buffer GameBuffer = {};
				GameBuffer.memory = GlobalBackBuffer.memory;
				GameBuffer.width = GlobalBackBuffer.width;
				GameBuffer.height = GlobalBackBuffer.height;
				GameBuffer.pitch = GlobalBackBuffer.pitch;
				GameBuffer.BytesPerPixel = GlobalBackBuffer.bytesPerPixel;
                
				if (TryOpenSaveDialog)
				{
					if(Win32OpenSaveDialog(WindowHandle))
					{
						GameGotSaveFileName(&GameMemory);
					}
					TryOpenSaveDialog = false;
				}
                
				if (TryOpenLoadDialog)
				{
					if (Win32OpenLoadDialog(WindowHandle))
					{
						GameGotOpenFileName(&GameMemory);  
					}
					TryOpenLoadDialog = false;
				}
                
				GameUpdateRender(&GameMemory, &GameBuffer, NewInput); //update and render the game
                
				//force a frame rate
				LARGE_INTEGER WorkCounter = Win32GetWallClock();
				float WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter, GlobalPerfCountFrequency64);
				float SecondsElapsedForFrame = WorkSecondsElapsed;
				if (SecondsElapsedForFrame < TargetSecondsElapsedPerFrame)
				{
					if (SleepGranular)
					{
						DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsElapsedPerFrame - WorkSecondsElapsed));
						if (SleepMS > 0)
						{
							Sleep(SleepMS);
						}
					}
					while (SecondsElapsedForFrame < TargetSecondsElapsedPerFrame)
					{
						SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock(), GlobalPerfCountFrequency64);
					}
				}
				else
				{
					//TODO(Noah): Logging, WE MISSED THE FRAME RATE! THATS BAD SHIT BOY! Maybe we should assert? lmao.
				}
                
				*OldInput = *NewInput; //clone the value at new input to be the value at oldinput
				LARGE_INTEGER EndCounter = Win32GetWallClock();
				LastCounter = EndCounter;
                
				HDC DeviceContext = GetDC(WindowHandle);
				win32_window_dimension dimensions = Win32GetWindowDimension(WindowHandle);
				Win32DisplayBufferWindow(DeviceContext,&GlobalBackBuffer,dimensions.width,dimensions.height); //blit backbuffer
				ReleaseDC(WindowHandle,DeviceContext);
			}
		}
		else
		{
			//TODO: Log some errors here
		}
	}
	else
	{
		//TODO: Log some errors here
	}
}