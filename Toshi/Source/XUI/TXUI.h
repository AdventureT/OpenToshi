#pragma once
#include "TXUIResource.h"
#include "TXUIElement.h"
#include "File/TTRB.h"
#include "Toshi/TSingleton.h"
#include "Toshi/T2DList.h"
#include "GUI/T2GUIContext.h"

#ifdef FindResource
#  undef FindResource
#endif

//-----------------------------------------------------------------------------
// Defines the TObject methods and the TClass of a XUI element class. XUI's CreateTObject allocates the elements in
// TXUI::ms_pXUIMemoryBlock (TDEFINE_CLASS uses the global heap). The stacked markers of each class are, in order:
// GetClass, CreateTObject, CreateTObjectInPlace, InitialiseClass, DeinitialiseClass, the TClass dynamic initializer.
//-----------------------------------------------------------------------------
#define TXUI_DEFINE_CLASS( CLASS )                                                                          \
	const Toshi::TClass* CLASS::GetClass() const { return CLASS::GetClassStatic(); }                        \
	Toshi::TObject*      CLASS::CreateTObject() { return new ( Toshi::TXUI::ms_pXUIMemoryBlock ) CLASS(); } \
	Toshi::TObject*      CLASS::CreateTObjectInPlace( void* a_pPtr ) { return new ( a_pPtr ) CLASS(); }     \
	void                 CLASS::InitialiseClass() {}                                                        \
	void                 CLASS::DeinitialiseClass() {}                                                      \
	Toshi::TClass        CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::GetParentClassStatic(), CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, 0, 1, sizeof( CLASS ), TALIGNOF( CLASS ) );

TOSHI_NAMESPACE_START

class TXUICanvas;
class TXUIRenderer;
class TXUIAudio;
class TXUIInput;
class TXUIScene;
class TXUIVisual;
class TXUIMovie;
class TTRB;

//-----------------------------------------------------------------------------
// A .xur stored in a .trb, created by TAssetInit from the "txui" symbols (0x18 bytes)
//-----------------------------------------------------------------------------
class TXUIResourceTRB
{
public:
	friend class TXUI;

public:
	void Init();
	void Deinit();

	const TCHAR*  GetFileName() const { return m_szFileName; }
	TUINT8*       GetData() const { return m_pData; }
	TXUIResource* GetResource() const { return m_pResource; }

private:
	TUINT32          m_uiUnknown;  // 0x00
	const TCHAR*     m_szFileName; // 0x04 lower case name of the .xur
	TUINT8*          m_pData;      // 0x08 the .xur
	TXUIResource*    m_pResource;  // 0x0C
	TXUIResourceTRB* m_pNext;      // 0x10
	TXUIResourceTRB* m_pPrev;      // 0x14
};

//-----------------------------------------------------------------------------
// TXUI (0x188): XUI's singleton. Owns the GUI context and canvas the scenes are created in, the XUI renderer, the
// loaded resources, the skins and the default font, the scene transitions and the scene cache.
//-----------------------------------------------------------------------------
class TXUI : public TSingleton<TXUI>
{
public:
	using t_CreateMovie = TXUIMovie* ( * )();

	// A cached scene (0xC), see TXUIScene::SetCached
	class SceneNode : public T2DList<SceneNode>::Node
	{
	public:
		TXUIScene* m_pScene; // 0x8
	};

public:
	TXUI();
	~TXUI();

	void SetXUIBasePath( const TCHAR* a_szBasePath );

	TXUIResource* FindResource( const TCHAR* a_szName );
	TXUIResource* LoadXUR( TBOOL a_bLoadStringTables, const TCHAR* a_szFileName, TBOOL a_bTRB, TTRB* a_pTRB );

	void SetSkin1( const TCHAR* a_szTRBFileName, const TCHAR* a_szXURName );
	void SetSkin2( const TCHAR* a_szTRBFileName, const TCHAR* a_szXURName );
	void UnloadSkin2();

	void          SetDefaultFont( const TCHAR* a_szTRBFileName, const TCHAR* a_szFontName );
	class T2GUIFont* GetDefaultFont();
	const TCHAR*  GetDefaultFontName() const { return m_szFontName; }

	static const TCHAR* GetXUILocaleDir();

	TXUIScene*  CreateScene( TXUIResource* a_pResource, const TWCHAR* a_wszName, TBOOL a_bCache );
	TXUIScene*  CreateScene( TXUIResource* a_pResource, TUINT32 a_uiIndex );
	TXUIVisual* CreateVisual( const TWCHAR* a_wszName, TBOOL a_bSkin2 );

	void  Tick( TFLOAT a_fDeltaTime );
	TBOOL TickInput( TFLOAT a_fDeltaTime );
	void  Render();

