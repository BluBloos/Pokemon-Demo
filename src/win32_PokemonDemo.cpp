//TODO NOT a final platform layer?
//Saved game locations
//Getting a handle to own exe file
//Need asset loading path
//How to launch a thread
//Raw input
//Support multiple keyboards
//Sleep/timebeginperiod
//ClipCursor for multi monitor support
//Fullscreen support
//QueryCancelAutoPLay
//WM_ACTIVATEAPP
//blit speed improvements
//hardware acceleration
//get keyboard layout for french kerboards, international WASD support
//Just a partial list, in order to put this into shipping state.
#include <Windows.h>
#include <xinput.h>
#include <dsound.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Dinputd.h>
#include <Dbt.h>
//TODO(Noah): Implement sine ourselves!

#include "PokemonDemo.h"
#include "win32_PikaBlue.h"

#include "win32_PikaBlue_Functions.cpp"

struct win32_DI_ENUM_CONTEXT 
{ 
    DIJOYCONFIG* pPreferredJoyCfg; 
    bool bPreferredJoyCfgValid; 
}; 

//NOTE(Noah): Here I declare a macro with the function defintions
//then we make synonym for the function with appropriate name
//then we make stub functions which the macro will literally insert the function def into w/ custom name, insane
//NOTE(Noah): Support for xinput get state
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	OutputDebugStringA("Called stub of XInputGetState!");
	return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_ 

//NOTE(Noah): Support for xInput set state
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

//NOTE(Noah): Support for directsound
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS,LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);


//TODO(Noah): These are global variables for now.
//Apparently global variables are bad so yeah.
global_variable bool globalRunning;
global_variable win32_offscreen_buffer globalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER SecondBuffer;
global_variable LPDIRECTINPUT8 DirectInputObject;
global_variable LPDIRECTINPUTDEVICE8 JoystickInterface; 
global_variable WINDOWPLACEMENT PreviousWindow = { sizeof(PreviousWindow) };
global_variable bool GameTryingFullscreen = false;

