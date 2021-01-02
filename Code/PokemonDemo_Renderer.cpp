unsigned int ColorFromFloat(float R, float G, float B)
{
    unsigned int Color;
    
    if (GLOBAL_COLOR_MODE == RGB)
    {
        Color = (255 << 24) | (FloatToInt(B * 255.0f) << 16) 
            | (FloatToInt(G * 255.0f) << 8) 
            | FloatToInt(R * 255.0f);
    } else if(GLOBAL_COLOR_MODE == BGR)
    {
        Color = (255 << 24) | (FloatToInt(R * 255.0f) << 16) 
            | (FloatToInt(G * 255.0f) << 8) 
            | FloatToInt(B * 255.0f);
    }
    
    return Color;
}

/*
internal void RenderGradient(game_offscreen_buffer *buffer, int xOffset, int yOffset)
{
 //TODO(Noah): Determine if passing buffer is more efficient as actual value
 unsigned char *Row = (unsigned char *)buffer->memory;
 for (int y = 0; y < buffer->height;++y)
 {
  unsigned int *Pixel = (unsigned int *)Row;
  for (int x = 0; x < buffer->width;++x)
  {
   unsigned char r = 0;
   unsigned char g = (unsigned char)(x+xOffset);
   unsigned char b = (unsigned char)(y+yOffset); 
   *Pixel++ = (r << 16) | (g << 16) | b;
  }
  Row += buffer->pitch;
 }
}*/

internal void DrawRect(game_offscreen_buffer *buffer, float RealMinX, float RealMaxX,
                       float RealMinY, float RealMaxY, float R, float G, float B)
{
	int MinX = FloatToInt(RealMinX);
	int MaxX = FloatToInt(RealMaxX);
	int MinY = FloatToInt(RealMinY);
	int MaxY = FloatToInt(RealMaxY);
    
	if(MinX < 0)
	{
		MinX = 0;
	}
    
	if(MinY < 0)
	{
		MinY = 0;
	}
    
	if(MaxX > buffer->width)
	{
		MaxX = buffer->width;
	}
    
	if(MaxY > buffer->height)
	{
		MaxY = buffer->height;
	}
    
	unsigned int Color = ColorFromFloat(R, G, B); 
    
	unsigned char *Row = ((unsigned char *)buffer->memory) 
        + MinX*buffer->BytesPerPixel 
        + MinY*buffer->pitch;
    
	for (int Y = MinY; Y < MaxY;++Y)
	{	
		unsigned int *Pixel = (unsigned int *)Row;
		for (int X = MinX; X < MaxX; ++X)
		{
			*Pixel++ = Color;
		}
		Row += buffer->pitch;
	}
}


internal void DrawRectWithOpacity(game_offscreen_buffer *buffer, float RealMinX, float RealMaxX,
                                  float RealMinY, float RealMaxY, float R, float G, float B, float Alpha)
{
    int MinX = FloatToInt(RealMinX);
    int MaxX = FloatToInt(RealMaxX);
    int MinY = FloatToInt(RealMinY);
    int MaxY = FloatToInt(RealMaxY);
    
    if(MinX < 0)
    {
        MinX = 0;
    }
    
    if(MinY < 0)
    {
        MinY = 0;
    }
    
    if(MaxX > buffer->width)
    {
        MaxX = buffer->width;
    }
    
    if(MaxY > buffer->height)
    {
        MaxY = buffer->height;
    }
    
    //unsigned int Color = ColorFromFloat(mode, R, G, B);
    
    unsigned char *Row = ((unsigned char *)buffer->memory) 
        + MinX*buffer->BytesPerPixel 
        + MinY*buffer->pitch;
    
    for (int Y = MinY; Y < MaxY;++Y)
    {	
        unsigned int *Pixel = (unsigned int *)Row;
        for (int X = MinX; X < MaxX; ++X)
        {
            
            float DestR = (float)( (*Pixel >> 16) & 0xFF );
            float DestG = (float)( (*Pixel >> 8) & 0xFF );
            float DestB = (float)( (*Pixel >> 0) & 0xFF );
            
            float AlphaValue = Alpha / 255.0f;
            float SourceR = R * 255.0f;
            float SourceG = G * 255.0f;
            float SourceB = B * 255.0f;
            
            float BlendedR = (1.0f - AlphaValue) * DestR + SourceR * AlphaValue;
            float BlendedG = (1.0f - AlphaValue) * DestG + SourceG * AlphaValue;
            float BlendedB = (1.0f - AlphaValue) * DestB + SourceB * AlphaValue;
            
            *Pixel++ =(255 << 24) | (((unsigned int)(BlendedR + 0.5f)) << 16) |
                (((unsigned int)(BlendedG + 0.5f)) << 8) |
                (((unsigned int)(BlendedB + 0.5f)) << 0);
        }
        Row += buffer->pitch;
    }
}

