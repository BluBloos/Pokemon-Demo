/*
Hi thank you for choosing the PikaBlue engine!
This file contains all the engine functions 
that don't adhere to some other category of 
functions.
*/

//NOTE: Make sure to keep functions in this file functional. Because, after all, the name of this file 
//is "*Functions.cpp", so it ought to be functional. Right?

//this is pretty functional. Just takes in a list and an Id and returns something
INLINE pokemon_move FillPokemonMove(pokemon_move *MoveDatabase, unsigned int MoveID)
{
	pokemon_move Result = {};
	pokemon_move Move = MoveDatabase[MoveID];
    
	//NOTe below I do a lot of cloning just a warning!
	//It's a warning because I think its dumb.
	//I think generally if you have to clone something you may be doing something wrong
	//although there are exceptions, sometimes. Maybe this is an exception. Who knows?
	Result.Effect = Move.Effect;
	Result.Type = Move.Type;
	Result.BasePower = Move.BasePower;
	Result.Accuracy = Move.Accuracy;
	Result.MaxPP = Move.PP;
	Result.PP = Move.PP;
	Result.EffectAccuracy = Move.EffectAccuracy;
	Result.AffectsWhom = Move.AffectsWhom;
	Result.Priority = Move.Priority;
	Result.Flags = Move.Flags;
	CloneString(Move.Name, Result.Name, 15);
    
	return Result;
}

//this function will ask if they are equal and if they are not it will check if they are equal by some 
//very stupid small amount.
INLINE unsigned int AreFloatsEqual(float Float1, float Float2)
{
	unsigned int Result = false;
	if (Float1 == Float2)
	{
		Result = true;
	}
	else if ( Abs(Float1 - Float2) < Pow(10.0f,-16.0f) )
	{
		Result = true;
	}
	return Result;
}


//will take ASCII and shoot back the official pokemon demo font
internal loaded_bitmap BitmapFromASCII(loaded_bitmap *PokemonDemoFont, char Character)
{
	loaded_bitmap Result = {};
	unsigned int Index = (unsigned int) Character;
    
	if ( (Character >= 65) && (Character <= 90) )
	{
		Index -= 65;
	}
	else if ( (Character >= 97) && (Character <= 122) )
	{
		Index -= 67;	
	}
	else if ( (Character >= 48) && (Character <= 57) )
	{
		Index += 12;
	}
	else if (Character == '!')
	{
		Index = 26;
	}
	else if (Character == '.')
	{
		Index = 27;
	}
	else if (Character == '?')
	{
		Index = 28;
	}
	else if ( Character == ',' || Character == 39 ) 
	{
		Index = 29;
	}
	else
	{
		Index = 0;
	}
    
	Result = PokemonDemoFont[Index];
	return Result;
}

internal loaded_bitmap GetSpriteByRect(memory_arena *WorldArena, loaded_bitmap *SpriteMap, unsigned int *CharacterPointer, int Width, int Height,
                                       unsigned int BackgroundColor)
{
	loaded_bitmap Result = {};
	Result.Scale = MASTER_BITMAP_SCALE;
    
	if ( !(Width < 0) )
	{
		Result.Width = Width;
		Result.Height = Height;
		Result.PixelPointer = PushArray(WorldArena, (Result.Width * Result.Height), unsigned int);
        
		unsigned int *Row = CharacterPointer;
		unsigned int *DestRow = Result.PixelPointer + Result.Width * (Result.Height - 1);
        
		for (unsigned int Y = 0; Y < Result.Height; Y++)
		{	
			unsigned int *Pixel = (unsigned int *)Row;
			unsigned int *DestPixel = (unsigned int *)DestRow;
			for (unsigned int X = 0; X < Result.Width; X++)
			{
				if ( (BackgroundColor) && (*Pixel == BackgroundColor) ) 
				{
					*DestPixel = 0x00000000;
				}
				else
				{
					*DestPixel = *Pixel | 0xFF000000;	
				}
				DestPixel++;
				Pixel++;
			}
			DestRow -= Result.Width;
			Row -= SpriteMap->Width;
		}
	}
	else
	{
		//There is no sprite here just background
		//so just return a fully black sprite.
		Result.Width = 16;
		Result.Height = 16;
		Result.PixelPointer = PushArray(WorldArena, Result.Width * Result.Height, unsigned int);
	}
    
	return Result;
}

