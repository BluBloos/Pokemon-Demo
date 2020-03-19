inline loaded_bitmap GrabTileBitmap(loaded_bitmap *TileSet, unsigned int Index)
{
	Assert(Index != 0);
	return TileSet[Index - 1];
}

inline unsigned int PackTile(unpacked_tile Tile)
{
	unsigned int Result = 0;
	Result = Result | Tile.TileType;
	Result = Result | (Tile.AreaIdentifier << 16);
	Result = Result | (Tile.GrassFlag << 24);
	Result = Result | (Tile.WaterFlag << 25);
	Result = Result | (Tile.LadderFlag << 26);
	Result = Result | (Tile.LadderDirection << 27);
	Result = Result | (Tile.Walkable << 28);
	Result = Result | (Tile.Transparent << 29);
	Result = Result | (Tile.AbovePlayer << 30);
	return Result;
}

inline unpacked_tile UnPackTile(unsigned int Tile)
{
	unpacked_tile Result = {};
	Result.TileType = Tile & 0x0000FFFF;
	Result.AreaIdentifier = (Tile >> 16) & 0x000000FF;
	Result.GrassFlag = (Tile >> 24) & 0x00000001;
	Result.WaterFlag = (Tile >> 25) & 0x00000001;
	Result.LadderFlag = (Tile >> 26) & 0x00000001;
	Result.LadderDirection = (Tile >> 27) & 0x00000001;
	Result.Walkable = (Tile >> 28) & 0x00000001;
	Result.Transparent = (Tile >> 29) & 0x00000001;
	Result.AbovePlayer = (Tile >> 30) & 0x00000001;
	return Result;
}

inline void UnPackTransparentTile(unpacked_tile Tile, unsigned int *BackgroundIndex, unsigned int *ForegroundIndex)
{
	*ForegroundIndex = Tile.TileType & 0x000000FF;
	*BackgroundIndex = Tile.TileType >> 8;
}

inline unpacked_tile MakeTransparentTile(unpacked_tile Tile, unsigned int BackgroundIndex, unsigned int ForegroundIndex)
{
	Tile.Transparent = 1;
	Tile.TileType = (BackgroundIndex << 8) | ForegroundIndex;
	return Tile;
}

inline unpacked_tile GetTileMapValueUnchecked(tile_map *TileMap, tile_chunk *Chunk,
 unsigned int TileX, unsigned int TileY)
{
	unsigned int Result = Chunk->Tiles[TileY * TileMap->ChunkSize + TileX];
	return UnPackTile(Result);
}

inline void SetTileMapValueUnchecked(tile_map *TileMap, tile_chunk *Chunk, 
	unsigned int X, unsigned int Y, unpacked_tile Value)
{
	unsigned int PackedTile = PackTile(Value);
	Chunk->Tiles[Y * TileMap->ChunkSize + X] = PackedTile; 
}

inline void SetTile(tile_map *TileMap, tile_chunk *Chunk,
 unsigned int TileX, unsigned int TileY, unpacked_tile Value)
{
	if(Chunk)
	{
		SetTileMapValueUnchecked(TileMap, Chunk, TileX, TileY, Value);
	}
}

inline tile_chunk *GetTileChunk(tile_map *TileMap, unsigned int X, unsigned int Y, unsigned int Z)
{
	tile_chunk *TileChunk = 0;
	if ( (X < TileMap->WorldCountX) && (Y < TileMap->WorldCountY) && (Z < TileMap->WorldCountZ))
	{
		TileChunk = &TileMap->TileChunks[Z *TileMap->WorldCountY * TileMap->WorldCountX + Y * TileMap->WorldCountX + X];
	}
	return TileChunk;
}

inline unpacked_tile GetTileValue(tile_map *TileMap, tile_chunk *Chunk,
 unsigned int TileX, unsigned int TileY)
{
	unpacked_tile TileMapValue = {};
	if (Chunk && Chunk->Tiles)
	{
		TileMapValue = GetTileMapValueUnchecked(TileMap, Chunk,
		 TileX, TileY);
	}

	return TileMapValue;
}