internal void DrawBitmap(game_offscreen_buffer *buffer, loaded_bitmap Bitmap, float RealMinX, float RealMaxX,
                         float RealMinY, float RealMaxY, signed int Flip)
{
	int MinX = FloatToInt(RealMinX);
	int MaxX = FloatToInt(RealMaxX);
	int MinY = FloatToInt(RealMinY);
	int MaxY = FloatToInt(RealMaxY);
    
	if(MinX < 0)
	{
		MinX = 0;
	}
    
	if(MinY < 0)
	{
		MinY = 0;
	}
    
	if(MaxX > buffer->width)
	{
		MaxX = buffer->width;
	}
    
	if(MaxY > buffer->height)
	{
		MaxY = buffer->height;
	}
    
	if(MinX + (int)Bitmap.Width < MaxX)
	{
		MaxX = MinX + Bitmap.Width;
	}
    
	if(MinY + (int)Bitmap.Height < MaxY)
	{
		MaxY = MinY + Bitmap.Height;
	}
    
	unsigned char *Row = ((unsigned char *)buffer->memory) 
        + MinX*buffer->BytesPerPixel 
        + MinY*buffer->pitch;
    
	unsigned int *SourceRow = Bitmap.PixelPointer + Bitmap.Width * (Bitmap.Height - 1);
	
	if (Flip == FLIPTRUE)
	{
		SourceRow += Bitmap.Width - 1;
	}
    
	for (int Y = MinY; Y < MaxY;++Y)
	{	
		unsigned int *Pixel = (unsigned int *)Row;
		unsigned int *SourcePixel = SourceRow;
		for (int X = MinX; X < MaxX; ++X)
		{
#if 0
			float DestR = (float)( (*Pixel >> 16) & 0xFF );
			float DestG = (float)( (*Pixel >> 8) & 0xFF );
			float DestB = (float)( (*Pixel >> 0) & 0xFF );
            
			float AlphaValue = (float)( (*SourcePixel >> 24) & 0xFF ) / 255.0f;
			float SourceR = (float)( (*SourcePixel >> 16) & 0xFF );
			float SourceG = (float)( (*SourcePixel >> 8) & 0xFF );
			float SourceB = (float)( (*SourcePixel >> 0) & 0xFF );
            
			float BlendedR = (1.0f - AlphaValue) * DestR + SourceR * AlphaValue;
			float BlendedG = (1.0f - AlphaValue) * DestG + SourceG * AlphaValue;
			float BlendedB = (1.0f - AlphaValue) * DestB + SourceB * AlphaValue;
            
			*Pixel = (((unsigned int)(BlendedR + 0.5f)) << 16) |
				(((unsigned int)(BlendedG + 0.5f)) << 8) |
				(((unsigned int)(BlendedB + 0.5f)) << 0);
#else
			if ( (*SourcePixel >> 24) & 1)
			{
				*Pixel = *SourcePixel;
			}
#endif
			Pixel++;
			SourcePixel += Flip;	
		}
		SourceRow -= Bitmap.Width;
		Row += buffer->pitch;
	}	
}

