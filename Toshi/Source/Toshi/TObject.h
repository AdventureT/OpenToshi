#pragma once
#include "TClass.h"

#include <type_traits>

//-----------------------------------------------------------------------------
// Disabled memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOff.h>

#define TClassObjectName   ms_oClass
#define TGetClass( CLASS ) CLASS::TClassObjectName

//-----------------------------------------------------------------------------
// Declares default methods used to register class in the dynamic class system.
// Note: Doesn't declare ms_oClass object (Use TDECLARE_CLASS to auto-declare).
//-----------------------------------------------------------------------------
#define TDECLARE_CLASS_BODY( THIS_CLASS, PARENT_CLASS )                                            \
public:                                                                                            \
	using ThisClass = THIS_CLASS;                                                                  \
	using BaseClass = PARENT_CLASS;                                                                \
	static TCONSTEXPR Toshi::TClass* GetParentClassStatic() { return &TGetClass( PARENT_CLASS ); } \
                                                                                                   \
	virtual const Toshi::TClass* GetClass() const override;                                        \
                                                                                                   \
	static Toshi::TObject* CreateTObject();                                                        \
	static Toshi::TObject* CreateTObjectInPlace( void* a_pPtr );                                   \
	static void            InitialiseClass();                                                      \
	static void            DeinitialiseClass();                                                    \
                                                                                                   \
	static TFORCEINLINE const Toshi::TClass* GetClassStatic() { return std::addressof( TClassObjectName ); }

//-----------------------------------------------------------------------------
// Declares default methods used to register class in the dynamic class system.
// Note: Doesn't declare ms_oClass object (Use TDECLARE_CLASS to auto-declare).
//-----------------------------------------------------------------------------
#define TDECLARE_CLASS_BODY_NO_PARENT( THIS_CLASS )                           \
public:                                                                       \
	using ThisClass = THIS_CLASS;                                             \
	static TCONSTEXPR Toshi::TClass* GetParentClassStatic() { return TNULL; } \
                                                                              \
	const Toshi::TClass* GetClass() const;                                    \
                                                                              \
	static Toshi::TObject* CreateTObject();                                   \
	static Toshi::TObject* CreateTObjectInPlace( void* a_pPtr );              \
	static void            InitialiseClass();                                 \
	static void            DeinitialiseClass();                               \
                                                                              \
	static TFORCEINLINE const Toshi::TClass* GetClassStatic() { return std::addressof( TClassObjectName ); }

//-----------------------------------------------------------------------------
// Declares default methods to register derived class and ms_oClass object.
// Note: Use one of the TDEFINE_CLASS macros in a cpp file to register class.
//-----------------------------------------------------------------------------
#define TDECLARE_CLASS( THIS_CLASS, PARENT_CLASS )   \
	TDECLARE_CLASS_BODY( THIS_CLASS, PARENT_CLASS ); \
	static Toshi::TClass TClassObjectName

//-----------------------------------------------------------------------------
// Declares default methods to register derived class and ms_oClass object.
// Note: Use one of the TDEFINE_CLASS macros in a cpp file to register class.
//-----------------------------------------------------------------------------
#define TDECLARE_CLASS_NO_PARENT( THIS_CLASS )   \
	TDECLARE_CLASS_BODY_NO_PARENT( THIS_CLASS ); \
	static Toshi::TClass TClassObjectName

//-----------------------------------------------------------------------------
// Declares default methods to register class and ms_oClass object.
// Note: Use one of the TDEFINE_CLASS macros in a cpp file to register class.
//-----------------------------------------------------------------------------
#define TDECLARE_CLASS_COMPILETIME( THIS_CLASS, PARENT_CLASS ) \
	TDECLARE_CLASS_BODY( THIS_CLASS, PARENT_CLASS );           \
	static TCONSTINIT Toshi::TClass TClassObjectName;

//-----------------------------------------------------------------------------
// Defines core methods of TObject and allows creating object at runtime.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_CORE( CLASS )                                                                                    \
	TSTATICASSERT( CLASS::IsTObject );                                                                                 \
	TSTATICASSERT( std::is_base_of<CLASS::BaseClass, CLASS>::value && !std::is_same<CLASS::BaseClass, CLASS>::value ); \
	const Toshi::TClass* CLASS::GetClass() const { return CLASS::GetClassStatic(); }                                   \
	Toshi::TObject*      CLASS::CreateTObject() { return new CLASS(); }                                                \
	Toshi::TObject*      CLASS::CreateTObjectInPlace( void* a_pPtr ) { return new ( a_pPtr ) CLASS(); }

