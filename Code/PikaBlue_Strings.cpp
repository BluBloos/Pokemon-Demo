internal void CatStrings(int SourceACount, char *SourceA, int SourceBCount,
                         char *SourceB, int DestCount, char *Dest)
{
	for (int index = 0; index < SourceACount; index++)
	{
		*Dest++ = *SourceA++;
	}
	for (int index = 0; index < SourceBCount; index++)
	{
		*Dest++ = *SourceB++;
	}
	*Dest = 0;
}

INLINE unsigned int GetStringLength(char *String)
{
	unsigned int Count = 0;
	while (*String != 0)
	{
		String++; Count++;
	}
	return Count;
}

//Here I have a destination length so I can manage overflows for small buffers.
INLINE void CloneString(char *String, char *Dest, int DestLength)
{
	int Count = 0;
	while ( (*String != 0) && (Count < DestLength) )
	{
		Count++;
		*Dest++ = *String++;
	}
	*Dest = 0;
}

internal char *GameCatStrings(char *StringA, char *StringB, char *Dest)
{
	CatStrings(GetStringLength(StringA), StringA, GetStringLength(StringB), StringB, 0, Dest);
	return Dest;
}

//currently this functions does not support negative numbers, 
//it only supports whole integers
INLINE char *NumberToASCII(unsigned int Number, char *Dest)
{
	char Buffer[256];
	char *Temp = Buffer;
	char *DestRoot = Dest; 
	if (Number)
	{  
		while (Number)
		{
			*Temp++ = Number % 10 + '0';
			Number = Number / 10;
		}		
		*Temp-- = 0;
		int Length = GetStringLength(Buffer);
		for (int x = 0; x < Length; x++)
		{
			*Dest++ = *Temp--;
		}
	}
	else
	{
		*Dest++ = '0';
	}
	*Dest = 0;
	return DestRoot;
}

//currently this functions does not support negative numbers, 
//it only supports whole integers
unsigned int ASCIIToNumber(char *String)
{
	unsigned int Result = 0;
	unsigned int StringLength = GetStringLength(String);
	unsigned int PlaceValue = (int)Pow(10.0f, (StringLength - 1.0f) );
	for (unsigned int x = 0; x < StringLength; x++)
	{
		Result += (SafeSubtract(*String,'0')) * PlaceValue;
		String++;
		PlaceValue = PlaceValue / 10;
	}
	return Result;
}

float ASCIIToFloat(char *String)
{
	//okay here are the steps
	//conver the string to the same string but without decimal
	//also store length of string after deciaml
	//convert new string to int and multiply by 10 to the power of length of characters after decimal
	//DOnE!
	char *Scan = String;
	char Temp[256];
	char *CleanNumber = Temp;
	unsigned int LengthAfterDecimal = 0;
	unsigned int Counting = 0;
	while(*Scan)
	{
		if (*Scan != '.')
		{
			if (Counting)
			{
				LengthAfterDecimal++;
			}		
			*CleanNumber++ = *Scan++;
		}
		else 
		{
			Counting= true;
			Scan++;
		}
	}
	*CleanNumber = 0;
	float Result = ASCIIToNumber(Temp) * Pow(10.0f, LengthAfterDecimal * -1.0f); 
	return Result;
}

internal unsigned int StringEquals(char *StringA, char *StringB)
{
	unsigned int StringALength = GetStringLength(StringA);
	unsigned int StringBLength = GetStringLength(StringB);
	if (StringALength == StringBLength)
	{
		for (unsigned int x = 0; x < StringALength; x++)
		{
			if (*StringA++ == *StringB++)
			{
				continue;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}