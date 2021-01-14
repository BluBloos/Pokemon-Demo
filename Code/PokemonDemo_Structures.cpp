// NOTE: This file is for data structue manipulations
// TODO: Find a use for these routines in the game code

// Delete an element from an array
template<typename T> int DelItemFromArray(T *array, unsigned int *arrayLength, unsigned int indexToDelete)
{
	unsigned int LastIndex = *arrayLength - 1;
	
    //Assert(GameState->SoundQueue.Count > 0);
	if ( !(*arrayLength > 0) ) {
        return -1; // error
    }
    
    for (unsigned int x = 0; x < LastIndex - indexToDelete; x++)
	{
		array[indexToDelete + x] = array[indexToDelete + x + 1];
	}
    
	*arrayLength = *arrayLength - 1;
    
    return 0; // success
}