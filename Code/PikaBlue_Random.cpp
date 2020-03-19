/*
Hi thank you for using the PikaBlue engine! 
Below is our in-house pseduo random number generator.
It's kinda terrible and I strongly advise not to use it.
However, I pledge to not use any libraries and therefore I 
shall not. 
*/

//this function will seed our random number generator
internal void SeedRandom(game_state *GameState)
{
	unsigned int Seed = FloatToInt( GameState->GameTimer ) + 1;
	GameState->RandomState = (Seed * 14) % 521;
	GameState->LastRandom = FloorFloatToInt( (GameState->RandomState / 520.0f) * 256.0f );
}

//The range of outputs will be 0 to 255
internal int GetRandom(game_state *GameState)
{
	GameState->RandomState = (GameState->LastRandom * 14) % 521;
	GameState->LastRandom = FloorFloatToInt( (GameState->RandomState / 520.0f) * 256.0f );
	return GameState->LastRandom - 1;
} 