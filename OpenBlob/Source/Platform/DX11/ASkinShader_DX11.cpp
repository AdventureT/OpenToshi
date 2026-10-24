#include "pch.h"
#include "ASkinShader_DX11.h"
#include "AStencilShadow_DX11.h"
#include "Render/Lighting/ALightingData.h"

#include <Render/TRender.h>
#include <Render/TRenderContext.h>
#include <Render/TSkeleton.h>
#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <Platform/DX11/TWorldShader_DX11.h>

#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TVector4 ASkinShaderHAL::s_vTevReg0Scale = { 0.0f, 0.0f, 0.0f, 1.0f };
TMemoryHeap*    ASkinShaderHAL::ms_pMemHeap;
Toshi::TVector4 ASkinShaderHAL::s_vTevReg1Scale    = { 1.0f, 1.0f, 1.0f, 1.0f };
Toshi::TVector4 ASkinShaderHAL::s_vTevMode16Colour = { 1.0f, 1.0f, 1.0f, 1.0f };
Toshi::TVector4 ASkinShaderHAL::s_vTevMode16Const;
Toshi::TVector4 ASkinShaderHAL::s_vTevMode1AColour;

//-----------------------------------------------------------------------------
// The HLSL of the skin shader. The cache in Data\Shaders\Skin\ was compiled from these exact bytes (hash.txt holds
// their djb2 hashes 0x9db791d1/0xddc9700d)
//-----------------------------------------------------------------------------
static const TCHAR s_szSkinShaderVS[] =
    "\n"
    "\n"
    "// Attribute input/output\n"
    "struct VS_IN\n"
    "{\n"
    "\tfloat3 ObjPos\t: POSITION;\t\t// Object space position\n"
    "\tfloat3 Normal\t: NORMAL;\t\t// Vertex normal\n"
    "\tfloat4 Weights\t: BLENDWEIGHT;\t// Weights\n"
    "\tfloat4 MIndices\t: BLENDINDICES;\t// Matrix Indices\n"
    "\tfloat2 UV\t\t: TEXCOORD;\t\t// UV\n"
    "\tfloat4 Color\t: COLOR;\t\t// Diffuse color\n"
    "};\n"
    "\n"
    "struct VS_OUT\n"
    "{\n"
    "\tfloat4 ProjPos\t: SV_POSITION;\t// Projected space position \n"
    "\tfloat3 Normal\t: NORMAL;\n"
    "\tfloat4 Color\t: COLOR;\n"
    "\tfloat2 UV0\t\t: TEXCOORD0;\t// UV\n"
    "\tfloat2 UV1\t\t: TEXCOORD1;\t// UV\n"
    "\tfloat2 UV2\t\t: TEXCOORD2;\t// UV\n"
    "\tfloat4 ViewPos\t: TEXCOORD3;\t// View space position\n"
    "\tfloat FogFactor\t: FOG;\n"
    "};\n"
    "\n"
    "// Uniforms\n"
    "float4x4 matWVP\t\t\t: register(c0);\n"
    "float4x4 matWV\t\t\t: register(c4);\n"
    "float3x3 matNorm\t\t: register(c8);\n"
    "float4x4 texMtx0\t\t: register(c12);\n"
    "float4x4 texMtx1\t\t: register(c16);\n"
    "float4x4 texMtx2\t\t: register(c20);\n"
    "float4x4 texMtx3\t\t: register(c24);\n"
    "float4x4 texMtx6\t\t: register(c28);\n"
    "float4x4 texMtx7\t\t: register(c32);\n"
    "float4\t fogStartEnd    : register(c36);\n"
    "float4x3 bones[30]\t\t: register(c40);\n"
    "float3x3 bonesNrm[30]\t: register(c130);\n"
    "\n"
    "\n"
    "//float4 lightCol\t\t: register(c12);\n"
    "//float4 shadowCol\t: register(c13);\n"
    "//float4 one\t\t\t: register(c14);\n"
    "//float4 offsetUV\t\t: register(c15);\n"
    "//float4 colorAlpha\t: register(c16);\n"
    "\n"
    "float2 Mtx2x4Tex0Idnt(float2 uv0, float3 pos, float3 nrm, float4 color)\n"
    "{\n"
    "\treturn uv0;\n"
    "}\n"
    "\n"
    "float2 Mtx3x4Tex0Mtx2(float2 uv0, float3 pos, float3 nrm, float4 color)\n"
    "{\n"
    "\tfloat4 temp = mul(texMtx2, float4(uv0, 1.0, 1.0));\n"
    "\t//\treturn temp.xy / temp.z;\n"
    "\treturn temp.xy; // [CESM] World Why this works better?\n"
    "}\n"
    "\n"
    "float2 Mtx3x4PosMtx3(float2 uv0, float3 pos, float3 nrm, float4 color)\n"
    "{\n"
    "\tfloat4 temp = mul(texMtx3, float4(pos, 1.0));\n"
    "\t//\treturn temp.xy / temp.z;\n"
    "\treturn temp.xy; // [CESM] World Why this works better?\n"
    "}\n"
    "\n"
    "float2 Mtx3x4NrmMtx1(float2 uv0, float3 pos, float3 nrm, float4 color)\n"
    "{\n"
    "\tfloat4 temp = mul(texMtx1, float4(nrm, 1.0));\n"
    "//\treturn temp.xy / temp.z;\n"
    "\treturn temp.xy; // [CESM] World Why this works better?\n"
    "}\n"
    "\n"
    "float2 SrtgColor0Idnt(float2 uv0, float3 pos, float3 nrm, float4 color)\n"
    "{\n"
    "\treturn color.xy;\n"
    "}\n"
    "\n"
    "\n"
    "float FogFactorEmpty(float4 pos)\n"
    "{\n"
    "\treturn 0.0;\n"
    "}\n"
    "\n"
    "float FogFactorLinear(float4 viewPos)\n"
    "{\n"
    "\treturn saturate((viewPos.z - fogStartEnd.x) / (fogStartEnd.y - fogStartEnd.x));\n"
    "}\n"
    "\n"
    "float FogFactorExponentialSquared(float4 viewPos)\n"
    "{\n"
    "\tfloat exponent = saturate((viewPos.z - fogStartEnd.x) / (fogStartEnd.y - fogStartEnd.x));\n"
    "\treturn saturate(1.0 - pow(2, -8 * (exponent * exponent)));\n"
    "}\n"
    "\n"
    "// Programs\n"
    "VS_OUT main(VS_IN In)\n"
    "{\n"
    "\tVS_OUT Out;\n"
    "\n"
    "\tfloat BoneWeights[4] = (float[4])In.Weights;\n"
    "\tint BoneIndices[4] = (int[4])(In.MIndices * float4(256.0, 256.0, 256.0, 256.0) / 3);\n"
    "\tfloat3 vertex = 0;\n"
    "\tfloat3 normal = 0;\n"
    "\tfor (int i = 0; i < 4; ++i)\n"
    "\t{\n"
    "\t\tfloat4x3 BoneMatrix = bones[BoneIndices[i]];\n"
    "\t\tvertex += mul(float4(In.ObjPos, 1.0), BoneMatrix) * BoneWeights[i];\n"
    "\t\tfloat3x3 NormalMatrix = bonesNrm[BoneIndices[i]];\n"
    "\t\tnormal += mul(In.Normal, NormalMatrix) * BoneWeights[i];\n"
    "\t}\n"
    "\tOut.ProjPos = mul(matWVP, float4(vertex, 1.0));\n"
    "\tOut.Normal = normalize(mul(matNorm, normal));\n"
    "\tOut.Color.xyz = In.Color;\n"
    "\tOut.Color.a = 1.0;\n"
    "\tOut.UV0 = TexCoordGen0(In.UV, vertex, normal, In.Color);\n"
    "\tOut.UV1 = TexCoordGen1(In.UV, vertex, normal, In.Color);\n"
    "\tOut.UV2 = TexCoordGen2(In.UV, vertex, normal, In.Color);\n"
    "\tOut.ViewPos = mul(matWV, float4(vertex, 1.0));\n"
    "\tOut.FogFactor = ComputeFogFactor(Out.ViewPos);\n"
    "\n"
    "\treturn Out;\n"
    "}\n";

