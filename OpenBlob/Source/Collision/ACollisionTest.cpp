#include "pch.h"
#include "ACollisionTest.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "ACollisionCommon.h"
#include "AAttackCollisionFunctor.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Physics/Havok/TPhysicsConstraintContainer.h>
#include <Physics/TSphereData.h>
#include <Physics/TBoxData.h>
#include <Physics/TCapsuleData.h>
#include <Physics/TCylinderData.h>
#include <Input/TInputInterface.h>
#include <Platform/Windows/Input/TInputDeviceController_Win.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Toshi/THPTimer.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL ACollisionTest::ms_bRunTimerTest;
TBOOL ACollisionTest::ms_bTestSurface  = TTRUE;
TBOOL ACollisionTest::ms_bRunMathTests = TTRUE;

TBOOL ACollisionTest::ms_bRunTimingTests = TFALSE;

// The colours of the test shapes (ARGB), unused
// $deBlob: FUNCTION 0053bab0
static TUINT32 s_auiTestColours[ 8 ] = { 0xC0FF0000, 0xC000FF00, 0xC00000FF, 0xC0FFFF00, 0xC000FFFF, 0xC0FF00FF, 0xC0555555, 0xC0AAAAAA }; // 0x00991938

// The physics test scene (CreateTestScene, UpdateTestScene, DestroyTestScene)
static TCONSTEXPR_VAR TINT NUM_TEST_BODIES = 20;

static TBOOL                    s_bOnlySpheres; // every test body is a sphere
static TBOOL                    s_bResetScene;  // the bodies go back to Blob
static TPhysicsObjectContainer* s_pAnchorBody;  // the fixed sphere the constraints hold on to

// The matrix of the anchor body (written by UpdateTestScene)
// $deBlob: FUNCTION 0053bb30
static TMatrix44 s_mTestShape = TMatrix44::IDENTITY; // 0x00991968

static TVector4                     s_vBallPivot;
static TVector4                     s_vHingePivot;
static TMatrix44                    s_aTestMatrices[ NUM_TEST_BODIES ];
static TPhysicsObjectContainer*     s_apTestBodies[ NUM_TEST_BODIES ];
static TShapeBaseClass*             s_apTestShapes[ NUM_TEST_BODIES ];
static TPhysicsConstraintContainer* s_pBallConstraint;  // the anchor and body 1
static TPhysicsConstraintContainer* s_pHingeConstraint; // the anchor and body 3
static TFLOAT                       s_fResetTimer;

// a_rOut.xyz += a_rcIn.xyz
static void AddXYZ( TVector4& a_rOut, const TVector4& a_rcIn )
{
	a_rOut.x = a_rcIn.x + a_rOut.x;
	a_rOut.y = a_rcIn.y + a_rOut.y;
	a_rOut.z = a_rcIn.z + a_rOut.z;
}

// $deBlob: FUNCTION 005aa480
ACollisionTest::ACollisionTest()
{
}

// $deBlob: FUNCTION 005aa4d0
// $deBlob: FUNCTION 005abf50
ACollisionTest::~ACollisionTest()
{
}

