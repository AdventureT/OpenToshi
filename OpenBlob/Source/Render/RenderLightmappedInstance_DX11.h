#pragma once

namespace TerrainTypes
{
struct Cell;
struct CellInstance;
} // namespace TerrainTypes

namespace Toshi
{
class TModelInstance;
} // namespace Toshi

// Draws the LOD 0 meshes of a low LOD model instance and pops its transform (the low LOD cells of ATerrain2Interface
// use it for their TModelInstance); NOTE: a global function so ATerrain2Interface can reach it
void RenderLowLODModel( Toshi::TModelInstance* a_pModelInstance );

//-----------------------------------------------------------------------------
// Purpose: draws the lightmapped instances placed in the terrain cells
//-----------------------------------------------------------------------------
namespace RenderLightmappedInstance
{

// Draws the streamed model (AModelInstance2) of a placed instance
void __stdcall RenderInstance( TerrainTypes::Cell* a_pCell, TerrainTypes::CellInstance* a_pInstance, TBOOL a_bCull, const TCHAR* a_szMeshFilter, TFLOAT a_fDistanceSq, void* a_pUserData );

// Draws the low LOD model of a placed instance
void __stdcall RenderLowLODInstance( TerrainTypes::Cell* a_pCell, TerrainTypes::CellInstance* a_pInstance, TBOOL a_bCull, TFLOAT a_fDistanceSq );

} // namespace RenderLightmappedInstance
