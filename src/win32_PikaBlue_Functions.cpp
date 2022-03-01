inline int EqualsPokemonDemo(char *String)
{
	char *PokemonDemo = "Pokemon-Demo";
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
	for (char *Scan = State->EXEFileName; *Scan;++Scan)
	{
		if (*Scan == 'P' && EqualsPokemonDemo(Scan))
		{
			CatStrings((int)(Scan - State->EXEFileName), State->EXEFileName, 
				17,"Pokemon-Demo\\", WIN32_FILE_NAME_COUNT, State->BaseFilePath);
			break;
		}
	}
	
}