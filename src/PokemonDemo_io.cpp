INLINE wav_file_cursor ParseChunkAt(void *BytePointer, void *EndOfFile)
{
	wav_file_cursor Result;
	Result.Cursor = (char *)BytePointer;
	Result.EndOfFile = (char *)EndOfFile;
	return Result;
}

INLINE wav_file_cursor NextChunk(wav_file_cursor FileCursor)
{
	wav_chunk_header *WavChunkHeader = (wav_chunk_header *)FileCursor.Cursor;
	int ChunkSize = WavChunkHeader->ChunkSize;
	if(ChunkSize & 1)
	{
		ChunkSize +=1 ;
	}
	FileCursor.Cursor += sizeof(wav_chunk_header) + ChunkSize;
	return FileCursor;
}

INLINE bool IsFileCursorValid(wav_file_cursor FileCursor)
{
	return ( FileCursor.Cursor < FileCursor.EndOfFile );
}

INLINE void *GetChunkData(wav_file_cursor FileCursor)
{
	void *Result = FileCursor.Cursor + sizeof(wav_chunk_header);
	return Result;
}

INLINE int GetChunkSize(wav_file_cursor FileCursor)
{
	wav_chunk_header *WavChunkHeader = (wav_chunk_header *)FileCursor.Cursor;
	return WavChunkHeader->ChunkSize;	
}

INLINE int GetType(wav_file_cursor FileCursor)
{
	wav_chunk_header *WavChunkHeader = (wav_chunk_header *)FileCursor.Cursor;
	int Result = WavChunkHeader->ChunkID;
	return Result;
}

internal loaded_bitmap DEBUGLoadBMP(debug_platform_read_entire_file *ReadEntireFile,
                                    char *FileName)
{
	loaded_bitmap Bitmap = {};
	Bitmap.Scale = MASTER_BITMAP_SCALE;
	debug_read_file_result FileResult = ReadEntireFile(FileName);
	if (FileResult.ContentSize != 0)
	{
		bitmap_header *Header = (bitmap_header *)FileResult.Contents;
		Bitmap.PixelPointer = (unsigned int *) ( (unsigned char *)FileResult.Contents + Header->BitmapOffset );
		Bitmap.Height = Header->Height;
		Bitmap.Width = Header->Width;
        
		//below is code to adjust the fact that in the bitmap the colors are reprresented like 
		//0xRRGGBBAA where we interperet colors as 0xAARRGGBB. It's trivial stuff.
        
        // TODO: In addition, proper handling of color mode is done here as well.
		unsigned int *SourceDest = Bitmap.PixelPointer;
		for (int y = 0; y < Header->Height; ++y)
		{
			for (int x = 0; x < Header->Width; ++x)
			{
				if (GLOBAL_COLOR_MODE == BGR)
                {
                    *SourceDest++ = ( (*SourceDest >> 8) | (*SourceDest << 24) );
                } else if (GLOBAL_COLOR_MODE == RGB)
                {
                    unsigned int pixel_val = *SourceDest;
                    
                    unsigned int R = (pixel_val >> 24) & 0x000000FF;
                    unsigned int G = (pixel_val >> 16) & 0x000000FF;
                    unsigned int B = (pixel_val >> 8) & 0x000000FF;
                    unsigned int A = pixel_val & 0x000000FF;
                    
                    *SourceDest++ = (A << 24) | (B << 16) | (G << 8) | (R);
                }
            }
		}
	}
	return Bitmap;
}

internal loaded_wav DEBUGLoadWav(debug_platform_read_entire_file *ReadEntireFile,
                                 char *FileName)
{
	loaded_wav WavFile = {};
	debug_read_file_result FileResult = ReadEntireFile(FileName);
	if (FileResult.ContentSize != 0 )
	{
		//do wave file stuff here
		wav_header *WavHeader = (wav_header *)FileResult.Contents;
		
		Assert(WavHeader->ChunkID == Wav_ChunkID_RIFF);
		Assert(WavHeader->WaveID == Wav_ChunkID_WAVE);
		//NOTE: THe end of file computation explained: We go ahead by the initial header size, then add WavHeader->FileSize,
		//which excludes the 4 byte value after it, so we subtract 4 bytes.
		short *Samples = 0;
		int Channels = 0;
		int SampleDataSize = 0;
		for( wav_file_cursor FileCursor = ParseChunkAt(WavHeader + 1, (char *)(WavHeader + 1) + WavHeader->FileSize - 4);
			IsFileCursorValid(FileCursor);
			FileCursor = NextChunk(FileCursor) )
		{
			switch( GetType(FileCursor) )
			{
				case Wav_ChunkID_fmt:
				{
					wav_fmt *Wavfmt = (wav_fmt *)GetChunkData(FileCursor);
					Assert(Wavfmt->wFormatTag == 1);
					Assert(Wavfmt->nSamplesPerSec == 48000);
					Assert(Wavfmt->wBitsPerSample == 16);
					Channels = Wavfmt->nChannels;
				}
				break;
				case Wav_ChunkID_data:
				{
					Samples = (short *)GetChunkData(FileCursor);
					SampleDataSize = GetChunkSize(FileCursor);
				}
				break;
				default:
				{
                    
				}
				break;
			}
		}
		Assert( (Channels && Samples && SampleDataSize) );
        
		WavFile.SampleCount = SampleDataSize / ( Channels * sizeof(short) );
		WavFile.Channels = Channels;
		if(Channels == 1)
		{
			WavFile.SampleData[0] = Samples;
		}
		else if (Channels == 2)
		{
			WavFile.SampleData[0] = Samples;
			WavFile.SampleData[1] = Samples + WavFile.SampleCount;
            
			//NOTE: Here I uninterleave the left channel. Very nice.
			for (int SampleIndex = 0; SampleIndex < WavFile.SampleCount;++SampleIndex)
			{
				Samples[SampleIndex] = Samples[SampleIndex * 2]; 
			}
		}
		else
		{
			Assert(!"Unsupported Channel type!");
		}
        
	}
	return WavFile;
}