void ToggleFullscreen(HWND WindowHandle)
{
	//This function can be credited to Raymond Chen, see
	//https://blogs.msdn.microsoft.com/oldnewthing/20100412-00/?p=14353
	
	//The GetWindowLong function returns the window style.
	DWORD dwStyle = GetWindowLong(WindowHandle, GWL_STYLE);
	//If the window is the normal window or whatever, we
	//set to fullscreen.
	if (dwStyle & WS_OVERLAPPEDWINDOW) 
	{
	    MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
	    //Get window placement will get the window placement, GetMonitorInfro will use a monitor handle and 
	    //fill a monitor info structure. MonitorFromWindow will get the monitor handle using
	    //the window handle.
	    if (GetWindowPlacement(WindowHandle, &PreviousWindow) &&
	    	GetMonitorInfo(MonitorFromWindow(WindowHandle, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
	    {
	    	//Here, the SetWindowLong function sets the window style to its current style, 
	    	//but without any of the attributes specified by WS_OVERLAPPEDWINDOW.
	    	//the tilda (~) operator is bitwise NOT operator.
	    	SetWindowLong(WindowHandle, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
	    	//SetWindowPos here sets our window to the top. The next inputs are X (top left pos),
	    	//Y (top pos), cx (window width), cy (window height), the last things are flags and 
	    	//in our case they specify to actually update the window since we set its style.
	    	SetWindowPos(WindowHandle, HWND_TOP, MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
						 MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
						 MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	    }
	}
	else
	{
		//set the style to the current style and the overlapped window properties, 
		//so back to a non fullscreen setup.
	    SetWindowLong(WindowHandle, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
	    //Sets the window placement to what it was before;
	    SetWindowPlacement(WindowHandle, &PreviousWindow);
	    //Next we do another SetWindowPos
	    SetWindowPos(WindowHandle, 0, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
					 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
	GameTryingFullscreen = false;
}

internal void Win32DebugDrawVertical(win32_offscreen_buffer *BackBuffer, int X, int Top, int Bottom, unsigned int Color)
{
	if (Top <= 0)
	{
		Top = 0;
	}
	
	if (Bottom > BackBuffer->height)
	{
		Bottom = BackBuffer->height;
	}
	
	if (X >= 0 && X < BackBuffer->width)
	{
		unsigned char *Pixel = (unsigned char *)BackBuffer->memory + X * BackBuffer->bytesPerPixel + Top * BackBuffer->pitch;
		for (int y = Top; y < Bottom;y++)
		{
			*(unsigned int *)Pixel = Color;
			Pixel += BackBuffer->pitch;
		}
	}
}

inline void Win32DrawSoundBufferMarker(win32_offscreen_buffer *BackBuffer, float C, int PadX, int Top, int Bottom,
									   DWORD Value, unsigned int Color)
{
	int X = PadX + (int)(C * Value);
	Win32DebugDrawVertical(BackBuffer, X, Top, Bottom, Color);
}

internal void Win32DebugSyncDisplay(win32_offscreen_buffer *BackBuffer, int MarkerCount, int CurrentMarker, win32_debug_time_markers *Markers,
									win32_sound_output *SoundOutput)
{
	int PadX = 16;
	int PadY = 64;
	int LineHeight = 64;
	int Top = PadY; 
	int Bottom = LineHeight + PadY; 
	float C = (float)(BackBuffer->width - 2 * PadX) / SoundOutput->BufferSize;
	for (int PlayCursorIndex = 0; PlayCursorIndex < MarkerCount; PlayCursorIndex++)
	{
		DWORD PlayCursorColor = 0xFFFFFFFF;
		DWORD WriteCursorColor = 0x00F442E5;
		if(PlayCursorIndex == MarkerCount - 1)
		{
			Top += PadY + LineHeight;
			Bottom += PadY + LineHeight;
			
			Win32DrawSoundBufferMarker(BackBuffer,C,PadX,Top,Bottom,Markers[PlayCursorIndex].OutputPlayCursor, PlayCursorColor);
			Win32DrawSoundBufferMarker(BackBuffer,C,PadX,Top,Bottom,Markers[PlayCursorIndex].OutputWriteCursor, WriteCursorColor);
			
			Top += PadY + LineHeight;
			Bottom += PadY + LineHeight;
			
			Win32DrawSoundBufferMarker(BackBuffer,C,PadX,Top,Bottom,Markers[PlayCursorIndex].OutputLocation, WriteCursorColor);
			Win32DrawSoundBufferMarker(BackBuffer,C,PadX,Top,Bottom,Markers[PlayCursorIndex].OutputLocation + Markers[PlayCursorIndex].OutputByteCount,
									   WriteCursorColor);
			
			Top += PadY + LineHeight;
			Bottom += PadY + LineHeight;
		}
		//go through each play cursor!
		Win32DrawSoundBufferMarker(BackBuffer,C,PadX,Top,Bottom,Markers[PlayCursorIndex].FlipPlayCursor, PlayCursorColor);
		Win32DrawSoundBufferMarker(BackBuffer,C,PadX,Top,Bottom,Markers[PlayCursorIndex].FlipWriteCursor, WriteCursorColor);
	}
}

TOGGLE_FULLSCREEN_CALLBACK(ToggleFullscreenCallback)
{
	GameTryingFullscreen = true;
}

internal void Win32BuildFilePath(win32_state *State, char *FileName,
								 int DestCount, char *Dest)
{
	CatStrings( (int)(State->OnePastLastSlash - State->EXEFileName), State->EXEFileName,
			   GetStringLength(FileName),FileName,DestCount,Dest);
}

internal void Win32FreeDirectInput() 
{ 
    // Unacquire the device one last time just in case  
    // the app tried to exit while the device is still acquired. 
    if( JoystickInterface )
    { 
    	JoystickInterface->Unacquire();
    } 
	
    // Release any DirectInput objects. 
    SAFE_RELEASE( JoystickInterface ); 
    SAFE_RELEASE( DirectInputObject );
} 

internal void Win32InitDInput(HWND Window)
{
	HRESULT hr;
	//using get module handle with a 0 will return the handle to the exe
	hr = DirectInput8Create(GetModuleHandle(0),DIRECTINPUT_VERSION,IID_IDirectInput8,(void **)&DirectInputObject,0);
	if (hr == DI_OK)
	{
		OutputDebugStringA("Succesfully created directinput\n");
	}
	else if (hr == DIERR_BETADIRECTINPUTVERSION)
    {
		OutputDebugStringA("DIERR_BETADIRECTINPUTVERSION\n");
	}
	else if (hr == DIERR_INVALIDPARAM)
	{
		OutputDebugStringA("DIERR_INVALIDPARAM\n");
	}
	else if (hr == DIERR_OLDDIRECTINPUTVERSION)
	{
		OutputDebugStringA("DIERR_OLDDIRECTINPUTVERSION\n");
	}
	else if (hr == DIERR_OUTOFMEMORY)
	{
		OutputDebugStringA("DIERR_OUTOFMEMORY\n");
	}
}

internal void Win32ProcessDigitalButton(game_user_gamepad_button_state *OldState, game_user_gamepad_button_state *NewState,
										DIJOYSTATE2 *JoystickState, int ButtonIndex)
{
	NewState->EndedDown = JoystickState->rgbButtons[ButtonIndex];
	NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown)? 1: 0;
}

BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext ) 
{
    // Obtain an interface to the enumerated joystick. 
    if ( DirectInputObject->CreateDevice( pdidInstance->guidInstance, &JoystickInterface, 0) != DI_OK )
    {
    	return DIENUM_CONTINUE; 
    } 
	
    // Stop enumeration. Note: we're just taking the first joystick we get. You could store all the enumerated joysticks and let the user pick. 
    return DIENUM_STOP; 
}

BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext ) 
{ 
    HWND hDlg = ( HWND )pContext; 
	
    static int nSliderCount = 0;  // Number of returned slider controls 
    static int nPOVCount = 0; // Number of returned POV controls 
	
    // For axes that are returned, set the DIPROP_RANGE property for the 
    // enumerated axis in order to scale min/max values. 
    if( pdidoi->dwType & DIDFT_AXIS ) 
    { 
        DIPROPRANGE diprg; 
        diprg.diph.dwSize = sizeof( DIPROPRANGE ); 
        diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER ); 
        diprg.diph.dwHow = DIPH_BYID; 
        diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis 
        diprg.lMin = -1000; 
        diprg.lMax = +1000; 
        // Set the range for the axis 
        if (JoystickInterface->SetProperty( DIPROP_RANGE, &diprg.diph ) != DI_OK )
        {  
        	return DIENUM_STOP;
        } 
		
    } 
	
    return DIENUM_CONTINUE; 
}  

internal bool Win32DInputGetGameControllers(HWND WindowHandle)
{
	DIJOYCONFIG PreferredJoyCfg = {}; 
    win32_DI_ENUM_CONTEXT enumContext; 
    enumContext.pPreferredJoyCfg = &PreferredJoyCfg; 
    enumContext.bPreferredJoyCfgValid = false;
	
	if (!DirectInputObject)
	{
		return false;
	}
	
	if ( DirectInputObject->EnumDevices(DI8DEVCLASS_GAMECTRL,EnumJoysticksCallback,&enumContext,DIEDFL_ATTACHEDONLY) == DI_OK)
	{
		//check to make sure we got the joystick
		if (!JoystickInterface)
		{
			return false;
		}
		
        // set the format of the joystick interface. Want to know about the interface?
		// Do a quick google for c_dfDIJoystick2
		if (JoystickInterface->SetDataFormat( &c_dfDIJoystick2 ) != DI_OK)
		{
			return false;
		}
		
        //Set the cooperative level of the joystick.
		if (JoystickInterface->SetCooperativeLevel( WindowHandle, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) != DI_OK)
		{
			return false;
		}
		if (JoystickInterface->EnumObjects(EnumObjectsCallback,(void *)WindowHandle, DIDFT_ALL ) != DI_OK)
		{
			return false;
		}
		return true; 
	}
	
	return false;
}

inline FILETIME Win32GetLastWriteTime(char *FileName)
{
	FILETIME LastFileWrite = {};
	WIN32_FIND_DATA FindData; 
	HANDLE FindHandle = FindFirstFileA(FileName, &FindData);
	if (FindHandle != INVALID_HANDLE_VALUE)
	{
		FindClose(FindHandle);
		LastFileWrite = FindData.ftLastWriteTime;
	}
	return LastFileWrite;
}

internal win32_game_code Win32LoadGameCode(char *SourceDLLName, char *TempDLLName)
{
	win32_game_code Result = {};
	
	Result.LastWriteTime = Win32GetLastWriteTime(SourceDLLName);
	
	CopyFile(SourceDLLName, TempDLLName, FALSE);
	Result.GameCodeDLL = LoadLibraryA(TempDLLName);
	
	if(Result.GameCodeDLL)
	{
		Result.UpdateRender = (game_update_render *)GetProcAddress(Result.GameCodeDLL,"GameUpdateRender");
		Result.GetSoundSamples = (game_get_sound_samples *)GetProcAddress(Result.GameCodeDLL,"GameGetSoundSamples");
		
		Result.IsValid = (Result.UpdateRender && Result.GetSoundSamples);
	}
	
	if (!Result.IsValid)
	{
		Result.UpdateRender = 0;
		Result.GetSoundSamples = 0;
	}
	
	return Result;
}

internal void Win32UnloadGameCode(win32_game_code *GameCode)
{
	if (GameCode->GameCodeDLL)
	{
		FreeLibrary(GameCode->GameCodeDLL);
	}
	GameCode->IsValid = false;
	GameCode->UpdateRender = 0;
	GameCode->GetSoundSamples = 0;
}

//NOTE(Noah):this function loads the location of Xinput functions 
//and sets the function pointers to the appropriate location in memory.
internal void Win32LoadXInput()
{
	//TODO(Noah):test on windows 8
	HMODULE library = LoadLibraryA("xinput1_4.dll");
	if(!library)
	{
		library = LoadLibraryA("xinput1_3.dll");
	}
	if(library)
	{
		OutputDebugStringA("Succesfully loaded Xinput library!");
		XInputGetState = (x_input_get_state *)GetProcAddress(library,"XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(library,"XInputGetState");
	}
}

internal void Win32InitDSound(HWND Window, int SamplesPerSecond, int BufferSize)
{
	//load library
	HMODULE library = LoadLibraryA("dsound.dll");
	if(library)
	{
		direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(library,"DirectSoundCreate");
		//TODO(Noah): Double check if it works on windows XP
		LPDIRECTSOUND DirectSound;
		if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0,&DirectSound,0)))
		{
			OutputDebugStringA("Created direct sound object!\n");
			//NOTE(Noah): Create wave format stuff.
			WAVEFORMATEX WaveFormat = {};
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nChannels = 2;
			WaveFormat.nSamplesPerSec = SamplesPerSecond;
			WaveFormat.wBitsPerSample = 16;  
			//WaveFormat.nBlockAlign is the amount of bytes per block of channels.
			WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8; 
			WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
			
			if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
			{
				//create primary buffer
				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription,&PrimaryBuffer,0)))
				{
					OutputDebugStringA("Created primary buffer!\n");
					if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
					{
						OutputDebugStringA("Set primary buffer format!\n");
					}
				}
				else
				{
					//TODO(Noah): Diagnostic stuff. Could not create buffer
				}
			}
			else
			{
				//TODO(Noah): COuld not set cooperative level of directSound: diagnostic.
			}
			
			DSBUFFERDESC BufferDescription = {};
			BufferDescription.dwSize = sizeof(BufferDescription);
			BufferDescription.lpwfxFormat = &WaveFormat;
			BufferDescription.dwBufferBytes = BufferSize;
			OutputDebugStringA("Ready to start creating secondary buffer!\n");
			if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription,&SecondBuffer,0)))
			{
				OutputDebugStringA("Created secondary buffer!\n");
			}
			
			//create secondary buffer which we write to
			//start playing!
		}
		else
		{
			//TODO(Noah): No direct sound! Diagnostic.
		}
	}
}

