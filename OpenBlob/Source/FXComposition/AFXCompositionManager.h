#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Toshi/TString8.h>
#include <File/TTRB.h>

class ATRBLoaderJob;
class AFXCompositionInstance;

//-----------------------------------------------------------------------------
// Purpose: owns the level's FX composition TRB (level data block 0x16, symbol "FXData" plus one symbol per
// composition) and the list of AFXCompositionInstances.
// The console command fx_reload reloads the TRB and initialises every instance again.
//-----------------------------------------------------------------------------
class AFXCompositionManager
    : public Toshi::TSingleton<AFXCompositionManager>
{
public:
	// A registered instance (0x10 bytes)
	class AFXInstanceNode
	    : public Toshi::T2DList<AFXInstanceNode>::Node
	{
	public:
		AFXInstanceNode( AFXCompositionInstance* a_pInstance );
		// $deBlob: FUNCTION 005b6560
		// $deBlob: FUNCTION 005b6400
		virtual ~AFXInstanceNode() = default;

		AFXCompositionInstance* GetInstance() const { return m_pInstance; }

	private:
		AFXCompositionInstance* m_pInstance; // 0x0C
	};

	friend class AFXCompositionInstance;

public:
	AFXCompositionManager();
	~AFXCompositionManager();

	// Starts loading a_szFileName on the asset streaming thread
	TBOOL Load( const TCHAR* a_szFileName );

	// Takes the level's data block a_iDataBlock.
	TBOOL LoadFromLevel( TINT a_iDataBlock );

	// TTRUE once the TRB started by Load is loaded
	TBOOL IsDoneLoading();

	// TTRUE if the TRB has the symbol a_szName.
	TBOOL HasFXData( const TCHAR* a_szName );

	void UnLoad();

	// Finishes a reload started by fx_reload
	void Update( TFLOAT a_fDeltaTime );

	void AddInstance( AFXCompositionInstance* a_pInstance );
	void RemoveInstance( AFXCompositionInstance* a_pInstance );

	Toshi::TTRB* GetTRB() const { return m_pTRB; }
	// $deBlob: FUNCTION 005b5b60
	TBOOL IsReloading() const { return m_bReloading; }

private:
	void ReinitializeInstances();

	static void __stdcall FXReloadCommand( Toshi::TString8* a_pArgs, TINT a_iNumArgs );

private:
	ATRBLoaderJob*                  m_pLoadJob;         // 0x00
	Toshi::TTRB*                    m_pTRB;             // 0x04
	TBOOL                           m_bOwnsTRB;         // 0x08
	void*                           m_pFXDataList;      // 0x0C "FXData"
	Toshi::T2DList<AFXInstanceNode> m_oInstances;       // 0x10
	Toshi::TString8                 m_strFileName;      // 0x18
	TBOOL                           m_bReloading;       // 0x24
	TBOOL                           m_bReloadRequested; // 0x25 set by fx_reload
};                                                      // 0x28