//-----------------------------------------------------------------------------
// Defines core methods of TObject and prohibits creating object at runtime.
// NOTE: de Blob's CreateTObject/CreateTObjectInPlace only return TNULL (no assert)
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_CORE_NORUNTIME( CLASS )                                                                          \
	TSTATICASSERT( CLASS::IsTObject );                                                                                 \
	TSTATICASSERT( std::is_base_of<CLASS::BaseClass, CLASS>::value && !std::is_same<CLASS::BaseClass, CLASS>::value ); \
	const Toshi::TClass* CLASS::GetClass() const { return CLASS::GetClassStatic(); }                                   \
	Toshi::TObject*      CLASS::CreateTObject()                                                                        \
	{                                                                                                                  \
		return TNULL;                                                                                                  \
	}                                                                                                                  \
	Toshi::TObject* CLASS::CreateTObjectInPlace( void* a_pPtr )                                                        \
	{                                                                                                                  \
		return TNULL;                                                                                                  \
	}

//-----------------------------------------------------------------------------
// Defines core methods of TObject and allows creating object at runtime.
// Note: Doesn't have type checks.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_CORE_COMPILETIME( CLASS )                                      \
	TSTATICASSERT( CLASS::IsTObject );                                               \
	const Toshi::TClass* CLASS::GetClass() const { return CLASS::GetClassStatic(); } \
	Toshi::TObject*      CLASS::CreateTObject() { return new CLASS(); }              \
	Toshi::TObject*      CLASS::CreateTObjectInPlace( void* a_pPtr ) { return new ( a_pPtr ) CLASS(); }

//-----------------------------------------------------------------------------
// Defines core methods of TObject and allows creating object at runtime.
// Also, adds empty InitialiseClass and DeinitialiseClass methods.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_FULL( CLASS )  \
	TDEFINE_CLASS_CORE( CLASS )      \
	void CLASS::InitialiseClass() {} \
	void CLASS::DeinitialiseClass() {}

//-----------------------------------------------------------------------------
// Defines core methods of TObject and prohibits creating object at runtime.
// NOTE: de Blob's CreateTObject/CreateTObjectInPlace only return TNULL (no assert)
// Also, adds empty InitialiseClass and DeinitialiseClass methods.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_FULL_NORUNTIME( CLASS ) \
	TDEFINE_CLASS_CORE_NORUNTIME( CLASS )     \
	void CLASS::InitialiseClass() {}          \
	void CLASS::DeinitialiseClass() {}

//-----------------------------------------------------------------------------
// Defines core methods of TObject and allows creating object at runtime.
// Also, adds empty InitialiseClass and DeinitialiseClass methods.
// Note: Doesn't have type checks.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_FULL_COMPILETIME( CLASS ) \
	TDEFINE_CLASS_CORE_COMPILETIME( CLASS )     \
	void CLASS::InitialiseClass() {}            \
	void CLASS::DeinitialiseClass() {}

//-----------------------------------------------------------------------------
// Defines all TObject methods and the ms_oClass object.
// Note: Can be created at runtime.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS( ... ) CALL_OVERLOAD( TDEFINE_CLASS, __VA_ARGS__ )
#define TDEFINE_CLASS1( CLASS ) \
	TDEFINE_CLASS_FULL( CLASS ) \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::GetParentClassStatic(), CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, 0, 1, sizeof( CLASS ), TALIGNOF( CLASS ) );
#define TDEFINE_CLASS3( CLASS, VER_MAJOR, VER_MINOR ) \
	TDEFINE_CLASS_FULL( CLASS )                       \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::GetParentClassStatic(), CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, VER_MAJOR, VER_MINOR, sizeof( CLASS ), TALIGNOF( CLASS ) );

//-----------------------------------------------------------------------------
// Defines all TObject methods and the ms_oClass object.
// Note: Cannot be created at runtime!
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_NORUNTIME1( CLASS ) \
	TDEFINE_CLASS_FULL_NORUNTIME( CLASS ) \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::GetParentClassStatic(), CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, 0, 1, sizeof( CLASS ), TALIGNOF_ABSTRACT( CLASS ) );
#define TDEFINE_CLASS_NORUNTIME3( CLASS, VER_MAJOR, VER_MINOR ) \
	TDEFINE_CLASS_FULL_NORUNTIME( CLASS )                       \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::GetParentClassStatic(), CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, VER_MAJOR, VER_MINOR, sizeof( CLASS ), TALIGNOF_ABSTRACT( CLASS ) );