inline void ReCalcCoord(tile_map *TileMap, unsigned int *Tile, float *offset)
{
	signed int TileOffset = FloorFloatToInt( *offset / TileMap->TileSizeInMeters );

	*Tile += TileOffset;

	*offset -= TileOffset * TileMap->TileSizeInMeters;
	/*
	Assert(*offset >= 0);
	Assert(*offset < World->TileSideInMeters);
	*/
}

inline tile_map_position ReCalcTileMapPosition(tile_map *TileMap,
 tile_map_position MapPos)
{
	tile_map_position Result = MapPos;

	ReCalcCoord(TileMap, &Result.AbsTileX, &Result.X);
	ReCalcCoord(TileMap, &Result.AbsTileY, &Result.Y);
	
	return Result;
}

inline tile_chunk_position GetChunkPos(tile_map *TileMap, int AbsTileX, int AbsTileY, int AbsTileZ)
{
	tile_chunk_position Result;
	Result.TileChunkX = AbsTileX >> TileMap->ChunkShift;
	Result.TileChunkY = AbsTileY >> TileMap->ChunkShift;
	Result.TileChunkZ = AbsTileZ;
	Result.TileX = AbsTileX & TileMap->ChunkMask;
	Result.TileY = AbsTileY & TileMap->ChunkMask;
	return Result;
}

internal unpacked_tile GetTileValue(tile_map *TileMap, unsigned int AbsTileX, unsigned int AbsTileY, unsigned int AbsTileZ)
{
	unpacked_tile Value = {};

	tile_chunk_position ChunkPos = GetChunkPos(TileMap, AbsTileX, AbsTileY, AbsTileZ);
	tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
	Value = GetTileValue(TileMap, TileChunk, ChunkPos.TileX, ChunkPos.TileY);

	return Value;
}

inline bool GetTileValueValid(unpacked_tile TileValue)
{
	return (TileValue.Walkable == 1);
}

internal bool QueryAdjacentTile(tile_map *TileMap, int TileX, int TileY, int TileZ, Direction _Direction)
{
	bool Result = true;
	if ( _Direction == LEFT)
	{
		unpacked_tile TileValue = GetTileValue(TileMap, TileX - 1, TileY, TileZ);
		if (GetTileValueValid(TileValue))
		{
			Result = false;
		} 
	}
	if (_Direction == RIGHT)
	{
		unpacked_tile TileValue = GetTileValue(TileMap, TileX + 1, TileY, TileZ);
		if (GetTileValueValid(TileValue))
		{
			Result = false;
		} 
	}
	if (_Direction == UP)
	{
		unpacked_tile TileValue = GetTileValue(TileMap, TileX, TileY + 1, TileZ);
		if (GetTileValueValid(TileValue))
		{
			Result = false;
		} 
	}
	if (_Direction == DOWN)
	{
		unpacked_tile TileValue = GetTileValue(TileMap, TileX, TileY - 1, TileZ);
		if (GetTileValueValid(TileValue))
		{
			Result = false;
		} 
	}
	return Result;
}

internal bool FloodTileWalkable(tile_map *TileMap, unsigned int TileX, unsigned int TileY,
	unsigned int TileZ, float X, float Y, unsigned int MovDir, float PlayerRadius)
{
	bool Result = false;
	unpacked_tile TileValue = GetTileValue(TileMap, TileX, TileY, TileZ);
	if (GetTileValueValid(TileValue))
	{
		if ( (MovDir == RIGHT) | (MovDir == LEFT) )
		{
			bool TileAbove = QueryAdjacentTile(TileMap, TileX, TileY, TileZ, UP);
			float MaxY = (TileAbove)? TileMap->TileSizeInMeters - PlayerRadius * 2.0f: TileMap->TileSizeInMeters;
			if ( Y <= MaxY )
			{
				Result = true;
			}
		}
		if ( (MovDir == UP) | (MovDir == DOWN) )
		{
			bool TileLeft = QueryAdjacentTile(TileMap, TileX, TileY, TileZ, LEFT);
			bool TileRight = QueryAdjacentTile(TileMap, TileX, TileY, TileZ, RIGHT);
			float MinX = (TileLeft)? PlayerRadius: 0.0f;
			float MaxX = (TileRight)? TileMap->TileSizeInMeters - PlayerRadius: TileMap->TileSizeInMeters;
			if ( (X >= MinX) && (X <= MaxX) )
			{
				Result = true;
			}
		}
	}
	return Result;
}