internal void Win32FillSoundBuffer(win32_sound_output *SoundOutput,game_sound_output_buffer *SourceBuffer,DWORD ByteToLock, DWORD BytesToWrite)
{
	void *region0;
	DWORD region0Size;
	void *region1;
	DWORD region1Size;
	//NOTE(Noah): Here I lock the byte region as defined before, and get the appropriate pointers
	//so we can write to it
	if (SUCCEEDED( SecondBuffer->Lock(ByteToLock,BytesToWrite,&region0,&region0Size,&region1,&region1Size,0)) )
	{
		//NOTE(Noah): Here I write to both regions that we got back, im writing a simple square wave. 
		short *SampleOut = (short *)region0;
		DWORD region0SampleCount = region0Size / SoundOutput->BytesPerSample;
		short *SourceSample = SourceBuffer->SampleOut;
		for (DWORD SampleIndex = 0;SampleIndex < region0SampleCount;++SampleIndex)
		{
			*SampleOut++ = *SourceSample++;
			*SampleOut++ = *SourceSample++;
			++SoundOutput->RunningSampleIndex;
		}
		
		SampleOut = (short *)region1;
		DWORD region1SampleCount = region1Size / SoundOutput->BytesPerSample;
		for (DWORD SampleIndex = 0;SampleIndex < region1SampleCount;++SampleIndex)
		{
			*SampleOut++ = *SourceSample++;
			*SampleOut++ = *SourceSample++;
			++SoundOutput->RunningSampleIndex;
		}
		//NOTE(Noah): Here I unlock the byte space so we reduce errors. See the MSDN documentation 
		//for further information as to why we are required to unlock the buffer. 
		SecondBuffer->Unlock(region0,region0Size,region1,region1Size);
	}
}