static const TCHAR s_szSkinShaderPS[] =
    "\n"
    "\n"
    "// Attribute input/output\n"
    "struct VS_OUT\n"
    "{\n"
    "\tfloat4 ProjPos\t: SV_POSITION;\t// Projected space position\n"
    "\tfloat3 Normal\t: NORMAL;\n"
    "\tfloat4 Color\t: COLOR;\n"
    "\tfloat2 UV0\t\t: TEXCOORD0;\t// UV\n"
    "\tfloat2 UV1\t\t: TEXCOORD1;\t// UV\n"
    "\tfloat2 UV2\t\t: TEXCOORD2;\t// UV\n"
    "\tfloat4 ViewPos\t: TEXCOORD3;\t// View space position \n"
    "\tfloat FogFactor : FOG;\n"
    "};\n"
    "\n"
    "// Uniforms\n"
    "uniform Texture2D texture0\t: register(t0);\n"
    "uniform Texture2D texture1\t: register(t1);\n"
    "uniform Texture2D texture2\t: register(t2);\n"
    "uniform Texture2D texture3\t: register(t3);\n"
    "\n"
    "uniform SamplerState sampler0 : register(s0);\n"
    "uniform SamplerState sampler1 : register(s1);\n"
    "uniform SamplerState sampler2 : register(s2);\n"
    "uniform SamplerState sampler3 : register(s3);\n"
    "\n"
    "float4 alphaTest\t\t\t: register(c0);\n"
    "float4 kColor0\t\t\t\t: register(c1);\n"
    "float4 kColor1\t\t\t\t: register(c2);\n"
    "float4 kColor2\t\t\t\t: register(c3);\n"
    "float4 kColor3\t\t\t\t: register(c4);\n"
    "float4 tevReg0\t\t\t\t: register(c5);\n"
    "float4 tevReg1\t\t\t\t: register(c6);\n"
    "float4 tevReg2\t\t\t\t: register(c7);\n"
    "\n"
    "float4 chanMatColor0\t\t: register(c8);\n"
    "float4 chanMatColor1\t\t: register(c9);\n"
    "float4 chanAmbColor0\t\t: register(c10);\n"
    "float4 chanAmbColor1\t\t: register(c11);\n"
    "\n"
    "float4 light0Pos\t\t\t: register(c12); // In view space\n"
    "float4 light0Color\t\t\t: register(c13);\n"
    "float4 light6Pos\t\t\t: register(c14); // In view space\n"
    "float4 light6Color\t\t\t: register(c15);\n"
    "float4 light7Pos\t\t\t: register(c16); // In view space\n"
    "float4 light7Color\t\t\t: register(c17);\n"
    "\n"
    "float4x4 indTexMtx1\t\t\t: register(c18);\n"
    "\n"
    "float4 fogColor\t\t\t\t: register(c22);\n"
    "\n"
    "static float4 g_tevReg0;\n"
    "static float4 g_tevReg1;\n"
    "static float4 g_tevReg2;\n"
    "static float4 g_tevPrev;\n"
    "static float4 g_ras; // result of per-vertex lighting equations\n"
    "\n"
    "static const float4 ONE = float4(1.0, 1.0, 1.0, 1.0);\n"
    "\n"
    "void PassEmpty(float4 texColor)\n"
    "{\n"
    "}\n"
    "\n"
    "void PassOverride(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - ras)*0 + ras*texturei)\n"
    "\tg_tevPrev.rgb = g_ras.rgb * texColor.rgb;\n"
    "\t// A: tevPrev.a = 0 + ((1 - ras.a)*0 + ras.a*texturei.a)\n"
    "\tg_tevPrev.a = g_ras.a * texColor.a;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassLerpColour(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - texturei)*tevReg0 + texturei*tevReg1)\n"
    "\t// A: tevPrev.a = 0 + ((1 - texturei.a)*tevReg0.a + texturei.a*tevReg1.a)\n"
    "\tg_tevPrev = (ONE - texColor) * g_tevReg0 + texColor * g_tevReg1;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassLerpColour2(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - ras)*tevReg0 + texturei*tevReg1)\n"
    "\tg_tevPrev.rgb = ((ONE.rgb - g_ras.rgb) * g_tevReg0.rgb + g_ras.rgb * g_tevReg1.rgb);\n"
    "\t// A: tevPrev.a = 0 + ((1 - ras.a)*0 + ras.a*kConst1.a)\n"
    "\tg_tevPrev.a = g_ras.a;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassLerpConst(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - tevReg2)*tevPrev + tevReg2*kColor1)\n"
    "\tg_tevPrev.rgb = 0 + ((ONE.rgb - g_tevReg2.rgb) * g_tevPrev.rgb + g_tevReg2.rgb * kColor1.rgb);\n"
    "\t// A: tevPrev.a = 0 + ((tevPrev.a > kColor1.a) ? tevReg2.a : 0)\n"
    "\tg_tevPrev.a = (g_tevPrev.a > kColor1.a) ? g_tevReg2.a : 0.0;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassLerpTexLM1(float4 texColor)\n"
    "{\n"
    "\t// C: tevReg2 = 0 + ((1 - tevPrev)*0 + tevPrev*texturei)\n"
    "\tg_tevReg2.rgb = g_tevPrev.rgb * texColor.rgb;\n"
    "\t// A: tevReg2.a = 0 + ((1 - ras.a)*0 + ras.a*texturei.a)\n"
    "\tg_tevReg2.a = g_ras.a * texColor.a;\n"
    "\n"
    "\tg_tevReg2 = clamp(g_tevReg2, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassLerpTexLM2(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - texturei)*0 + texturei*tevPrev)\n"
    "\tg_tevPrev.rgb = texColor.rgb * g_tevPrev.rgb;\n"
    "\t// A: tevPrev.a = 0 + ((1 - texturei.a)*0 + texturei.a*ras.a)\n"
    "\tg_tevPrev.a = texColor.a * g_ras.a;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "\n"
    "\t// C: tevPrev = 0 + ((1 - kColor3)*tevPrev + kColor3*tevReg2)\n"
    "\t// A: tevPrev.a = 0 + ((1 - kColor3.a)*tevPrev.a + kColor3.a*tevReg2.a)\n"
    "\tg_tevPrev = (ONE - kColor3) * g_tevPrev + kColor3 * g_tevReg2;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassLerpTex(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - kColor3)*texturei + kColor3*tevPrev)\n"
    "\tg_tevPrev.rgb = clamp((ONE.rgb - kColor3.rgb) * texColor.rgb + kColor3.rgb * g_tevPrev.rgb, 0.0, 1.0);\n"
    "\t// A: tevReg2.a = 0 + ((1 - ras.a)*0 + ras.a*texturei.a)\n"
    "\tg_tevReg2.a = clamp(g_ras.a * texColor.a, 0.0, 1.0);\n"
    "\n"
    "\t// C: tevPrev = tevPrev + ((1 - 0)*0 + 0*0)\n"
    "//\tg_tevPrev.rgb = g_tevPrev.rgb;\n"
    "\t// A: tevPrev.a = 0 + ((1 - kColor3.a)*tevReg2.a + kColor3.a*tevPrev.a)\n"
    "\tg_tevPrev.a = clamp((ONE.a - kColor3.a) * g_tevReg2.a + kColor3.a * g_tevPrev.a, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassProjectedTex(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = tevPrev + ((1 - 0)*0 + 0*0)\n"
    "//\tg_tevPrev.rgb = g_tevPrev.rgb;\n"
    "\t// A: tevPrev.a = texturei.a + ((1 - 0)*0 + 0*0)\n"
    "\tg_tevPrev.a = texColor.a;\n"
    "}\n"
    "\n"
    "void PassWriteAlpha(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = tevPrev + ((1 - 0)*0 + 0*0)\n"
    "//\tg_tevPrev.rgb = g_tevPrev.rgb;\n"
    "\t// A: tevPrev.a = texturei.a + ((1 - 0)*0 + 0*0)\n"
    "\tg_tevPrev.a = texColor.a;\n"
    "}\n"
    "\n"
    "void PassModulate(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - tevPrev)*0 + tevPrev*texturei)\n"
    "\tg_tevPrev.rgb = g_tevPrev.rgb * texColor.rgb;\n"
    "\t// A: tevPrev.a = tevPrev.a + ((1 - 0)*0 + 0*0)\n"
    "//\tg_tevPrev.a = g_tevPrev.a;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassModulateAlpha(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - tevPrev)*0 + tevPrev*texturei)\n"
    "\t// A: tevPrev.a = 0 + ((1 - tevPrevr.a)*0 + tevPrev.a*texturei.a)\n"
    "\tg_tevPrev = clamp(g_tevPrev * texColor, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassAdd(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = tevPrev + ((1 - 1)*0 + 1*texturei)\n"
    "\tg_tevPrev.rgb = g_tevPrev.rgb + texColor.rgb;\n"
    "\t// A: tevPrev.a = tevPrev.a + ((1 - 0)*0 + 0*0)\n"
    "//\tg_tevPrev.a = g_tevPrev.a;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassAddWriteAlpha(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = tevPrev + ((1 - 1)*0 + 1*texturei)\n"
    "\tg_tevPrev.rgb = g_tevPrev.rgb + texColor.rgb;\n"
    "\t// A: tevPrev.a = texturei.a + ((1 - 0)*0 + 0*0)\n"
    "\tg_tevPrev.a = texColor.a;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassAddAlpha(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = tevPrev + ((1 - 1)*0 + 1*texturei)\n"
    "\tg_tevPrev.rgb = g_tevPrev.rgb + texColor.rgb;\n"
    "\t// A: tevPrev.a = texturei.a + ((1 - 0)*0 + 0*0)\n"
    "\tg_tevPrev.a = texColor.a;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassTexAlphaBlend(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - texturei.a)*1 + texturei.a*texturei)\n"
    "\tg_tevPrev.rgb = (ONE.rgb - texColor.aaa) + texColor.aaa * texColor.rgb;\n"
    "\n"
    "\t// A: tevPrev.a = texturei.a + ((1 - 0)*0 + 0*0)\n"
    "\tg_tevPrev.a = texColor.a;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassOverrideChan2(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - ras)*0 + ras*texturei)\n"
    "\tg_tevPrev.rgb = g_ras.rgb * texColor.rgb;\n"
    "\t// A: tevPrev.a = 0 + ((1 - ras.a)*0 + ras.a*texturei.a)\n"
    "\tg_tevPrev.a = g_ras.a * texColor.a;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void PassAddAlphaLerp(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = tevPrev + ((1 - 1)*0 + 1*texturei)\n"
    "\tg_tevPrev.rgb = g_tevPrev.rgb + texColor.rgb;\n"
    "\n"
    "\t// A: tevPrev.a = 0 + ((1 - kColor3.a)*0 + kColor3.a*texturei.a)\n"
    "\tg_tevPrev.a = kColor3.a * texColor.a;\n"
    "\n"
    "\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
    "}\n"
    "\n"
    "// d + ((1.0 - c)*a + c*b)\n"
    "\n"
    "\n"
    "// Blend mode special passes\n"
    "void ExtraPassPassColor(float4 texColor)\n"
    "{\n"
    "\tg_tevPrev = g_ras;\n"
    "}\n"
    "\n"
    "void ExtraPassJelly(float4 texColor)\n"
    "{\n"
    "\t// C: tevPrev = 0 + ((1 - tevReg0)*0 + tevReg0*1)\n"
    "\tg_tevPrev.rgb = g_tevReg0.rgb;\n"
    "\n"
    "\t// A: tevPrev.a = 0 + ((1 - kConst1.a)*0 + kConst1.a*tevReg0.a)\n"
    "\tg_tevPrev.a = g_tevReg0.a;\n"
    "\n"
    "\t// No clamping!\n"
    "}\n"
    "\n"
    "\n"
    "// Lighting\n"
    "float4 LightingMatRegNoLigth(float4 chanMatColor, float4 chanAmbColor, float4 vertexColor, float4 pos, float3 normal)\n"
    "{\n"
    "\treturn chanMatColor;\n"
    "}\n"
    "\n"
    "float4 LightingMatAmbRegL6L7NoAlpha(float4 chanMatColor, float4 chanAmbColor, float4 vertexColor, float4 pos, float3 normal)\n"
    "{\n"
    "\t// LightFunc = Clamp [ Ambient + SUM( LightMask(i) * Attn(i) * DiffuseAttn(i) * Color(i) ) // Attn(i) is disabled\n"
    "\tfloat4 finalColor = chanAmbColor;\n"
    "\n"
    "\t// Accum light6\n"
    "\tfloat3 lightDir = normalize(light6Pos.xyz - pos.xyz);\n"
    "\tfinalColor += (max(0.0, dot(normal, lightDir)) * light6Color);\n"
    "\n"
    "\t// Accum light7\n"
    "\tlightDir = normalize(light7Pos.xyz - pos.xyz);\n"
    "\tfinalColor += (max(0.0, dot(normal, lightDir)) * light7Color);\n"
    "\n"
    "\t// Rasterized Color = Material x LightFunc\n"
    "\tfinalColor = chanMatColor * clamp(finalColor, 0.0, 1.0);\n"
    "\n"
    "\tfinalColor.a = chanMatColor.a;\n"
    "\n"
    "\treturn finalColor;\n"
    "}\n"
    "\n"
    "float4 LightingMatAmbRegL0AlphaNoLight(float4 chanMatColor, float4 chanAmbColor, float4 vertexColor, float4 pos, float3 normal)\n"
    "{\n"
    "\t// LightFunc = Clamp [ Ambient + SUM( LightMask(i) * Attn(i) * DiffuseAttn(i) * Color(i) ) // Attn(i) is disabled\n"
    "\tfloat4 finalColor = chanAmbColor;\n"
    "\n"
    "\t// Accum light0\n"
    "\tfloat3 lightDir = normalize(light0Pos.xyz - pos.xyz);\n"
    "\tfinalColor.rgb += (max(0.0, dot(normal, lightDir)) * light0Color.rgb); // Alpha no modified\n"
    "\n"
    "\t// Rasterized Color = Material x LightFunc\n"
    "\tfinalColor = chanMatColor * clamp(finalColor, 0.0, 1.0);\n"
    "\n"
    "\treturn finalColor;\n"
    "}\n"
    "\n"
    "float4 LightingMatVtxNoLight(float4 chanMatColor, float4 chanAmbColor, float4 vertexColor, float4 pos, float3 normal)\n"
    "{\n"
    "\treturn vertexColor;\n"
    "}\n"
    "\n"
    "\n"
    "void TevColor0A0(float4 chanColor0, float4 chanColor1)\n"
    "{\n"
    "\tg_ras = chanColor0;\n"
    "}\n"
    "\n"
    "void TevColor1A1(float4 chanColor0, float4 chanColor1)\n"
    "{\n"
    "\tg_ras = chanColor1;\n"
    "}\n"
    "\n"
    "float4 TevTexture0(float2 uv)\n"
    "{\n"
    "\treturn texture0.Sample(sampler0, uv);\n"
    "}\n"
    "\n"
    "float4 TevTexture1(float2 uv)\n"
    "{\n"
    "\treturn texture1.Sample(sampler1, uv);\n"
    "}\n"
    "\n"
    "float4 TevTexture2(float2 uv)\n"
    "{\n"
    "\treturn texture2.Sample(sampler2, uv);\n"
    "}\n"
    "\n"
    "float2 TevTexCoord0(float2 uv0, float2 uv1, float2 uv2)\n"
    "{\n"
    "\treturn uv0;\n"
    "}\n"
    "\n"
    "float2 TevTexCoord1(float2 uv0, float2 uv1, float2 uv2)\n"
    "{\n"
    "\treturn uv1;\n"
    "}\n"
    "\n"
    "float2 TevTexCoord2(float2 uv0, float2 uv1, float2 uv2)\n"
    "{\n"
    "\treturn uv2;\n"
    "}\n"
    "\n"
    "float2 TevTexCoordSrtg(float2 uv0, float2 uv1, float2 uv2)\n"
    "{\n"
    "\treturn g_ras.rg;\n"
    "}\n"
    "\n"
    "\n"
    "void FogEmpty(float factor)\n"
    "{\n"
    "}\n"
    "\n"
    "void FogLinear(float factor)\n"
    "{\n"
    "\tg_tevPrev.rgb = (1.0 - factor) * g_tevPrev.rgb + factor * fogColor.rgb;\n"
    "}\n"
    "\n"
    "\n"
    "// Programs\n"
    "float4 main(VS_OUT In) : SV_TARGET\n"
    "{\n"
    "\tg_tevReg0 = tevReg0;\n"
    "\tg_tevReg1 = tevReg1;\n"
    "\tg_tevReg2 = tevReg2;\n"
    "\n"
    "\tfloat4 chanColor0 = Lighting0(chanMatColor0, chanAmbColor0, In.Color, In.ViewPos, In.Normal);\n"
    "\tfloat4 chanColor1 = Lighting1(chanMatColor1, chanAmbColor1, In.Color, In.ViewPos, In.Normal);\n"
    "\n"
    "\tTevColorPass0(chanColor0, chanColor1);\n"
    "\tPass0(TevTexturePass0(TevTexCoordPass0(In.UV0, In.UV1, In.UV2)));\n"
    "\n"
    "\tTevColorPass1(chanColor0, chanColor1);\n"
    "\tPass1(TevTexturePass1(TevTexCoordPass1(In.UV0, In.UV1, In.UV2)));\n"
    "\n"
    "\tTevColorPass2(chanColor0, chanColor1);\n"
    "\tPass2(TevTexturePass2(TevTexCoordPass2(In.UV0, In.UV1, In.UV2)));\n"
    "\n"
    "//\tPass3(tex2D(texture3, In.UV0));\n"
    "\n"
    "\n"
    "\tif (alphaTest.x > 0 && g_tevPrev.a < alphaTest.y) // Here always GX_GEQUAL was used\n"
    "\t\tdiscard;\n"
    "\n"
    "\tComputeFog(In.FogFactor);\n"
    "\n"
    "\treturn g_tevPrev;\n"
    "}\n";

