inline int EqualsPokemonDemo(char *String)
{
	char *PokemonDemo = "pokemondemo";
	for (char *Scan = String; *Scan && *PokemonDemo;Scan++)
	{
		if (*Scan == *PokemonDemo++)
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
}

internal void Win32GetRelativePaths(win32_state *State)
{
	DWORD SizeofFileName = GetModuleFileNameA(0, State->EXEFileName,
											  sizeof(State->EXEFileName));
	State->OnePastLastSlash = State->EXEFileName;
	for (char *Scan = State->EXEFileName;*Scan;++Scan)
	{
		if (*Scan == '\\')
		{
			State->OnePastLastSlash = Scan + 1;
		}
	}
	for (char *Scan = State->EXEFileName;*Scan;++Scan)
	{
		if (*Scan == 'p')
		{
			if ( EqualsPokemonDemo(Scan) )
			{
				//NOTE: Here I put the dest count to 0 because it's not actually
				//being used
				CatStrings((int)(Scan - State->EXEFileName), State->EXEFileName, 
						   17,"PokemonDemo\\Data\\",0, State->BaseFilePath);
				break;
			}
		}
	}
	
}