internal void DrawBitmapWithOpacity(game_offscreen_buffer *buffer, loaded_bitmap Bitmap, float RealMinX, float RealMaxX,
                                    float RealMinY, float RealMaxY, SpriteRenderDirection Flip, float Alpha)
{
	int MinX = FloatToInt(RealMinX);
	int MaxX = FloatToInt(RealMaxX);
	int MinY = FloatToInt(RealMinY);
	int MaxY = FloatToInt(RealMaxY);
    
	if(MinX < 0)
	{
		MinX = 0;
	}
    
	if(MinY < 0)
	{
		MinY = 0;
	}
    
	if(MaxX > buffer->width)
	{
		MaxX = buffer->width;
	}
    
	if(MaxY > buffer->height)
	{
		MaxY = buffer->height;
	}
    
	if(MinX + (int)Bitmap.Width < MaxX)
	{
		MaxX = MinX + Bitmap.Width;
	}
    
	if(MinY + (int)Bitmap.Height < MaxY)
	{
		MaxY = MinY + Bitmap.Height;
	}
    
	unsigned char *Row = ((unsigned char *)buffer->memory) 
        + MinX*buffer->BytesPerPixel 
        + MinY*buffer->pitch;
    
	unsigned int *SourceRow = Bitmap.PixelPointer + Bitmap.Width * (Bitmap.Height - 1);
	
	if (Flip == FLIPTRUE)
	{
		SourceRow += Bitmap.Width - 1;
	}
    
	for (int Y = MinY; Y < MaxY;++Y)
	{	
		unsigned int *Pixel = (unsigned int *)Row;
		unsigned int *SourcePixel = SourceRow;
		for (int X = MinX; X < MaxX; ++X)
		{
#if 1
			float DestR = (float)( (*Pixel >> 16) & 0xFF );
			float DestG = (float)( (*Pixel >> 8) & 0xFF );
			float DestB = (float)( (*Pixel >> 0) & 0xFF );
            
			float AlphaValue = Alpha / 255.0f;
			float SourceR = (float)( (*SourcePixel >> 16) & 0xFF );
			float SourceG = (float)( (*SourcePixel >> 8) & 0xFF );
			float SourceB = (float)( (*SourcePixel >> 0) & 0xFF );
            
			float BlendedR = (1.0f - AlphaValue) * DestR + SourceR * AlphaValue;
			float BlendedG = (1.0f - AlphaValue) * DestG + SourceG * AlphaValue;
			float BlendedB = (1.0f - AlphaValue) * DestB + SourceB * AlphaValue;
            
			*Pixel = (255 << 24) |(((unsigned int)(BlendedR + 0.5f)) << 16) |
				(((unsigned int)(BlendedG + 0.5f)) << 8) |
				(((unsigned int)(BlendedB + 0.5f)) << 0);
#else
			if ( (*SourcePixel >> 24) & 1 )
			{
				*Pixel = *SourcePixel;
			}
#endif
			Pixel++;
			SourcePixel += (int)Flip;	
		}
		SourceRow -= Bitmap.Width;
		Row += buffer->pitch;
	}	
}

