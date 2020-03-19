#define MASTER_BITMAP_SCALE 2
#define RIFF_CODE(a,b,c,d) (((int)(a << 0)) | ((int)(b << 8)) | ((int)(c << 16)) | ((int)(d << 24)))
enum
{
	Wav_ChunkID_fmt = RIFF_CODE('f','m','t',' '),
	Wav_ChunkID_WAVE = RIFF_CODE('W','A','V','E'),
	Wav_ChunkID_RIFF = RIFF_CODE('R','I','F','F'),
	Wav_ChunkID_data = RIFF_CODE('d','a','t','a')
};

struct wav_header
{
	int ChunkID;
	int FileSize;
	int WaveID;
};

struct wav_chunk_header
{
	int ChunkID;
	int ChunkSize;
};

struct wav_fmt
{
	short wFormatTag; //Format code
	short nChannels; //Number of interleaved channels
	int nSamplesPerSec; //Sampling rate (blocks per second)
	int nAvgBytesPerSec; //Data rate
	short nBlockAlign; //Data block size (bytes)
	short wBitsPerSample; //Bits per sample
	short cbSize; //Size of the extension (0 or 22)
	short wValidBitsPerSample; //Number of valid bits
	int dwChannelMask; //Speaker position mask
	char SubFormat[16]; //GUID, including the data format code
};

struct wav_file_cursor
{
	//Alright so we are going to have a byte pointer and the current chunk that we are in.
	char *Cursor;
	char *EndOfFile;
};

#pragma pack(push, 1)
struct bitmap_header
{
	unsigned short FileType;     /* File type, always 4D42h ("BM") */
	unsigned int FileSize;     /* Size of the file in bytes */
	unsigned short Reserved1;    /* Always 0 */
	unsigned short Reserved2;    /* Always 0 */
	unsigned int BitmapOffset; /* Starting position of image data in bytes */
	unsigned int size;            /* Size of this header in bytes */
	int Width;           /* Image width in pixels */
	int Height;          /* Image height in pixels */
	unsigned short Planes;          /* Number of color planes */
	unsigned short BitsPerPixel;    /* Number of bits per pixel */
};
#pragma pack(pop)

struct debug_read_file_result
{
	void *Contents;
	int ContentSize;
};

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char *FileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) int name(char *FileName, int MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

struct loaded_wav
{
	int SampleCount;
	int Channels;
	short *SampleData[2];
};

struct asset_header
{
	unsigned int HeaderCode;
	unsigned int OffsetBytes; //from start of file to end of header
	unsigned int NextChunkSize; //size in bytes of next chunk.
};

struct loaded_asset
{
	unsigned char *RawData; //I guess we can interpret the raw data on integer boundaries.
	asset_header Headers[16]; //For now we support a maximum of 16 headers.
	unsigned int HeaderCount;
	unsigned int AssetSize;
};

struct pokemon_database_data
{
	unsigned int NationalDexID;
	char Name[16];
	unsigned int Types[2];
	unsigned int Abilities[2];
	unsigned int GenderRatio; //read as how much percent male
	unsigned int CatchRate;
	unsigned int EggGroups[2];
	unsigned int HatchTime;
	float Height; //in meters
	float Weight; //in kilograms
	unsigned int BaseExperienceYield;
	unsigned int LevelingRate;
	unsigned int EvYeilds[6];
	unsigned int BaseStats[6];
	unsigned int BaseFriendship;
}; 

struct save_game
{
	entity PlayerEntity;
	entity_npc Player;
	pokemon PokemonParty;
};

#define TILE_CHUNKS 10

//below is the world save and it stores all the tiles!
struct save_world
{
	unsigned int ChunkCount;
	vector3 ChunkPos[TILE_CHUNKS]; //positions for chunks 
	unsigned int Chunks[TILE_CHUNKS][16][16]; //acutal chunks
};