//-----------------------------------------------------------------------------
// Shader permutation tables. A table gives the permutation flag of each choice; the HLSL macro of the chosen
// entry is passed to the compiler. The flags of the vertex shader are VertexShaderId::uiFlags, the ones of the
// pixel shader PixelShaderId::uiFlags.
//-----------------------------------------------------------------------------

// Vertex shader: the texture coordinate generation of the three texture maps
static const TUINT32 s_aTexCoordGenFlags[ 3 ][ 5 ] = {
	{ 0x0001, 0x0002, 0x0004, 0x0008, 0x0010 },
	{ 0x0020, 0x0040, 0x0080, 0x0100, 0x0200 },
	{ 0x0400, 0x0800, 0x1000, 0x2000, 0x4000 },
};

static const TCHAR* s_aszTexCoordGen[ 5 ] = { "Mtx2x4Tex0Idnt", "Mtx3x4Tex0Mtx2", "Mtx3x4PosMtx3", "Mtx3x4NrmMtx1", "SrtgColor0Idnt" };

static TCONSTEXPR_VAR TUINT32 VSFLAG_FOG = 0x8000;

// Pixel shader: the pass (TEV stage) modes, indexed by ASkinMaterialHAL's tev mode
static const TCHAR* s_aszPassModes[ 27 ] = {
	"PassOverride", "PassModulate", "PassAdd", "PassSustractive", "PassBlend", "PassAddHalf", "PassShadow",
	"PassProjectedTex", "PassWindows", "PassWindows2", "PassAddWriteAlpha", "PassWriteAlpha", "PassAddAlpha",
	"PassSustractiveAlpha", "PassModulateAlpha", "PassTexAlphaBlend", "PassOverrideZeroAlpha", "PassOverrideChan2",
	"PassLerpColour", "PassLerpColour2", "PassWindowsProj", "PassBlendWriteAlpha", "PassLerpConst", "PassLerpTex",
	"PassLerpTexLM1", "PassLerpTexLM2", "PassAddAlphaLerp"
};

// The colour register of each stage
static const TUINT32 s_aTevColorFlags[ 3 ][ 2 ] = { { 0, 0x1 }, { 0, 0x2 }, { 0, 0x4 } };
static const TCHAR*  s_aszTevColor[ 2 ]         = { "TevColor0A0", "TevColor1A1" };

// The texture map of each stage
static const TUINT32 s_aTevTextureFlags[ 3 ][ 3 ] = { { 0, 0x8, 0x10 }, { 0, 0x20, 0x40 }, { 0, 0x80, 0x100 } };
static const TCHAR*  s_aszTevTexture[ 3 ]         = { "TevTexture0", "TevTexture1", "TevTexture2" };

// The texture coordinates of each stage
static const TUINT32 s_aTevTexCoordFlags[ 3 ][ 4 ] = {
	{ 0, 0x00200, 0x00400, 0x00800 },
	{ 0, 0x01000, 0x02000, 0x04000 },
	{ 0, 0x08000, 0x10000, 0x20000 },
};
static const TCHAR* s_aszTevTexCoord[ 4 ] = { "TevTexCoord0", "TevTexCoord1", "TevTexCoord2", "TevTexCoordSrtg" };

// A pass replacing the first one
static const TUINT32 s_aExtraPassFlags[ 3 ] = { 0, 0x40000, 0x80000 };
static const TCHAR*  s_aszExtraPass[ 3 ]    = { TNULL, "ExtraPassPassColor", "ExtraPassJelly" };

// The lighting of the two colour channels
static const TUINT32 s_aLightingFlags[ 2 ][ 4 ] = {
	{ 0, 0x100000, 0x200000, 0x400000 },
	{ 0, 0x800000, 0x1000000, 0x2000000 },
};
static const TCHAR* s_aszLighting[ 4 ] = { "LightingMatRegNoLigth", "LightingMatAmbRegL6L7NoAlpha", "LightingMatAmbRegL0AlphaNoLight", "LightingMatVtxNoLight" };

static TCONSTEXPR_VAR TUINT32 PSFLAG_FOG = 0x4000000;

// The permutation of the packet being drawn, set up by ASkinMaterialHAL::PreRender and ASkinShaderHAL::Render
static TINT s_aStageTexMap[ 3 ]   = { 0, 1, 2 }; // texture map of each stage
static TINT s_aStageTexCoord[ 3 ] = { 0, 1, 2 }; // texture coordinates of each stage
static TINT s_iLighting0;
static TINT s_iLighting1;
static TINT s_aStageTevColor[ 3 ];
static TINT s_iExtraPass;
static TINT s_aTexMapCoordGen[ 3 ];

static TBOOL s_bDebugBlobMaterials; // (never set)
static TBOOL s_bDebugUnk;           // (never set)

// Constant of Render
static const TVector4 s_vRimLightColour = { 1.0f, 0.0f, 0.0f, 1.0f };

static TUINT8 s_uiOpaqueAlphaRef       = 0x80;
static TFLOAT s_fJellyPass0DstAlpha    = 131 / 255.0f;
static TFLOAT s_fJellyColour           = 128 / 255.0f;
static TUINT8 s_uiJellyPass3Colour     = 0x83;
static TFLOAT s_fFakeOpaqueFadeLimit   = 0.99f;
static TFLOAT s_fFakeOpaqueFadeLimit2  = 0.5f;
static TFLOAT s_fProjectedTextureFOV   = 40.0f;
static TUINT8 s_bAlphaTable0DepthWrite = 1;
static TUINT8 s_bAlphaTableDepthWrite;
static TFLOAT s_fAlphaDepthBias = 0.99999f;

// The cull mode bits (2-3) of TRenderDX11's rasterizer state
static void SetCullMode( TRenderDX11* a_pRender, D3D11_CULL_MODE a_eCullMode )
{
	TUINT8& uiFlags = *TREINTERPRETCAST( TUINT8*, &a_pRender->GetCurrentRasterizer().uiFlags );
	uiFlags         = TUINT8( ( uiFlags & ~0xC ) | ( a_eCullMode << 2 ) );
}

// XMMatrixMultiply( a_rA, a_rB ): a_rA then a_rB (TMatrix44::Multiply takes them the other way round)
static TMatrix44 MultiplyMatrices( const TMatrix44& a_rA, const TMatrix44& a_rB )
{
	TMatrix44 oResult;
	oResult.Multiply( a_rB, a_rA );
	return oResult;
}

// $deBlob: FUNCTION 00565c50
// Returns the index of the entry of a_pTable (a_iMaxIndex + 1 entries) set in a_uiFlags, or the one that is 0
static TINT FindFlagIndex( TUINT32 a_uiFlags, TINT a_iMaxIndex, const TUINT32* a_pTable )
{
	TINT iZeroFlag = -1;

	for ( TINT i = 0; i <= a_iMaxIndex; i++ )
	{
		if ( a_uiFlags & a_pTable[ i ] )
			return i;

		if ( a_pTable[ i ] == 0 )
			iZeroFlag = i;
	}

	TASSERT( iZeroFlag >= 0 );
	return iZeroFlag;
}

// $deBlob: FUNCTION 00561400
// $deBlob: FUNCTION 00561410
// $deBlob: FUNCTION 00561480
// $deBlob: FUNCTION 005614e0
// $deBlob: FUNCTION 005614f0
// $deBlob: FUNCTION 0053b520
// $deBlob: FUNCTION 00857ef0
TDEFINE_CLASS( ASkinShaderHAL );

// $deBlob: FUNCTION 00564180
// $deBlob: FUNCTION 00564190
// $deBlob: FUNCTION 00564200
// $deBlob: FUNCTION 00564260
// $deBlob: FUNCTION 00564270
// $deBlob: FUNCTION 0053b4a0
// $deBlob: FUNCTION 00857ed0
TDEFINE_CLASS( ASkinMaterialHAL );

// $deBlob: FUNCTION 00565890
// $deBlob: FUNCTION 005658a0
// $deBlob: FUNCTION 00565920
// $deBlob: FUNCTION 005659a0
// $deBlob: FUNCTION 005659b0
// $deBlob: FUNCTION 0053b4e0
// $deBlob: FUNCTION 00857ee0
TDEFINE_CLASS( ASkinMeshHAL );

// $deBlob: FUNCTION 00561500
ASkinShaderHAL::ASkinShaderHAL()
{
	m_iNumFakeOpaqueFades = 0;
}

// $deBlob: FUNCTION 00561640
// $deBlob: FUNCTION 005671a0
ASkinShaderHAL::~ASkinShaderHAL()
{
	for ( TINT i = 0; i < MAX_FAKE_OPAQUE_FADES; i++ )
	{
		if ( m_aFakeOpaqueFades[ i ].pMaterial != TNULL )
			delete m_aFakeOpaqueFades[ i ].pMaterial;

		if ( m_aFakeOpaqueFades[ i ].pSourceMaterial != TNULL )
			delete m_aFakeOpaqueFades[ i ].pSourceMaterial;

		if ( m_aFakeOpaqueFades2[ i ].pMaterial != TNULL )
			delete m_aFakeOpaqueFades2[ i ].pMaterial;

		if ( m_aFakeOpaqueFades2[ i ].pSourceMaterial != TNULL )
			delete m_aFakeOpaqueFades2[ i ].pSourceMaterial;
	}
}