internal tile_map_position Flood(tile_map *TileMap, tile_map_position InitialPos, tile_map_position TargetPos, unsigned int MovDir, float PlayerRadius)
{
	tile_map_position Result = InitialPos;
	if (MovDir == UP)
	{
		for (unsigned int y = InitialPos.AbsTileY + 1; y != (TargetPos.AbsTileY + 1); ++y )
		{
			if (!FloodTileWalkable(TileMap, TargetPos.AbsTileX, y, TargetPos.AbsTileZ, TargetPos.X, TargetPos.Y, MovDir, PlayerRadius))
			{
				TargetPos.AbsTileY = y - 1;
				TargetPos.Y = TileMap->TileSizeInMeters - 2.0f * PlayerRadius;
				Result = TargetPos;
				break;
			}
			else
			{
				if ( y == TargetPos.AbsTileY )
				{
					if( QueryAdjacentTile(TileMap, TargetPos.AbsTileX, y, TargetPos.AbsTileZ, UP) )
					{
						float MaxY = TileMap->TileSizeInMeters - 2.0f * PlayerRadius;
						TargetPos.Y = (TargetPos.Y > MaxY)? MaxY: TargetPos.Y;
					}
					Result = TargetPos;
					break;
				}
				else
				{
					continue;
				}
			}
		}
	}
	if (MovDir == DOWN)
	{
		for (unsigned int y = InitialPos.AbsTileY - 1; y != (TargetPos.AbsTileY - 1); --y )
		{
			if (!FloodTileWalkable(TileMap, TargetPos.AbsTileX, y, TargetPos.AbsTileZ, TargetPos.X, TargetPos.Y, MovDir, PlayerRadius))
			{
				TargetPos.AbsTileY = y + 1;
				TargetPos.Y = 0.0f;
				Result = TargetPos;
				break;
			}
			else
			{
				if ( y == TargetPos.AbsTileY )
				{
					Result = TargetPos;
					break;
				}
				else
				{
					continue;
				}
			}
		}
	}
	if (MovDir == LEFT)
	{
		for (unsigned int x = InitialPos.AbsTileX - 1; x != (TargetPos.AbsTileX - 1); --x )
		{
			if (!FloodTileWalkable(TileMap, x, TargetPos.AbsTileY, TargetPos.AbsTileZ, TargetPos.X, TargetPos.Y, MovDir, PlayerRadius))
			{
				TargetPos.AbsTileX = x + 1;
				TargetPos.X = PlayerRadius;
				Result = TargetPos;
				break;
			}
			else
			{
				if ( x == TargetPos.AbsTileX )
				{
					if (QueryAdjacentTile(TileMap, x, TargetPos.AbsTileY, TargetPos.AbsTileZ, LEFT))
					{
						float MinX = PlayerRadius;
						TargetPos.X = (TargetPos.X < MinX)? MinX: TargetPos.X;
					}
					Result = TargetPos;
					break;
				}
				else
				{
					continue;
				}
			}
		}
	}
	if (MovDir == RIGHT)
	{
		for (unsigned int x = InitialPos.AbsTileX + 1; x != (TargetPos.AbsTileX + 1); ++x )
		{
			if (!FloodTileWalkable(TileMap, x, TargetPos.AbsTileY, TargetPos.AbsTileZ, TargetPos.X, TargetPos.Y, MovDir, PlayerRadius))
			{
				TargetPos.AbsTileX = x - 1;
				TargetPos.X = TileMap->TileSizeInMeters - PlayerRadius;
				Result = TargetPos;
				break;
			}
			else
			{
				if ( x == TargetPos.AbsTileX )
				{
					if (QueryAdjacentTile(TileMap, x, TargetPos.AbsTileY, TargetPos.AbsTileZ, RIGHT))
					{
						float MaxX = TileMap->TileSizeInMeters - PlayerRadius;
						TargetPos.X = (TargetPos.X > MaxX)? MaxX: TargetPos.X;
					}
					Result = TargetPos;
					break;
				}
				else
				{
					continue;
				}
			}
		}
	}
	return Result;
}

