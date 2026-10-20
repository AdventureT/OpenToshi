#pragma once
#include "T2GUITransform.h"
#include "Toshi/Typedefs.h"
#include "Math/TVector2.h"

TOSHI_NAMESPACE_START

class T2GUIElement;

// NOTE: retail's compiler sees FLOAT_QUALITY (6.0f) as a constant in every user of PackFloat
static const TFLOAT s_fPackQuality = 6.0f;

// NOTE: de Blob: m_pPrev is at +0x4 and m_pNext at +0x8 (Barnyard has them the other way around)
class T2GUIElementNode
{
public:
	friend T2GUIElement;

public:
	// $deBlob: FUNCTION 006f5fb0
	T2GUIElementNode()
	{
		m_pPrev = this;
		m_pNext = this;
	}

	// $deBlob: FUNCTION 006f5fc0
	// $deBlob: FUNCTION 006f6d80
	virtual ~T2GUIElementNode()
	{
		Unlink();
	}

	void Unlink()
	{
		m_pPrev->m_pNext = m_pNext;
		m_pNext->m_pPrev = m_pPrev;
		m_pPrev          = this;
		m_pNext          = this;
	}

	// Inserts this node after a_pNode
	// $deBlob: FUNCTION 006f6dc0
	void LinkAfter( T2GUIElementNode* a_pNode )
	{
		m_pPrev          = a_pNode;
		m_pNext          = a_pNode->m_pNext;
		a_pNode->m_pNext = this;
		m_pNext->m_pPrev = this;
	}

	// Inserts this node before a_pNode
	// $deBlob: FUNCTION 006f6de0
	void LinkBefore( T2GUIElementNode* a_pNode )
	{
		m_pPrev          = a_pNode->m_pPrev;
		m_pNext          = a_pNode;
		m_pPrev->m_pNext = this;
		m_pNext->m_pPrev = this;
	}

	TBOOL             IsLinked() const { return this != m_pNext; }
	T2GUIElementNode* GetNext() const { return m_pNext; }
	T2GUIElementNode* GetPrev() const { return m_pPrev; }

public:
	T2GUIElementNode* m_pPrev; // 0x04
	T2GUIElementNode* m_pNext; // 0x08
};

enum T2GUIATTACHMENT : TUINT8
{
	T2GUIATTACHMENT_TOPLEFT,
	T2GUIATTACHMENT_TOPCENTER,
	T2GUIATTACHMENT_TOPRIGHT,
	T2GUIATTACHMENT_MIDDLELEFT,
	T2GUIATTACHMENT_MIDDLECENTER,
	T2GUIATTACHMENT_MIDDLERIGHT,
	T2GUIATTACHMENT_BOTTOMLEFT,
	T2GUIATTACHMENT_BOTTOMCENTER,
	T2GUIATTACHMENT_BOTTOMRIGHT,
	T2GUIATTACHMENT_NUMOF,
};

// de Blob's T2GUIElement (0x3C bytes, 23 virtuals): the dimensions are packed shorts (value * 6), the colour, state
// (visibility mask), anchor, pivot and flags are kept in 8 bytes. Compared to Barnyard it adds IsPaused, SkipRender,
// GetAlpha, SetVisible, GetScreenTransform and GetPivot as virtuals (used by XUI) and has no SetShadowAlpha.
class T2GUIElement : public T2GUIElementNode
{
public:
	static const TFLOAT FLOAT_QUALITY;
	static const TFLOAT FLOAT_SCALE;

	using Float        = TINT16;
	using t_PostRender = void ( * )();

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE       = 0,
		FLAGS_VISIBLE    = BITFLAG( 0 ),
		FLAGS_FOCUSED    = BITFLAG( 1 ),
		FLAGS_XUIELEMENT = BITFLAG( 2 ),
	};

	// The bits of m_uiState: an element is shown while one of them is set in s_uiGlobalVisMask, which the game sets for
	// each of its states
	enum STATE_ : TUINT8
	{
		STATE_NONE      = 0,
		STATE_INGAME    = BITFLAG( 0 ), // only in the mask of the game itself
		STATE_UNK1      = BITFLAG( 1 ),
		STATE_MENU      = BITFLAG( 2 ), // the default mask: the XUI scenes
		STATE_UNK3      = BITFLAG( 3 ), // in the masks of the game and of the flythrough
		STATE_SUBTITLES = BITFLAG( 4 ), // the only bit of the mask of a movie
		STATE_ALL       = 0xFF,
	};