internal loaded_bitmap GetSpriteFromSpriteMap(memory_arena *WorldArena,
                                              loaded_bitmap *SpriteMap, unsigned int *PixelPointer, unsigned int TileSize)
{
	unsigned int MinX = TileSize;
	unsigned int MaxX = 0;
	unsigned int MinY = TileSize;
	unsigned int MaxY = 0;
    
	unsigned int *Row = PixelPointer;
	unsigned int BackgroundColor = *Row;
    
	for (unsigned int Y = 0; Y < TileSize;++Y)
	{	
		unsigned int *Pixel = (unsigned int *)Row;
		for (unsigned int X = 0; X < TileSize; ++X)
		{
			if (*Pixel != BackgroundColor)
			{
				//so I found something that actually has valid data.
				if (X < MinX)
				{
					MinX = X;
				}
				if (X > MaxX)
				{
					MaxX = X;
				}
				if (Y < MinY)
				{
					MinY = Y;
				}
				if (Y > MaxY)					
				{
					MaxY = Y;
				}
			}
			Pixel++;
		}
		Row -= SpriteMap->Width;
	}
    
	unsigned int *NewRow = PixelPointer - MinY * SpriteMap->Width + MinX;
	return GetSpriteByRect(WorldArena, SpriteMap, NewRow, 1 + MaxX - MinX, 1 + MaxY - MinY, BackgroundColor);
}

internal void GenerateByTiles(memory_arena *MemoryArena, loaded_bitmap Bitmap, unsigned int TileWidth, 
                              unsigned int TileHeight, loaded_bitmap *Dest)
{
	unsigned int BitmapTileWidth = Bitmap.Width / TileWidth;
	unsigned int BitmapTileHeight = Bitmap.Height / TileHeight;
	Assert( !(BitmapTileWidth * TileWidth > Bitmap.Width) );
	Assert( !(BitmapTileHeight * TileHeight > Bitmap.Height) );
	unsigned int *Row = Bitmap.PixelPointer + Bitmap.Width * (Bitmap.Height - 1);
	
	for (unsigned int y = 0; y < BitmapTileHeight; y++)
	{
		unsigned int *Block = Row;
		for (unsigned int x = 0; x < BitmapTileWidth; x++)
		{
			Dest[y * BitmapTileWidth +x] = GetSpriteFromSpriteMap(MemoryArena, &Bitmap, Block, TileWidth);
			Block += TileWidth;
		}
		Row -= Bitmap.Width * TileHeight;
	}
}

internal void UnPackBitmapTiles(memory_arena *WorldArena, loaded_bitmap Bitmap,
                                unsigned int TileWidth, unsigned int TileHeight, unsigned int TilePadding, loaded_bitmap *Dest)
{
	unsigned int BitmapTileWidth = (Bitmap.Width + TilePadding) / (TileWidth + TilePadding);
	unsigned int BitmapTileHeight = (Bitmap.Height + TilePadding) / (TileHeight + TilePadding);
	Assert( !(BitmapTileWidth * TileWidth > Bitmap.Width) );
	Assert( !(BitmapTileHeight * TileHeight > Bitmap.Height) );
	unsigned int *Row = Bitmap.PixelPointer + Bitmap.Width * (Bitmap.Height - 1);
	
	for (unsigned int y = 0; y < BitmapTileHeight; y++)
	{
		unsigned int *Block = Row;
		for (unsigned int x = 0; x < BitmapTileWidth; x++)
		{
			Dest[y * BitmapTileWidth +x] = GetSpriteByRect(WorldArena, &Bitmap, Block, TileWidth, TileHeight, 0xFFFFFFFF);
			Block += TileWidth + TilePadding;
		}
		Row -= Bitmap.Width * (TileHeight + TilePadding);
	}
}

internal unsigned int ScreenPositionInRect(game_screen_position ScreenPos, rect Rect)
{
	unsigned int Result = false;	
	if ( (ScreenPos.X > Rect.MinX) && (ScreenPos.X < Rect.MaxX) && (ScreenPos.Y > Rect.MinY) && (ScreenPos.Y < Rect.MaxY) )
	{
		Result = true;
	}
	return Result;
}

