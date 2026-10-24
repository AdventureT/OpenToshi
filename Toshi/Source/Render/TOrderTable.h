#pragma once
#include "TRenderPacket.h"
#include "Toshi/TNodeList.h"
#include "Toshi/TDList.h"

TOSHI_NAMESPACE_START

class TOrderTable;
class TShader;

//-----------------------------------------------------------------------------
// Purpose: tracks all of the buffered render commands of specified TMaterial
//-----------------------------------------------------------------------------
class TRegMaterial : public TNodeList<TRegMaterial>::TNode
{
public:
	typedef TUINT32 State;
	enum State_ : State
	{
		State_Registered = BITFLAG( 0 ),
		State_Used       = BITFLAG( 1 )
	};

public:
	// $deBlob: FUNCTION 006ae6c0
	TRegMaterial()
	{
		m_State             = 0;
		m_pOrderTable       = TNULL;
		m_pMaterial         = TNULL;
		m_pLastRenderPacket = TNULL;
		m_pNextRegMat       = TNULL;
	}

	// NOTE: TNode's destructor asserts the node isn't linked
	// $deBlob: FUNCTION 006ae9d0
	~TRegMaterial() = default;

	// Renders all of the render packets attached to this registered material
	void Render();

	// Allocates new render packet (command) for the specified mesh
	TRenderPacket* AddRenderPacket( TMesh* a_pMesh );

	State GetFlags() const { return m_State; }
	// $deBlob: FUNCTION 006ae740
	void SetFlags( State a_eFlags ) { m_State = a_eFlags; }

	TOrderTable* GetOrderTable() const { return m_pOrderTable; }
	// $deBlob: FUNCTION 006ae750
	void SetOrderTable( TOrderTable* a_pOrderTable ) { m_pOrderTable = a_pOrderTable; }

	TMaterial* GetMaterial() const { return m_pMaterial; }
	// $deBlob: FUNCTION 006ae760
	void SetMaterial( TMaterial* a_pMaterial ) { m_pMaterial = a_pMaterial; }

	TRegMaterial* GetNextRegMat() const { return m_pNextRegMat; }
	// $deBlob: FUNCTION 006ae7e0
	void SetNextUsedMaterial( TRegMaterial* a_pRegMat ) { m_pNextRegMat = a_pRegMat; }

#ifndef TOSHI_SDK
private:
#endif
	State          m_State;             // 0x0C
	TOrderTable*   m_pOrderTable;       // 0x10
	TMaterial*     m_pMaterial;         // 0x14
	TRenderPacket* m_pLastRenderPacket; // 0x18
	TRegMaterial*  m_pNextRegMat;       // 0x1C
}; // 0x20

//-----------------------------------------------------------------------------
// Purpose: helps to order rendering models of specified shader using priority. Registers TMaterial instances to render
// all render packets attached to them.
//-----------------------------------------------------------------------------
class TOrderTable : public TPriList<TOrderTable>::TNode
{
public:
	using t_PreFlushCallback  = void( __stdcall* )( TOrderTable* a_pOrderTable, void* a_pCustomData );
	using t_PostFlushCallback = void( __stdcall* )( TOrderTable* a_pOrderTable, void* a_pCustomData );

public:
	TOrderTable();

	// NOTE: destroys the static data (registered materials and render packets), as in retail
	~TOrderTable();

	// Registers this order table in the renderer and assigns priority to it
	TBOOL Create( TShader* a_pShader, TINT a_iPriority, TUINT8 a_uiIndex );

	// Registers material and returns registered material which can be used to alloc render commands
	TRegMaterial* RegisterMaterial( TMaterial* a_pMat );

	static void DeregisterMaterial( TRegMaterial* a_pRegMat );
	static void DeregisterAllMaterials();

	// Allocates render packet from the memory allocated by CreateStaticData
	static TRenderPacket* AllocRenderPacket();

	// Marks the registered material as used and links it to this order table.
	// Note: before registered material is used, it won't render
	void UseMaterial( TRegMaterial* a_pRegMat );

	// Renders all render packets of registered materials
	void Render();

	// Flushes the shader before rendering
	void Flush();

	// Allocate memory for storing render packets and registered materials
	static TBOOL CreateStaticData( TUINT a_uiMaxMaterials, TUINT a_uiMaxRenderPackets );

	// Takes the end of the render packet memory (a_uiSize bytes) for other use
	static void* GetRenderPacketMem( TUINT a_uiSize );
	static void  RestoreRenderPacketMem();

	void SetPreFlushCallback( t_PreFlushCallback a_fnCallback, void* a_pCustomData );
	void SetPostFlushCallback( t_PostFlushCallback a_fnCallback, void* a_pCustomData );

	// Returns bound shader
	TShader* GetShader() const { return m_pShader; }
	TUINT8   GetIndex() const { return m_uiIndex; }

public:
	static TUINT          s_uiNumRegisteredMaterials;
	static TUINT          s_uiMaxMaterials;
	static TRegMaterial*  s_pRegMaterials;
	static TUINT          s_uiMaxRenderPackets;
	static TUINT          s_uiNumRenderPackets;
	static TRenderPacket* s_pRenderPackets;
	static TUINT          s_uiOrigMaxRenderPackets;

	static TNodeList<TRegMaterial> s_llRegMatRegisteredList;
	static TNodeList<TRegMaterial> s_llRegMatFreeList;

#ifndef TOSHI_SDK
private:
#endif
	TShader*            m_pShader;                // 0x0C
	t_PreFlushCallback  m_fnPreFlushCallback;     // 0x10
	void*               m_pPreFlushCallbackData;  // 0x14
	t_PostFlushCallback m_fnPostFlushCallback;    // 0x18
	void*               m_pPostFlushCallbackData; // 0x1C
	TRegMaterial*       m_pLastRegMat;            // 0x20
	TUINT8              m_uiIndex;                // 0x24
}; // 0x28

TOSHI_NAMESPACE_END