	void SetCanvasDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight );
	void SetFocussedScene( TXUIScene* a_pScene );

	void TransitionTo( TXUIScene* a_pFrom, TXUIScene* a_pTo );
	void TransitionToPopup( TXUIScene* a_pFrom, TXUIScene* a_pTo );
	void TransitionBack( TXUIScene* a_pScene );
	void RemovePopup( TXUIScene* a_pScene );

	void EndTransTo();

	void EndTransBackTo();

	void EndTransFrom();

	void EndTransBackFrom();

	TBOOL HandleButton( TINT a_eButton, TINT a_iController );
	TBOOL HandleButtonRelease( TINT a_eButton, TINT a_iController );
	TBOOL HandleAnalog( TINT a_iParam1, TINT a_iParam2, TINT a_iController );

	void       Reset();
	TXUIMovie* CreateMovie();
	void       SetDefaultController( TINT a_iController );

	void       FlushSceneCache();
	TXUIScene* FindCachedScene( const TWCHAR* a_wszName );

	void AddResource( TXUIResourceTRB* a_pResourceTRB );
	void RemoveResource( TXUIResourceTRB* a_pResourceTRB );

	T2GUIContext* GetContext() const { return m_pContext; }
	TXUICanvas*   GetCanvas() const { return m_pCanvas; }
	// $deBlob: FUNCTION 005532b0
	TXUIRenderer* GetRenderer() const { return m_pRenderer; }
	TXUIInput*    GetInput() const { return m_pInput; }
	TXUIAudio*    GetAudio() const { return m_pXUIAudio; }
	void          SetAudio( TXUIAudio* a_pAudio ) { m_pXUIAudio = a_pAudio; }
	// $deBlob: FUNCTION 00580420
	TXUIScene*    GetActiveScene() const { return m_pActiveScene; }
	// $deBlob: FUNCTION 00652310
	TXUIScene*    GetTransToScene() const { return m_pTransToScene; }
	TXUIScene*    GetTransFromScene() const { return m_pTransFromScene; }
	// $deBlob: FUNCTION 006cf400
	TXUIScene*    GetTransBackFromScene() const { return m_pTransFromScene; }
	// $deBlob: FUNCTION 006d7860
	TXUIControl*  GetCapturing() const { return m_pCapturing; }

	// $deBlob: FUNCTION 006e6590
	void SetCapturing( TXUIControl* a_pControl ) { m_pCapturing = a_pControl; }

	// $deBlob: FUNCTION 006cf380
	void AddCachedScene( TXUIScene* a_pScene )
	{
		SceneNode* pNode = new SceneNode;
		pNode->m_pScene  = a_pScene;
		m_oSceneCache.PushBack( pNode );
	}

	// NOTE: inlined in TXUIScene
	void ClearActiveScene() { m_pActiveScene = TNULL; }
	TINT          GetDefaultController() const { return m_iDefaultController; }
	// $deBlob: FUNCTION 006cf420
	TBOOL         IsPopup() const { return m_bPopup; }
	TBOOL         IsInputLocked() const { return m_bInputLocked; }
	// $deBlob: FUNCTION 0062afe0
	void          SetInputLocked( TBOOL a_bLocked ) { m_bInputLocked = a_bLocked; }
	void          SetCreateMovieCallback( t_CreateMovie a_cbCreateMovie ) { m_cbCreateMovie = a_cbCreateMovie; }
	TXUIResource* GetSkin1Resource() const { return m_pSkin1Resource; }
	TXUIResource* GetSkin2Resource() const { return m_pSkin2Resource; }

	static void* __stdcall AssetTRBAllocator( TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData );
	static void __stdcall  AssetTRBDeallocator( TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData );

	static void RefXUIClasses();

public:
	static TMemoryHeap* ms_pXUIMemoryBlock;
	static TMemoryHeap* ms_pXUITRBMemoryBlock;

private:
	TCHAR                 m_szXUIBasePath[ 128 ]; // 0x000
	TTRB                  m_oFontTRB;             // 0x080
	TTRB                  m_oSkin1TRB;            // 0x094
	TTRB                  m_oSkin2TRB;            // 0x0A8
	TXUIResource*         m_pSkin1Resource;       // 0x0BC
	TXUIResource*         m_pSkin2Resource;       // 0x0C0
	const TCHAR*          m_pDefaultFont;         // 0x0C4
	const TCHAR*          m_sDefaultFont;         // 0x0C8 the font's TRB symbol, "<name>_<size>"
	TCHAR                 m_szFontName[ 128 ];    // 0x0CC m_sDefaultFont up to the last '_'
	TXUICanvas*           m_pCanvas;              // 0x14C
	TXUIScene*            m_pActiveScene;         // 0x150
	TXUIScene*            m_pTransToScene;        // 0x154
	TXUIScene*            m_pTransFromScene;      // 0x158
	T2GUIContext*         m_pContext;             // 0x15C
	TXUIRenderer*         m_pRenderer;            // 0x160
	TXUIResourceTRB*      m_pHeadTRBResource;     // 0x164
	TXUIInput*            m_pInput;               // 0x168
	TINT                  m_iDefaultController;   // 0x16C
	TXUIAudio*            m_pXUIAudio;            // 0x170
	t_CreateMovie         m_cbCreateMovie;        // 0x174
	T2DList<SceneNode>    m_oSceneCache;          // 0x178
	TBOOL                 m_bPopup;               // 0x180
	TBOOL                 m_bInputLocked;         // 0x181 set while a control is pressed (TXUIControl::EndPress clears it)
	TXUIControl*          m_pCapturing;           // 0x184 the control capturing the input (gets all of it when set)
};

TOSHI_NAMESPACE_END