// $deBlob: FUNCTION 005617b0
TBOOL ASkinShaderHAL::Create()
{
	TASSERT( TFALSE == IsCreated() );

	TBOOL bResult = m_OrderTable.Create( this, -390, 0xFF );
	TASSERT( bResult );

	bResult = m_aOrderTables[ 0 ].Create( this, -450, 0xFF );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 1 ].Create( this, -445, 0xFF );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 2 ].Create( this, -440, 0xFF );
	TASSERT( bResult );

	bResult = m_aAlphaOrderTables[ 0 ].Create( this, 1000, 0xFF );
	TASSERT( bResult );
	bResult = m_aAlphaOrderTables[ 1 ].Create( this, 4100, 0xFF );
	TASSERT( bResult );
	bResult = m_aAlphaOrderTables[ 2 ].Create( this, 5310, 0xFF );
	TASSERT( bResult );

	bResult = m_UnkOrderTable.Create( this, -400, 0xFF );
	TASSERT( bResult );

	bResult = m_aJellyOrderTables[ 0 ].Create( this, -3900, 0xFF );
	bResult = m_aJellyOrderTables[ 1 ].Create( this, -3800, 0xFF ) && bResult;
	bResult = m_aJellyOrderTables[ 2 ].Create( this, -3700, 0xFF ) && bResult;
	bResult = m_aJellyOrderTables[ 3 ].Create( this, -3600, 0xFF ) && bResult;
	bResult = m_aJellyOrderTables[ 4 ].Create( this, -3500, 0xFF ) && bResult;
	bResult = m_aJellyOrderTables[ 5 ].Create( this, -3400, 0xFF ) && bResult;
	TASSERT( bResult );

	bResult = m_aStencilOrderTables[ 0 ].Create( this, 800, 0xFF );
	bResult = m_aStencilOrderTables[ 1 ].Create( this, 810, 0xFF ) && bResult;
	TASSERT( bResult );

	Validate();
	TShader::Create();

	// The materials of the fake opaque fades (the second set is drawn in the second alpha order table)
	TTextureManager* pTextureManager = TTextureManager::GetSingleton();

	for ( TINT i = 0; i < MAX_FAKE_OPAQUE_FADES; i++ )
	{
		ASkinMaterialHAL* pMaterial       = TSTATICCAST( ASkinMaterialHAL, CreateMaterial( "no_mat_fade" ) );
		m_aFakeOpaqueFades[ i ].pMaterial = pMaterial;
		pMaterial->Create( 0x11 );
		pMaterial->SetTexture( 0, pTextureManager->GetInvalidTexture() );
		pMaterial->SetTextureNum( 1 );
		m_aFakeOpaqueFades[ i ].pSourceMaterial = TNULL;

		pMaterial                          = TSTATICCAST( ASkinMaterialHAL, CreateMaterial( "no_mat_fade" ) );
		m_aFakeOpaqueFades2[ i ].pMaterial = pMaterial;
		pMaterial->Create( 0x11 );
		pMaterial->SetOrderTable( &m_aAlphaOrderTables[ 1 ], TFALSE );
		pMaterial->SetTexture( 0, pTextureManager->GetInvalidTexture() );
		pMaterial->SetTextureNum( 1 );
		m_aFakeOpaqueFades2[ i ].pSourceMaterial = TNULL;
	}

	m_ShaderCache.Create(
	    "Data\\Shaders\\Skin\\",
	    s_szSkinShaderVS,
	    s_szSkinShaderPS,
	    TShaderCache_DX11::t_CompileVS( CompileVertexShader ),
	    TShaderCache_DX11::t_CreateInputLayout( CreateInputLayout ),
	    TShaderCache_DX11::t_CompilePS( CompilePixelShader )
	);

	return TTRUE;
}

// $deBlob: FUNCTION 00561d70
void ASkinShaderHAL::OnDestroy()
{
	TASSERT( TTRUE == IsCreated() );
	TShader::OnDestroy();
}

// $deBlob: FUNCTION 00563470
TMaterial* ASkinShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	TASSERT( TTRUE == IsCreated() );

	ASkinMaterialHAL* pMaterial;

	if ( ASkinShaderHAL::ms_pMemHeap == TNULL )
		pMaterial = new ASkinMaterialHAL();
	else
		pMaterial = new ( ASkinShaderHAL::ms_pMemHeap ) ASkinMaterialHAL();

	pMaterial->SetShader( this );
	pMaterial->SetName( a_szName );

	return pMaterial;
}

// $deBlob: FUNCTION 00561db0
TMesh* ASkinShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	ASkinMeshHAL* pMesh;

	if ( ASkinShaderHAL::ms_pMemHeap == TNULL )
		pMesh = new ASkinMeshHAL();
	else
		pMesh = new ( ASkinShaderHAL::ms_pMemHeap ) ASkinMeshHAL();

	pMesh->SetOwnerShader( this );
	return pMesh;
}

// $deBlob: FUNCTION 00561e70
void ASkinShaderHAL::StartFlush()
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	pRender->SetDstAlpha( 0.0f );
	pRender->SetColorUpdate( TTRUE );
	pRender->SetAlphaUpdate( TTRUE );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );

	TVector4 vOne( 1.0f, 1.0f, 1.0f, 1.0f );
	pRender->SetVec4InPSBuffer( 8, &vOne, 1 );

	TRenderContext* pContext = pRender->GetCurrentContext();

	if ( pContext->GetFlags() & TRenderContext::FLAG_FOG )
	{
		TRenderContext* pFogContext = pRender->GetCurrentContext();
		s_fogColor.x                = pFogContext->GetFogColour().x;
		s_fogColor.y                = pFogContext->GetFogColour().y;
		s_fogColor.z                = pFogContext->GetFogColour().z;
		s_FogStartEnd.x             = pFogContext->GetFogDistanceStart();
		s_FogStartEnd.y             = pFogContext->GetFogDistanceEnd();
		s_FogStartEnd.z             = 1.0f;
		s_fogColor.w                = 1.0f;
	}

	// The sun, far away in the opposite direction of its light
	TVector4 vLightPos = ALightingData::GetSingleton()->GetCurrent()->vLightDirection;
	vLightPos.Multiply( -1e11f );
	m_LightPos.Set( vLightPos.x, vLightPos.y, vLightPos.z, 1.0f );
	TMatrix44::TransformVector( m_LightPos, pContext->GetWorldViewMatrix(), TVector4( m_LightPos ) );

	s_aStageTevColor[ 0 ] = 0;
	s_aStageTevColor[ 1 ] = 0;
	s_aStageTevColor[ 2 ] = 0;
	s_iLighting0          = 0;
	s_iLighting1          = 0;
	s_aStageTexMap[ 0 ]   = 0;
	s_aStageTexMap[ 1 ]   = 1;
	s_aStageTexMap[ 2 ]   = 2;
	s_aStageTexCoord[ 0 ] = 0;
	s_aStageTexCoord[ 1 ] = 1;
	s_aStageTexCoord[ 2 ] = 2;
}

// $deBlob: FUNCTION 005620a0
void ASkinShaderHAL::Flush()
{
}

// $deBlob: FUNCTION 005620b0
void ASkinShaderHAL::EndFlush()
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	s_FogStartEnd.z = 0.0f;
	s_NumTevStages  = 1;
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
	s_AlphaTest.x = 1.0f;
	s_AlphaTest.y = 10 / 255.0f;
	pRender->SetColorUpdate( TTRUE );
	pRender->SetAlphaUpdate( TTRUE );

	m_iNumFakeOpaqueFades = 0;
}