internal void DrawBitmapScaled(game_offscreen_buffer *buffer, loaded_bitmap Bitmap, float RealMinX, float RealMaxX,
                               float RealMinY, float RealMaxY, unsigned int Flip, unsigned int Scalar)
{
    float DrawHeight = (RealMaxY - RealMinY) / (float)Scalar;
    RealMaxY = RealMinY + DrawHeight;
    
    int MinX = FloatToInt(RealMinX);
    int MaxX = FloatToInt(RealMaxX);
    int MinY = FloatToInt(RealMinY);
    int MaxY = FloatToInt(RealMaxY);
    
    unsigned int PitchShift = 0;
    unsigned int PixelShift = 0;
    
    if(MaxX > buffer->width)
    {
        MaxX = buffer->width;
    }
    
    if( (MinY + (int)Bitmap.Height) < MaxY)
    {
        MaxY = MinY + Bitmap.Height;
    }
    
    if( (MinX + (int)Bitmap.Width * (int)Scalar) < MaxX)
    {
        MaxX = MinX + Bitmap.Width * Scalar;
    }
    
    int MaxDrawY = (MaxY - MinY) * (int)Scalar + MinY;
    if( MaxDrawY > buffer->height)
    {
        MaxY = (buffer->height - MinY) / (int)Scalar + MinY;
    }
    
    if(MinX < 0)
    {
        PixelShift = (unsigned int)Abs((float)MinX / (float)Scalar);
        MinX = 0;
    }
    
    if(MinY < 0)
    {
        PitchShift = (unsigned int)Abs((float)MinY / (float)Scalar);
        MaxY += FloatToInt(Abs((float)MinY / (float)Scalar));
        MinY = 0;
    }
    
    unsigned char *Row = ((unsigned char *)buffer->memory) 
        + MinX*buffer->BytesPerPixel 
        + MinY*buffer->pitch;
    
    unsigned int *SourceRow = Bitmap.PixelPointer + Bitmap.Width * (Bitmap.Height - 1 - PitchShift) + PixelShift;
    
    if (Flip == FLIPTRUE)
    {
        SourceRow += Bitmap.Width - 1;
    }
    
    int Counter = 1;
    
    //int ClampedMaxY = Bitmap.Height + MinY - MaxY;
    //MaxY = (ClampedMaxY < 0)? MaxY + ClampedMaxY: MaxY;
    
    for (int Y = MinY; Y < MaxY;++Y)
    {	
        unsigned int *Pixel = (unsigned int *)Row;
        unsigned int *SourcePixel = SourceRow;
        for (int X = MinX; X < MaxX; ++X)
        {
            if ( (*SourcePixel >> 24) & 1 )
            {
                *Pixel = *SourcePixel;
                for (unsigned int j = 1; j < Scalar; j++)
                {
                    unsigned int *PixelBelow = Pixel + buffer->width * j;
                    *PixelBelow = *SourcePixel;
                }
            }
            
            Pixel++;
            
            if (Counter % Scalar == 0)
            {
                SourcePixel += (int)Flip;
            }	
            Counter++;
        }
        
        SourceRow -= Bitmap.Width;
        Row =  Row + buffer->pitch * Scalar;
        Counter = 1;
    }	
}


internal void DrawBitmapScaledInvert(game_offscreen_buffer *buffer, loaded_bitmap Bitmap, float RealMinX, float RealMaxX,
                                     float RealMinY, float RealMaxY, unsigned int Flip, int Scalar, int Invert)
{
    if (Scalar < 0)
    {
        Scalar = 0;
    }
    
    RealMaxY = RealMaxY - (RealMaxY - RealMinY) / (float)Scalar;
    
    int MinX = FloatToInt(RealMinX);
    int MaxX = FloatToInt(RealMaxX);
    int MinY = FloatToInt(RealMinY);
    int MaxY = FloatToInt(RealMaxY);
    
    if(MinX < 0)
    {
        MinX = 0;
    }
    
    if(MinY < 0)
    {
        MinY = 0;
    }
    
    if(MaxX > buffer->width)
    {
        MaxX = buffer->width;
    }
    
    if(MaxY > buffer->height)
    {
        MaxY = buffer->height;
    }
    
    if( (MinX + (int)Bitmap.Width * Scalar) < MaxX)
    {
        MaxX = MinX + Bitmap.Width * Scalar;
    }
    
    if( (MinY + (int)Bitmap.Height) < MaxY)
    {
        MaxY = MinY + Bitmap.Height;
    }
    
    unsigned char *Row = ((unsigned char *)buffer->memory) 
        + MinX*buffer->BytesPerPixel 
        + MinY*buffer->pitch;
    
    unsigned int *SourceRow = Bitmap.PixelPointer + Bitmap.Width * (Bitmap.Height - 1);
    
    if (Flip == FLIPTRUE)
    {
        SourceRow += Bitmap.Width - 1;
    }
    
    int Counter = 1;
    
    //int ClampedMaxY = Bitmap.Height + MinY - MaxY;
    //MaxY = (ClampedMaxY < 0)? MaxY + ClampedMaxY: MaxY;
    
    for (int Y = MinY; Y < MaxY;++Y)
    {	
        unsigned int *Pixel = (unsigned int *)Row;
        unsigned int *SourcePixel = SourceRow;
        for (int X = MinX; X < MaxX; ++X)
        {
            unsigned int SourcePixelMod = *SourcePixel;
            if(Invert)
            {
                unsigned int SourceR = 0x000000FF - ((*SourcePixel >> 16) & 0x000000FF);
                unsigned int SourceG = 0x000000FF - ((*SourcePixel >> 8) & 0x000000FF);
                unsigned int SourceB = 0x000000FF - ((*SourcePixel >> 0) & 0x000000FF);
                SourcePixelMod = (*SourcePixel & 0xFF000000) |  (SourceR << 16) | (SourceG << 8) | SourceB;
            }
            if ( (SourcePixelMod >> 24) & 1 )
            {
                *Pixel = SourcePixelMod;
                for (int j = 1; j < Scalar; j++)
                {
                    unsigned int *PixelBelow = Pixel + buffer->width * j;
                    *PixelBelow = SourcePixelMod;
                }
            }
            
            Pixel++;
            
            if (Counter % Scalar == 0)
            {
                SourcePixel += (int)Flip;
            }	
            Counter++;
        }
        
        SourceRow -= Bitmap.Width;
        Row =  Row + buffer->pitch * Scalar;
        Counter = 1;
    }	
}