#define TDEFINE_CLASS_NORUNTIME( ... ) CALL_OVERLOAD( TDEFINE_CLASS_NORUNTIME, __VA_ARGS__ )

//-----------------------------------------------------------------------------
// Defines core TObject methods and the ms_oClass object.
// Provide InitialiseClass and DeinitialiseClass methods manually.
// Note: Can be created at runtime.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_INIT1( CLASS ) \
	TDEFINE_CLASS_CORE( CLASS )      \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::GetParentClassStatic(), CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, 0, 1, sizeof( CLASS ), TALIGNOF( CLASS ) );
#define TDEFINE_CLASS_INIT3( CLASS, VER_MAJOR, VER_MINOR ) \
	TDEFINE_CLASS_CORE( CLASS )                            \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::GetParentClassStatic(), CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, VER_MAJOR, VER_MINOR, sizeof( CLASS ), TALIGNOF( CLASS ) );
#define TDEFINE_CLASS_INIT( ... ) CALL_OVERLOAD( TDEFINE_CLASS_INIT, __VA_ARGS__ )

//-----------------------------------------------------------------------------
// Defines core TObject methods and the ms_oClass object.
// Provide InitialiseClass and DeinitialiseClass methods manually.
// Note: Cannot be created at runtime!
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_INIT_NORUNTIME1( CLASS ) \
	TDEFINE_CLASS_CORE_NORUNTIME( CLASS )      \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::GetParentClassStatic(), CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, 0, 1, sizeof( CLASS ), TALIGNOF( CLASS ) );
#define TDEFINE_CLASS_INIT_NORUNTIME3( CLASS, VER_MAJOR, VER_MINOR ) \
	TDEFINE_CLASS_CORE_NORUNTIME( CLASS )                            \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::GetParentClassStatic(), CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, VER_MAJOR, VER_MINOR, sizeof( CLASS ), TALIGNOF( CLASS ) );
#define TDEFINE_CLASS_INIT_NORUNTIME( ... ) CALL_OVERLOAD( TDEFINE_CLASS_INIT_NORUNTIME, __VA_ARGS__ )

//-----------------------------------------------------------------------------
// Defines all TObject methods and the ms_oClass object.
// Note: ms_oClass object is created at compile time.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_COMPILETIME( CLASS, VER_MAJOR, VER_MINOR ) \
	TDEFINE_CLASS_FULL_COMPILETIME( CLASS )                      \
	TCONSTINIT Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, VER_MAJOR, VER_MINOR, sizeof( CLASS ), TALIGNOF( CLASS ) );

//-----------------------------------------------------------------------------
// Defines TClass for a class without parent
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_NO_PARENT( CLASS, VER_MAJOR, VER_MINOR ) \
	TCONSTINIT Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, Toshi::TClass::CreateObject_t( TNULL ), TNULL, TNULL, TNULL, VER_MAJOR, VER_MINOR, sizeof( CLASS ), TALIGNOF( CLASS ) );

TOSHI_NAMESPACE_START

class TObject
{
public:
	enum
	{
		IsTObject = TTRUE
	};
	static TCONSTEXPR Toshi::TClass* GetParentClassStatic() { return TNULL; }

public:
	TObject();

	virtual const Toshi::TClass* GetClass() const; // +0x00
	virtual void                 Delete();         // +0x04
	virtual ~TObject();                            // +0x08

public:
	TBOOL IsExactly( const TClass* a_pClass ) const { return GetClass() == a_pClass; }
	TBOOL IsA( const TClass* a_pClass ) const { return GetClass()->IsA( a_pClass ); }