// $deBlob: FUNCTION 00562120
void ASkinShaderHAL::Render( TRenderPacket* a_pRenderPacket )
{
	ASkinMaterialHAL* pMaterial = ASkinMaterialHAL::Cast( a_pRenderPacket->GetMaterial() );

	if ( s_bDebugBlobMaterials )
	{
		// NOTE: a debug leftover: the results are unused
		const TCHAR* szName = pMaterial->GetName();

		if ( TStringManager::String8CompareNoCase( "ss_blob_eye_rt", szName, -1 ) != 0 &&
		     TStringManager::String8CompareNoCase( "ss_blob_eye_lf", szName, -1 ) != 0 &&
		     TStringManager::String8CompareNoCase( "ss_blob_mouth", szName, -1 ) != 0 &&
		     TStringManager::String8CompareNoCase( "ss_blob_body", szName, -1 ) != 0 )
		{
			TStringManager::String8CompareNoCase( "Blob_Shocked", szName, -1 );
		}
	}

	TRenderDX11* pRender = TRenderDX11::Interface();
	TASSERT( a_pRenderPacket->GetMesh()->IsA( &TGetClass( ASkinMeshHAL ) ) );

	ASkinMeshHAL* pMesh = TSTATICCAST( ASkinMeshHAL, a_pRenderPacket->GetMesh() );
	TINT          iFade = a_pRenderPacket->GetFade();
	TASSERT( 255 >= iFade );

	TFLOAT   fFade = iFade / 255.0f;
	TVector4 vChanAmbColor1( a_pRenderPacket->GetAmbientColour().x, a_pRenderPacket->GetAmbientColour().y, a_pRenderPacket->GetAmbientColour().z, 1.0f );
	TVector4 vLight0Colour( a_pRenderPacket->GetLightColour().x, a_pRenderPacket->GetLightColour().y, a_pRenderPacket->GetLightColour().z, 1.0f );

	if ( pMaterial->GetFlags() & ASkinMaterial::SKINFLAG_RIMLIGHT )
	{
		pRender->SetVec4InPSBuffer( 15, &s_vRimLightColour, 1 );
		pRender->SetVec4InPSBuffer( 14, &m_LightPos, 1 );
		pRender->SetVec4InPSBuffer( 17, &s_Light7Color, 1 );
		pRender->SetVec4InPSBuffer( 16, &s_Light7Pos, 1 );
		pRender->SetVec4InPSBuffer( 10, &s_ChanAmbColor0, 1 );
		pRender->SetVec4InPSBuffer( 8, &s_ChanMatColor0, 1 );
		pRender->SetVec4InPSBuffer( 13, &vLight0Colour, 1 );
		pRender->SetVec4InPSBuffer( 12, &m_LightPos, 1 );
		vChanAmbColor1.w = fFade;
		pRender->SetVec4InPSBuffer( 11, &vChanAmbColor1, 1 );
		pRender->SetVec4InPSBuffer( 9, &s_ChanMatColor0, 1 );
	}
	else
	{
		if ( pMaterial->GetTevMode( 0 ) == 0x13 )
		{
			TVector4 vTevReg0;
			vTevReg0.Multiply( vChanAmbColor1, ASkinShaderHAL::s_vTevReg0Scale );
			TVector4 vTevReg1;
			vTevReg1.Multiply( vChanAmbColor1, ASkinShaderHAL::s_vTevReg1Scale );
			pRender->SetVec4InPSBuffer( 5, &vTevReg0, 1 );
			pRender->SetVec4InPSBuffer( 6, &vTevReg1, 1 );

			TFLOAT fShade = a_pRenderPacket->GetAlpha();
			vLight0Colour.Set( fShade, fShade, fShade, 1.0f );
			vChanAmbColor1.Set( fShade, fShade, fShade, 1.0f );
		}

		vChanAmbColor1.w = 1.0f;
		vLight0Colour.w  = 1.0f;
		pRender->SetVec4InPSBuffer( 13, &vLight0Colour, 1 );
		pRender->SetVec4InPSBuffer( 12, &m_LightPos, 1 );
		vChanAmbColor1.w = fFade;
		pRender->SetVec4InPSBuffer( 10, &vChanAmbColor1, 1 );
	}

	TINT                     iPass        = pMaterial->GetPass();
	ASkinMaterial::BLENDMODE eBlendMode   = pMaterial->GetBlendMode();
	TBOOL                    bJellyPass0  = eBlendMode == ASkinMaterial::BLENDMODE_JELLY && iPass == 0;
	TBOOL                    bJellyPass4  = eBlendMode == ASkinMaterial::BLENDMODE_JELLY && iPass == 4;
	TBOOL                    bJellyPass5  = eBlendMode == ASkinMaterial::BLENDMODE_JELLY && iPass == 5;
	TBOOL                    bJellyOpaque = bJellyPass0 || bJellyPass4 || bJellyPass5;

	if ( eBlendMode == ASkinMaterial::BLENDMODE_DEFAULT || bJellyOpaque )
	{
		s_AlphaTest.x = 1.0f;

		if ( iFade < 0xFE )
		{
			s_AlphaTest.y = ( iFade * 0.5f ) / 255.0f;
		}
		else
		{
			s_AlphaTest.y = s_uiOpaqueAlphaRef / 255.0f;
			pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
		}
	}
	else
	{
		s_AlphaTest.x = 1.0f;
		s_AlphaTest.y = pMaterial->GetAlphaRef() / 255.0f;
	}

	vChanAmbColor1.w = 1.0f;

	if ( bJellyPass0 )
	{
		TASSERT( TFALSE );
		pRender->SetAlphaUpdate( TTRUE );
		pRender->SetColorUpdate( TTRUE );
		pRender->SetDstAlpha( s_fJellyPass0DstAlpha );
	}

	if ( bJellyPass4 )
	{
		TASSERT( TFALSE );
		pRender->SetAlphaUpdate( TTRUE );
		pRender->SetColorUpdate( TFALSE );
		pRender->SetDstAlpha( -1.0f );
		pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ONE );
		vChanAmbColor1.x = 1.0f;
		vChanAmbColor1.y = 1.0f;
		vChanAmbColor1.z = 1.0f;
	}
	else if ( bJellyPass5 )
	{
		TASSERT( TFALSE );
		pRender->SetColorUpdate( TTRUE );
		pRender->SetAlphaUpdate( TFALSE );
		pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA );
	}

	if ( pMaterial->GetBlendMode() == ASkinMaterial::BLENDMODE_STENCIL )
	{
		TASSERT( GetStencilOrderTable( pMaterial->GetPass() ) == pMaterial->GetOrderTable() );

		s_FogStartEnd.z = 0.0f;

		TFLOAT   fStencil = AStencilShadow::ms_uiStencilColour / 255.0f;
		TVector4 vStencilColour( fStencil, fStencil, fStencil, 0.0f );

		if ( iPass == 0 )
		{
			pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
			SetCullMode( pRender, D3D11_CULL_BACK );
			pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE );
		}
		else
		{
			TASSERT( 1 == iPass );
			pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
			SetCullMode( pRender, D3D11_CULL_FRONT );
			pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_ONE, D3D11_BLEND_ONE );
		}

		pRender->SetColorUpdate( TTRUE );
		pRender->SetAlphaUpdate( TFALSE );
		s_AlphaTest.x  = 0.0f;
		s_NumTevStages = 1;
		s_iLighting0   = 3;
		pRender->SetVec4InPSBuffer( 5, &vStencilColour, 1 );
		s_iExtraPass = 2;
	}

	if ( pMaterial->GetBlendMode() == ASkinMaterial::BLENDMODE_JELLY && !bJellyOpaque )
	{
		s_FogStartEnd.z = 0.0f;
		TASSERT( pMaterial->GetOrderTable() == GetJellyOrderTable( iPass ) );

		s_AlphaTest.x = 0.0f;
		pRender->SetDstAlpha( -1.0f );
		pRender->SetAlphaUpdate( TTRUE );

		TBOOL bSetColour = TTRUE;

		if ( iPass == 1 )
		{
			pRender->SetZMode( TTRUE, D3D11_COMPARISON_GREATER, D3D11_DEPTH_WRITE_MASK_ZERO );
			SetCullMode( pRender, D3D11_CULL_FRONT );
			pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE );
		}
		else if ( iPass == 2 )
		{
			pRender->SetZMode( TTRUE, D3D11_COMPARISON_GREATER, D3D11_DEPTH_WRITE_MASK_ZERO );
			SetCullMode( pRender, D3D11_CULL_BACK );
			pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_ONE, D3D11_BLEND_ONE );
		}
		else if ( iPass == 3 )
		{
			s_NumTevStages = 1;
			s_iLighting0   = 3;
			pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_ONE, D3D11_BLEND_ONE );
			pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
			pRender->SetColorUpdate( TFALSE );

			vChanAmbColor1.w = s_uiJellyPass3Colour / 255.0f;
			vChanAmbColor1.x = TUINT8( ( TUINT8( TINT( vChanAmbColor1.x * 255.0f ) ) >> 1 ) | s_uiJellyPass3Colour ) / 255.0f;
			vChanAmbColor1.y = TUINT8( ( TUINT8( TINT( vChanAmbColor1.y * 255.0f ) ) >> 1 ) | s_uiJellyPass3Colour ) / 255.0f;
			vChanAmbColor1.z = TUINT8( ( TUINT8( TINT( vChanAmbColor1.z * 255.0f ) ) >> 1 ) | s_uiJellyPass3Colour ) / 255.0f;
			bSetColour       = TFALSE;

			pRender->SetVec4InPSBuffer( 5, &vChanAmbColor1, 1 );
			s_iExtraPass = 2;
		}
		else
		{
			bSetColour = TFALSE;
		}

		if ( bSetColour )
		{
			pRender->SetColorUpdate( TFALSE );
			s_NumTevStages = 1;
			s_iLighting0   = 3;
			vChanAmbColor1.Set( s_fJellyColour, s_fJellyColour, s_fJellyColour, s_fJellyColour );

			pRender->SetVec4InPSBuffer( 5, &vChanAmbColor1, 1 );
			s_iExtraPass = 2;
		}
	}
	else
	{
		TMaterial::Flags eFlags = pMaterial->GetFlags();

		if ( eFlags & ASkinMaterial::SKINFLAG_RIMLIGHT )
		{
			s_iLighting0 = 1;
			s_iLighting1 = 2;
		}
		else
		{
			s_iLighting0 = ( eFlags & ASkinMaterial::SKINFLAG_UNK13 ) ? 2 : 0;
		}
	}

	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	// The vertex shader permutation
	VertexShaderId oVSId;
	oVSId.uiFlags = s_aTexCoordGenFlags[ 2 ][ s_aTexMapCoordGen[ 2 ] ] | s_aTexCoordGenFlags[ 1 ][ s_aTexMapCoordGen[ 1 ] ] | s_aTexCoordGenFlags[ 0 ][ s_aTexMapCoordGen[ 0 ] ];
	oVSId.uiTexGen23 = 0;
	oVSId.uiTexGen4  = 0;

	if ( 0.0f < s_FogStartEnd.z )
		oVSId.uiFlags |= VSFLAG_FOG;

	ID3D11VertexShader* pVertexShader = TNULL;
	ID3D11InputLayout*  pInputLayout  = TNULL;

	if ( !m_ShaderCache.FindVertexShader( oVSId, pVertexShader, pInputLayout ) )
	{
		ID3DBlob* pBlob = CompileVertexShader( oVSId );
		pRender->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pVertexShader );
		pInputLayout = CreateInputLayout( oVSId, pBlob );
		m_ShaderCache.AddVertexShader( oVSId, pVertexShader, pInputLayout, pBlob->GetBufferPointer(), pBlob->GetBufferSize() );
		pBlob->Release();
	}

	TASSERT( pInputLayout );
	pDeviceContext->IASetInputLayout( pInputLayout );
	TASSERT( pVertexShader );
	pDeviceContext->VSSetShader( pVertexShader, TNULL, 0 );

	// The pixel shader permutation
	PixelShaderId oPSId;
	oPSId.aStages[ 0 ] = -1;
	oPSId.aStages[ 1 ] = -1;
	oPSId.aStages[ 2 ] = -1;
	oPSId.aStages[ 3 ] = -1;
	oPSId.aStages[ 4 ] = -1;
	oPSId.uiFlags      = 0;

	TASSERT( s_NumTevStages < 4 );

	for ( TINT i = 0; i < s_NumTevStages; i++ )
		oPSId.aStages[ i ] = TINT8( pMaterial->GetTevMode( i ) );

	oPSId.uiFlags |= s_aExtraPassFlags[ s_iExtraPass ] | s_aLightingFlags[ 0 ][ s_iLighting0 ] | s_aLightingFlags[ 1 ][ s_iLighting1 ] |
	    s_aTevTexCoordFlags[ 2 ][ s_aStageTexCoord[ 2 ] ] | s_aTevTexCoordFlags[ 1 ][ s_aStageTexCoord[ 1 ] ] | s_aTevTexCoordFlags[ 0 ][ s_aStageTexCoord[ 0 ] ] |
	    s_aTevTextureFlags[ 2 ][ s_aStageTexMap[ 2 ] ] | s_aTevTextureFlags[ 1 ][ s_aStageTexMap[ 1 ] ] | s_aTevTextureFlags[ 0 ][ s_aStageTexMap[ 0 ] ] |
	    s_aTevColorFlags[ 2 ][ s_aStageTevColor[ 2 ] ] | s_aTevColorFlags[ 1 ][ s_aStageTevColor[ 1 ] ] | s_aTevColorFlags[ 0 ][ s_aStageTevColor[ 0 ] ];

	if ( 0.0f < s_FogStartEnd.z )
		oPSId.uiFlags |= PSFLAG_FOG;

	ID3D11PixelShader* pPixelShader = TNULL;

	if ( !m_ShaderCache.FindPixelShader( oPSId, pPixelShader ) )
	{
		ID3DBlob* pBlob = CompilePixelShader( oPSId );
		pRender->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pPixelShader );
		m_ShaderCache.AddPixelShader( oPSId, pPixelShader, pBlob->GetBufferPointer(), pBlob->GetBufferSize() );
		pBlob->Release();
	}

	TASSERT( pPixelShader );
	pDeviceContext->PSSetShader( pPixelShader, TNULL, 0 );
	pRender->SetVec4InPSBuffer( 0, &s_AlphaTest, 1 );

	if ( 0.0f < s_FogStartEnd.z )
	{
		pRender->SetVec4InVSBuffer( 36, &s_FogStartEnd, 1 );
		pRender->SetVec4InPSBuffer( 22, &s_fogColor, 1 );
	}

	TMatrix44 oModelView = a_pRenderPacket->GetModelViewMatrix();

	if ( pMaterial->GetFlags() & ASkinMaterial::SKINFLAG_ENVMAP )
	{
		// The environment map coordinates: the normals in view space, scaled into [0;1]
		TMatrix44 oInverse = a_pRenderPacket->GetModelViewMatrix();
		TVector4  vScale;
		oInverse.GetScale( vScale );

		TBOOL bRes = oInverse.Invert( oInverse );
		TASSERT( TTRUE == bRes );

		TMatrix44 oNormalMatrix;
		oNormalMatrix.Transpose( oInverse );
		oNormalMatrix.m_f14 = 0.0f;
		oNormalMatrix.m_f24 = 0.0f;
		oNormalMatrix.m_f34 = 0.0f;
		oNormalMatrix.m_f44 = 1.0f;

		TMatrix44 oScale;
		oScale.Set( vScale.x * 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, vScale.y * -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f );

		TMatrix44 oBias;
		oBias.Set( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f );

		TMatrix44 oEnvMatrix = MultiplyMatrices( MultiplyMatrices( oNormalMatrix, oScale ), oBias );
		pRender->SetVec4InVSBuffer( 16, &oEnvMatrix, 4 );
	}

	if ( s_bDebugUnk )
		TASSERT( TFALSE );

	TRenderContextDX11* pContext = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );
	TMatrix44           oWVP     = MultiplyMatrices( oModelView, pContext->GetProjectionMatrix() );
	pRender->SetVec4InVSBuffer( 0, &oWVP, 4 );
	pRender->SetVec4InVSBuffer( 4, &oModelView, 4 );

	TBOOL bRes = oModelView.Invert( oModelView );
	TASSERT( bRes );

	TMatrix44 oNormalMatrix;
	oNormalMatrix.Transpose( oModelView );
	pRender->SetVec4InVSBuffer( 8, &oNormalMatrix, 3 );

	// Draw each sub mesh with its bones
	TSkeletonInstance*           pSkeletonInstance = a_pRenderPacket->GetSkeletonInstance();
	const TSkeletonInstanceBone* pBones            = pSkeletonInstance->GetBones();
	D3D11_PRIMITIVE_TOPOLOGY     eTopology         = D3D11_PRIMITIVE_TOPOLOGY( ( ( pMesh->GetState() & 0x40 ) | 0x100 ) >> 6 );

	for ( TUINT32 i = 0; i < pMesh->GetNumSubMeshes(); i++ )
	{
		ASkinSubMesh* pSubMesh = pMesh->GetSubMesh( i );
		TASSERT( pSubMesh->GetNumBoneInfluences() <= ASkinSubMesh::MAXBONES );

		for ( TUINT32 b = 0; b < pSubMesh->GetNumBoneInfluences(); b++ )
		{
			TASSERT( pSubMesh->GetRemappedMatrix( b ) >= 0 && pSubMesh->GetRemappedMatrix( b ) < pSkeletonInstance->GetSkeleton()->GetBoneCount() );

			const TMatrix44& rBone = pBones[ pSubMesh->GetRemappedMatrix( b ) ].m_Transform;

			TMatrix44 oBone;
			oBone.Transpose( rBone );
			pRender->SetVec4InVSBuffer( 40 + b * 3, &oBone, 3 );

			bRes = oBone.Invert( rBone );
			TASSERT( bRes );
			pRender->SetVec4InVSBuffer( 130 + b * 3, &oBone, 3 );
		}

		pRender->DrawIndexed( eTopology, pSubMesh->GetNumIndices(), pSubMesh->GetIndexBuffer(), 0, DXGI_FORMAT_R16_UINT, pMesh->GetVertexPool(), ASkinMesh::VERTEX_SIZE, 0 );
	}
}

