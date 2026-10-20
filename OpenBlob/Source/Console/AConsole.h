#pragma once

//-----------------------------------------------------------------------------
// Purpose: registers the game's console commands with AConsoleState (mem_*, pN_colour, pN_mp_colour, pN_points,
// pN_energy, lvl_timeleft, chal_timeleft, phys_show_collision). A static class.
// NOTE: the console itself can't be opened in retail (see AConsoleState)
//-----------------------------------------------------------------------------
class AConsole
{
public:
	static void RegisterCommands();
	static void UnregisterCommands();
};