internal loaded_bitmap BlitCharacter(game_offscreen_buffer *buffer, loaded_bitmap *PikaBlueFont,
                                     float PosX, float PosY, char Character, int Invert)
{
    loaded_bitmap Bitmap = BitmapFromASCII(PikaBlueFont, Character);
    
    float OffsetY = 0.0f;
    if ( (Character == 'p') || (Character == 'g') || (Character == 'j') || (Character == 'y') || (Character == 'q') || (Character == ',') )
    {
        OffsetY = 2.0f * Bitmap.Scale;
    }
    else if (Character == 39)
    {
        OffsetY = -7.0f * Bitmap.Scale;
        
    }
    
    DrawBitmapScaledInvert(buffer, Bitmap, PosX, PosX + Bitmap.Width * Bitmap.Scale, PosY - Bitmap.Height * Bitmap.Scale + OffsetY,
                           PosY + OffsetY, FLIPFALSE, Bitmap.Scale, Invert);
    
    return Bitmap;
}

internal void BlitStringBoundless(game_offscreen_buffer *buffer, loaded_bitmap *PikaBlueFont,
                                  float PosX, float PosY, char *String, int Invert)
{
    float SpaceChar = 10.0f;
    int AmountOfCharacters = GetStringLength(String);
    for (int x = 0; x < AmountOfCharacters; x++)
    {
        char Character = *String;
        if (Character)
        {
            if (Character == 32) //the character is a space character
            {
                PosX += SpaceChar;
            }
            else
            {
                loaded_bitmap Bitmap = BlitCharacter(buffer, PikaBlueFont, PosX, PosY, Character, Invert);
                PosX += Bitmap.Width * Bitmap.Scale;
            }
            String++;
            
        }
        else
        {
            break; //NOTE: We break here because once we reach a null character 
            //we do not want to process anything more.
        }
    }
}

