#pragma once

//-----------------------------------------------------------------------------
// Purpose: references the TClasses of the classes the game creates by name (entities from the level data, XUI scenes),
// so the linker keeps them. NOTE: retail has the three functions but nothing calls them (no reference to them in the
// executable), so their classes are kept anyway
//-----------------------------------------------------------------------------
namespace ClassLink_Win
{

// The entity classes
void ReferenceGenericClasses();

// The XUI scene classes
void ReferenceXUIClasses();

// The PC classes
void ReferenceSKUClasses();

} // namespace ClassLink_Win