// $deBlob: FUNCTION 00563590
TOrderTable* ASkinShaderHAL::GetAlphaOrderTable( TINT a_iIndex )
{
	TASSERT( 0 <= a_iIndex );
	TASSERT( ALPHA_ORDER_TABLE_COUNT > a_iIndex );
	return &m_aAlphaOrderTables[ a_iIndex ];
}

// $deBlob: FUNCTION 00563620
TOrderTable* ASkinShaderHAL::GetOrderTableByIndex( TINT a_iIndex )
{
	if ( a_iIndex == 3 )
		return &m_OrderTable;

	if ( TUINT( a_iIndex - 4 ) < 3 )
		return GetAlphaOrderTable( a_iIndex - 4 );

	return TNULL;
}

// $deBlob: FUNCTION 00563650
void ASkinShaderHAL::ResetFakeOpaqueFades()
{
	TTexture* pInvalidTexture = TTextureManager::GetSingleton()->GetInvalidTexture();

	for ( TINT i = 0; i < MAX_FAKE_OPAQUE_FADES; i++ )
	{
		m_aFakeOpaqueFades[ i ].pMaterial->SetTexture( 0, pInvalidTexture );
		m_aFakeOpaqueFades[ i ].pMaterial->SetFakeOpaqueFadeID( -1 );
		m_aFakeOpaqueFades2[ i ].pMaterial->SetTexture( 0, pInvalidTexture );
		m_aFakeOpaqueFades2[ i ].pMaterial->SetFakeOpaqueFadeID( -1 );

		if ( m_aFakeOpaqueFades[ i ].pSourceMaterial != TNULL )
		{
			m_aFakeOpaqueFades[ i ].pSourceMaterial->SetFakeOpaqueFadeID( -1 );
			m_aFakeOpaqueFades[ i ].pSourceMaterial = TNULL;
		}

		if ( m_aFakeOpaqueFades2[ i ].pSourceMaterial != TNULL )
		{
			m_aFakeOpaqueFades2[ i ].pSourceMaterial->SetFakeOpaqueFadeID( -1 );
			m_aFakeOpaqueFades2[ i ].pSourceMaterial = TNULL;
		}
	}
}

// $deBlob: FUNCTION 005636e0
ID3DBlob* __stdcall ASkinShaderHAL::CompileVertexShader( const VertexShaderId& a_rId )
{
	TINT iTexCoordGen0 = FindFlagIndex( a_rId.uiFlags, 4, s_aTexCoordGenFlags[ 0 ] );
	TINT iTexCoordGen1 = FindFlagIndex( a_rId.uiFlags, 4, s_aTexCoordGenFlags[ 1 ] );
	TINT iTexCoordGen2 = FindFlagIndex( a_rId.uiFlags, 4, s_aTexCoordGenFlags[ 2 ] );

	D3D_SHADER_MACRO aDefines[ 5 ];
	aDefines[ 0 ].Name       = "TexCoordGen0";
	aDefines[ 0 ].Definition = s_aszTexCoordGen[ iTexCoordGen0 ];
	aDefines[ 1 ].Name       = "TexCoordGen1";
	aDefines[ 1 ].Definition = s_aszTexCoordGen[ iTexCoordGen1 ];
	aDefines[ 2 ].Name       = "TexCoordGen2";
	aDefines[ 2 ].Definition = s_aszTexCoordGen[ iTexCoordGen2 ];
	aDefines[ 3 ].Name       = "ComputeFogFactor";
	aDefines[ 3 ].Definition = ( a_rId.uiFlags & VSFLAG_FOG ) ? "FogFactorExponentialSquared" : "FogFactorEmpty";
	aDefines[ 4 ].Name       = TNULL;
	aDefines[ 4 ].Definition = TNULL;

	return TRenderDX11::CompileShader( s_szSkinShaderVS, "main", "vs_4_0_level_9_3", aDefines );
}

// $deBlob: FUNCTION 005638b0
ID3D11InputLayout* __stdcall ASkinShaderHAL::CreateInputLayout( const VertexShaderId& a_rId, ID3DBlob* a_pBlob )
{
	D3D11_INPUT_ELEMENT_DESC aElements[ 6 ] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ID3D11InputLayout* pInputLayout = TNULL;
	TRenderDX11::Interface()->GetDevice()->CreateInputLayout( aElements, 6, a_pBlob->GetBufferPointer(), a_pBlob->GetBufferSize(), &pInputLayout );

	return pInputLayout;
}

// $deBlob: FUNCTION 00563a70
ID3DBlob* __stdcall ASkinShaderHAL::CompilePixelShader( const PixelShaderId& a_rId )
{
	TUINT32 uiNumTevStages = 1;
	if ( a_rId.aStages[ 1 ] != -1 ) uiNumTevStages = 2;
	if ( a_rId.aStages[ 2 ] != -1 ) uiNumTevStages = 3;
	if ( a_rId.aStages[ 3 ] != -1 ) uiNumTevStages = 4;

	TUINT32 uiFlags       = a_rId.uiFlags;
	TINT    iLighting0    = FindFlagIndex( uiFlags, 3, s_aLightingFlags[ 0 ] );
	TINT    iLighting1    = FindFlagIndex( uiFlags, 3, s_aLightingFlags[ 1 ] );
	TINT    iTevColor0    = FindFlagIndex( uiFlags, 1, s_aTevColorFlags[ 0 ] );
	TINT    iTevColor1    = FindFlagIndex( uiFlags, 1, s_aTevColorFlags[ 1 ] );
	TINT    iTevColor2    = FindFlagIndex( uiFlags, 1, s_aTevColorFlags[ 2 ] );
	TINT    iTevTexture0  = FindFlagIndex( uiFlags, 2, s_aTevTextureFlags[ 0 ] );
	TINT    iTevTexture1  = FindFlagIndex( uiFlags, 2, s_aTevTextureFlags[ 1 ] );
	TINT    iTevTexture2  = FindFlagIndex( uiFlags, 2, s_aTevTextureFlags[ 2 ] );
	TINT    iTevTexCoord0 = FindFlagIndex( uiFlags, 3, s_aTevTexCoordFlags[ 0 ] );
	TINT    iTevTexCoord1 = FindFlagIndex( uiFlags, 3, s_aTevTexCoordFlags[ 1 ] );
	TINT    iTevTexCoord2 = FindFlagIndex( uiFlags, 3, s_aTevTexCoordFlags[ 2 ] );

	D3D_SHADER_MACRO aDefines[ 17 ];
	aDefines[ 0 ]  = { "Pass0", s_aszPassModes[ a_rId.aStages[ 0 ] ] };
	aDefines[ 1 ]  = { "Pass1", "PassEmpty" };
	aDefines[ 2 ]  = { "Pass2", "PassEmpty" };
	aDefines[ 3 ]  = { "Pass3", "PassEmpty" };
	aDefines[ 4 ]  = { "Lighting0", s_aszLighting[ iLighting0 ] };
	aDefines[ 5 ]  = { "Lighting1", s_aszLighting[ iLighting1 ] };
	aDefines[ 6 ]  = { "TevColorPass0", s_aszTevColor[ iTevColor0 ] };
	aDefines[ 7 ]  = { "TevColorPass1", s_aszTevColor[ iTevColor1 ] };
	aDefines[ 8 ]  = { "TevColorPass2", s_aszTevColor[ iTevColor2 ] };
	aDefines[ 9 ]  = { "TevTexturePass0", s_aszTevTexture[ iTevTexture0 ] };
	aDefines[ 10 ] = { "TevTexturePass1", s_aszTevTexture[ iTevTexture1 ] };
	aDefines[ 11 ] = { "TevTexturePass2", s_aszTevTexture[ iTevTexture2 ] };
	aDefines[ 12 ] = { "TevTexCoordPass0", s_aszTevTexCoord[ iTevTexCoord0 ] };
	aDefines[ 13 ] = { "TevTexCoordPass1", s_aszTevTexCoord[ iTevTexCoord1 ] };
	aDefines[ 14 ] = { "TevTexCoordPass2", s_aszTevTexCoord[ iTevTexCoord2 ] };
	aDefines[ 15 ] = { "ComputeFog", ( uiFlags & PSFLAG_FOG ) ? "FogLinear" : "FogEmpty" };
	aDefines[ 16 ] = { TNULL, TNULL };

	for ( TUINT32 i = 1; i < uiNumTevStages; i++ )
		aDefines[ i ].Definition = s_aszPassModes[ a_rId.aStages[ i ] ];

	TINT iExtraPass = FindFlagIndex( uiFlags, 2, s_aExtraPassFlags );

	if ( iExtraPass != 0 )
	{
		TASSERT( uiNumTevStages == 1 );
		aDefines[ 0 ].Definition = s_aszExtraPass[ iExtraPass ];
	}

	return TRenderDX11::CompileShader( s_szSkinShaderPS, "main", "ps_4_0", aDefines );
}