	// A checked static cast
	// $deBlob: FUNCTION 0054ed90 (C = ASkinMesh)
	// $deBlob: FUNCTION 0054ede0 (C = TMesh)
	// $deBlob: FUNCTION 00554a00 (C = TWorldMeshHAL)
	// $deBlob: FUNCTION 00557dd0 (C = TWorldMaterialHAL)
	// $deBlob: FUNCTION 00557e20 (C = TWorldShaderHAL)
	// $deBlob: FUNCTION 0055d7c0 (C = ASkinMaterialHAL)
	// $deBlob: FUNCTION 0056bc80 (C = TRenderDX11)
	// $deBlob: FUNCTION 005810b0 (C = ABlobEntity)
	// $deBlob: FUNCTION 0058c640 (C = AScriptedCamera)
	// $deBlob: FUNCTION 0058ea50 (C = ABlobFollowCamera)
	// $deBlob: FUNCTION 0058eaa0 (C = AFreeCamera)
	// $deBlob: FUNCTION 00591500 (C = TInputDeviceKeyboard)
	// $deBlob: FUNCTION 0059c440 (C = ACiv)
	// $deBlob: FUNCTION 0059c490 (C = ALeechBot2)
	// $deBlob: FUNCTION 0059c4e0 (C = APropWaterHydrant)
	// $deBlob: FUNCTION 0059dc80 (C = AEnemyWrapper)
	// $deBlob: FUNCTION 005abe60 (C = TInputDXDeviceController)
	// $deBlob: FUNCTION 005acaa0 (C = ABaseEntity)
	// $deBlob: FUNCTION 005b9cd0 (C = AIOEntity)
	// $deBlob: FUNCTION 005c0150 (C = ACivSpawner)
	// $deBlob: FUNCTION 005c2420 (C = ABlobControllerPopup)
	// $deBlob: FUNCTION 005d11f0 (C = ACamera)
	// $deBlob: FUNCTION 005d1240 (C = AKeyFrameCamera)
	// $deBlob: FUNCTION 005d6cc0 (C = TInputDeviceController)
	// $deBlob: FUNCTION 005d6d10 (C = TInputDeviceMouse)
	// $deBlob: FUNCTION 005d9080 (C = TInputDXDeviceMouse)
	// $deBlob: FUNCTION 005f6750 (C = TSysShaderHAL)
	// $deBlob: FUNCTION 0060bd10 (C = AXUIState)
	// $deBlob: FUNCTION 00638c60 (C = ABlobPopupMusicMixer)
	// $deBlob: FUNCTION 006423e0 (C = TInputDevice)
	// $deBlob: FUNCTION 0064ec10 (C = TXUIText)
	// $deBlob: FUNCTION 00653a20 (C = AInGameState)
	// $deBlob: FUNCTION 006bb9b0 (C = TWorldShader)
	// $deBlob: FUNCTION 00705910 (C = TWorldMaterial)
	// $deBlob: FUNCTION 0070e360 (C = TSysMaterial)
	// $deBlob: FUNCTION 00750fc0 (C = APickupLife)
	// $deBlob: FUNCTION 00751010 (C = APropHazard)
	// $deBlob: FUNCTION 00751060 (C = APropTriggerLevelComplete)
	// $deBlob: FUNCTION 007510b0 (C = APropTriggerTimeBonus)
	// $deBlob: FUNCTION 00758450 (C = ASkinMaterial)
	// $deBlob: FUNCTION 0075c440 (C = APickup)
	// $deBlob: FUNCTION 00769d50 (C = APropPlatform)
	// $deBlob: FUNCTION 00769da0 (C = APropTrigger)
	// $deBlob: FUNCTION 00789690 (C = ASkinMeshHAL)
	// $deBlob: FUNCTION 0078b890 (C = AEnemy)
	// $deBlob: FUNCTION 0079c1d0 (C = ACharEntity)
	// $deBlob: FUNCTION 0079f230 (C = ABlobChallengeEndHud)
	// $deBlob: FUNCTION 0079f290 (C = APropDoor)
	// $deBlob: FUNCTION 007a4410 (C = AStreetSpawner)
	// $deBlob: FUNCTION 007a5250 (C = AGodRay)
	// $deBlob: FUNCTION 007a89f0 (C = APickupSolid)
	template <class C>
	C* CastTo()
	{
		TASSERT( ( this == TNULL ) || IsA( &TGetClass( C ) ) );
		return TSTATICCAST( C, this );
	}

public:
	static Toshi::TObject* CreateTObject();
	static Toshi::TObject* CreateTObjectInPlace( void* a_pPtr );
	static void            InitialiseClass();
	static void            DeinitialiseClass();

	static TFORCEINLINE TClass* GetClassStatic() { return std::addressof( TClassObjectName ); }

public:
	static TCONSTINIT Toshi::TClass TClassObjectName;
};

TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Safely casts TObject to specified TClass
//-----------------------------------------------------------------------------
template <class T>
TFORCEINLINE T* TDynamicCast( Toshi::TObject* a_pObject )
{
	TSTATICASSERT( T::IsTObject );

	if ( a_pObject )
	{
		if ( a_pObject->IsA( &TGetClass( T ) ) )
		{
			return TSTATICCAST( T, a_pObject );
		}
	}

	return TNULL;
}

#define TDYNAMICCAST( T, OBJECT ) ( TDynamicCast<T>( OBJECT ) )