internal loaded_asset InitializeAsset(memory_arena *WorldArena, unsigned int AssetSize, unsigned int AssetChunks)
{
	loaded_asset NewAsset = {};
    
	unsigned int NewAssetSize = AssetChunks * AssetSize + AssetChunks * sizeof(asset_header); 
	NewAsset.AssetSize = NewAssetSize;
	NewAsset.RawData = (unsigned char *)PushSize(WorldArena, NewAssetSize);
	
	return NewAsset; 
}

//this function will take an asset, some data, the data length, and pump that shit to an asset file.
internal loaded_asset PumpAssetUnchecked(loaded_asset Asset, unsigned char *Data, unsigned int ChunkSize)
{
	asset_header AssetHeader = {};
	asset_header LastHeader = Asset.Headers[ SafeSubtract(Asset.HeaderCount, 1) ];
    
	AssetHeader.OffsetBytes = LastHeader.OffsetBytes + LastHeader.NextChunkSize + sizeof(asset_header);
	AssetHeader.NextChunkSize = ChunkSize;
	AssetHeader.HeaderCode = RIFF_CODE('h','e','a','d');
	Asset.Headers[Asset.HeaderCount++] = AssetHeader;
    
	unsigned char *BytePointer = Asset.RawData + AssetHeader.OffsetBytes - sizeof(asset_header);
	*(asset_header *)BytePointer = AssetHeader;
	
	BytePointer += sizeof(asset_header);
	for (unsigned int x = 0; x < ChunkSize; x++)
	{
		*BytePointer++ = *Data++;
	}
    
	return Asset;
}

//this will finalize the asset, and write it to disk.
internal void WriteAsset(debug_platform_write_entire_file *WriteEntireFile, loaded_asset Asset, char *BaseFilePath, char *FileName)
{
	char StringBuffer[256];
	char *AbsoluteFilePath = GameCatStrings(BaseFilePath, FileName, StringBuffer);
	WriteEntireFile(AbsoluteFilePath, Asset.AssetSize, Asset.RawData);
}

//MemoryArena is a memory arena
//data is a list of structs
//datasize is the size of one of those structs
//chunk count is the amount of elements in array
#define BuildAsset(MemoryArena, Data, DataSize, ChunkCount, BaseFilePath, FileName)\
{\
	unsigned char *DataAsBytes = (unsigned char *)Data;\
	loaded_asset Asset = InitializeAsset( MemoryArena, DataSize, ChunkCount);\
	for (unsigned int x = 0; x < ChunkCount; x++)\
	{\
		Asset = PumpAssetUnchecked(Asset, DataAsBytes, DataSize );\
		DataAsBytes += DataSize;\
	}\
	WriteAsset(DEBUGPlatformWriteEntireFile, Asset, BaseFilePath, FileName);\
}

//NOTE: Assets must exist in the same folder always and therefore we only need to pass this function a 
//non absolute file path but instead a local one.
internal loaded_asset LoadAsset(debug_platform_read_entire_file *ReadEntireFile, char *BaseFilePath, char *FileName)
{
	loaded_asset Result = {};
    
	char StringBuffer[256];
	char *AbsoluteFilePath = GameCatStrings(BaseFilePath, FileName, StringBuffer);
	debug_read_file_result FileResult = ReadEntireFile(AbsoluteFilePath);
	
	if (FileResult.Contents && FileResult.ContentSize)
	{
		Result.RawData = (unsigned char *)FileResult.Contents;
		unsigned char *FilePointer = Result.RawData;
		for (int x = 0; x < FileResult.ContentSize; x++ )
		{
			if ( *(unsigned int *)FilePointer == RIFF_CODE('h','e','a','d') )
			{
				if (Result.HeaderCount <= 16) //ensure that we don't grab more headers than we can.
				{
					Result.Headers[Result.HeaderCount++] = *(asset_header *)FilePointer;
				}
			}
			FilePointer++;
		} 
	}
    
	return Result;	
}