internal void Win32ClearSoundBuffer(win32_sound_output *SoundOutput)
{
	void *region0;
	DWORD region0Size;
	void *region1;
	DWORD region1Size;
	if (SUCCEEDED( SecondBuffer->Lock(0,SoundOutput->BufferSize,&region0,&region0Size,&region1,&region1Size,0)) )
	{
		unsigned char *ByteOut = (unsigned char *)region0;
		for (DWORD ByteIndex = 0;ByteIndex < region0Size;++ByteIndex)
		{
			*ByteOut++ = 0;
		}
		ByteOut = (unsigned char *)region1;
		for (DWORD ByteIndex = 0;ByteIndex < region1Size;++ByteIndex)
		{
			*ByteOut++ = 0;
		}
		SecondBuffer->Unlock(region0,region0Size,region1,region1Size);
	}
}

internal win32_replay_buffer *Win32GetReplayBuffer(win32_state *Win32State, unsigned int Index)
{
	Assert(Index < ArrayCount(Win32State->ReplayBuffers))
		win32_replay_buffer *Result = &Win32State->ReplayBuffers[Index];
	return Result;
}

internal void Win32GetInputRecordingFile(win32_state *Win32State,bool InputSteam, 
										 int SlotIndex, int DestCount, char *Dest)
{
	char Temp[64];
	wsprintf(Temp,"PokemonDemo_playback_%d_%s.pdi",SlotIndex, InputSteam ? "input" : "state");
	Win32BuildFilePath(Win32State, Temp, DestCount, Dest);	
}

internal void Win32BeginRecordingInput(win32_state *Win32State, int InputRecordingIndex)
{
	win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(Win32State, InputRecordingIndex);
	if (ReplayBuffer->MemoryBlock)
	{
		Win32State->InputRecordingIndex = InputRecordingIndex;
		
		char FileName[WIN32_FILE_NAME_COUNT];
		Win32GetInputRecordingFile(Win32State, true, InputRecordingIndex, sizeof(FileName), FileName);
		Win32State->RecordingHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		
		CopyMemory(ReplayBuffer->MemoryBlock, Win32State->GameMemoryBlock, (size_t) Win32State->TotalSize);
	}
}

internal void Win32EndRecordingInput(win32_state *Win32State)
{
	CloseHandle(Win32State->RecordingHandle);
	Win32State->InputRecordingIndex = 0;
}

internal void Win32BeginPlayBackInput(win32_state *Win32State, int InputPlayingIndex)
{
	win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(Win32State, InputPlayingIndex);
	if (ReplayBuffer->MemoryBlock)
	{
		Win32State->InputPlayingIndex = InputPlayingIndex;
		
		char FileName[WIN32_FILE_NAME_COUNT];
		Win32GetInputRecordingFile(Win32State, true, InputPlayingIndex, sizeof(FileName), FileName);
		Win32State->PlayBackHandle = CreateFileA(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		
		//LARGE_INTEGER FilePosition;
		//FilePosition.QuadPart = Win32State->TotalSize;
		//SetFilePointerEx(Win32State->PlayBackHandle,FilePosition,0,FILE_BEGIN);
		
		//so here it is when we begin playback input we read the block of memory
		//ReadFile(Win32State->PlayBackHandle,Win32State->GameMemoryBlock,BytesToRead,&BytesRead,0);
		CopyMemory(Win32State->GameMemoryBlock, ReplayBuffer->MemoryBlock, (size_t) Win32State->TotalSize);
	}
}

internal void Win32EndPlayBackInput(win32_state *Win32State)
{
	CloseHandle(Win32State->PlayBackHandle);
	Win32State->InputPlayingIndex = 0;
}

internal void Win32RecordInput(win32_state *Win32State,game_user_input *NewInput)
{
	DWORD BytesWritten;
	WriteFile(Win32State->RecordingHandle,NewInput,sizeof(*NewInput),&BytesWritten,0);
}

internal void Win32PlayBackInput(win32_state *Win32State,game_user_input *NewInput)
{
	DWORD BytesRead = 0;
	if(ReadFile(Win32State->PlayBackHandle,NewInput,sizeof(*NewInput),&BytesRead,0))
	{
		if (BytesRead == 0)
		{
			int PlayIndex = Win32State->InputPlayingIndex;
			Win32EndPlayBackInput(Win32State);
			Win32BeginPlayBackInput(Win32State, PlayIndex);
		}
	}
}

internal void Win32ProcessKey(win32_state *Win32State, game_user_gamepad_input *KeyboardGamepad, unsigned int VKCode, bool IsDown)
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
	if (VKCode == 'L')
	{
		if (IsDown)
		{
			//start recording. Just a quick toggle.
			if(Win32State->InputPlayingIndex == 0)
			{
				if(Win32State->InputRecordingIndex == 0)
				{
					Win32BeginRecordingInput(Win32State, 1);
				}
				else
				{
					Win32EndRecordingInput(Win32State);
					Win32BeginPlayBackInput(Win32State, 1);
					//the playing and input index define the multiple recording slots.
				}
			}
			else
			{
				Win32EndPlayBackInput(Win32State);
			}
			
		}
	}
}

