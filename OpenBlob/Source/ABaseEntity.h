#pragma once

#include "Toshi/Core/TObject.h"
#include "Toshi2/T2DList.h"
#include "Toshi/Math/Math.h"

class ABaseEntity : public Toshi::TGenericClassDerived<ABaseEntity, Toshi::TObject, "ABaseEntity", TMAKEVERSION(1, 0), TTRUE>
    , public Toshi::T2DList<ABaseEntity>::Node
{
	static constexpr TINT ENTITY_HASH_SIZE = 0x400;

	enum FLAG
	{
		FLAG_Created      = BITFIELD(0),
		FLAG_Visible      = BITFIELD(2),
		FLAG_DontTick     = BITFIELD(8),
		FLAG_LowLOD       = BITFIELD(9),
		FLAG_MediumLOD    = BITFIELD(10),
		FLAG_HighLOD      = BITFIELD(11),
		FLAG_LODMask      = FLAG_LowLOD | FLAG_MediumLOD | FLAG_HighLOD,
		FLAG_UPDATEMATRIX = BITFIELD(15)
	};

public:
	virtual ~ABaseEntity() = default;
	// $deBlob: FUNCTION 0075e8b0
	virtual void Tick(TFLOAT a_fTicks) { m_fTicks += a_fTicks; }
	// $deBlob: FUNCTION 00751e50
	virtual void OnLODChanging(){};
	// $deBlob: FUNCTION 0075e970
	virtual void Create()
	{
		TASSERT(0 == (m_Flags & FLAG_Created));
		m_Flags |= FLAG_Created;
	}
	// $deBlob: FUNCTION 0075e8d0
	virtual void PostCreate(){};
	// $deBlob: FUNCTION 00752390
	virtual void Destroy(){};

	// $deBlob: FUNCTION 007516d0
	virtual TFLOAT GetAnimationSpeed(TINT) const { return 1.0f; }
	// $deBlob: FUNCTION 00752310
	virtual void   SetVisible(TBOOL bEnable)
	{
		bEnable ? m_Flags |= FLAG_Visible : m_Flags &= ~FLAG_Visible;
	}
	// $deBlob: FUNCTION 00752250
	virtual void SetPosition(const Toshi::TVector4& a_pos)
	{
		m_mMatrix.m_f41 = a_pos.x;
		m_mMatrix.m_f42 = a_pos.y;
		m_mMatrix.m_f43 = a_pos.z;
		m_mMatrix.m_f44 = a_pos.w;
		m_Flags |= FLAG_UPDATEMATRIX;
	}
	// $deBlob: FUNCTION 0075f770
	virtual void SetFacing(const Toshi::TVector4& a_facing)
	{
		m_mMatrix.LookAtDirection(a_facing, Toshi::TVector4::VEC_NEGY);
		m_Flags |= FLAG_UPDATEMATRIX;
	}
	// $deBlob: FUNCTION 007521e0
	virtual void SetMatrix(const Toshi::TMatrix44& a_matrix)
	{
		m_mMatrix = a_matrix;
		m_Flags |= FLAG_UPDATEMATRIX;
	}

	// $deBlob: FUNCTION 00751950
	virtual void GetReactionForce(Toshi::TVector4& a_reactionForce)
	{
		a_reactionForce = Toshi::TVector4::VEC_ZERO;
	}

	// $deBlob: FUNCTION 00751a80
	virtual TFLOAT GetStepUpHeight() const { return 0.3f; }
	// $deBlob: FUNCTION 0075eb60
	virtual void   SetBBox(Toshi::TVector4 a_bbox)
	{
		m_vBoundingBox          = a_bbox;
		m_fBoundingSphereRadius = a_bbox.Magnitude();
	}
	// $deBlob: FUNCTION 00751830
	virtual TFLOAT GetInteractionRadius() const { return 2.5f; }
	// $deBlob: FUNCTION 00751e60
	virtual void   OnLeaveState(TINT a_State){};
	// $deBlob: FUNCTION 00751e10
	virtual void   OnEnterState(TINT a_State){};

	void SetState(TINT a_State);


	static TINT sm_EntityHash[ENTITY_HASH_SIZE];

	TSHORT                             m_State;                 // 0xC
	TSHORT                             m_iID;                   // 0xE
	Toshi::TMatrix44                   m_mMatrix;               // 0x10
	Toshi::TVector4                    m_vBoundingBox;          // 0x50
	TFLOAT                             m_fBoundingSphereRadius; // 0x68
	TFLOAT                             m_fTicks;                // 0x6C
	TUINT8                             m_Flags;                 // 0x70
	Toshi::T2DList<ABaseEntity>::Node* m_pEntityCellPair;       // 0x7C
};