// $deBlob: FUNCTION 005aa530
void ACollisionTest::CreateTestScene()
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	s_pBallConstraint  = TNULL;
	s_pHingeConstraint = TNULL;

	// The anchor: a tiny fixed sphere
	s_mTestShape.Identity();
	s_pAnchorBody          = pPhysicsManager->CreateBody( &s_mTestShape, TNULL, TPhysicsManager::MotionType_Static, TPhysicsManager::SubType_Default );
	s_pAnchorBody->m_fMass = 10.0f;
	pPhysicsManager->SetCollisionLayer( s_pAnchorBody, 0 );
	pPhysicsManager->SetCollisionGroup( s_pAnchorBody, 0 );
	TPhysicsManager::SetSphere( s_pAnchorBody, 0.01f );
	pPhysicsManager->AddToSimulation( s_pAnchorBody, TFALSE );

	// A column of 20 bodies below the origin
	for ( TINT i = 0; i < NUM_TEST_BODIES; i++ )
	{
		TMatrix44& rMatrix = s_aTestMatrices[ i ];
		rMatrix.Identity();

		TVector4 vTranslation;
		vTranslation.Multiply( TVector4::VEC_NEGY, TFLOAT( i ) * 3.0f + 15.0f );
		rMatrix.SetBasisVector4( BASISVECTOR_TRANSLATION, vTranslation );

		TPhysicsObjectContainer* pBody = pPhysicsManager->CreateBody( &rMatrix, TNULL, TPhysicsManager::MotionType_Dynamic, TPhysicsManager::SubType_Default );
		s_apTestBodies[ i ]            = pBody;
		pBody->m_fMass                 = ( i == 2 ) ? 40.0f : 10.0f;

		TShapeBaseClass* pShape;

		if ( !s_bOnlySpheres )
		{
			switch ( i )
			{
				case 3:
				case 4:
				case 5:
				case 14:
				case 15:
					pShape = new TBoxData();
					break;
				case 6:
				case 7:
				case 8:
				case 16:
				case 17:
					pShape = new TCapsuleData();
					break;
				case 9:
				case 10:
				case 11:
				case 18:
				case 19:
					pShape = new TCylinderData();
					break;
				default:
					pShape = new TSphereData();
					break;
			}
		}
		else
		{
			pShape = new TSphereData();
		}

		s_apTestShapes[ i ] = pShape;

		switch ( pShape->GetType() )
		{
			case TShapeBaseClass::Type_Box:
			{
				TBoxData* pBox = TSTATICCAST( TBoxData, pShape );
				pBox->SetHalfExtents( TVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
				TPhysicsManager::SetBox( s_apTestBodies[ i ], pBox->GetHalfExtents() );
				break;
			}
			case TShapeBaseClass::Type_Capsule:
			{
				TCapsuleData* pCapsule = TSTATICCAST( TCapsuleData, pShape );
				pCapsule->SetRadius( 1.0f );
				pCapsule->SetHeight( 2.0f );
				TPhysicsManager::SetCapsule( s_apTestBodies[ i ], 2.0f, 1.0f );
				break;
			}
			case TShapeBaseClass::Type_Cylinder:
			{
				TCylinderData* pCylinder = TSTATICCAST( TCylinderData, pShape );
				pCylinder->SetRadius( 1.0f );
				pCylinder->SetHeight( 0.5f );
				TPhysicsManager::SetCylinder( s_apTestBodies[ i ], 0.5f, 1.0f );
				break;
			}
			default:
			{
				TSphereData* pSphere = TSTATICCAST( TSphereData, pShape );
				pSphere->SetRadius( 1.0f );

				if ( i == 2 )
				{
					// Body 2 is a list of a sphere, a box, a capsule and a cylinder along x and z, its centre of mass in
					// the middle of the four
					TVector4  vCentreOfMass( 0.0f, 0.0f, 0.0f, 1.0f );
					TMatrix44 mShape;
					mShape.Identity();

					TPhysicsManager::AddListSphere( s_apTestBodies[ 2 ], mShape, pSphere->GetRadius() );
					AddXYZ( vCentreOfMass, mShape.GetTranslation() );

					TVector4 vOffset;
					vOffset.Multiply( TVector4::VEC_POSX, pSphere->GetRadius() + 1.0f );
					mShape.SetBasisVector4( BASISVECTOR_TRANSLATION, vOffset );
					TPhysicsManager::AddListBox( s_apTestBodies[ 2 ], mShape, TVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
					AddXYZ( vCentreOfMass, mShape.GetTranslation() );

					vOffset.Multiply( TVector4::VEC_POSZ, pSphere->GetRadius() + 2.0f );
					mShape.SetBasisVector4( BASISVECTOR_TRANSLATION, vOffset );
					TPhysicsManager::AddListCapsule( s_apTestBodies[ 2 ], mShape, 2.0f, 1.0f );
					AddXYZ( vCentreOfMass, mShape.GetTranslation() );

					vOffset.Multiply( TVector4::VEC_POSZ, 3.0f );
					TVector4 vPrev = mShape.GetTranslation();
					mShape.SetBasisVector4( BASISVECTOR_TRANSLATION, TVector4( vPrev.x + vOffset.x, vPrev.y + vOffset.y, vPrev.z + vOffset.z, vPrev.w ) );
					TPhysicsManager::AddListCylinder( s_apTestBodies[ 2 ], mShape, 2.0f, 1.0f );
					AddXYZ( vCentreOfMass, mShape.GetTranslation() );

					vCentreOfMass.Multiply( 0.25f );
					TPhysicsManager::SetCenterOfMass( s_apTestBodies[ 2 ], vCentreOfMass );
				}
				else
				{
					TPhysicsManager::SetSphere( s_apTestBodies[ i ], 1.0f );
				}

				break;
			}
		}

		TPhysicsObjectContainer* pTestBody = s_apTestBodies[ i ];

		if ( pTestBody != TNULL )
		{
			// NOTE: retail writes Havok 5.5's hkpRigidBody damping fields (+0x184, +0x188)
			pTestBody->m_fFriction      = 0.5f;
			pTestBody->m_fRestitution   = 0.1f;
			pTestBody->m_fLinearDamping = 0.1f;

			if ( pTestBody->m_body != TNULL )
				pTestBody->m_body->setLinearDamping( 0.1f );

			pTestBody->m_fAngularDamping = 0.2f;

			if ( pTestBody->m_body != TNULL )
				pTestBody->m_body->setAngularDamping( 0.2f );
		}

		TASSERT( pTestBody );
		pTestBody->m_flags &= ~0x10;
		pTestBody->m_fMaxLinearVelocity = 200.0f;

		// NOTE: retail sets Havok 5.5's hkUFloat8 at +0x18c
		if ( pTestBody->m_body != TNULL )
			pTestBody->m_body->setMaxLinearVelocity( 200.0f );

		pTestBody = s_apTestBodies[ i ];
		TASSERT( pTestBody );
		pTestBody->m_fMaxAngularVelocity = 200.0f;

		// NOTE: retail sets Havok 5.5's hkUFloat8 at +0x18d
		if ( pTestBody->m_body != TNULL )
			pTestBody->m_body->setMaxAngularVelocity( 200.0f );

		pPhysicsManager->SetCollisionLayer( s_apTestBodies[ i ], 64 );
		pPhysicsManager->SetCollisionGroup( s_apTestBodies[ i ], 199 );
		pPhysicsManager->AddToSimulation( s_apTestBodies[ i ], TFALSE );

		pTestBody = s_apTestBodies[ i ];

		if ( pTestBody != TNULL && pTestBody->IsInSimulation() )
		{
			if ( pTestBody->m_eMotionType == 0 )
				pTestBody->m_body->activate();
			else
				pTestBody->m_body->deactivate();
		}
	}

	// A ball and socket between the anchor and body 1 (at body 2's position, 0.25 along x and one below)
	s_pBallConstraint = TNULL;
	s_pBallConstraint = pPhysicsManager->CreateConstraint( s_pAnchorBody, s_apTestBodies[ 1 ], TFALSE );

	TVector4 vOffset;
	vOffset.Multiply( TVector4::VEC_POSX, 0.25f );

	const TVector4& rcBody2 = s_aTestMatrices[ 2 ].GetTranslation();
	s_vBallPivot            = TVector4( vOffset.x + ( rcBody2.x + TVector4::VEC_NEGY.x ), vOffset.y + ( rcBody2.y + TVector4::VEC_NEGY.y ), vOffset.z + ( rcBody2.z + TVector4::VEC_NEGY.z ), rcBody2.w );

	if ( !s_pBallConstraint->m_bInWorld )
	{
		s_pBallConstraint->m_eType   = TPhysicsConstraintContainer::TYPE_BALLANDSOCKET;
		s_pBallConstraint->m_vPivotA = s_vBallPivot;
	}

	pPhysicsManager->AddConstraint( s_pBallConstraint );

	// A hinge along x between the anchor and body 3 (one below it)
	s_pHingeConstraint = TNULL;
	s_pHingeConstraint = pPhysicsManager->CreateConstraint( s_pAnchorBody, s_apTestBodies[ 3 ], TFALSE );

	const TVector4& rcBody3 = s_aTestMatrices[ 3 ].GetTranslation();
	s_vHingePivot           = TVector4( rcBody3.x + TVector4::VEC_NEGY.x, rcBody3.y + TVector4::VEC_NEGY.y, rcBody3.z + TVector4::VEC_NEGY.z, rcBody3.w );

	TVector4 vHalfAxis;
	vHalfAxis.Multiply( TVector4::VEC_POSX, 0.5f );
	TVector4 vAxisEnd( s_vHingePivot.x + vHalfAxis.x, s_vHingePivot.y + vHalfAxis.y, s_vHingePivot.z + vHalfAxis.z, s_vHingePivot.w );

	vHalfAxis.Multiply( TVector4::VEC_POSX, 0.5f );
	TVector4 vAxisStart( s_vHingePivot.x - vHalfAxis.x, s_vHingePivot.y - vHalfAxis.y, s_vHingePivot.z - vHalfAxis.z, s_vHingePivot.w );

	TPhysicsManager::SetConstraintHinge( s_pHingeConstraint, vAxisStart, vAxisEnd );
	pPhysicsManager->AddConstraint( s_pHingeConstraint );
}

// $deBlob: FUNCTION 005aafd0
void ACollisionTest::DestroyTestScene()
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	pPhysicsManager->DestroyConstraint( s_pBallConstraint );
	pPhysicsManager->DestroyConstraint( s_pHingeConstraint );

	for ( TINT i = 0; i < NUM_TEST_BODIES; i++ )
	{
		pPhysicsManager->DestroyBody( s_apTestBodies[ i ] );
		s_apTestBodies[ i ] = TNULL;

		if ( s_apTestShapes[ i ] != TNULL )
			delete s_apTestShapes[ i ];
	}

	pPhysicsManager->DestroyBody( s_pAnchorBody );
}