internal void BlitStringBounds(game_offscreen_buffer *buffer, loaded_bitmap *PikaBlueFont,
                               float MinX, float MaxX, float MinY, float MaxY, char *String, int Length, int Invert)
{
    float X = MinX;
    float Y = MinY;
    
    for (int x = 0; x < Length; x++)
    {
        char Character = *String;
        //NOTE: Below we do bad things because we essentially get the character bitmap twice.
        loaded_bitmap CharacterBitmap = BitmapFromASCII(PikaBlueFont, Character); 
        if ( (CharacterBitmap.Width * CharacterBitmap.Scale + X) > MaxX)
        {
            Y += 20.0f; X = MinX; //wrap the text back down to next line.
        }
        
        if ( Character && (Y <= MaxY) )
        {
            if (Character == 32)
            {
                X += 10.0f;
            }
            else
            {
                loaded_bitmap Bitmap = BlitCharacter(buffer, PikaBlueFont, X, Y, Character, Invert);
                X += Bitmap.Width * Bitmap.Scale;
            }
            String++;
        }
        else
        {
            break; //Do not blit anything more since we reached a null character.
        }
    }
}

internal void BlitStringReverseBoundless(game_offscreen_buffer *buffer, loaded_bitmap *PikaBlueFont,
                                         float PosX, float PosY, char *String, int Invert)
{
    float SpaceChar = 10.0f;
    int AmountOfCharacters = GetStringLength(String);
    String += AmountOfCharacters - 1;
    for (int x = 0; x < AmountOfCharacters; x++)
    {
        char Character = *String;
        if ( Character )
        {
            if (Character == 32)
            {
                PosX -= SpaceChar;
            }
            else
            {
                loaded_bitmap Bitmap = BitmapFromASCII(PikaBlueFont, Character);
                PosX -= Bitmap.Width * Bitmap.Scale;
                float OffsetY = 0.0f;
                BlitCharacter(buffer, PikaBlueFont, PosX, PosY, Character, Invert);
            }
            String--;
        }
    }
}

internal void DrawUIElement(game_offscreen_buffer *buffer, loaded_bitmap *Bitmaps, loaded_bitmap *Font, game_ui_element UIElement, game_screen_position ScreenPos)
{
    if (UIElement.IsString)
    {
        //blit the string
        if (UIElement.Flip)
        {
            //draw the string reversed
            BlitStringReverseBoundless(buffer, Font,
                                       ScreenPos.X + UIElement.RelativePosition.X, 
                                       ScreenPos.Y + UIElement.RelativePosition.Y,  
                                       UIElement.String, UIElement.Invert);
        }
        else
        {
            //draw the string regurally
            BlitStringBoundless(buffer, Font,
                                ScreenPos.X + UIElement.RelativePosition.X, 
                                ScreenPos.Y + UIElement.RelativePosition.Y,
                                UIElement.String, UIElement.Invert);
        }
    }
    else 
    {
        //blit the bitmap
        loaded_bitmap Bitmap = Bitmaps[UIElement.BitmapIndex]; 
        DrawBitmapScaled(buffer, Bitmap, ScreenPos.X + UIElement.RelativePosition.X, 
                         ScreenPos.X + UIElement.RelativePosition.X + Bitmap.Width * Bitmap.Scale,
                         ScreenPos.Y + UIElement.RelativePosition.Y, 
                         ScreenPos.Y + UIElement.RelativePosition.Y + Bitmap.Height * Bitmap.Scale,
                         UIElement.Flip, Bitmap.Scale);
    }
}

internal void DrawUIScene(game_offscreen_buffer *buffer, loaded_bitmap *Bitmaps, loaded_bitmap *Font, game_ui_scene UIScene, game_screen_position ScreenPos)
{
    //Go throught all the bitmap elements in the list
    //we can do a for loop like this and still acount for elements on top of each other because
    //we will ensure that when we make the UIScene that top most items are added to the array last, simple.
    for (unsigned int x = 0; x < UIScene.Count; x++)
    {
        game_ui_element CurrentElement = UIScene.Elements[x];
        DrawUIElement(buffer, Bitmaps, Font, CurrentElement, ScreenPos);
    }	
}

