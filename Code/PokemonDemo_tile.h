typedef struct 
{
	unsigned int TileType;
	unsigned int AreaIdentifier;
	unsigned int GrassFlag;
	unsigned int WaterFlag;
	unsigned int LadderDirection;
	unsigned int LadderFlag;
	unsigned int Walkable;
	unsigned int Transparent;
	unsigned int AbovePlayer;
} unpacked_tile;

typedef struct 
{
	unsigned int *Tiles;
} tile_chunk;

typedef struct 
{
	unsigned int TileChunkX;
	unsigned int TileChunkY;
	unsigned int TileChunkZ;
	
	unsigned int TileX;
	unsigned int TileY;
} tile_chunk_position; 

typedef struct 
{
	unsigned int AbsTileX;
	unsigned int AbsTileY;
	unsigned int AbsTileZ;
    
    float X; //now these are in meters, yay!
    float Y;
} tile_map_position;

//NOTE: The tile map structure is a thing that contains like all 
//the tile chunks
typedef struct  
{
	unsigned int ChunkMask;
	unsigned int ChunkShift;
    
	float TileSizeInMeters;
    
	unsigned int WorldCountX;
	unsigned int WorldCountY;
	unsigned int WorldCountZ;
	int ChunkSize;
    
	tile_chunk *TileChunks;
} tile_map;