// $deBlob: FUNCTION 005ab040
void ACollisionTest::UpdateTestScene( TFLOAT a_fDeltaTime )
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	s_fResetTimer -= a_fDeltaTime;
	s_fResetTimer = TMath::Max( s_fResetTimer, 0.0f );

	// The controller's context button 3 puts the bodies back (at most once a second)
	TInputDXDeviceController* pController = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDXDeviceController>( 0 );

	if ( pController != TNULL && pController->WasDown( TInputDeviceController::BUTTON_CONTEXT_3 ) && s_fResetTimer <= 0.0f )
	{
		s_bResetScene = TTRUE;
		s_fResetTimer = 1.0f;
	}

	TVector4 vBlobPosition( 0.0f, 0.0f, 0.0f, 1.0f );

	if ( APlayerManager::IsSingletonCreated() )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

		if ( pBlob != TNULL )
			vBlobPosition = pBlob->GetBodyPosition();
	}

	if ( !s_bResetScene )
		return;

	// The column of bodies from Blob down
	for ( TINT i = 0; i < NUM_TEST_BODIES; i++ )
	{
		s_aTestMatrices[ i ].Identity();

		TVector4 vOffset;
		vOffset.Multiply( TVector4::VEC_NEGY, TFLOAT( i ) * 3.0f );
		s_aTestMatrices[ i ].SetBasisVector4( BASISVECTOR_TRANSLATION, TVector4( vOffset.x + vBlobPosition.x, vBlobPosition.y + vOffset.y, vOffset.z + vBlobPosition.z, vBlobPosition.w ) );

		TPhysicsManager::UpdateBodyTransform( s_apTestBodies[ i ], TFALSE );
		TPhysicsManager::SetLinearVelocity( s_apTestBodies[ i ], TVector4::VEC_ZERO );
		TPhysicsManager::SetAngularVelocity( s_apTestBodies[ i ], TVector4::VEC_ZERO );

		if ( s_apTestBodies[ i ] != TNULL && s_apTestBodies[ i ]->IsInSimulation() )
			s_apTestBodies[ i ]->m_body->activate();
	}

	s_mTestShape = s_aTestMatrices[ 4 ];

	TPhysicsManager::UpdateBodyTransform( s_pAnchorBody, TFALSE );
	TPhysicsManager::SetLinearVelocity( s_pAnchorBody, TVector4::VEC_ZERO );
	TPhysicsManager::SetAngularVelocity( s_pAnchorBody, TVector4::VEC_ZERO );

	if ( s_pAnchorBody != TNULL && s_pAnchorBody->IsInSimulation() )
	{
		if ( s_pAnchorBody->m_eMotionType == 0 )
			s_pAnchorBody->m_body->activate();
		else
			s_pAnchorBody->m_body->deactivate();
	}

	// The constraints again
	pPhysicsManager->RemoveConstraint( s_pBallConstraint );

	TVector4 vOffset;
	vOffset.Multiply( TVector4::VEC_POSX, 0.25f );

	const TVector4& rcBody2 = s_aTestMatrices[ 2 ].GetTranslation();
	s_vBallPivot            = TVector4( vOffset.x + rcBody2.x + TVector4::VEC_NEGY.x, vOffset.y + rcBody2.y + TVector4::VEC_NEGY.y, vOffset.z + rcBody2.z + TVector4::VEC_NEGY.z, rcBody2.w );

	if ( !s_pBallConstraint->m_bInWorld )
	{
		s_pBallConstraint->m_eType   = TPhysicsConstraintContainer::TYPE_BALLANDSOCKET;
		s_pBallConstraint->m_vPivotA = s_vBallPivot;
	}

	pPhysicsManager->AddConstraint( s_pBallConstraint );
	pPhysicsManager->RemoveConstraint( s_pHingeConstraint );

	vOffset.Multiply( TVector4::VEC_POSZ, 0.5f );

	const TVector4& rcBody3 = s_aTestMatrices[ 3 ].GetTranslation();
	s_vHingePivot           = TVector4( vOffset.x + rcBody3.x + TVector4::VEC_NEGY.x, vOffset.y + rcBody3.y + TVector4::VEC_NEGY.y, vOffset.z + rcBody3.z + TVector4::VEC_NEGY.z, rcBody3.w );

	TVector4 vHalfAxis;
	vHalfAxis.Multiply( TVector4::VEC_POSX, 0.5f );
	TVector4 vAxisEnd( s_vHingePivot.x + vHalfAxis.x, s_vHingePivot.y + vHalfAxis.y, s_vHingePivot.z + vHalfAxis.z, s_vHingePivot.w );

	vHalfAxis.Multiply( TVector4::VEC_POSX, 0.5f );
	TVector4 vAxisStart( s_vHingePivot.x - vHalfAxis.x, s_vHingePivot.y - vHalfAxis.y, s_vHingePivot.z - vHalfAxis.z, s_vHingePivot.w );

	TPhysicsManager::SetConstraintHinge( s_pHingeConstraint, vAxisStart, vAxisEnd );
	pPhysicsManager->AddConstraint( s_pHingeConstraint );

	s_bResetScene = TFALSE;
}