internal void Win32ProcessMessages(win32_state *Win32State, game_user_gamepad_input *KeyboardGamepad)
{
	MSG message;
	while( PeekMessage(&message,0,0,0,PM_REMOVE))
	{
		if(message.message == WM_QUIT)
		{
			globalRunning = false;
		}
		switch(message.message)
		{
			case WM_SYSKEYDOWN:
			{
			}
			break;
			case WM_SYSKEYUP:
			{
			}
			case WM_KEYDOWN:
			{
				unsigned int VKCode = (unsigned int)message.wParam;
				bool WasDown = ((message.lParam & (1 << 30)) != 0);
				bool IsDown = ((message.lParam & (1 << 31)) == 0);
				if (WasDown != IsDown)
				{
					Win32ProcessKey(Win32State, KeyboardGamepad, VKCode, IsDown);
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
					Win32ProcessKey(Win32State, KeyboardGamepad, VKCode, IsDown);
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
		case WM_SIZE:
		{
			OutputDebugStringA("WM_SIZE\n");
		}
		break;
		case WM_DESTROY:
		{
			//TODO(Noah): Hanle as error
			globalRunning = false;
		}
		break;
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
		}
		break;
		case WM_CLOSE:
		{
			//TODO(Noah): Handle as message to user
			globalRunning = false;
		}
		break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		}
		break;
		case WM_DEVICECHANGE:
		{
			if (wParam == DBT_DEVNODES_CHANGED)
			{
				//grab all the controllers
				Win32DInputGetGameControllers(window); 
			}
		}
		break;
		default:
		{
			result = DefWindowProc(window,message,wParam,lParam);
		}
		break;
	}
	
	return result;
}

global_variable LONGLONG PerfCountFrequency64; 

#if POKEMON_DEMO_DEBUG
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#define MAX_CONSOLE_LINES 500

internal void Win32OpenConsole()
{
    int hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    FILE *fp;
    
    // allocate a console for this app
    //https://docs.microsoft.com/en-us/windows/console/allocconsole
    AllocConsole();
    SetConsoleTitleA("Maccis Console");
    
    // set the screen buffer to be big enough to let us scroll text
    //https://docs.microsoft.com/en-us/windows/console/getconsolescreenbufferinfo
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
    
    // redirect unbuffered STDOUT to the console
    //https://msdn.microsoft.com/en-us/library/bdts1c9x.aspx
    //https://msdn.microsoft.com/en-us/library/88k7d7a7.aspx
    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT); //convert windows handle to c runtime handle
    fp = _fdopen( hConHandle, "w" );
    *stdout = *fp;
    //setvbuf( stdout, NULL, _IONBF, 0 ); //associate no buffer
    freopen_s( &fp, "CONOUT$", "w", stdout);
    
    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    //setvbuf( stdout, NULL, _IONBF, 0 ); //associate no buffer
    freopen_s(&fp, "CONOUT$" , "w" , stderr);
}

#endif

