struct unpacked_tile
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
};

struct tile_chunk
{
	unsigned int *Tiles;
};

struct tile_chunk_position
{
	unsigned int TileChunkX;
	unsigned int TileChunkY;
	unsigned int TileChunkZ;
	
	unsigned int TileX;
	unsigned int TileY;
};

struct tile_map_position
{
	unsigned int AbsTileX;
	unsigned int AbsTileY;
	unsigned int AbsTileZ;

 	float X; //now these are in meters, yay!
 	float Y;
};

//NOTE: The tile map structure is a thing that contains like all 
//the tile chunks
struct tile_map 
{
	unsigned int ChunkMask;
	unsigned int ChunkShift;

	float TileSizeInMeters;

	unsigned int WorldCountX;
	unsigned int WorldCountY;
	unsigned int WorldCountZ;
	int ChunkSize;

	tile_chunk *TileChunks;
};