// $deBlob: FUNCTION 005ab590
void ACollisionTest::RunTimingTests()
{
	if ( !ms_bRunTimingTests )
		return;

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	// A 20 m ray straight up from Blob's body
	TPhysicsRay oRay;
	oRay.m_vFrom = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() )->GetBodyPosition();

	TVector4 vUp;
	vUp.Multiply( TVector4::VEC_POSY, 20.0f );
	oRay.m_vTo              = TVector4( vUp.x + oRay.m_vFrom.x, vUp.y + oRay.m_vFrom.y, vUp.z + oRay.m_vFrom.z, oRay.m_vFrom.w );
	oRay.m_uiCollisionLayer = 0x81;

	TPhysicsShapeContact oContact;

	THPTimer oTimer;
	oTimer.Reset();

	oTimer.Update();
	for ( TINT i = 100; i != 0; i-- )
		pPhysicsManager->CollideRay( oRay, oContact, TNULL );
	oTimer.Update();

	oRay.m_uiCollisionLayer = 8;
	TFLOAT fStaticGeoTime   = oTimer.GetDelta() * 1000.0f;

	oTimer.Update();
	for ( TINT i = 100; i != 0; i-- )
		pPhysicsManager->CollideRay( oRay, oContact, TNULL );
	oTimer.Update();

	TDebug_FinalPrintf( "ACollisionTest: StaticGeo Ray Test (%d) (%.2f ms) DecalGeo Ray Test (%d) (%.2f ms)\n", 100, fStaticGeoTime, 100, oTimer.GetDelta() * 1000.0f );

	// The same through an AABB phantom
	TPhysicsPhantomContainer* pAabbPhantom = pPhysicsManager->CreateAabbPhantom();
	pPhysicsManager->AddPhantomToSimulation( pAabbPhantom, oRay.m_vFrom, TNULL );

	oRay.m_uiCollisionLayer = 0x81;

	oTimer.Update();
	for ( TINT i = 100; i != 0; i-- )
		pPhysicsManager->CollideRay( oRay, oContact, pAabbPhantom );
	oTimer.Update();

	oRay.m_uiCollisionLayer = 8;
	TFLOAT fHelperTime      = oTimer.GetDelta() * 1000.0f;

	oTimer.Update();
	for ( TINT i = 100; i != 0; i-- )
		pPhysicsManager->CollideRay( oRay, oContact, pAabbPhantom );
	oTimer.Update();

	TDebug_FinalPrintf( "ACollisionTest: Helper Style Ray Test (%d) (%.2f ms) (%d) (%.2f ms)\n", 100, fHelperTime, 100, oTimer.GetDelta() * 1000.0f );
	pPhysicsManager->DestroyPhantom( pAabbPhantom );

	// A sphere of radius 0.5 swept along the ray
	TSphereData oSphere;
	oSphere.SetRadius( 0.5f );

	TMatrix44 mFrom = TMatrix44::IDENTITY;
	mFrom.SetTranslation( oRay.m_vFrom );
	TVector4 vTo = oRay.m_vTo;

	oTimer.Update();
	for ( TINT i = 100; i != 0; i-- )
		pPhysicsManager->SweepShapeClosest( &oSphere, mFrom, vTo, 0x81, oContact, TNULL );
	oTimer.Update();

	TDebug_FinalPrintf( "ACollisionTest: Swept Shape Test (%d) (%.2f ms)\n", 100, oTimer.GetDelta() * 1000.0f );

	// The same through a shape phantom
	TPhysicsPhantomContainer* pShapePhantom = pPhysicsManager->CreateShapePhantom( &oSphere );
	pPhysicsManager->AddPhantomToSimulation( pShapePhantom, oRay.m_vFrom, TNULL );

	oTimer.Update();
	for ( TINT i = 100; i != 0; i-- )
		pPhysicsManager->SweepShapeClosest( &oSphere, mFrom, vTo, 0x81, oContact, pShapePhantom );
	oTimer.Update();

	TDebug_FinalPrintf( "ACollisionTest: Helper Style Swept Shape Test (%d) (%.2f ms)\n", 100, oTimer.GetDelta() * 1000.0f );
	pPhysicsManager->DestroyPhantom( pShapePhantom );

	ms_bRunTimingTests = TFALSE;
}