internal tile_map_position QueryNewTileMapPos(tile_map *TileMap, tile_map_position InitialPos, tile_map_position TargetPos, unsigned int MovDir, float PlayerRadius)
{
	//If anything fails we don't move
	tile_map_position Result = InitialPos;
	if ( (InitialPos.AbsTileX == TargetPos.AbsTileX) && (InitialPos.AbsTileY == TargetPos.AbsTileY) )
	{
		bool TileAbove = !GetTileValueValid( GetTileValue(TileMap, InitialPos.AbsTileX, InitialPos.AbsTileY + 1, InitialPos.AbsTileZ) );
		bool TileLeft = !GetTileValueValid( GetTileValue(TileMap, InitialPos.AbsTileX - 1, InitialPos.AbsTileY, InitialPos.AbsTileZ) );
		bool TileRight = !GetTileValueValid( GetTileValue(TileMap, InitialPos.AbsTileX + 1, InitialPos.AbsTileY, InitialPos.AbsTileZ) );
		bool TileUpperRight = !GetTileValueValid( GetTileValue(TileMap, InitialPos.AbsTileX + 1, InitialPos.AbsTileY + 1, InitialPos.AbsTileZ) );
		bool TileUpperLeft = !GetTileValueValid( GetTileValue(TileMap, InitialPos.AbsTileX - 1, InitialPos.AbsTileY + 1, InitialPos.AbsTileZ) );

		float MaxY = (TileAbove)? TileMap->TileSizeInMeters - PlayerRadius * 2.0f: TileMap->TileSizeInMeters;
		float MinY = 0.0f;
		float MinX = (TileLeft)? PlayerRadius: 0.0f;
		float MaxX = (TileRight)? TileMap->TileSizeInMeters - PlayerRadius: TileMap->TileSizeInMeters;

		if (!TileAbove)
		{
			if (TileUpperLeft && (TargetPos.Y > (TileMap->TileSizeInMeters - PlayerRadius * 2.0f)  ) )
			{
				MinX = PlayerRadius;
			}
			if (TileUpperRight && (TargetPos.Y > (TileMap->TileSizeInMeters - PlayerRadius * 2.0f)  ) )
			{
				MaxX = TileMap->TileSizeInMeters - PlayerRadius;
			}
		}

		if (TargetPos.X < MinX)
		{
			TargetPos.X = MinX;
		}
		if (TargetPos.X > MaxX)
		{
			TargetPos.X = MaxX;
		}

		if (TargetPos.Y < MinY)
		{
			TargetPos.Y = MinY;
		}
		if (TargetPos.Y > MaxY)
		{
			TargetPos.Y = MaxY;
		}

		Result = TargetPos;
	}
	else
	{
		TargetPos = Flood(TileMap, InitialPos, TargetPos, MovDir, PlayerRadius);
		Result = TargetPos;
	}
	return Result;
}

internal void SetTile(memory_arena *Arena, tile_map *TileMap, unsigned int AbsTileX, unsigned int AbsTileY, unsigned int AbsTileZ, unpacked_tile Value)
{
	tile_chunk_position ChunkPos = GetChunkPos(TileMap, AbsTileX, AbsTileY, AbsTileZ);
	tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);

	if (!TileChunk->Tiles)
	{
		TileChunk->Tiles = PushArray(Arena, SquareInt( TileMap->ChunkSize ), unsigned int );
	}

	SetTile(TileMap, TileChunk, ChunkPos.TileX, ChunkPos.TileY, Value);
}

internal void SetChunk(tile_map *TileMap, unsigned int ChunkX, unsigned int ChunkY, unsigned int ChunkZ, unsigned int *Tiles)
{
	tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkX, ChunkY, ChunkZ);
	TileChunk->Tiles = Tiles;
}

internal tile_chunk LoadChunk(debug_platform_read_entire_file *ReadEntireFile, char *FileName)
{
	tile_chunk Result = {};
	debug_read_file_result FileResult = ReadEntireFile(FileName);
	if (FileResult.ContentSize != 0)
	{
		Result.Tiles = (unsigned int *)FileResult.Contents;
	}
	return Result;
}