//NOTE (Noah): main entry point of program
//NOTE (Noah): use devenv command to debug in Visual Studio
int CALLBACK WinMain(HINSTANCE Instance,
					 HINSTANCE PrevInstance, 
					 LPSTR cmdLine,
					 int showCode) 
{
#if POKEMON_DEMO_DEBUG
    //Win32OpenConsole();
    //printf("Hello, World!\n");
#endif
    
    win32_state Win32State = {};
	
	Win32GetRelativePaths(&Win32State);
	
	char SourceDLLName[WIN32_FILE_NAME_COUNT]; 
	Win32BuildFilePath(&Win32State,"PokemonDemo.dll", sizeof(SourceDLLName), SourceDLLName);
	
	char TempDLLName[WIN32_FILE_NAME_COUNT];
	Win32BuildFilePath(&Win32State,"PokemonDemo_temp.dll", sizeof(TempDLLName), TempDLLName);
	
	LARGE_INTEGER PerfCountFrequency;
	QueryPerformanceFrequency(&PerfCountFrequency);
	PerfCountFrequency64 = PerfCountFrequency.QuadPart;
	
	UINT DesiredSchedularGranularity = 1;
	bool SleepGranular = (timeBeginPeriod(DesiredSchedularGranularity) == TIMERR_NOERROR);
	
	//Initialization things
	//previous res: 960, 540
	Win32ResizeDIBSection(&globalBackBuffer, 1280, 720);
	Win32LoadXInput();
	WNDCLASS WindowClass = {}; 
	WindowClass.style = CS_VREDRAW|CS_HREDRAW; //ensuring the window will redraw after being resized
	WindowClass.lpfnWndProc = Win32WindowProc; //Setting callback to recieve windows messages
	WindowClass.hInstance = Instance;
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	//WindowClass.hIcon;
	WindowClass.lpszClassName = "PokemonDemoWindowClass"; //rather pointless line, eh.
	
	//TODO(Noah): How to reliably get this? Use direct draw?
	
	if(RegisterClass(&WindowClass))
	{
		HWND WindowHandle = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Pokemon Demo",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
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
			float GameUpdateHz (MonitorRefreshRateHz/1.0f);
			float TargetSecondsElapsedPerFrame = 1.0f / GameUpdateHz;
			ReleaseDC(WindowHandle,RefreshDC);
			//NOTE(Noah): Okay so here I define a ton of variables useful for initializes the direct sound
			//api? Yeah, and I think its important that later we fix this to be a better system.
			win32_sound_output SoundOutput = {};
			SoundOutput.SamplesPerSecond = 48000;
			SoundOutput.RunningSampleIndex = 0;
			SoundOutput.BytesPerSample = sizeof(short)*2;
			SoundOutput.BufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
			
            //TODO(Noah): Test if we can reduce the safety bytes. Be a little sly maybe.
			SoundOutput.SafetyBytes = (int)(SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample / GameUpdateHz / 3);
			
            // TODO(Noah): So later we want to get rid of all these virutal allocs and 
			// allocate all our memory at once.
			short *GameSoundSamples = (short *)VirtualAlloc(0,SoundOutput.BufferSize,MEM_COMMIT,PAGE_READWRITE);
			
            //NOTE(Noah): Here we allocate all the memory we will ever use for our game, yay!
			game_memory GameMemory = {};
			GameMemory.StorageSize = MegaBytes(64);
			
			Win32State.TotalSize = GameMemory.StorageSize;
			Win32State.GameMemoryBlock = VirtualAlloc(0,GameMemory.StorageSize,MEM_COMMIT,PAGE_READWRITE);
			
			GameMemory.Storage = Win32State.GameMemoryBlock;
			
			GameMemory.TransientStorageSize = GigaBytes((long int)1);
			GameMemory.TransientStorage = VirtualAlloc(0,GameMemory.TransientStorageSize,MEM_COMMIT,PAGE_READWRITE);
			GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
			GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
			GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
			GameMemory.ToggleFullscreen = ToggleFullscreenCallback;
			
			for (int ReplayIndex = 0; ReplayIndex < ArrayCount(Win32State.ReplayBuffers);++ReplayIndex)
			{
				win32_replay_buffer *ReplayBuffer = &Win32State.ReplayBuffers[ReplayIndex];
				
				Win32GetInputRecordingFile(&Win32State,false,ReplayIndex, 
										   sizeof(ReplayBuffer->FileName), ReplayBuffer->FileName);
				
				ReplayBuffer->FileHandle = CreateFileA(ReplayBuffer->FileName,GENERIC_WRITE|GENERIC_READ, 0,0,CREATE_ALWAYS,0,0);
				
				DWORD MaxSizeHigh = (Win32State.TotalSize >> 32);
				DWORD MaxSizeLow = (Win32State.TotalSize & 0xFFFFFFFF);
				ReplayBuffer->MemoryMap = CreateFileMapping(ReplayBuffer->FileHandle,0,PAGE_READWRITE,
															MaxSizeHigh,MaxSizeLow,0);
				
				ReplayBuffer->MemoryBlock = MapViewOfFile(ReplayBuffer->MemoryMap,FILE_MAP_ALL_ACCESS,
														  0,0,(SIZE_T) Win32State.TotalSize);
				//ReplayBuffer->MemoryBlock = VirtualAlloc(0, Win32State.TotalSize,MEM_RESERVE|MEM_COMMIT,
				//PAGE_READWRITE);
				//TODO(Noah): Change to log message.
				Assert(ReplayBuffer->MemoryBlock)
			}
			
			if (GameMemory.Storage && GameMemory.TransientStorage)
			{
				OutputDebugStringA("Commited all memory, Nice!");
				GameMemory.Valid = true;
			}
			
			Win32InitDInput(WindowHandle);
			Win32DInputGetGameControllers(WindowHandle);
			//Here we initialize the game
			Win32InitDSound(WindowHandle,SoundOutput.SamplesPerSecond,SoundOutput.BufferSize);
			Win32ClearSoundBuffer(&SoundOutput);
			SecondBuffer->Play(0,0,DSBPLAY_LOOPING);
			//Below is some code we use for counting debug values and things of the like. 
			LARGE_INTEGER LastCounter = Win32GetWallClock();
			long LastCycleCount = (long) __rdtsc();
			
			globalRunning = true;
			
			game_user_input GameInput[2] = {};
			game_user_input *NewInput = &GameInput[0];
			game_user_input *OldInput = &GameInput[1];
			
			//OldInput->GamepadInput[1] = &OldKeyboardInput;
			bool SoundIsValid = false;
			DWORD AudioLatencyBytes = 0;
			float AudioLatencySeconds = 0;
			//handle startup specially.
			
			//this is debug stuff
#if POKEMON_DEMO_DEBUG
			int DEBUGMarkerIndex = 0;
			int DEBUGLastMarkersIndex = 0;
			win32_debug_time_markers DEBUGLastMarkers[20] = {};
#endif
			
			win32_game_code GameCode = Win32LoadGameCode(SourceDLLName, TempDLLName);
			
			while(globalRunning)
			{
				FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceDLLName);
				if (CompareFileTime(&NewDLLWriteTime, &GameCode.LastWriteTime))
				{
					Win32UnloadGameCode(&GameCode);
					GameCode = Win32LoadGameCode(SourceDLLName, TempDLLName);
				}
				
				//Here we are clearing the half transition counts of the keyboard each frame to ensure that they are actually acurate.
				game_user_gamepad_input *Keyboard = &NewInput->GamepadInput[1];
				for(int ButtonIndex = 0; ButtonIndex < ArrayCount(Keyboard->DebugButtons); ButtonIndex++)
				{
					Keyboard->DebugButtons[ButtonIndex].HalfTransitionCount = 0; 
				}
                
				Keyboard->Right.HalfTransitionCount = 0;
				Keyboard->Left.HalfTransitionCount = 0;
				Keyboard->Up.HalfTransitionCount = 0;
				Keyboard->Down.HalfTransitionCount = 0;
				
				
				//NOTE(Noah): Here we are going to process windows messages since we are required to do 
				//so, someone told me so I think its rather good practice. We do it in a while loop because 
				//obviously windows messages take priority over the actual game loop, obviously.
				DIJOYSTATE2 JoystickState = {};
				Win32ProcessMessages(&Win32State, &NewInput->GamepadInput[1]);
				
				{
					HRESULT hr;
					//provided I have the controller
					if (JoystickInterface)
					{
						//OutputDebugStringA("Was able to get a controller!\n");
						hr = JoystickInterface->Poll();
						if (hr == DI_OK)
						{
							JoystickInterface->GetDeviceState( sizeof( DIJOYSTATE2 ), &JoystickState ); 
						}
						else if (hr == DI_NOEFFECT)
						{
							JoystickInterface->GetDeviceState( sizeof( DIJOYSTATE2 ), &JoystickState );
						}
						else
						{
							JoystickInterface->Acquire(); 
						}
					}
					else
					{
						//we don't have a joystick so grab one.
						//Win32DInputGetGameControllers(WindowHandle);
					}
				}
				
				//TODO(Noah): Should pull data from controller more frequently?
				//I really just don't know man...!
				for (DWORD i = 0;i < XUSER_MAX_COUNT;i++)
				{
					XINPUT_STATE controllerState;
					if(XInputGetState(i,&controllerState) == ERROR_SUCCESS)
					{
						OutputDebugStringA("Controller is very much so plugged in!\n");
						//NOTE(Noah): Controller is plugged in
						//TODO(Noah): check if controllerState.dwPacketNumber increment too rapidly
						//then I guess we can determine if we are not pulling fast enough
						XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
						bool dpad_up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool dpad_down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool dpad_left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool dpad_right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool startButton = (pad->wButtons & XINPUT_GAMEPAD_START);
						bool aButton = (pad->wButtons & XINPUT_GAMEPAD_A);
						bool bButton = (pad->wButtons & XINPUT_GAMEPAD_B);
						bool xButton = (pad->wButtons & XINPUT_GAMEPAD_X);
						bool yButton = (pad->wButtons & XINPUT_GAMEPAD_Y);
						signed short leftThumbX = pad->sThumbLX;
						signed short leftThumbY = pad->sThumbLY;
					}
					else
					{
						//NOTE(Noah): Controller is not plugged in, sucks.
					}
				} 
				
				game_offscreen_buffer GameBuffer = {};
				GameBuffer.memory = globalBackBuffer.memory;
				GameBuffer.width = globalBackBuffer.width;
				GameBuffer.height = globalBackBuffer.height;
				GameBuffer.pitch = globalBackBuffer.pitch;
				GameBuffer.BytesPerPixel = globalBackBuffer.bytesPerPixel;
				
				//set the left thumb axis
				Win32ProcessDigitalButton(&OldInput->GamepadInput[0].Right, &NewInput->GamepadInput[0].Right,
										  (JoystickState.lX >= DEADZONE));
				Win32ProcessDigitalButton(&OldInput->GamepadInput[0].Left, &NewInput->GamepadInput[0].Left, 
										  (JoystickState.lX <= (0 - DEADZONE)));
				Win32ProcessDigitalButton(&OldInput->GamepadInput[0].Up, &NewInput->GamepadInput[0].Up, 
										  (JoystickState.lY >= DEADZONE));
				Win32ProcessDigitalButton(&OldInput->GamepadInput[0].Down, &NewInput->GamepadInput[0].Down, 
										  (JoystickState.lY <= (0 - DEADZONE)));
				Win32ProcessDigitalButton(&OldInput->GamepadInput[0].AButton, &NewInput->GamepadInput[0].AButton, &JoystickState, 0);
				Win32ProcessDigitalButton(&OldInput->GamepadInput[0].BButton, &NewInput->GamepadInput[0].BButton, &JoystickState, 1);
				
				//Process Mouse Input, very debug code esque
				POINT MouseP;
				GetCursorPos(&MouseP);
				ScreenToClient(WindowHandle, &MouseP);
				NewInput->MouseX = MouseP.x;
				NewInput->MouseY = MouseP.y;
				NewInput->MouseZ = 0;
				Win32ProcessKeyboardButton(&NewInput->MouseButtons[0], 
										   GetKeyState(VK_LBUTTON) & (1 << 15));
				Win32ProcessKeyboardButton(&NewInput->MouseButtons[1], 
										   GetKeyState(VK_RBUTTON) & (1 << 15));
				Win32ProcessKeyboardButton(&NewInput->MouseButtons[2], 
										   GetKeyState(VK_MBUTTON) & (1 << 15));
				Win32ProcessKeyboardButton(&NewInput->MouseButtons[3], 
										   GetKeyState(VK_XBUTTON1) & (1 << 15));
				Win32ProcessKeyboardButton(&NewInput->MouseButtons[4], 
										   GetKeyState(VK_XBUTTON2) & (1 << 15));
				
				NewInput->DeltaTime = TargetSecondsElapsedPerFrame;
				NewInput->BaseFilePath = Win32State.BaseFilePath;
				
				if(GameTryingFullscreen)
				{
					ToggleFullscreen(WindowHandle);
				}
				
				if (Win32State.InputRecordingIndex)
				{
					//if we are recording record the input
					Win32RecordInput(&Win32State, NewInput);
				}
				
				if (Win32State.InputPlayingIndex)
				{
					Win32PlayBackInput(&Win32State, NewInput);
				}
				
				if (GameCode.UpdateRender)
				{
					GameCode.UpdateRender(&GameMemory, &GameBuffer, NewInput, BGR);//NOTE(Noah): Here is where our game will render
				}
				
				DWORD WriteCursor;
				DWORD PlayCursor;
				if (SUCCEEDED( SecondBuffer->GetCurrentPosition(&PlayCursor,&WriteCursor) ) )
				{
					if(!SoundIsValid)
					{
						SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
						SoundIsValid = true;
					}
					
					DWORD TargetCursor = 0;
					DWORD BytesToWrite = 0;
					DWORD ByteToLock = (SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) % SoundOutput.BufferSize;
					
					DWORD ExpectedSoundBytesPerFrame = (int)((SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample) / GameUpdateHz);
					DWORD ExpectedFrameBoundaryBytes = PlayCursor + ExpectedSoundBytesPerFrame;
					
					DWORD SafeWriteCursor = WriteCursor;
					if (SafeWriteCursor < PlayCursor)
					{
						SafeWriteCursor += SoundOutput.BufferSize;
					}
					SafeWriteCursor += SoundOutput.SafetyBytes;
					bool AudioCardLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryBytes);
					
					if(AudioCardLowLatency)
					{
						TargetCursor = (ExpectedSoundBytesPerFrame + ExpectedFrameBoundaryBytes);
					}
					else
					{
						TargetCursor = (WriteCursor + SoundOutput.SafetyBytes + ExpectedSoundBytesPerFrame);
					}
					TargetCursor = TargetCursor % SoundOutput.BufferSize;
					
					if (ByteToLock > TargetCursor)
					{
						//NOTE(Noah): So here the 'WriteCursor' is past the playcuror and therefore 
						//we will write all the bytes from the 'WriteCursor' to the end of the buffer
						//and from the buffer to the beggining of the PlayCursor.
						BytesToWrite = (SoundOutput.BufferSize - ByteToLock);
						BytesToWrite += TargetCursor;
					}
					else
					{
						//NOTE(Noah): Here the 'WriteCursor' is seen before the PlayCursor and so we aught
						//to fill the bytes from the 'WriteCursor' to the PlayCursor. 
						BytesToWrite = TargetCursor - ByteToLock;
					}
					
					game_sound_output_buffer GameSoundBuffer = {};
					GameSoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
					GameSoundBuffer.SampleAmount = BytesToWrite / SoundOutput.BytesPerSample;
					GameSoundBuffer.SampleOut = GameSoundSamples; 
					if(GameCode.GetSoundSamples)
					{
						GameCode.GetSoundSamples(&GameMemory,&GameSoundBuffer, NewInput, false);
					}
					Win32FillSoundBuffer(&SoundOutput, &GameSoundBuffer, ByteToLock, BytesToWrite);
					
#if POKEMON_DEMO_DEBUG
					DEBUGLastMarkers[DEBUGMarkerIndex].OutputPlayCursor = PlayCursor;
					DEBUGLastMarkers[DEBUGMarkerIndex].OutputWriteCursor = WriteCursor;
					DEBUGLastMarkers[DEBUGMarkerIndex].OutputLocation = ByteToLock;
					DEBUGLastMarkers[DEBUGMarkerIndex].OutputByteCount = BytesToWrite;
#endif
					
				}
				else
				{
					SoundIsValid = false;
				}
				
				/*
 if (SoundIsValid)
 {
 
  DWORD WriteCursor;
  DWORD PlayCursor;
  
  if (SUCCEEDED( SecondBuffer->GetCurrentPosition(&PlayCursor,&WriteCursor) ) )
  {
   DWORD UnwrappedWriteCursor = WriteCursor;
   if (UnwrappedWriteCursor < PlayCursor)
   {
    UnwrappedWriteCursor += SoundOutput.BufferSize;
   }
   AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
   AudioLatencySeconds = ((float)AudioLatencyBytes / (float)SoundOutput.BytesPerSample) / 
   (float)SoundOutput.SamplesPerSecond;
  }
  
  
 }
 else
 {
  //TODO(Noah): Diagnostics. We were unable to lock the buffer. Sound is not valid.
 }
 */
				
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
				
				float MilliSecondsPerFrame = (1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter, PerfCountFrequency64));
				
				//TODO(Noah): Kill this. Never ship this code.
				char Buffer[256];
				wsprintf(Buffer, "%dms \n", (int)MilliSecondsPerFrame);
				OutputDebugStringA(Buffer);
				
				LastCounter = EndCounter;
				
				//NOTE(Noah): Here we are going to call our custom windows platform layer function that 
				//will write our custom buffer to the screen.
				HDC DeviceContext = GetDC(WindowHandle);
				win32_window_dimension dimensions = Win32GetWindowDimension(WindowHandle);