internal void DrawNPC(game_offscreen_buffer *buffer, entity_npc *Entity, vector2f ScreenPos)
{
    loaded_bitmap PlayerBitmap = {};
    SpriteRenderDirection Flip = FLIPFALSE;
    int MoveDirection = (int)Entity->MoveDirection;
    animation_player AnimationPlayer = Entity->Entity->AnimationPlayer;
    
    if (Entity->MoveDirection == RIGHT)
    {
        MoveDirection = LEFT;
        Flip = FLIPTRUE; 
    }
    if (Entity->MoveDirection == LEFT)
    {
        Flip = FLIPFALSE;
    }
    
    if (Entity->Walking)
    {
        PlayerBitmap = AnimationPlayer.Animations[0].Frames[MoveDirection * 4 + (AnimationPlayer.Animations[0].Counter % 4)  ];
    }
    else
    {
        // grab the first frame
        PlayerBitmap = AnimationPlayer.Animations[0].Frames[MoveDirection * 4];
    }
    
    int PlayerScale = 2;
    float PlayerWidthScaled = (float)PlayerBitmap.Width * (float)PlayerScale;
    float PlayerHeightScaled = (float)PlayerBitmap.Height * (float)PlayerScale;
    
    DrawBitmapScaled(buffer, PlayerBitmap, ScreenPos.X - 0.5f * PlayerWidthScaled,
                     ScreenPos.X + 0.5f * PlayerWidthScaled, ScreenPos.Y - PlayerHeightScaled,
                     ScreenPos.Y, Flip, PlayerScale);
}

internal void ExecuteShader(game_offscreen_buffer *dest, shader *Shader, float DeltaTime)
{
    Shader->ActiveTime += DeltaTime;
    if (Shader->ActiveTime > Shader->Length)
    {
        Shader->Active = false;
    }
    else
    {
        // do the shader code here
        float Cutoff = Shader->ActiveTime / Shader->Length;
        // screenshot of game tho
        unsigned int *Source = (unsigned int *)Shader->source->memory; 
        unsigned int *Dest = (unsigned int *)dest->memory;
        unsigned int *LUT = Shader->LUT.PixelPointer + Shader->LUT.Width * (Shader->LUT.Height - 1);
        
        for (int Y = 0; Y < dest->height;++Y)
        {	
            unsigned int *Pixel = Source;
            unsigned int *DestPixel = Dest;
            unsigned int *LUTPixel = LUT;
            for (int X = 0; X < dest->width; ++X)
            {
                //for each pixel what do I do?
                
                *DestPixel = ( (*LUTPixel & 0x000000FF) / 255.0f > Cutoff)? *Pixel : (255 << 24) | 0;
                
                LUTPixel++;
                Pixel++;
                DestPixel++;
            }
            LUT -= Shader->LUT.Width;
            Dest += dest->width;
            Source += Shader->source->width;
        }	
    }
}

internal void ExecuteShader2(game_offscreen_buffer *dest, shader *Shader, float DeltaTime)
{
    Shader->ActiveTime += DeltaTime;
    if (Shader->ActiveTime > Shader->Length)
    {
        Shader->Active = false;
    }
    else
    {
        //do the shader code here
        float Cutoff = Shader->ActiveTime / Shader->Length;
        unsigned int *Source = (unsigned int *)Shader->source->memory;
        unsigned int *Source2 = (unsigned int *)Shader->source2->memory; 
        unsigned int *Dest = (unsigned int *)dest->memory;
        unsigned int *LUT = Shader->LUT.PixelPointer + Shader->LUT.Width * (Shader->LUT.Height - 1);
        
        for (int Y = 0; Y < dest->height;++Y)
        {	
            unsigned int *Pixel = Source;
            unsigned int *DestPixel = Dest;
            unsigned int *LUTPixel = LUT;
            unsigned int *Pixel2 = Source2;
            for (int X = 0; X < dest->width; ++X)
            {
                //for each pixel what do I do?
                
                *DestPixel = ( (*LUTPixel & 0x000000FF) / 255.0f > Cutoff)? *Pixel: *Pixel2;
                
                Pixel2++; LUTPixel++;
                Pixel++; DestPixel++;
            }
            Source2 += Shader->source2->width;
            LUT -= Shader->LUT.Width;
            Dest += dest->width;
            Source += Shader->source->width;
        }	
    }
}