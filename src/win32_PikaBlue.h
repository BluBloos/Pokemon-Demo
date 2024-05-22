#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=nullptr; } }
#define DEADZONE 100 

struct win32_offscreen_buffer
{
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int pitch;
	int bytesPerPixel;
};

struct win32_window_dimension
{
	int width;
	int height;
};

struct win32_sound_output
{
	int SamplesPerSecond; 
	unsigned int RunningSampleIndex;
	int LatencySampleCount;
	int BytesPerSample;
	int BufferSize;
	DWORD SafetyBytes;
};

struct win32_debug_time_markers
{
	DWORD OutputPlayCursor;
	DWORD OutputWriteCursor;
	DWORD OutputLocation;
	DWORD OutputByteCount;

	DWORD FlipWriteCursor;
	DWORD FlipPlayCursor;
};

struct win32_game_code
{
	HMODULE GameCodeDLL;
	FILETIME LastWriteTime;
	game_update_render *UpdateRender;
	game_get_sound_samples *GetSoundSamples;
	bool IsValid;
};
#define WIN32_FILE_NAME_COUNT MAX_PATH
struct win32_replay_buffer
{
	HANDLE MemoryMap;
	HANDLE FileHandle;
	char FileName[WIN32_FILE_NAME_COUNT];
	void *MemoryBlock;
};

struct win32_state
{
	_int64 TotalSize;
	void *GameMemoryBlock;
	win32_replay_buffer ReplayBuffers[4];

	HANDLE RecordingHandle;
	HANDLE PlayBackHandle;
	int InputRecordingIndex;
	int InputPlayingIndex;
	char BaseFilePath[WIN32_FILE_NAME_COUNT];
	char EXEFileName[WIN32_FILE_NAME_COUNT];
	char *OnePastLastSlash;
};

//NOTE(Noah): Another beautiful inline function. I kinda love these functions. 
//They are very funcitonal.
inline LARGE_INTEGER
Win32GetWallClock()
{
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return (Result);
}

//note this is a functional function. I think functional functions should be inline. 
//That I deside as of now.
inline float
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End, LONGLONG PerfCountFrequency64)
{
	float Result = 1.0f * (End.QuadPart - Start.QuadPart) / PerfCountFrequency64;
	return (Result);
}

internal void Win32DisplayBufferWindow(HDC deviceContext,win32_offscreen_buffer *buffer,int windowWidth,int windowHeight)
{
	int OffsetX = (windowWidth - buffer->width) / 2;
	int OffsetY = (windowHeight - buffer->height) / 2;

	PatBlt(deviceContext, 0, 0, windowWidth, OffsetY, BLACKNESS);
	PatBlt(deviceContext, 0, OffsetY + buffer->height, windowHeight, windowHeight, BLACKNESS);
	PatBlt(deviceContext, 0, 0, OffsetX, windowHeight, BLACKNESS);
	PatBlt(deviceContext, OffsetX + buffer->width, 0, windowWidth, windowHeight, BLACKNESS);

	//TODO(Noah): Correct aspect ratio.
	StretchDIBits(deviceContext,
		OffsetX,OffsetY,buffer->width,buffer->height,
		0,0,buffer->width,buffer->height,
		buffer->memory,
		&buffer->info,
		DIB_RGB_COLORS,
		SRCCOPY);
}

internal win32_window_dimension Win32GetWindowDimension(HWND window)
{
	win32_window_dimension output;
	RECT clientRect;
	GetClientRect(window,&clientRect);
	output.width = clientRect.right - clientRect.left;
	output.height = clientRect.bottom - clientRect.top;
	return output;
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *buffer,int width, int height)
{
	//NOTE(Noah): we can do this since we know that bitmapMemory
	//is initialized to zero since it's a global variable.
	if(buffer->memory) 
	{
		VirtualFree(buffer->memory,0,MEM_RELEASE);
	}
	//NOTE(Noah): set bitmap heights and widths for global struct
	buffer->width = width;
	buffer->height = height;
	//NOTE(Noah): set values in the bitmapinfo
	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = width;
	buffer->info.bmiHeader.biHeight = -height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;
	//NOTE(Noah): Here we are allocating memory to our bitmap since we resized it
	buffer->bytesPerPixel = 4;
	int bitmapMemorySize = width * height * buffer->bytesPerPixel;
	buffer->memory = VirtualAlloc(0,bitmapMemorySize,MEM_COMMIT,PAGE_READWRITE);
	//NOTE(Noah): setting pitch.
	buffer->pitch = width*buffer->bytesPerPixel;
	//TODD(Noah): probrably want to clear to black each time we resize the window.
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
	if (Memory)
	{
		VirtualFree(Memory,0,MEM_RELEASE);
	}
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
	void *Result = 0;
	int FileSize32;

	HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ,
		FILE_SHARE_READ, 0, OPEN_EXISTING,0,0);
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if ( GetFileSizeEx(FileHandle,&FileSize))
		{
			//TODO(Noah): Defines for maximum value.
			Assert(FileSize.QuadPart <= 0xFFFFFFF);
			FileSize32 = (int)FileSize.QuadPart;
			Result = VirtualAlloc(0,FileSize32,MEM_COMMIT,PAGE_READWRITE);
			if (Result)
			{
				DWORD BytesRead;
				if (ReadFile (FileHandle,Result,FileSize32,
					&BytesRead,0) && (FileSize32 == (int)BytesRead) ) 
				{
					//File read succesfully
				}	
				else
				{
					//could not read from the file, do some logging;
					DEBUGPlatformFreeFileMemory(Result);
					Result = 0;
				}		
			}
		}
		CloseHandle(FileHandle);
	}

	debug_read_file_result FileResult = {};
	FileResult.Contents = Result;
	FileResult.ContentSize = FileSize32;
	return FileResult;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
	bool Result = false;

	HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD BytesWritten;
		if (WriteFile(FileHandle,Memory,MemorySize,&BytesWritten,0)) 
		{
			//File read succesfully
			Result = ((int)BytesWritten == MemorySize);
		}	
		else
		{
			//could not write to file, do some logging!
		}		
		CloseHandle(FileHandle);
	}
	else
	{
		//Could not open file handle do some logging!
	}
	return Result;
} 

internal void Win32ProcessKeyboardButton(game_user_gamepad_button_state *NewState, bool IsDown)
{
	NewState->EndedDown = IsDown;
	++NewState->HalfTransitionCount;
}

internal void Win32ProcessDigitalButton(game_user_gamepad_button_state *OldState, game_user_gamepad_button_state *NewState, bool IsDown)
{
	NewState->EndedDown = IsDown;
	NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown)? 1: 0;
}