#if POKEMON_DEMO_DEBUG
				{
					//Win32DebugSyncDisplay(&globalBackBuffer, ArrayCount(DEBUGLastMarkers),0,
					//DEBUGLastMarkers, &SoundOutput);
				}
#endif
				Win32DisplayBufferWindow(DeviceContext,&globalBackBuffer,dimensions.width,dimensions.height);
				ReleaseDC(WindowHandle, DeviceContext);
				
				
#if POKEMON_DEMO_DEBUG
				{
					DWORD WriteCursor;
					DWORD PlayCursor;
					SecondBuffer->GetCurrentPosition(&PlayCursor,&WriteCursor);
					Assert(DEBUGMarkerIndex < ArrayCount(DEBUGLastMarkers))
						DEBUGLastMarkers[DEBUGMarkerIndex].FlipPlayCursor = PlayCursor;
					DEBUGLastMarkers[DEBUGMarkerIndex].FlipWriteCursor = WriteCursor;	
				}
#endif	
				
				*OldInput = *NewInput;
				
#if POKEMON_DEMO_DEBUG
				DEBUGMarkerIndex++;
				if(DEBUGMarkerIndex >= ArrayCount(DEBUGLastMarkers))
				{
					DEBUGMarkerIndex = 0;
				}
#endif
#if 0
				long EndCycleCount = (long)__rdtsc();
				long CyclesElapsed = EndCycleCount - LastCycleCount;
				LastCycleCount = EndCycleCount;
#endif
			}
			Win32FreeDirectInput();
		}
		else
		{
			//TODO(Noah): Logging
		}	
	}
	else
	{
		//TODO(Noah): Logging
	}
	return 0;
}