// $deBlob: FUNCTION 005aba10
const TCHAR* ACollisionTest::GetSurfaceAboveBlob()
{
	const TCHAR* szSurface = ACollisionCommon::GetSurfaceName( ACollisionCommon::SURFACE_None );

	if ( ms_bTestSurface )
	{
		TPhysicsRay oRay;
		oRay.m_vFrom = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() )->GetBodyPosition();

		TVector4 vUp;
		vUp.Multiply( TVector4::VEC_POSY, 20.0f );
		oRay.m_vTo              = TVector4( vUp.x + oRay.m_vFrom.x, vUp.y + oRay.m_vFrom.y, vUp.z + oRay.m_vFrom.z, oRay.m_vFrom.w );
		oRay.m_uiCollisionLayer = 0xA1;

		TPhysicsShapeContact oContact;

		if ( TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, TNULL ) )
			return ACollisionCommon::GetSurfaceName( oContact.m_uiMaterial );
	}

	return szSurface;
}

// $deBlob: FUNCTION 005abae0
void ACollisionTest::RunTimerTest()
{
	if ( !ms_bRunTimerTest )
		return;

	THPTimer oTimer;
	oTimer.Reset();

	oTimer.Update();
	oTimer.Update();
	oTimer.Update();
	oTimer.Update();
	oTimer.Update();
	oTimer.Update();
	oTimer.Update();
	oTimer.Update();

	ms_bRunTimerTest = TFALSE;
}