// $deBlob: FUNCTION 00564280
ASkinMaterialHAL::ASkinMaterialHAL()
{
	m_pOrderTable   = TNULL;
	m_iNumTevStages = 0;

	for ( TINT i = 0; i < MAX_PASSES; i++ )
	{
		SetPassMode( i, PASSMODE_DEFAULT );
		m_afUOffset[ i ] = 0.0f;
		m_afVOffset[ i ] = 0.0f;
		m_aTevModes[ i ] = 1;
	}

	m_aTevModes[ 0 ]    = 0;
	m_iFakeOpaqueFadeID = -1;
	m_iUserDataIndex    = -1;
	SetFlags( SKINFLAG_DEPTHWRITE, TTRUE );
	SetFlags( SKINFLAG_UNK13, TTRUE );
}

// $deBlob: FUNCTION 00567140
// $deBlob: FUNCTION 00566dd0
ASkinMaterialHAL::~ASkinMaterialHAL()
{
}

// $deBlob: FUNCTION 005643e0
void ASkinMaterialHAL::Create( BLENDMODE a_eBlendMode )
{
	// NOTE: retail sets the blend mode twice
	SetBlendMode( a_eBlendMode, 0 );
	SetBlendMode( a_eBlendMode, 0 );
	TMaterial::Create();
}

// $deBlob: FUNCTION 00564410
void ASkinMaterialHAL::OnDestroy()
{
	SetOrderTable( TNULL, TFALSE );
	TMaterial::OnDestroy();
}

// $deBlob: FUNCTION 00564430
void ASkinMaterialHAL::PreRender()
{
	TRenderDX11* pRender = TRenderDX11::Interface();
	s_iExtraPass         = 0;

	if ( -1 < m_iFakeOpaqueFadeID )
	{
		// The fake opaque fade of the previous frame is over
		ASkinShaderHAL*   pShader  = TSTATICCAST( ASkinShaderHAL, ASkinShader::GetSingleton() );
		ASkinMaterialHAL* pSource  = pShader->GetFakeOpaqueFade( m_iFakeOpaqueFadeID, TFALSE ).pSourceMaterial;
		ASkinMaterialHAL* pSource2 = pShader->GetFakeOpaqueFade( m_iFakeOpaqueFadeID, TTRUE ).pSourceMaterial;

		if ( pSource != TNULL )
			pSource->m_iFakeOpaqueFadeID = -1;

		if ( pSource2 != TNULL )
			pSource2->m_iFakeOpaqueFadeID = -1;

		m_iFakeOpaqueFadeID = -1;
	}

	TINT  iNumTextures     = m_iNumTex;
	TBOOL bHasAlpha        = TFALSE;
	TINT  iCurrentTexStage = 0;
	TINT  iCurrentTexMap   = 0;
	m_iNumTevStages        = 0;

	for ( TINT iIndex = 0; iIndex < iNumTextures; iIndex++ )
	{
		TASSERT( iIndex < MAXTEXTURES );
		TTexture* pTex = m_pTextures[ iIndex ];

		if ( pTex == TNULL )
		{
			// A stage without texture: the colour only
			m_iNumTevStages++;
			TASSERT( iCurrentTexStage < 3 );
			s_aStageTevColor[ iCurrentTexStage ] = 0;

			if ( m_aTevModes[ iIndex ] == 0x13 )
			{
				iCurrentTexStage++;
			}
			else
			{
				TASSERT( iCurrentTexStage == 0 );
				s_iExtraPass = 1;
			}

			continue;
		}

		if ( bHasAlpha || ( pTex->GetFlags1() & 1 ) )
			bHasAlpha = TTRUE;

		pTex->Bind( iCurrentTexMap );

		switch ( GetPassMode( iIndex ) )
		{
			case PASSMODE_DEFAULT:
				TASSERT( iCurrentTexMap < 3 );
				s_aTexMapCoordGen[ iCurrentTexMap ] = 0;
				break;
			case PASSMODE_UVOFFSET:
			{
				TASSERT( iIndex < 3 && iCurrentTexMap < 3 );
				TMatrix44 oUVMatrix;
				oUVMatrix.Set( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, m_afUOffset[ iIndex ], m_afVOffset[ iIndex ], 0.0f, 1.0f );
				pRender->SetVec4InVSBuffer( 20, &oUVMatrix, 4 );
				s_aTexMapCoordGen[ iCurrentTexMap ] = 1;
				break;
			}
			case PASSMODE_ENVMAP:
				TASSERT( iCurrentTexMap < 3 );
				s_aTexMapCoordGen[ iCurrentTexMap ] = 3;
				break;
			case PASSMODE_PROJECTED:
			{
				TASSERT( TFALSE );
				TASSERT( iCurrentTexMap < 3 );
				s_aTexMapCoordGen[ iCurrentTexMap ] = 2;
				TASSERT( m_iUserDataIndex > -1 );

				ASkinShaderHAL*                   pShader = TSTATICCAST( ASkinShaderHAL, ASkinShader::GetSingleton() );
				ASkinShaderHAL::ProjectedTexture& rProj   = pShader->GetProjectedTexture( m_iUserDataIndex );
				TASSERT( D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP == pTex->GetWrapS() && D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP == pTex->GetWrapT() );

				TMatrix44 oProjection;

				if ( rProj.bHasFrustum == 0 )
				{
					// XMMatrixPerspectiveFovLH( s_fProjectedTextureFOV, the aspect ratio, 0.0f, 1.0f )
					TFLOAT fAspect = ( TRender::GetSingleton()->GetAspectRatio() == TRender::ASPECT_RATIO_16_9 ) ? 1.7777778f : 1.3333334f;
					TFLOAT fSin;
					TFLOAT fCos;
					DirectX::XMScalarSinCos( &fSin, &fCos, s_fProjectedTextureFOV * 0.017453292f * 0.5f );
					TFLOAT fHeight = fCos / fSin;
					TFLOAT fWidth  = fHeight / fAspect;
					oProjection.Set( fWidth, 0.0f, 0.0f, 0.0f, 0.0f, fHeight, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -0.0f, 0.0f );
				}
				else
				{
					// XMMatrixOrthographicOffCenterLH( fUnk50, fUnk54, fUnk4C, fUnk48, 0.0f, 1.0f)
					TFLOAT fLeft   = rProj.fUnk50;
					TFLOAT fRight  = rProj.fUnk54;
					TFLOAT fBottom = rProj.fUnk4C;
					TFLOAT fTop    = rProj.fUnk48;
					TFLOAT fNear   = 0.0f;
					TFLOAT fRecipW = 1.0f / ( fRight - fLeft );
					TFLOAT fRecipH = 1.0f / ( fTop - fBottom );
					TFLOAT fRange  = 1.0f / ( 1.0f - fNear );
					oProjection.Set( fRecipW + fRecipW, 0.0f, 0.0f, 0.0f, 0.0f, fRecipH + fRecipH, 0.0f, 0.0f, 0.0f, 0.0f, fRange, 0.0f, -( fLeft + fRight ) * fRecipW, -( fTop + fBottom ) * fRecipH, -fRange * fNear, 1.0f );
				}

				// Into the texture space (scaled by 0.5, then offset by 0.5)
				TMatrix44 oScale;
				oScale.Set( 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f );
				TMatrix44 oOffset;
				oOffset.Set( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f );

				TMatrix44 oTexMatrix = MultiplyMatrices( rProj.oMatrix, MultiplyMatrices( MultiplyMatrices( oProjection, oScale ), oOffset ) );

				TVector4 vColour;
				UnpackColour( rProj.uiColour, vColour );
				pRender->SetVec4InPSBuffer( 3, &vColour, 1 );
				pRender->SetVec4InVSBuffer( 24, &oTexMatrix, 4 );
				break;
			}
			case PASSMODE_RIMLIGHT:
				TASSERT( iCurrentTexMap < 3 );
				TASSERT( m_Flags & SKINFLAG_RIMLIGHT );
				s_aTexMapCoordGen[ iCurrentTexMap ] = 4;
				break;
		}

		s_aStageTevColor[ iCurrentTexStage ] = ( m_aTevModes[ iIndex ] == 0x11 ) ? 1 : 0;
		s_aStageTexMap[ iCurrentTexStage ]   = iCurrentTexMap;
		s_aStageTexCoord[ iCurrentTexStage ] = ( s_aTexMapCoordGen[ iCurrentTexMap ] == 4 ) ? 3 : iCurrentTexMap;

		TASSERT( iIndex == iCurrentTexStage && iCurrentTexStage < 3 && iCurrentTexMap < 3 );

		switch ( m_aTevModes[ iIndex ] )
		{
			case 0:
			case 1:
			case 2:
			case 0xC:
			case 0xF:
			case 0x11:
				iCurrentTexStage++;
				m_iNumTevStages++;
				break;
			case 3:
				TASSERT( TFALSE );
				iCurrentTexMap++;
				continue;
			case 4:
				TASSERT( TFALSE );
				iCurrentTexStage++;
				m_iNumTevStages++;
				break;
			case 0x10:
				TASSERT( TFALSE );
				iCurrentTexMap++;
				continue;
			case 0x16:
				pRender->SetVec4InPSBuffer( 2, &ASkinShaderHAL::s_vTevMode16Colour, 1 );
				pRender->SetVec4InPSBuffer( 7, &ASkinShaderHAL::s_vTevMode16Const, 1 );
				iCurrentTexStage++;
				m_iNumTevStages++;
				break;
			case 0x1A:
				pRender->SetVec4InPSBuffer( 4, &ASkinShaderHAL::s_vTevMode1AColour, 1 );
				iCurrentTexStage++;
				m_iNumTevStages++;
				break;
		}

		iCurrentTexMap++;
	}

	if ( iNumTextures == 0 )
	{
		m_iNumTevStages++;
		TASSERT( iCurrentTexStage == 0 );
		s_aStageTevColor[ iCurrentTexStage ] = 0;
		s_iExtraPass                         = 1;
	}

	ASkinShader::s_NumTevStages = m_iNumTevStages;

	// The cull mode
	if ( m_Flags & FLAGS_NO_CULL )
		SetCullMode( pRender, D3D11_CULL_NONE );
	else if ( m_Flags & FLAGS_UNK2 )
		SetCullMode( pRender, D3D11_CULL_BACK );
	else
		SetCullMode( pRender, D3D11_CULL_FRONT );

	// The blend mode
	switch ( m_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
		case 0xD:
		case 0x19:
		{
			D3D11_DEPTH_WRITE_MASK eDepthWrite = ( m_Flags & SKINFLAG_DEPTHWRITE ) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
			m_uiAlphaRef                       = bHasAlpha ? 1 : 0x80;

			pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, eDepthWrite );
			pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );

			if ( m_eBlendMode != BLENDMODE_DEFAULT )
			{
				pRender->SetDstAlpha( -1.0f );
				pRender->SetAlphaUpdate( TTRUE );
				m_uiAlphaRef = ( m_eBlendMode == 0x19 );
			}

			break;
		}
		case 1:
		case 0x11:
		case 0x18:
		{
			m_uiAlphaRef = 1;
			D3D11_BLEND eDestBlend;

			if ( m_eBlendMode == 0x18 )
			{
				pRender->SetDstAlpha( -1.0f );
				pRender->SetAlphaUpdate( TTRUE );
				eDestBlend = D3D11_BLEND_ONE;
			}
			else if ( m_eBlendMode == 0x11 )
			{
				ASkinShaderHAL* pShader      = TSTATICCAST( ASkinShaderHAL, ASkinShader::GetSingleton() );
				TUINT8          uiDepthWrite = ( GetOrderTable() == pShader->GetAlphaOrderTable( 0 ) ) ? s_bAlphaTable0DepthWrite : s_bAlphaTableDepthWrite;

				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK( uiDepthWrite ) );
				m_uiAlphaRef = 0;
				pRender->SetDstAlpha( 0.0f );
				pRender->SetAlphaUpdate( TTRUE );
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
				pRender->SetDepthBias( s_fAlphaDepthBias, 0.0f );
				return;
			}
			else
			{
				eDestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			}

			pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, eDestBlend );
			pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK( ( m_Flags & SKINFLAG_DEPTHWRITE ) ? 1 : 0 ) );
			break;
		}
		case 3:
		case 0xE:
			m_uiAlphaRef = 1;
			pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE );
			break;
		case 5:
		case 0xF:
			m_uiAlphaRef = 1;
			break;
		case 6:
			SetCullMode( pRender, D3D11_CULL_NONE );
			m_uiAlphaRef = 1;
			pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
			pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
			pRender->SetAlphaUpdate( TFALSE );
			pRender->SetColorUpdate( TFALSE );
			break;
	}
}