internal float Dot(vector2f v1, vector2f v2)
{
	return v1.X * v2.X + v1.Y * v2.Y;
}

internal float LineLength(vector2f p1, vector2f p2)
{
	return Pow(Pow(p2.X - p1.X, 2.0f) + Pow(p2.Y - p1.Y, 2.0f), 0.5f);
}

internal vector2f Normalized(vector2f v)
{
	vector2f origin = {};
	float Mag = LineLength(origin, v);
	v.X = v.X / Mag;
	v.Y = v.Y / Mag;
	return v;
}


vector2f TileMapPosToWorldSpace(tile_map *map, tile_map_position map_pos)
{
    vector2f world_pos;
    
    world_pos.X = map_pos.AbsTileX * map->TileSizeInMeters + 
        map_pos.X;
    world_pos.Y = map_pos.AbsTileY * map->TileSizeInMeters + 
        map_pos.X;
    
    return world_pos;
}


// w.r.t to player
internal entity *GetEntityAhead(tile_map *map, game_state *GameState, entity_npc *Player)
{
	// so what we are going to do is determine the tile right in front of us
	// then check if any entity out of all the entities collide with that tile
	// if they do return that entity
	//entity *Result = NULL;
    
	for (unsigned int x = 0; x < GameState->EntityCount; x++)
	{
		entity Entity = GameState->AllEntities[x];
        
        // Make sure that the entity is not thyself.
        if (Entity.npc != Player)
        {
            
            tile_map_position playerPos = Player->Entity->TileMapPos;
            tile_map_position entityPos = Entity.TileMapPos;
            
            //bool player_on_valid_tile = false;
            
            if (entityPos.AbsTileX == playerPos.AbsTileX)
            {
                if (entityPos.AbsTileY + 1 == playerPos.AbsTileY)
                {
                    if (Player->MoveDirection == DOWN)
                        return &GameState->AllEntities[x];
                }else if (entityPos.AbsTileY - 1 == playerPos.AbsTileY)
                {
                    if (Player->MoveDirection == UP)
                        return &GameState->AllEntities[x];
                }
            }else if (entityPos.AbsTileY == playerPos.AbsTileY)
            {
                if (entityPos.AbsTileX + 1 == playerPos.AbsTileX)
                {
                    if (Player->MoveDirection == LEFT)
                        return &GameState->AllEntities[x];
                }else if (entityPos.AbsTileX - 1 == playerPos.AbsTileX)
                {
                    if (Player->MoveDirection == RIGHT)
                        return &GameState->AllEntities[x];
                }
            }
            
            /*
            vector2f PlayerPos = {};
            vector2f InteractablePos = {}; 
            vector2f PlayerDirection = {};
            PlayerPos = TileMapPosToWorldSpace(map, Player->Entity->TileMapPos);  
            InteractablePos = TileMapPosToWorldSpace(map, Entity.TileMapPos);
            
            //below we shift both positions such that the player is at the origin
            InteractablePos.X -= PlayerPos.X; PlayerPos.X = 0.0f;
            InteractablePos.Y -= PlayerPos.Y; PlayerPos.Y = 0.0f;
            
            // break the player's current direction into
            // the normal vector direction
            if ()
            {
                PlayerDirection.X = 0.0f; PlayerDirection.Y = 1.0f; 
            }
            else if (Player->MoveDirection == DOWN)
            {
                PlayerDirection.X = 0.0f; PlayerDirection.Y = -1.0f;
            }
            else if (Player->MoveDirection == LEFT)
            {
                PlayerDirection.X = -1.0f; PlayerDirection.Y = 0.0f;
            }
            else if (Player->MoveDirection == RIGHT)
            {
                PlayerDirection.X = 1.0f; PlayerDirection.Y = 0.0f;
            }
            
            if ( (LineLength(PlayerPos, InteractablePos) < 1.6f) && 
                (Dot(PlayerDirection, Normalized(InteractablePos)) > 0.5f) )
            {
                Result = &GameState->AllEntities[x];
                break;
            }*/
        }
    }
    
    return NULL; 
} 

