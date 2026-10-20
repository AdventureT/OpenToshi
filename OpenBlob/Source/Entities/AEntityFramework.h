#pragma once

//-----------------------------------------------------------------------------
// Purpose: creates and destroys the managers of the entity framework (AEntityFramework.cpp) when a level loads.
//-----------------------------------------------------------------------------
class AEntityFramework
{
public:
	static void Startup();

	// Destroys the managers Startup created (on the level unload)
	static void Shutdown();

	// The per frame update of the managers before the entities tick (AEntityFactory::Tick): the platforms.
	static void PreTick( TFLOAT a_fDeltaTime );

	// Empty in retail (AInGameState::UpdateReady calls it on the input button 10)
	// $deBlob: FUNCTION 0078d970
	static void ReloadResources() {}

	// Empty in retail (AInGameState::UpdateReady calls it with the delta time while the level isn't ready)
	// $deBlob: FUNCTION 0078de80
	static void ReadyTick( TFLOAT a_fDeltaTime ) {}

	// Creates AEntityHash over the first terrain region (with a margin of two cells)
	static void InitialiseEntityHash();
};