// $deBlob: FUNCTION 00565320
void ASkinMaterialHAL::PostRender()
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	switch ( m_eBlendMode )
	{
		case 6:
			pRender->SetColorUpdate( TTRUE );
			pRender->SetAlphaUpdate( TTRUE );
			break;
		case 0x11:
			pRender->SetDepthBias( 1.0f, 0.0f );
			pRender->SetDstAlpha( 0.0f );
			break;
		case 0xD:
		case 0x18:
		case 0x19:
			pRender->SetDstAlpha( 0.0f );
			break;
	}
}

// $deBlob: FUNCTION 005653b0
void ASkinMaterialHAL::SetBlendMode( BLENDMODE a_eBlendMode, TINT a_iPass )
{
	TASSERT( 0 <= a_eBlendMode && a_eBlendMode < BLENDMODE_NUMOF );

	ASkinShaderHAL* pShader = TSTATICCAST( ASkinShaderHAL, ASkinShader::GetSingleton() );

	if ( a_iPass == 0 )
	{
		switch ( a_eBlendMode )
		{
			case BLENDMODE_DEFAULT:
			case 0xD:
			case 0x19:
				SetOrderTable( pShader->GetOpaqueOrderTable(), TFALSE );
				ASkinMaterial::SetBlendMode( a_eBlendMode, 0 );
				return;
			case 1:
			case 0x11:
			case 0x18:
				SetFlags( SKINFLAG_DEPTHWRITE, TFALSE );
				SetAlphaOrderTable();
				ASkinMaterial::SetBlendMode( a_eBlendMode, 0 );
				return;
			case 2:
			case 3:
			case 5:
			case 0xE:
			case 0xF:
				SetAlphaOrderTable();
				ASkinMaterial::SetBlendMode( a_eBlendMode, 0 );
				return;
			case 6:
				if ( GetOrderTable() == TNULL )
				{
					SetOrderTable( pShader->GetUnkOrderTable(), TFALSE );
					ASkinMaterial::SetBlendMode( a_eBlendMode, 0 );
					return;
				}

				ASkinMaterial::SetBlendMode( a_eBlendMode, 0 );
				return;
		}
	}

	if ( a_eBlendMode == BLENDMODE_JELLY )
	{
		TASSERT( 0 <= a_iPass && a_iPass < ASkinShaderHAL::JELLY_ORDER_TABLE_COUNT );
		SetOrderTable( pShader->GetJellyOrderTable( a_iPass ), TFALSE );
		ASkinMaterial::SetBlendMode( BLENDMODE_JELLY, a_iPass );
		return;
	}

	if ( a_eBlendMode == BLENDMODE_STENCIL )
	{
		TASSERT( 0 <= a_iPass && a_iPass < ASkinShaderHAL::STENCIL_ORDER_TABLE_COUNT );
		SetOrderTable( pShader->GetStencilOrderTable( a_iPass ), TFALSE );
		ASkinMaterial::SetBlendMode( BLENDMODE_STENCIL, a_iPass );
		return;
	}

	ASkinMaterial::SetBlendMode( a_eBlendMode, 0 );
}

// $deBlob: FUNCTION 00565590
void ASkinMaterialHAL::SetOrderTable( TOrderTable* a_pOrderTable, TBOOL a_bUnk )
{
	if ( a_pOrderTable != m_pOrderTable )
	{
		if ( m_pOrderTable != TNULL )
			TOrderTable::DeregisterMaterial( GetRegMaterial() );

		if ( a_pOrderTable != TNULL )
			a_pOrderTable->RegisterMaterial( this );

		m_pOrderTable = a_pOrderTable;
	}
}

// $deBlob: FUNCTION 005674f0
TOrderTable* ASkinMaterialHAL::GetOrderTable()
{
	return m_pOrderTable;
}

// $deBlob: FUNCTION 005655d0
void ASkinMaterialHAL::SetPassMode( TINT a_iPass, PASSMODE a_eMode )
{
	TASSERT( a_iPass < MAX_PASSES );

	if ( a_eMode == PASSMODE_ENVMAP )
	{
		m_aPassModes[ a_iPass ] = PASSMODE_ENVMAP;
		SetFlags( SKINFLAG_ENVMAP, TTRUE );
		return;
	}

	PASSMODE eOldMode = m_aPassModes[ a_iPass ];

	if ( eOldMode == PASSMODE_ENVMAP )
	{
		m_aPassModes[ a_iPass ] = a_eMode;

		for ( TINT i = 0; i < MAX_PASSES; i++ )
			if ( m_aPassModes[ i ] == PASSMODE_ENVMAP ) return;

		SetFlags( SKINFLAG_ENVMAP, TFALSE );
		return;
	}

	if ( a_eMode == PASSMODE_RIMLIGHT )
	{
		m_aPassModes[ a_iPass ] = PASSMODE_RIMLIGHT;
		SetFlags( SKINFLAG_RIMLIGHT, TTRUE );
		return;
	}

	m_aPassModes[ a_iPass ] = a_eMode;

	if ( eOldMode == PASSMODE_RIMLIGHT )
	{
		for ( TINT i = 0; i < MAX_PASSES; i++ )
			if ( m_aPassModes[ i ] == PASSMODE_RIMLIGHT ) return;

		SetFlags( SKINFLAG_RIMLIGHT, TFALSE );
	}
}

// $deBlob: FUNCTION 005656d0
void ASkinMaterialHAL::SetUVOffset( TFLOAT a_fUOffset, TFLOAT a_fVOffset, TINT a_iPass )
{
	TASSERT( -1.0f <= a_fUOffset );
	TASSERT( 1.0f >= a_fUOffset );
	TASSERT( -1.0f <= a_fVOffset );
	TASSERT( 1.0f >= a_fVOffset );
	TASSERT( a_iPass >= 0 && a_iPass < MAX_PASSES );

	m_afUOffset[ a_iPass ]  = a_fUOffset;
	m_afVOffset[ a_iPass ]  = a_fVOffset;
	m_aPassModes[ a_iPass ] = PASSMODE_UVOFFSET;
}

// $deBlob: FUNCTION 00567230
void ASkinMaterialHAL::EnableEnvMap( TBOOL a_bEnable, TINT a_iPass )
{
	if ( a_bEnable )
	{
		TASSERT( a_iPass < MAX_PASSES );
		m_aPassModes[ a_iPass ] = PASSMODE_ENVMAP;
		SetFlags( SKINFLAG_ENVMAP, TTRUE );
		return;
	}

	SetPassMode( a_iPass, PASSMODE_DEFAULT );
}

// $deBlob: FUNCTION 00565840
void ASkinMaterial::SetAlphaOrderTable()
{
	ASkinShaderHAL* pShader = TSTATICCAST( ASkinShaderHAL, ASkinShader::GetSingleton() );

	if ( m_Flags & SKINFLAG_ALPHATABLE1 )
		SetOrderTable( pShader->GetAlphaOrderTable( 1 ), TFALSE );
	else if ( m_Flags & SKINFLAG_ALPHATABLE2 )
		SetOrderTable( pShader->GetAlphaOrderTable( 2 ), TFALSE );
	else
		SetOrderTable( pShader->GetAlphaOrderTable( 0 ), TFALSE );
}

// $deBlob: FUNCTION 005659c0
ASkinMeshHAL::ASkinMeshHAL()
{
	m_uiUnk28 = 0;
}

// $deBlob: FUNCTION 00567170
// $deBlob: FUNCTION 00566de0
ASkinMeshHAL::~ASkinMeshHAL()
{
}

// $deBlob: FUNCTION 005659e0
void ASkinMeshHAL::OnDestroy()
{
	ASkinMesh::OnDestroy();
}

// $deBlob: FUNCTION 00565a00
TBOOL ASkinMeshHAL::Render()
{
	TRenderDX11*    pRender  = TRenderDX11::Interface();
	TRenderContext* pContext = pRender->GetCurrentContext();
	ASkinShaderHAL* pShader  = TSTATICCAST( ASkinShaderHAL, ASkinShader::GetSingleton() );
	TFLOAT          fAlpha   = pContext->GetAlphaBlend();
	TRenderPacket*  pPacket;

	if ( fAlpha <= s_fFakeOpaqueFadeLimit )
	{
		ASkinMaterialHAL*        pMaterial  = TSTATICCAST( ASkinMaterialHAL, m_pMaterial );
		ASkinMaterial::BLENDMODE eBlendMode = pMaterial->GetBlendMode();

		if ( eBlendMode != 1 && eBlendMode != 0x18 && eBlendMode != 0x11 && pShader->GetNumFakeOpaqueFades() < ASkinShaderHAL::MAX_FAKE_OPAQUE_FADES )
		{
			// An opaque mesh fading out: drawn with one of the shader's fake opaque fade materials
			pMaterial  = ASkinMaterialHAL::Cast( pMaterial );
			TINT16 iID = pMaterial->GetFakeOpaqueFadeID();

			if ( iID < 0 )
			{
				iID = pShader->GetNumFakeOpaqueFades();

				if ( iID > ASkinShaderHAL::MAX_FAKE_OPAQUE_FADES - 1 )
					return TFALSE;

				pMaterial->SetFakeOpaqueFadeID( iID );
				pShader->SetNumFakeOpaqueFades( pShader->GetNumFakeOpaqueFades() + 1 );
			}

			TBOOL                           bSecond = fAlpha <= s_fFakeOpaqueFadeLimit2;
			ASkinShaderHAL::FakeOpaqueFade& rFade   = pShader->GetFakeOpaqueFade( iID, bSecond );
			rFade.pMaterial->SetTexture( 0, pMaterial->GetTexture( 0 ) );
			rFade.pMaterial->SetFakeOpaqueFadeID( iID );
			pShader->GetFakeOpaqueFade( iID, bSecond ).pSourceMaterial = pMaterial;

			pPacket = rFade.pMaterial->GetRegMaterial()->AddRenderPacket( this );
			goto SetPacket;
		}
	}

	pPacket = m_pMaterial->GetRegMaterial()->AddRenderPacket( this );

SetPacket:
	pPacket->SetModelViewMatrix( pContext->GetModelViewMatrix() );
	pPacket->SetSkeletonInstance( pContext->GetSkeletonInstance() );
	pPacket->SetAmbientColour( pContext->GetAmbientColour().AsVector3() );
	pPacket->SetLightColour( pRender->GetLightColour().AsBasisVector3( 0 ) );
	pPacket->SetLightDirection( pRender->GetLightDirection().AsBasisVector3( 0 ) );
	pPacket->SetFade( TUINT8( TINT( fAlpha * 255.0f ) ) );
	pPacket->SetAlpha( pContext->GetShadeCoeff() );

	return TTRUE;
}
