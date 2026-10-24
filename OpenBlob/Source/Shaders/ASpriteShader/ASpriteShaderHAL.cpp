#include "pch.h"
#include "ASpriteShaderHAL.h"
#include "ASpriteMeshHAL.h"
#include "ASpriteMaterialHAL.h"

#include <Render/TShader.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00568d00
// $deBlob: FUNCTION 00568d10
// $deBlob: FUNCTION 00568d90
// $deBlob: FUNCTION 00568e00
// $deBlob: FUNCTION 00568e10
// $deBlob: FUNCTION 0053b5e0
// $deBlob: FUNCTION 00857f20
TDEFINE_CLASS( ASpriteShaderHAL );

// $deBlob: FUNCTION 00568e20
ASpriteShaderHAL::ASpriteShaderHAL()
{
}

// $deBlob: FUNCTION 00569660
// $deBlob: FUNCTION 00568e40
ASpriteShaderHAL::~ASpriteShaderHAL()
{
}

// $deBlob: FUNCTION 00568ee0
void ASpriteShaderHAL::OnDestroy()
{
	Toshi::TShader::OnDestroy();
}

// $deBlob: FUNCTION 00568e50
void ASpriteShaderHAL::Flush()
{
}

// $deBlob: FUNCTION 00569740
void ASpriteShaderHAL::StartFlush()
{
}

// $deBlob: FUNCTION 00569690
void ASpriteShaderHAL::EndFlush()
{
}

// $deBlob: FUNCTION 00568e60
TBOOL ASpriteShaderHAL::Create()
{
	TBOOL bCreated = ASpriteShader::Create();
	if ( bCreated )
	{
		m_pLineMaterial = TSTATICCAST( ASpriteMaterial, CreateMaterial( "LineMat" ) );
		m_pFillMaterial = TSTATICCAST( ASpriteMaterial, CreateMaterial( "FillMat" ) );

		m_pLineMaterial->Create();
		m_pFillMaterial->Create();

		m_pLineMaterial->SetPrimitiveType( 1 );
		m_pLineMaterial->SetBlendMode( 1 );
		m_pFillMaterial->SetBlendMode( 1 );
	}

	return bCreated;
}

// $deBlob: FUNCTION 00568ef0
TBOOL ASpriteShaderHAL::Validate()
{
	if ( IsValidated() )
		return TTRUE;

	return Toshi::TShader::Validate();
}

// $deBlob: FUNCTION 00568f00
void ASpriteShaderHAL::Invalidate()
{
	if ( IsValidated() )
		Toshi::TShader::Invalidate();
}

// $deBlob: FUNCTION 005691f0
void ASpriteShaderHAL::Render( Toshi::TRenderPacket* a_pRenderPacket )
{
}

// $deBlob: FUNCTION 00568f10
Toshi::TMaterial* ASpriteShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	TASSERT( IsCreated() );

	Validate();

	ASpriteMaterialHAL* pMaterial = new ASpriteMaterialHAL();
	TASSERT( pMaterial->m_pShader == TNULL );
	pMaterial->m_pShader = this;
	pMaterial->SetFlags( Toshi::TMaterial::FLAGS_NO_CULL, TTRUE );

	return pMaterial;
}

// $deBlob: FUNCTION 005690e0
ASpriteMesh* ASpriteShaderHAL::CreateMeshPool( TUINT32 a_uiNumMeshes, TUINT32* a_puiStride )
{
	TASSERT( IsCreated() );

	ASpriteMeshHAL* pMeshes = TREINTERPRETCAST( ASpriteMeshHAL*, TMalloc( sizeof( ASpriteMeshHAL ) * a_uiNumMeshes ) );

	for ( TUINT32 i = 0; i < a_uiNumMeshes; i++ )
	{
		ASpriteMeshHAL* pMesh = new ( &pMeshes[ i ] ) ASpriteMeshHAL();
		TASSERT( pMesh->m_pOwnerShader == TNULL );
		pMesh->m_pOwnerShader = this;
	}

	*a_puiStride = sizeof( ASpriteMeshHAL );
	return pMeshes;
}

// $deBlob: FUNCTION 00569000
ASpriteMesh* ASpriteShaderHAL::CreateMesh( TINT a_iUnk )
{
	TASSERT( IsCreated() );

	Validate();

	ASpriteMeshHAL* pMesh = new ASpriteMeshHAL();
	TASSERT( pMesh->m_pOwnerShader == TNULL );
	pMesh->m_pOwnerShader = this;

	return pMesh;
}

// $deBlob: FUNCTION 00569200
void ASpriteShaderHAL::Unknown44( TINT a_iUnk )
{
}

// $deBlob: FUNCTION 00569210
void ASpriteShaderHAL::Unknown48( TINT a_iUnk )
{
}

// $deBlob: FUNCTION 00569220
void ASpriteShaderHAL::Unknown4C( TINT a_iUnk )
{
}

// $deBlob: FUNCTION 00569230
void ASpriteShaderHAL::Unknown50( TINT a_iUnk )
{
}

// $deBlob: FUNCTION 00569240
void ASpriteShaderHAL::Unknown54( TINT a_iUnk )
{
}

// $deBlob: FUNCTION 00569250
void ASpriteShaderHAL::Unknown5C( TINT a_iUnk1, TINT a_iUnk2 )
{
}

// $deBlob: FUNCTION 00569260
void ASpriteShaderHAL::Unknown60( TINT a_iUnk1, TINT a_iUnk2 )
{
}

// $deBlob: FUNCTION 00569270
void ASpriteShaderHAL::Unknown64( TINT a_iUnk )
{
}

// $deBlob: FUNCTION 00569280
void ASpriteShaderHAL::Unknown68( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 )
{
}

// $deBlob: FUNCTION 00569290
void ASpriteShaderHAL::Unknown6C( TINT a_iUnk )
{
}

// $deBlob: FUNCTION 005692a0
void ASpriteShaderHAL::SetMaterial( ASpriteMaterial* a_pMaterial )
{
	ASpriteShader::SetMaterial( a_pMaterial );
}

// $deBlob: FUNCTION 005692b0
void ASpriteShaderHAL::Unknown78( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4 )
{
}

// $deBlob: FUNCTION 005692d0
void ASpriteShaderHAL::UnknownA4( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4, TINT a_iUnk5 )
{
}

// $deBlob: FUNCTION 005692c0
void ASpriteShaderHAL::UnknownA8( TINT a_iUnk1, TINT a_iUnk2 )
{
}

// $deBlob: FUNCTION 005692e0
void ASpriteShaderHAL::BeginMeshGeneration()
{
	ASpriteShader::BeginMeshGeneration();
}

// $deBlob: FUNCTION 005692f0
void ASpriteShaderHAL::EndMeshGeneration()
{
	ASpriteShader::EndMeshGeneration();
}