// NOTE: This function literally makes an empty entity, with absolutely nothing.
entity *CreateEntity(game_state *gameState)
{
    // Check to see if there is room for a new entity
    if (gameState->EntityCount >= MAX_ENTITIES)
    {
        // NOTE: I throw the program here because I have no error handling code for making too many entities... SO, I hope that it is always the case that there are a small amount of entities.
        Assert(0);
        return NULL;
    }
    
    entity Entity = gameState->AllEntities[gameState->EntityCount++];
    
    return &gameState->AllEntities[gameState->EntityCount - 1];
}

entity_npc *CreateNPC(game_state *gameState)
{
    // make a new entity
    entity *Entity = CreateEntity(gameState);
    
    // make a new npc
    gameState->NPC_Count++;
    
    // entity points to npc
    Entity->npc = &gameState->_npc_storage[gameState->NPC_Count - 1];
    
    // npc points back to entity
    entity_npc *npc = Entity->npc;
    npc->Entity = Entity;
    
    return npc;
}

// make the tile below the entity walkable again, i.e., unfreeze them
void ThawEntity(game_state *GameState, entity *Entity)
{
    tile_map *tileMap = GameState->World->TileMap;
    
    unpacked_tile Tile = GetTileValue(tileMap, Entity->TileMapPos.AbsTileX, Entity->TileMapPos.AbsTileY,
                                      Entity->TileMapPos.AbsTileZ);
    Tile.Walkable = true;
    SetTile(&GameState->WorldArena, tileMap, Entity->TileMapPos.AbsTileX, Entity->TileMapPos.AbsTileY,
            Entity->TileMapPos.AbsTileZ, Tile);
}

int DeleteEntity(game_state *GameState, entity *entityToDelete)
{
    // NOTE: There is not a linear mapping between NPC and entity.
    
    /* Ideas:
    
    - Go through the list of NPC, find it.
    - Get an index instead
    - Put index INSIDE the npc.
    
    */
    
    /* For getting rid of the Entity:
    
    - Remove from the list.
    - If there is an NPC, delete that too.
    
    */
    
    // Find the entity
    int entityIndex = -1;
    int npcIndex = -1;
    
    for (unsigned int i = 0; i < GameState->EntityCount; i++)
    {
        if (&GameState->AllEntities[i] == entityToDelete) 
        {
            // we found the entity
            entityIndex = i;
            break;
        }
    }
    
    if (entityIndex == -1)
        return entityIndex; // failure to find entity, bad return code
    
    // Thaw the entity before deletion
    ThawEntity(GameState, &GameState->AllEntities[entityIndex]);
    
    // Next step: Delete the npc from the array (if there is an NPC)
    
    if (entityToDelete->npc != NULL) 
    {
        for (unsigned int i = 0; i < GameState->NPC_Count; i++)
        {
            if (&GameState->_npc_storage[i] == entityToDelete->npc)
            {
                npcIndex = i;
            }
        }
    }
    
    // DelItemFromArray(GameState->AllEntities, GameState->EntityCount, entityIndex);
    {
        unsigned int LastIndex = GameState->EntityCount - 1;
        
        
        for (unsigned int x = 0; x < LastIndex - entityIndex; x++)
        {
            // Shift entity left
            GameState->AllEntities[entityIndex + x] = GameState->AllEntities[entityIndex + x + 1];
            // Fix the entity pointer in the npc
            entity_npc *npc = GameState->AllEntities[entityIndex + x].npc;
            if (npc != NULL)
                npc->Entity = &GameState->AllEntities[entityIndex + x]; 
        }
        
        GameState->EntityCount--;
    }
    
    if (npcIndex != -1)
    {
        unsigned int LastIndex = GameState->NPC_Count - 1;
        
        for (unsigned int x = 0; x < LastIndex - npcIndex; x++)
        {
            // Shift npc left
            GameState->_npc_storage[npcIndex + x] = GameState->_npc_storage[npcIndex + x + 1];
            // Fix the npc pointer in the entity
            entity *e = GameState->_npc_storage[npcIndex + x].Entity;
            e->npc = &GameState->_npc_storage[npcIndex + x]; 
        }
        
        GameState->NPC_Count--;
    }
    
    return 0;
}