// NOTE: retail's compiler hoisted the Sin/Cos loops (one call each) and removed the "unoptimised" ones
// $deBlob: FUNCTION 005abbb0
void ACollisionTest::RunMathTests()
{
	if ( !ms_bRunMathTests )
		return;

	TFLOAT fResult = 0.0f;

	THPTimer oTimer;
	oTimer.Reset();

	oTimer.Update();
	for ( TINT i = 0; i < 1000000; i++ )
		fResult = TMath::Sin( -3.0f * TMath::PI );
	oTimer.Update();
	TDebug_FinalPrintf( "Optimised Sin()      %f seconds\n", oTimer.GetDelta() );

	oTimer.Update();
	oTimer.Update();
	TDebug_FinalPrintf( "UnOptimised Sin()    %f seconds\n", oTimer.GetDelta() );

	oTimer.Update();
	for ( TINT i = 0; i < 1000000; i++ )
		fResult = TMath::Cos( -3.0f * TMath::PI );
	oTimer.Update();
	TDebug_FinalPrintf( "Optimised Cos()      %f seconds\n", oTimer.GetDelta() );

	oTimer.Update();
	oTimer.Update();
	TDebug_FinalPrintf( "UnOptimised Cos()    %f seconds\n", oTimer.GetDelta() );

	oTimer.Update();
	for ( TINT i = 0; i < 1000000; i++ )
		TMath::SinCos( -3.0f * TMath::PI, fResult, fResult );
	oTimer.Update();
	TDebug_FinalPrintf( "Optimised SinCos()   %f seconds\n", oTimer.GetDelta() );

	oTimer.Update();
	oTimer.Update();
	TDebug_FinalPrintf( "UnOptimised SinCos() %f seconds\n", oTimer.GetDelta() );

	ms_bRunMathTests = TFALSE;
}