public:
	T2GUIElement();
	virtual ~T2GUIElement();                                                // +0x00

	virtual void            Tick( TFLOAT a_fDeltaTime );                    // +0x04
	virtual TBOOL           IsPaused();                                     // +0x08
	virtual TBOOL           SkipRender();                                   // +0x0C
	virtual void            PreRender();                                    // +0x10
	virtual void            Render();                                       // +0x14
	virtual void            PostRender();                                   // +0x18
	virtual void            GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight ); // +0x1C
	virtual TFLOAT          GetWidth();                                     // +0x20
	virtual TFLOAT          GetHeight();                                    // +0x24
	virtual void            SetWidth( TFLOAT a_fWidth );                    // +0x28
	virtual void            SetHeight( TFLOAT a_fHeight );                  // +0x2C
	virtual void            SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ); // +0x30
	virtual void            GetMins( TFLOAT& a_rX, TFLOAT& a_rY );          // +0x34
	virtual void            GetMaxs( TFLOAT& a_rX, TFLOAT& a_rY );          // +0x38
	virtual void            SetColour( TUINT32 a_uiColour );                // +0x3C
	virtual void            SetAlpha( TFLOAT a_fAlpha );                    // +0x40
	virtual TFLOAT          GetAlpha();                                     // +0x44
	virtual void            SetVisible( TBOOL a_bVisible );                 // +0x48
	virtual void            SetFocus( TBOOL a_bFocused );                   // +0x4C
	virtual void            GetScreenTransform( T2GUITransform& a_rOutTransform ); // +0x50
	virtual TBOOL           IsPointInside( const TVector2& a_rPoint );      // +0x54
	virtual const TVector2& GetPivot();                                     // +0x58

	// Offsets a_rX/a_rY by the pivot point of an element of the given size
	void ApplyPivot( TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight );

	void GetInvScreenTransform( T2GUITransform& a_rOutTransform );

	void AddChildBefore( T2GUIElement* a_pElement, T2GUIElement* a_pNext, TBOOL a_bUnlink );
	void AddChildAfter( T2GUIElement* a_pElement, T2GUIElement* a_pPrevious, TBOOL a_bUnlink );

	void AddChildTail( T2GUIElement* a_pElement );

	void AddChildHead( T2GUIElement* a_pElement );

	// $deBlob: FUNCTION 006f65b0
	void SetTransform( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fAngle = 0.0f )
	{
		m_oTransform.SetIdentity();
		m_oTransform.Rotate( a_fAngle );
		m_oTransform.m_Pos.x = a_fX;
		m_oTransform.m_Pos.y = a_fY;
	}

	// $deBlob: FUNCTION 005a2b00
	void RemoveSelf()
	{
		T2GUIElementNode::Unlink();
		m_pParent = TNULL;
	}

	void Show() { m_eFlags |= FLAGS_VISIBLE; }
	void Hide() { m_eFlags &= ~FLAGS_VISIBLE; }

	TBOOL GetFlag( FLAGS a_eFlag ) const { return ( m_eFlags & a_eFlag ) != 0; }
	void  SetFlag( FLAGS a_eFlag ) { m_eFlags |= a_eFlag; }
	void  ClearFlag( FLAGS a_eFlag ) { m_eFlags &= ~a_eFlag; }

	// NOTE: an instance lives in the game code
	// $deBlob: FUNCTION 005805f0
	TBOOL IsVisible() const { return ( m_eFlags & FLAGS_VISIBLE ) && ( m_uiState & s_uiGlobalVisMask ) != 0; }
	TBOOL IsFocused() const { return ( m_eFlags & FLAGS_FOCUSED ) != 0; }

	TUINT8 GetState() const { return m_uiState; }
	void   SetState( TUINT8 a_uiState ) { m_uiState = a_uiState; }

	void SetAnchor( T2GUIATTACHMENT a_eAnchor ) { m_eAnchor = a_eAnchor; }
	void SetPivot( T2GUIATTACHMENT a_ePivot ) { m_ePivot = a_ePivot; }

	void SetAttachment( T2GUIATTACHMENT a_eAnchor, T2GUIATTACHMENT a_ePivot )
	{
		m_eAnchor = a_eAnchor;
		m_ePivot  = a_ePivot;
	}

	t_PostRender SetPostRenderCallback( t_PostRender a_cbNewCallback )
	{
		t_PostRender cbOld = m_cbPostRender;
		m_cbPostRender     = a_cbNewCallback;
		return cbOld;
	}

	T2GUIElement*   GetParent() const { return m_pParent; }
	T2GUITransform& GetTransform() { return m_oTransform; }
	TUINT32         GetColour() const { return m_uiColour; }

	// Returns the first child or TNULL
	T2GUIElement* ChildBegin() const { return ( m_Children.IsLinked() ) ? TSTATICCAST( T2GUIElement, m_Children.m_pNext ) : TNULL; }

	// Returns the last child or TNULL
	T2GUIElement* ChildRBegin() const { return ( m_Children.IsLinked() ) ? TSTATICCAST( T2GUIElement, m_Children.m_pPrev ) : TNULL; }

	// Returns the next child or TNULL
	T2GUIElement* GetNextChild( T2GUIElement* a_pCurrentChild ) const
	{
		if ( a_pCurrentChild->m_pNext == &m_Children || a_pCurrentChild->m_pNext == a_pCurrentChild )
			return TNULL;

		return TSTATICCAST( T2GUIElement, a_pCurrentChild->m_pNext );
	}

	// Returns the previous child or TNULL
	T2GUIElement* GetPrevChild( T2GUIElement* a_pCurrentChild ) const
	{
		if ( a_pCurrentChild->m_pPrev == &m_Children || a_pCurrentChild->m_pPrev == a_pCurrentChild )
			return TNULL;

		return TSTATICCAST( T2GUIElement, a_pCurrentChild->m_pPrev );
	}

	const T2GUIElementNode* GetChildrenEnd() const { return &m_Children; }

	static TFLOAT UnpackFloat( Float a_iValue ) { return a_iValue * FLOAT_SCALE; }
	static Float  PackFloat( TFLOAT a_fValue ) { return Float( TINT( a_fValue * s_fPackQuality ) ); }

public:
	static TUINT8 s_uiGlobalVisMask;

protected:
	T2GUIElement*    m_pParent;      // 0x0C
	T2GUITransform   m_oTransform;   // 0x10
	T2GUIElementNode m_Children;     // 0x20
	Float            m_iWidth;       // 0x2C
	Float            m_iHeight;      // 0x2E
	TUINT32          m_uiColour;     // 0x30
	t_PostRender     m_cbPostRender; // 0x34
	TUINT8           m_uiState;      // 0x38 visibility mask, tested against s_uiGlobalVisMask
	T2GUIATTACHMENT  m_eAnchor;      // 0x39
	T2GUIATTACHMENT  m_ePivot;       // 0x3A
	FLAGS            m_eFlags;       // 0x3B
};

TOSHI_NAMESPACE_END
