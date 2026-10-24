#pragma once
#include "Math/TMatrix44.h"
#include "Render/TMaterial.h"
#include "Render/TMesh.h"
#include "Render/TAnimation.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// de Blob: 0x9C bytes (TOrderTable::CreateStaticData allocates them with TMemalign, no constructor runs).
// NOTE: only the next packet, mesh, model view matrix and material (+0x50) are verified for de Blob; the rest are
// Barnyard's fields until the model shaders are ported
//-----------------------------------------------------------------------------
class TRenderPacket
{
public:
	TRenderPacket();
	~TRenderPacket();

	TRenderPacket* GetNextPacket() { return m_pNextPacket; }
	void           SetNextPacket( TRenderPacket* a_pNextPacket ) { m_pNextPacket = a_pNextPacket; }

	TMesh* GetMesh() const { return m_pMesh; }
	// $deBlob: FUNCTION 006ae8c0
	void   SetMesh( TMesh* a_pMesh ) { m_pMesh = a_pMesh; }

	TSkeletonInstance* GetSkeletonInstance() const { return m_pSkeletonInstance; }
	void               SetSkeletonInstance( TSkeletonInstance* a_pSkeletonInstance ) { m_pSkeletonInstance = a_pSkeletonInstance; }

	TMaterial* GetMaterial() const { return m_pMaterial; }
	void       SetMaterial( TMaterial* pMaterial ) { m_pMaterial = pMaterial; }

	TMatrix44& GetModelViewMatrix() { return m_oModelView; }
	// $deBlob: FUNCTION 006ae8d0
	// $Barnyard: FUNCTION 006d58c0
	void SetModelViewMatrix( const TMatrix44& a_rMatrix44 ) { m_oModelView = a_rMatrix44; }

	void            SetAmbientColour( const TVector3& a_rColour ) { m_oAmbientColour = a_rColour; }
	const TVector3& GetAmbientColour() const { return m_oAmbientColour; }

	void            SetLightColour( const TVector3& a_rColour ) { m_oLightColour = a_rColour; }
	const TVector3& GetLightColour() const { return m_oLightColour; }

	void            SetLightDirection( const TVector3& a_rDirection ) { m_oLightDirection = a_rDirection; }
	const TVector3& GetLightDirection() const { return m_oLightDirection; }

	TUINT8 GetShadeCoeff() const { return m_ui8ShadeCoeff; }
	void   SetShadeCoeff( TUINT8 val ) { m_ui8ShadeCoeff = val; }

	// The render context's alpha blend (x255) when the mesh was queued (ASkinMeshHAL::Render)
	TUINT8 GetFade() const { return m_ui8Fade; }
	void   SetFade( TUINT8 a_uiFade ) { m_ui8Fade = a_uiFade; }

	// NOTE: de Blob's skin meshes store the render context's shade coefficient (light texture magnitude) here
	TFLOAT GetAlpha() const { return m_fAlpha; }
	void   SetAlpha( TFLOAT val ) { m_fAlpha = val; }

	// de Blob: called by TWorldShaderHAL::Render before (a_bBefore TTRUE) and after the packet is rendered
	using t_RenderCallback = void( __stdcall* )( TRenderPacket* a_pPacket, void* a_pUserData, TBOOL a_bBefore );

	// $deBlob: FUNCTION 006ae8b0
	void SetUnk( void* a_pUnk ) { m_pUnk = a_pUnk; }
	t_RenderCallback GetRenderCallback() const { return m_fnRenderCallback; }
	void*            GetRenderUserData() const { return m_pRenderUserData; }

	void SetRenderCallback( t_RenderCallback a_fnCallback, void* a_pUserData )
	{
		m_fnRenderCallback = a_fnCallback;
		m_pRenderUserData  = a_pUserData;
	}

#ifndef TOSHI_SDK
private:
#endif
	TRenderPacket*     m_pNextPacket;       // 0x00
	void*              m_pUnk;              // 0x04
	TMesh*             m_pMesh;             // 0x08
	TMatrix44          m_oModelView;        // 0x0C
	TSkeletonInstance* m_pSkeletonInstance; // 0x4C
	TMaterial*         m_pMaterial;         // 0x50
	TVector3           m_oAmbientColour;    // 0x54
	TVector3           m_oLightColour;      // 0x60
	TVector3           m_oLightDirection;   // 0x6C
	TUINT8             m_ui8Fade;           // 0x78
	TUINT8             m_ui8ShadeCoeff;     // 0x79
	TFLOAT             m_fAlpha;            // 0x7C
	TUINT8             m_aUnk80[ 0x14 ];    // 0x80
	t_RenderCallback   m_fnRenderCallback;  // 0x94 de Blob: TWorldMesh::RenderWithCallback
	void*              m_pRenderUserData;   // 0x98
}; // 0x9C

TSTATICASSERT( sizeof( TRenderPacket ) == 0x9C );

TOSHI_NAMESPACE_END