//will parse through the asset and contruct a list of all blocks 
//into a destination list who is some generic structure.
#define ParseAsset(Asset, Dest, type){\
    type *DestPointer = Dest;\
    for (unsigned int x =  0; x < Asset.HeaderCount; x++)\
    {\
        asset_header Header = Asset.Headers[x];\
        *DestPointer++ = *(type *)(Asset.RawData + Header.OffsetBytes);\
    }\
}

#define PIKABLUE_DATABASE_MAX_LINE_COUNT 1000

//parse through file return list of all lines
internal unsigned int ReadLines(debug_platform_read_entire_file *ReadEntireFile, memory_arena *MemoryArena, char *BaseFilePath, char *FileName, char **Dest)
{
	char StringBuffer[256];
	char *AbsoluteFilePath = GameCatStrings(BaseFilePath, FileName, StringBuffer);
    
	//char *Dest[PIKABLUE_DATABASE_MAX_LINE_COUNT]; //list of strings (character pointers)
	char BackBuffer[256];
	char *Buffer = BackBuffer;
	unsigned int LineIndex = 0;
	unsigned int LookingForLineEnd = 0;
    
	debug_read_file_result FileResult = ReadEntireFile(AbsoluteFilePath);
	if (FileResult.Contents && FileResult.ContentSize)
	{
		unsigned char *Scan = (unsigned char *)FileResult.Contents;
		for (int x = 0; x < FileResult.ContentSize; x++)
		{
			if (!LookingForLineEnd)
			{
				*Buffer++ = *Scan;
			}
			Scan++;
			if (*Scan == ('\n')) {
				LookingForLineEnd = false;
				Scan += 1;
			} else if ( *(unsigned short *)Scan == (unsigned short)(('\n' << 8) | '\r') ) 
			{
				LookingForLineEnd = false;
				Scan += 2;
			}
			else if ( *Scan == ';' )
			{
				*Buffer = 0; //end the string 
				LookingForLineEnd = true;
				///////////
				unsigned int StringLength = GetStringLength(BackBuffer);
				Dest[LineIndex] = (char *)PushArray(MemoryArena, StringLength + 1, char);
				CloneString(BackBuffer, Dest[LineIndex++], StringLength);
				//////////////
				Buffer = BackBuffer; //reset buffer
			}
		}	
	}
	return LineIndex;
}

internal unsigned int SplitLineToInt(char *Line, unsigned int *Dest)
{
	char *Scan = Line;
	char BackBuffer[256] = {};
	char *Buffer = BackBuffer;
	unsigned int Count = 0;
	while(*Scan)
	{
		*Buffer++ = *Scan++;
		if (*Scan == ' ')
		{
			*Buffer = 0; //end the string
			*Dest++ = ASCIIToNumber(BackBuffer);
			Buffer = BackBuffer;
			Count++;  
		}
		
	}
	*Buffer = 0; //end the string
	*Dest = ASCIIToNumber(BackBuffer);
	return ++Count;	
}

internal unsigned int SplitLineToFloat(char *Line, float *Dest)
{
	char *Scan = Line;
	char BackBuffer[256] = {};
	char *Buffer = BackBuffer;
	unsigned int Count = 0;
	while(*Scan)
	{
		*Buffer++ = *Scan++;
		if (*Scan == ' ')
		{
			*Buffer = 0; //end the string
			*Dest++ = ASCIIToFloat(BackBuffer);
			Buffer = BackBuffer;
			Count++;  
		}
		
	}
	*Buffer = 0; //end the string
	*Dest = ASCIIToFloat(BackBuffer);
	return ++Count;
}

internal void LoadFloatMatrix(float *Dest, char **Lines, unsigned int AmountOfLines)
{
	//char StringBuffer[256];
	for (unsigned int y = 0; y < AmountOfLines; y++)
	{
		//split the line
		float Line[256];
		unsigned int LineCount = SplitLineToFloat(Lines[y],Line); 
		for (unsigned int x = 0; x < LineCount; x++)
		{
			Dest[y * LineCount + x] = Line[x];
		}
        
	}
}

internal void LoadIntegerMatrix(unsigned int *Dest, char **Lines, unsigned int AmountOfLines)
{
	// char StringBuffer[256];
	for (unsigned int y = 0; y < AmountOfLines; y++)
	{
		// split the line
		unsigned int Line[256];
		unsigned int LineCount = SplitLineToInt(Lines[y],Line);
		for (unsigned int x = 0; x < LineCount; x++)
		{
			Dest[y * LineCount + x] = Line[x];
		}
	}
}

internal void LoadRawFile(debug_platform_read_entire_file ReadEntireFile,char *FileName, void *Dest)
{  
	debug_read_file_result FileResult = ReadEntireFile(FileName);
	if (FileResult.Contents && FileResult.ContentSize)
	{
		char *Source = (char *)FileResult.Contents;
		char *Location = (char *)Dest;
		for (int x = 0; x < FileResult.ContentSize; x++)
		{
			*Location++ = *Source++;
		}
	}
}