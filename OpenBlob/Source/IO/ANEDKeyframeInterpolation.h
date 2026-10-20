#pragma once
#include <Math/TMatrix44.h>

class ABaseEntity;

//-----------------------------------------------------------------------------
// Purpose: the key frame data of the IO entities (AIOKeyframeComponent, loaded from the level's TRB) and its
// interpolation (ANEDKeyframeInterpolation.cpp).
//-----------------------------------------------------------------------------
class ANEDKeyframeInterpolation
{
public:
	enum DataType : TINT
	{
		DataType_Transform = 0,
	};

	// The scalars of a transform key
	enum KeyComponentType : TINT
	{
		KeyComponent_TranslationX = 0,
		KeyComponent_TranslationY = 1,
		KeyComponent_TranslationZ = 2,
		KeyComponent_ScaleX       = 6,
		KeyComponent_ScaleY       = 7,
		KeyComponent_ScaleZ       = 8,
	};

	// The interpolation of a component to the next key
	enum InterpolationType : TINT16
	{
		Interpolation_Linear = 0,
		Interpolation_None   = 1, // 0
		Interpolation_Step   = 2, // the value of the first key
	};

	// A key (0x1C bytes)
	struct TransformKeyData
	{
		TINT                    componentCount;      // 0x00
		TFLOAT                  fTime;               // 0x04
		TINT                    iUnk08;              // 0x08
		TINT                    iUnk0C;              // 0x0C
		const TINT16*           pInterpolationTypes; // 0x10 InterpolationType of each component
		TINT                    iUnk14;              // 0x14
		const Toshi::TMatrix44* pMatrix;             // 0x18
	};

	// The keys of a track (0xC bytes)
	struct KeyList
	{
		TransformKeyData* pKeys;    // 0x00
		TINT              iNumKeys; // 0x04
		TINT              iUnk08;   // 0x08
	};

	// The keys of one entity (0x18 bytes)
	struct Track
	{
		KeyList*     pKeyLists;    // 0x00
		TINT         iNumKeyLists; // 0x04
		TINT         iEntityID;    // 0x08
		ABaseEntity* pEntity;      // 0x0C set by AIOKeyframeComponent's PostCreate
		TFLOAT       fStartTime;   // 0x10
		TFLOAT       fEndTime;     // 0x14
	};

	// The tracks of an AIOKeyframeComponent (its TRB symbol)
	struct TrackSet
	{
		Track* pTracks;    // 0x00
		TINT   iNumTracks; // 0x04
		TFLOAT fTime;      // 0x08 the current time
		TFLOAT fEndTime;   // 0x0C
	};

public:
	// The matrix between two keys at a_fTime (0..1): the rotation slerped, the translation and scale interpolated
	static void InterpolateMatrix( const TransformKeyData* a_pKey1, const TransformKeyData* a_pKey2, TFLOAT a_fTime, Toshi::TMatrix44& a_rOutMatrix );

	// The component a_iComponent between two keys at a_fTime (0..1)
	static TFLOAT GetInterpolatedScalar( const TransformKeyData* a_source, const TransformKeyData* a_pKey2, TFLOAT a_time, DataType a_eDataType, TINT a_component );

	// The component a_iComponent of a key
	static TFLOAT GetRootScalar( const TransformKeyData* a_pKey, DataType a_eDataType, TINT a_iComponent );

	// The key before a_fTime (its time <= a_fTime < the next key's), -1 if none
	static TINT FindKey( const KeyList* a_pKeyList, TFLOAT a_fTime );
};
