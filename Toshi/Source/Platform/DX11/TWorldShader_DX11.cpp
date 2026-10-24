#include "ToshiPCH.h"
#include "TWorldShader_DX11.h"
#include "TRender_DX11.h"
#include "TTexture_DX11.h"
#include "Render/TOrderTable.h"
#include "Render/TRenderPacket.h"
#include "Render/TRenderContext.h"
#include "TRenderContext_DX11.h"

#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TWorldMaterialHAL::t_SetBlendModeCallback TWorldMaterialHAL::s_fnSetBlendModeCallback = TNULL;
TWorldMaterialHAL::t_RenderCallback       TWorldMaterialHAL::s_fnPreRenderCallback    = TNULL;
TWorldMaterialHAL::t_RenderCallback       TWorldMaterialHAL::s_fnInstanceCallback     = TNULL;
TWorldMaterialHAL::t_RenderCallback       TWorldMaterialHAL::s_fnPostRenderCallback   = TNULL;
TVector4                                  TWorldMaterialHAL::s_vMultiPassColour       = { 1.0f, 1.0f, 1.0f, 1.0f };
TVector4                                  TWorldMaterialHAL::s_vLerpConstColour;
TBOOL                                     TWorldShaderHAL::s_bInstancing           = TTRUE;
TWorldShaderHAL::t_RenderCallback         TWorldShaderHAL::s_fnRenderCallback      = TNULL;
TUINT                                     TWorldShaderHAL::s_uiMaxInstances        = 2000;
TUINT                                     TWorldShaderHAL::s_iMaxOfCurNumInstances = 0;
TVector4                                  TWorldShaderHAL::s_vLightingScale5       = { 0.0f, 0.0f, 0.0f, 1.0f };
TVector4                                  TWorldShaderHAL::s_vLightingScale6       = { 1.0f, 1.0f, 1.0f, 1.0f };

//-----------------------------------------------------------------------------
// HLSL of the world shader, compiled at runtime by TShaderCache_DX11 into the permutations of the shader ids
// (the macros pick the functions, see CompileVertexShader/CompilePixelShader)
//-----------------------------------------------------------------------------

static const TCHAR s_szWorldShaderVS[] =
	"\n"
	"\n"
	"// Attribute input/output\n"
	"struct VS_IN\n"
	"{\n"
	"\tfloat3 ObjPos\t: POSITION;\t\t// Object space position\n"
	"\tfloat3 Normal\t: NORMAL;\t\t// Vertex normal\n"
	"\tfloat3 Color\t: COLOR;\t\t// Vertex color\n"
	"\tfloat2 UV0\t\t: TEXCOORD0;\t// UV0\n"
	"\tfloat2 UV1\t\t: TEXCOORD1;\t// UV1\n"
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
	"\tfloat2 UV3\t\t: TEXCOORD3;\t// UV\n"
	"\tfloat2 UV4\t\t: TEXCOORD4;\t// UV\n"
	"\tfloat FogFactor : FOG;\n"
	"};\n"
	"\n"
	"// Uniforms\n"
	"float4x4 matWVP\t\t: register(c0);\n"
	"float4x4 matWV\t\t: register(c4);\n"
	"float3x3 matNorm\t: register(c8);\n"
	"float4x4 texMtx0\t: register(c12);\n"
	"float4x4 texMtx1\t: register(c16);\n"
	"float4x4 texMtx2\t: register(c20);\n"
	"float4x4 texMtx3\t: register(c24);\n"
	"float4x4 texMtx6\t: register(c28);\n"
	"float4x4 texMtx7\t: register(c32);\n"
	"float4 fogStartEnd\t: register(c36);\n"
	"\n"
	"\n"
	"float2 Mtx2x4Tex0Idnt(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\treturn uv0;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4Tex1Idnt(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\treturn uv1;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4Tex0Mtx0(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx0, float4(uv0, 0.0, 1.0));\n"
	"\treturn temp.xy;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4Tex0Mtx1(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx1, float4(uv0, 0.0, 1.0));\n"
	"\treturn temp.xy;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4Tex0Mtx2(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx2, float4(uv0, 0.0, 1.0));\n"
	"\treturn temp.xy;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4Tex0Mtx3(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx3, float4(uv0, 0.0, 1.0));\n"
	"\treturn temp.xy;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4Tex1Mtx0(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx0, float4(uv1, 0.0, 1.0));\n"
	"\treturn temp.xy;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4Tex1Mtx1(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx1, float4(uv1, 0.0, 1.0));\n"
	"\treturn temp.xy;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4Tex1Mtx2(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx2, float4(uv1, 0.0, 1.0));\n"
	"\treturn temp.xy;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4Tex1Mtx3(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx3, float4(uv1, 0.0, 1.0));\n"
	"\treturn temp.xy;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4PosMtx0(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx0, float4(pos, 1.0));\n"
	"\treturn temp.xy;\n"
	"}\n"
	"\n"
	"float2 Mtx2x4PosMtx1(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx1, float4(pos, 1.0));\n"
	"\treturn temp.xy;\n"
	"}\n"
	"\n"
	"float2 Mtx3x4Tex0Mtx0(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\t// TEXCOORD0 = TEXCOORD0 * MTX0    GX_TG_MTX3x4 performs a 3x4 matrix multiplication on the input attribute, and generates S, T, and Q texture coordinates.S and T are then divided by Q to produce the actual 2D texture coordinates.\n"
	"\tfloat4 temp = mul(texMtx0, float4(uv0, 1.0, 1.0));\n"
	"//\treturn temp.xy / temp.z;\n"
	"\treturn temp.xy; // [CESM] World Why this works better?\n"
	"}\n"
	"\n"
	"float2 Mtx3x4PosMtx0(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx0, float4(pos, 1.0));\n"
	"//\treturn temp.xy / temp.z;\n"
	"\treturn temp.xy; // [CESM] World Why this works better?\n"
	"}\n"
	"\n"
	"float2 Mtx3x4PosMtx1(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx1, float4(pos, 1.0));\n"
	"//\treturn temp.xy / temp.z;\n"
	"\treturn temp.xy; // [CESM] World Why this works better?\n"
	"}\n"
	"\n"
	"float2 Mtx3x4PosMtx6(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx6, float4(pos, 1.0));\n"
	"//\treturn temp.xy / temp.z;\n"
	"\treturn temp.xy; // [CESM] World Why this works better?\n"
	"}\n"
	"\n"
	"float2 Mtx3x4NrmMtx7(float2 uv0, float2 uv1, float3 pos, float3 nrm)\n"
	"{\n"
	"\tfloat4 temp = mul(texMtx7, float4(nrm, 1.0));\n"
	"//\treturn temp.xy / temp.z;\n"
	"\treturn temp.xy; // [CESM] World Why this works better?\n"
	"}\n"
	"\n"
	"\n"
	"float FogFactorEmpty(float4 pos)\n"
	"{\n"
	"\treturn 0.0;\n"
	"}\n"
	"\n"
	"float FogFactorLinear(float4 pos)\n"
	"{\n"
	"\tfloat4 viewPos = mul(matWV, pos);\n"
	"\treturn saturate((viewPos.z - fogStartEnd.x) / (fogStartEnd.y - fogStartEnd.x));\n"
	"}\n"
	"\n"
	"float FogFactorExponentialSquared(float4 pos)\n"
	"{\n"
	"\tfloat4 viewPos = mul(matWV, pos);\n"
	"\tfloat exponent = (viewPos.z - fogStartEnd.x) / (fogStartEnd.y - fogStartEnd.x);\n"
	"\texponent = saturate(exponent); // [DNR] clamp fog to fogStartEnd range.\n"
	"\treturn saturate(1.0 - pow(2, -8 * (exponent * exponent)));\n"
	"}\n"
	"\n"
	"// Programs\n"
	"VS_OUT main(VS_IN In)\n"
	"{\n"
	"\tVS_OUT Out;\n"
	"\tOut.ProjPos = mul(matWVP, float4(In.ObjPos, 1.0));\n"
	"\tOut.Normal = mul(matNorm, In.Normal);\n"
	"\tOut.Color.xyz = In.Color;\n"
	"\tOut.Color.a = 1.0;\n"
	"\tOut.UV0 = TexCoordGen0(In.UV0, In.UV1, In.ObjPos, In.Normal);\n"
	"\tOut.UV1 = TexCoordGen1(In.UV0, In.UV1, In.ObjPos, In.Normal);\n"
	"\tOut.UV2 = TexCoordGen2(In.UV0, In.UV1, In.ObjPos, In.Normal);\n"
	"\tOut.UV3 = TexCoordGen3(In.UV0, In.UV1, In.ObjPos, In.Normal);\n"
	"\tOut.UV4 = TexCoordGen4(In.UV0, In.UV1, In.ObjPos, In.Normal);\n"
	"\tOut.FogFactor = ComputeFogFactor(float4(In.ObjPos, 1.0));\n"
	"\n"
	"\treturn Out;\n"
	"}\n";

static const TCHAR s_szWorldShaderPS[] =
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
	"\tfloat2 UV3\t\t: TEXCOORD3;\t// UV\n"
	"\tfloat2 UV4\t\t: TEXCOORD4;\t// UV\n"
	"\tfloat FogFactor : FOG;\n"
	"};\n"
	"\n"
	"// Uniforms\n"
	"uniform Texture2D texture0\t: register(t0);\n"
	"uniform Texture2D texture1\t: register(t1);\n"
	"uniform Texture2D texture2\t: register(t2);\n"
	"uniform Texture2D texture3\t: register(t3);\n"
	"uniform Texture2D texture4\t: register(t4);\n"
	"\n"
	"uniform SamplerState sampler0 : register(s0);\n"
	"uniform SamplerState sampler1 : register(s1);\n"
	"uniform SamplerState sampler2 : register(s2);\n"
	"uniform SamplerState sampler3 : register(s3);\n"
	"uniform SamplerState sampler4 : register(s4);\n"
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
	"float4 light0Pos\t\t\t: register(c12);\n"
	"float4 light0Color\t\t\t: register(c13);\n"
	"float4 light6Pos\t\t\t: register(c14);\n"
	"float4 light6Color\t\t\t: register(c15);\n"
	"float4 light7Pos\t\t\t: register(c16);\n"
	"float4 light7Color\t\t\t: register(c17);\n"
	"\n"
	"float4 indTexMtx1\t\t\t: register(c18); // x = m[1][1], y = scale exponent\n"
	"\n"
	"float4 fogColor\t\t\t\t: register(c22);\n"
	"float4 uScreenSize\t\t\t: register(c26);\n"
	"\n"
	"static float4 g_tevReg0;\n"
	"static float4 g_tevReg1;\n"
	"static float4 g_tevReg2;\n"
	"static float4 g_tevPrev;\n"
	"static float4 g_ras; // result of per-vertex lighting equations\n"
	"\n"
	"static const float4 ONE = float4(1.0, 1.0, 1.0, 1.0);\n"
	"\n"
	"void PassEmpty(float4 inColor, float4 texColor)\n"
	"{\n"
	"}\n"
	"\n"
	"void PassOverride(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - texturei)*0 + texturei*kColor0)\n"
	"\tg_tevPrev.rgb = texColor.rgb * kColor0.rgb;\n"
	"\t// A: tevPrev.a = 0 + ((1 - In.Color.a)*0 + In.Color.a*texturei.a)\n"
	"\tg_tevPrev.a = inColor.a * texColor.a;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassLerpColour(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - texturei)*tevReg0 + texturei*tevReg1)\n"
	"\t// A: tevPrev.a = 0 + ((1 - texturei.a)*tevReg0.a + texturei.a*tevReg1.a)\n"
	"\tg_tevPrev = (ONE - texColor) * g_tevReg0 + texColor * g_tevReg1;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassLerpColour2(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = kColor0 + ((1 - texturei)*tevReg0 + texturei*tevReg1)\n"
	"\tg_tevPrev.rgb = kColor0.rgb + ((ONE.rgb - texColor.rgb) * g_tevReg0.rgb + texColor.rgb * g_tevReg1.rgb);\n"
	"\t// A: tevPrev.a = 0 + ((1 - texturei.a)*tevReg0.a + texturei.a*tevReg1.a)\n"
	"\tg_tevPrev.a = ((ONE.a - texColor.a) * g_tevReg0.a + texColor.a * g_tevReg1.a);\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassLerpConst(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - tevReg2)*tevPrev + tevReg2*kColor1)\n"
	"\tg_tevPrev.rgb = ((ONE.rgb - g_tevReg2.rgb) * g_tevPrev.rgb + g_tevReg2.rgb * kColor1.rgb);\n"
	"\t// A: tevPrev.a = 0 + ((tevPrev.a > kColor1.a) ? tevReg2.a : 0)\n"
	"\tg_tevPrev.a = (g_tevPrev.a > kColor1.a) ? g_tevReg2.a : 0.0;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassLerpTexLM1(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevReg2 = 0 + ((1 - tevPrev)*0 + tevPrev*texturei)\n"
	"\tg_tevReg2.rgb = g_tevPrev.rgb * texColor.rgb;\n"
	"\t// A: tevReg2.a = 0 + ((1 - In.Color.a)*0 + In.Color.a*texturei.a)\n"
	"\tg_tevReg2.a = inColor.a * texColor.a;\n"
	"\n"
	"\tg_tevReg2 = clamp(g_tevReg2, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassLerpTexLM2(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - texturei)*0 + texturei*tevPrev)\n"
	"\tg_tevPrev.rgb = texColor.rgb * g_tevPrev.rgb;\n"
	"\t// A: tevPrev.a = 0 + ((1 - texturei.a)*0 + texturei.a*In.Color.a)\n"
	"\tg_tevPrev.a = texColor.a * inColor.a;\n"
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
	"void PassLerpTex(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - kColor3)*texturei + kColor3*tevPrev)\n"
	"\tg_tevPrev.rgb = clamp((ONE.rgb - kColor3.rgb) * texColor.rgb + kColor3.rgb * g_tevPrev.rgb, 0.0, 1.0);\n"
	"\t// A: tevReg2.a = 0 + ((1 - In.Color.a)*0 + In.Color.a*texturei.a)\n"
	"\tg_tevReg2.a = clamp(inColor.a * texColor.a, 0.0, 1.0);\n"
	"\n"
	"\t// C: tevPrev = tevPrev + ((1 - 0)*0 + 0*0)\n"
	"//\tg_tevPrev.rgb = g_tevPrev.rgb;\n"
	"\t// A: tevPrev.a = 0 + ((1 - kColor3.a)*tevReg2.a + kColor3.a*tevPrev.a)\n"
	"\tg_tevPrev.a = clamp((ONE.a - kColor3.a) * g_tevReg2.a + kColor3.a * g_tevPrev.a, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassProjectedTex(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = tevPrev + ((1 - 0)*0 + 0*0)\n"
	"//\tg_tevPrev.rgb = g_tevPrev.rgb;\n"
	"\t// A: tevPrev.a = texturei.a + ((1 - 0)*0 + 0*0)\n"
	"\tg_tevPrev.a = texColor.a;\n"
	"}\n"
	"\n"
	"void PassWriteAlpha(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = tevPrev + ((1 - 0)*0 + 0*0)\n"
	"//\tg_tevPrev.rgb = g_tevPrev.rgb;\n"
	"\t// A: tevPrev.a = texturei.a + ((1 - 0)*0 + 0*0)\n"
	"\tg_tevPrev.a = texColor.a;\n"
	"}\n"
	"\n"
	"void PassModulate(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - tevPrev)*0 + tevPrev*texturei)\n"
	"\tg_tevPrev.rgb = g_tevPrev.rgb * texColor.rgb;\n"
	"\t// A: tevPrev.a = 0 + ((1 - In.Color.a)*0 + In.Color.a*texturei.a)\n"
	"\tg_tevPrev.a = inColor.a * texColor.a;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassModulateAlpha(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - tevPrev)*0 + tevPrev*texturei)\n"
	"\t// A: tevPrev.a = 0 + ((1 - tevPrevr.a)*0 + tevPrev.a*texturei.a)\n"
	"\tg_tevPrev = clamp(g_tevPrev * texColor, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassAdd(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = tevPrev + ((1 - 1)*0 + 1*texturei)\n"
	"\tg_tevPrev.rgb = g_tevPrev.rgb + texColor.rgb;\n"
	"\t// A: tevPrev.a = tevPrev.a + ((1 - kConst1.a)*0 + kConst1.a*texturei.a)\n"
	"\tg_tevPrev.a = g_tevPrev.a + texColor.a;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassAddAlpha(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = tevPrev + ((1 - 1)*0 + 1*texturei)\n"
	"\tg_tevPrev.rgb = g_tevPrev.rgb + texColor.rgb;\n"
	"\t// A: tevPrev.a = tevPrev.a + ((1 - kConst1.a)*0 + kConst1.a*texturei.a)\n"
	"\tg_tevPrev.a = g_tevPrev.a + texColor.a;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassAddWriteAlpha(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = tevPrev + ((1 - 1)*0 + 1*texturei)\n"
	"\tg_tevPrev.rgb = g_tevPrev.rgb + texColor.rgb;\n"
	"\t// A: tevPrev.a = texturei.a + ((1 - 0)*0 + 0*0)\n"
	"\tg_tevPrev.a = texColor.a;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassWindows(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - tevReg1)*0 + tevReg1*texturei)\n"
	"\tg_tevPrev.rgb = g_tevReg1.rgb * texColor.rgb;\n"
	"\t// A: tevPrev.a = texturei.a + ((1 - 0)*0 + 0*0)\n"
	"\tg_tevPrev.a = texColor.a;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassWindows2(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - tevPrev)*0 + tevPrev*texturei)\n"
	"\tg_tevPrev.rgb = g_tevPrev.rgb * texColor.rgb;\n"
	"\t// A: tevPrev.a = 0 + ((texturei.a > kColor3.a) ? tevPrev.a : 0);\n"
	"\tg_tevPrev.a = (texColor.a > kColor3.a) ? g_tevPrev.a : 0.0;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassSubtractive(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = tevPrev - ((1 - 1)*0 + 1*texturei)\n"
	"\tg_tevPrev.rgb = g_tevPrev.rgb + texColor.rgb;\n"
	"\t// A: tevPrev.a = tevPrev.a + ((1 - 0)*0 + 0*0)\n"
	"//\tg_tevPrev.a = g_tevPrev.a;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassSubtractiveAlpha(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = tevPrev - ((1 - 1)*0 + 1*texturei)\n"
	"\tg_tevPrev.rgb = g_tevPrev.rgb + texColor.rgb;\n"
	"\t// A: tevPrev.a = tevPrev.a - ((1 - 0)*texturei.a + 0*0)\n"
	"\tg_tevPrev.a = g_tevPrev.a - texColor.a;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"void PassBlendWriteAlpha(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - texturei)*tevPrev + texturei*kColor1)\n"
	"\tg_tevPrev.rgb = (ONE.rgb - texColor.rgb) * g_tevPrev.rgb + texColor.rgb * kColor1.rgb;\n"
	"\t// A: tevPrev.a = 0 + ((1 - texturei.a)*0 + texturei.a*kColor1.a)\n"
	"\tg_tevPrev.a = texColor.a * kColor1.a;\n"
	"\n"
	"\tg_tevPrev = clamp(g_tevPrev, 0.0, 1.0);\n"
	"}\n"
	"\n"
	"// d + ((1 - c)*a + c*b)\n"
	"\n"
	"\n"
	"// Blend mode special passes\n"
	"void ExtraPassWater(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = tevPrev + ((1 - 0)*0 + 0*0)\n"
	"//\tg_tevPrev.rgb = g_tevPrev.rgb;\n"
	"\n"
	"\t// A: tevPrev.a = 0 + ((tevReg2.a > texturei.a) ? tevPrev.a : 0)\n"
	"\tg_tevPrev.a = (g_tevReg2.a > texColor.a) ? g_tevPrev.a : 0.0;\n"
	"}\n"
	"\n"
	"void ExtraPassWaterPass2(float4 inColor, float4 texColor)\n"
	"{\n"
	"\n"
	"}\n"
	"\n"
	"void ExtraPassHeatPass2(float4 inColor, float4 texColor)\n"
	"{\n"
	"\n"
	"}\n"
	"\n"
	"void ExtraPassJelly0(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - tevReg2)*0 + tevReg2*1)\n"
	"\tg_tevPrev.rgb = g_tevReg2.rgb;\n"
	"\n"
	"\t// A: tevPrev.a = 0 + ((1 - kConst1.a)*0 + kConst1.a*tevReg2.a)\n"
	"\tg_tevPrev.a = g_tevReg2.a;\n"
	"\n"
	"\t// No clamping!\n"
	"}\n"
	"\n"
	"void ExtraPassJelly1(float4 inColor, float4 texColor)\n"
	"{\n"
	"\t// C: tevPrev = 0 + ((1 - 0)*0 + 0*0)\n"
	"\tg_tevPrev.rgb = float3(0, 0, 0);\n"
	"\n"
	"\t// A: tevPrev.a = texturei.a + ((1 - 0)*0 + 0*0)\n"
	"\tg_tevPrev.a = texColor.a;\n"
	"\n"
	"\t// No clamping!\n"
	"}\n"
	"\n"
	"\n"
	"// Lighting\n"
	"float4 LightingMatRegNoLigth(float4 chanMatColor, float4 vertexColor)\n"
	"{\n"
	"\treturn chanMatColor;\n"
	"}\n"
	"\n"
	"float4 LightingMatVtxNoLight(float4 chanMatColor, float4 vertexColor)\n"
	"{\n"
	"\treturn vertexColor;\n"
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
	"/*\n"
	"float2 IndirectLookupEmpty(float2 uv0, float2 uv1)\n"
	"{\n"
	"\treturn uv0;\n"
	"}\n"
	"\n"
	"float2 IndirectLookupWaterPass2(float2 uv0, float2 uv1)\n"
	"{\n"
	"\tfloat4 noise = tex2D(texture1, uv1);\n"
	"\tuv0.y += (noise.g * 0.125 * 0.5);\n"
	"\treturn uv0;\n"
	"}\n"
	"*/\n"
	"\n"
	"float4 IndirectLookupEmpty(float2 uv0, float2 uv1, float2 uvscreen)\n"
	"{\n"
	"\treturn texture0.Sample(sampler0, uv0);\n"
	"}\n"
	"\n"
	"float4 IndirectLookupWaterPass2(float2 uv0, float2 uv1, float2 uvscreen)\n"
	"{\n"
	"\tfloat scale = indTexMtx1.y * indTexMtx1.x;\n"
	"\tfloat2 tc = texture1.Sample(sampler1, uv1).xy;\n"
	"\tfloat2 uv = uvscreen + float2(0,(tc.y-0.5)*scale);\n"
	"\treturn texture0.Sample(sampler0,uv );\n"
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
	"\tfloat4 chanColor0 = Lighting0(chanMatColor0, In.Color);\n"
	"\n"
	"\tfloat2 uvscreen = In.ProjPos.xy / uScreenSize.xy;\n"
	"\tPass0(chanColor0, IndirectLookup0(In.UV0, In.UV1,uvscreen ));\n"
	"\n"
	"\tPass1(chanColor0, texture1.Sample(sampler1, In.UV1));\n"
	"\n"
	"\tPass2(chanColor0, texture2.Sample(sampler2, In.UV2));\n"
	"\n"
	"\tPass3(chanColor0, texture3.Sample(sampler3, In.UV3));\n"
	"\n"
	"\tPass4(chanColor0, texture4.Sample(sampler4, In.UV4));\n"
	"\n"
	"\tif (alphaTest.x > 0 && g_tevPrev.a < alphaTest.y) // Here always GX_GEQUAL was used\n"
	"\t\tdiscard;\n"
	"\n"
	"\tComputeFog(In.FogFactor);\n"
	"\n"
	"\treturn g_tevPrev;\n"
	"}\n";

static const TCHAR* s_pszWorldShaderVS = s_szWorldShaderVS;
static const TCHAR* s_pszWorldShaderPS = s_szWorldShaderPS;

// VertexShaderId::uiFlags/uiTexGen23/uiTexGen4 bits of the texture coordinate generators: a 16 bit half selects one of
// s_aszTexGenFunctions (the first table for the low half, the second for the high half)
static const TUINT32 s_auiTexGenFlagsLo[ 17 ] = {
	0x0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000
};

static const TUINT32 s_auiTexGenFlagsHi[ 17 ] = {
	0x10000, 0x0, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000, 0x1000000, 0x2000000, 0x4000000,
	0x8000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000
};

static const TCHAR* s_aszTexGenFunctions[ 17 ] = {
	"Mtx2x4Tex0Idnt", "Mtx2x4Tex1Idnt", "Mtx2x4Tex0Mtx0", "Mtx2x4Tex0Mtx1", "Mtx2x4Tex0Mtx2", "Mtx2x4Tex0Mtx3",
	"Mtx2x4Tex1Mtx0", "Mtx2x4Tex1Mtx1", "Mtx2x4Tex1Mtx2", "Mtx2x4Tex1Mtx3", "Mtx2x4PosMtx0", "Mtx2x4PosMtx1",
	"Mtx3x4Tex0Mtx0", "Mtx3x4PosMtx0", "Mtx3x4PosMtx1", "Mtx3x4PosMtx6", "Mtx3x4NrmMtx7"
};

// PixelShaderId::uiFlags bit of the lighting function
static const TUINT32 s_auiLightingFlags[ 2 ] = { 0x0, 0x1 };

static const TCHAR* s_aszLightingFunctions[ 2 ] = { "LightingMatRegNoLigth", "LightingMatVtxNoLight" };

// PixelShaderId::uiFlags bits of the indirect lookups / extra passes
static const TUINT32 s_auiIndirectFlags[ 6 ] = { 0x0, 0x2, 0x4, 0x8, 0x10, 0x20 };

static const TCHAR* s_aszIndirectFunctions[ 6 ] = {
	TNULL, "ExtraPassWater", "IndirectLookupWaterPass2", "IndirectLookupWaterPass2", "ExtraPassJelly0", "ExtraPassJelly1"
};

// The pass functions by TWorldMaterialHAL::MULTIPASSBLENDTYPE (PixelShaderId::aStages)
static const TCHAR* s_aszPassFunctions[ TWorldMaterialHAL::MULTIPASSBLENDTYPE_NUMOF ] = {
	"PassOverride", "PassModulate", "PassAdd", "PassSubtractive", "PassBlend", "PassAddHalf", "PassShadow",
	"PassProjectedTex", "PassWindows", "PassWindows2", "PassAddWriteAlpha", "PassWriteAlpha", "PassAddAlpha",
	"PassSubtractiveAlpha", "PassModulateAlpha", "PassTexAlphaBlend", "PassOverrideZeroAlpha", "PassOverrideChan2",
	"PassLerpColour", "PassLerpColour2", "PassWindowsProj", "PassBlendWriteAlpha", "PassLerpConst", "PassLerpTex",
	"PassLerpTexLM1", "PassLerpTexLM2", "PassAddAlphaLerp"
};

static const TCHAR* s_pszFogFactorExponentialSquared = "FogFactorExponentialSquared";
static const TCHAR* s_pszFogLinear                   = "FogLinear";

//-----------------------------------------------------------------------------
// Render state of the world shader: PreRender sets it up for Render, the Lazy ones cache what was last set
// (StartFlush resets them, s_abLazyForce makes PreRender/Render set everything again)
//-----------------------------------------------------------------------------
static TBOOL s_abLazyForce[ 15 ] = { TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE };

static TUINT8 s_auiLazyUserData2[ 8 ]          = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static TINT   s_aiLazyMultiPassBlendTypes[ 8 ] = { -1, -1, -1, -1, -1, -1, -1, -1 };
static TINT   s_iLazyBlendMode                 = -1;
static TINT   s_iLazyNumTevStages              = -1;
static TUINT  s_uiLazyAlpha                    = 0xFFFFFFFF;
static TINT   s_aiLazyUnk98246C[ 7 ]           = { -1, -1, -1, -1, -1, -1, -1 };
static TINT   s_NumTevStages                   = 1;

// The VS registers of the texture matrices of the passes (texMtx0..3)
static const TINT s_aiTexMtxRegisters[ 6 ] = { 12, 16, 20, 24, 10, 11 };

// The TexCoordGen functions (s_aszTexGenFunctions) of UVMODE_STREAM by UV stream, of the texture matrix modes by
// UV stream and pass
static const TINT s_aiTexGenIdentity[ 2 ]   = { 0, 1 };
static const TINT s_aiTexGenMatrix[ 2 * 4 ] = { 2, 3, 4, 5, 6, 7, 8, 9 }; // [ stream ][ pass ]

// The TexCoordGen function of each pass (indices into s_aszTexGenFunctions), set by PreRender
TINT s_aiTexCoordGen[ 5 ] = { 0, 1, 1, 1, 1 };

static const TFLOAT s_fWindows2Constant = 0.7843137f; // (200/255)
static const TFLOAT s_fLightmapScale    = 0.5f;
static const TINT   s_iLightmapExponent = -6;
static TVector4     s_vFogColour        = { 0.0f, 0.0f, 0.0f, 1.0f };
static const TFLOAT s_fDepthBias        = 1.0f;

static const TTexture* s_apLazyTextures[ 8 ];
static TINT            s_iReflectionSet; // the reflection matrix was set (Render, StartFlush resets it)
static TBOOL           s_bLightingSet;
static TBOOL           s_bUnk9B241D; // never set
static TBOOL           s_bLazyFog;
static TBOOL           s_bWaterFrameReset1;
TBOOL                  s_bWaterFrameReset2;
TVector4               s_vAlphaTest;   // x: 1.0 to test, y: the reference (PS c0)
TVector4               s_vFogStartEnd; // x: start, y: end, z: 1.0 when fogged (VS c36)
TINT                   s_iLighting;    // index into s_auiLightingFlags
TINT                   s_iIndirect;    // index into s_auiIndirectFlags
static TMatrix44       s_oLazyModelView;
// $deBlob: FUNCTION 0053fad0
static TVector4 s_vLazyFogColour = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
static TBOOL    s_bUnk9B25EC; // depth write of blend mode 19 (never set)


// XMMatrixMultiply( a_rA, a_rB ): a_rA then a_rB, summed in DirectXMath's order
static TMatrix44 MultiplyMatrices( const TMatrix44& a_rA, const TMatrix44& a_rB )
{
	TMatrix44 oResult;
	DirectX::XMStoreFloat4x4(
	    TREINTERPRETCAST( DirectX::XMFLOAT4X4*, &oResult ),
	    DirectX::XMMatrixMultiply(
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &a_rA ) ),
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &a_rB ) )
	    )
	);

	return oResult;
}

// XMMatrixTranslation
static TMatrix44 Translation( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ )
{
	TMatrix44 oResult;
	oResult.Set( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, a_fX, a_fY, a_fZ, 1.0f );

	return oResult;
}

// XMMatrixPerspectiveFovLH (without DirectXMath's asserts: Render passes a near plane of 0)
// $deBlob: FUNCTION 0055e580
static TMatrix44 PerspectiveFovLH( TFLOAT a_fFovAngleY, TFLOAT a_fAspectRatio, TFLOAT a_fNearZ, TFLOAT a_fFarZ )
{
	TFLOAT fSinFov;
	TFLOAT fCosFov;
	DirectX::XMScalarSinCos( &fSinFov, &fCosFov, 0.5f * a_fFovAngleY );

	TFLOAT fHeight = fCosFov / fSinFov;
	TFLOAT fWidth  = fHeight / a_fAspectRatio;
	TFLOAT fRange  = a_fFarZ / ( a_fFarZ - a_fNearZ );

	TMatrix44 oResult;
	oResult.Set( fWidth, 0.0f, 0.0f, 0.0f, 0.0f, fHeight, 0.0f, 0.0f, 0.0f, 0.0f, fRange, 1.0f, 0.0f, 0.0f, -fRange * a_fNearZ, 0.0f );

	return oResult;
}

// Unpacks an ARGB colour into ( r, g, b, a ) in [0;1]
// $deBlob: FUNCTION 0054f2b0
void UnpackColour( TUINT32 a_uiColour, TVector4& a_rOutColour )
{
	a_rOutColour.x = TFLOAT( ( a_uiColour >> 16 ) & 0xFF ) * ( 1.0f / 255.0f );
	a_rOutColour.y = TFLOAT( ( a_uiColour >> 8 ) & 0xFF ) * ( 1.0f / 255.0f );
	a_rOutColour.z = TFLOAT( a_uiColour & 0xFF ) * ( 1.0f / 255.0f );
	a_rOutColour.w = TFLOAT( a_uiColour >> 24 ) * ( 1.0f / 255.0f );
}

// The cull mode bits of the current rasterizer state
static void SetCullMode( TRenderDX11* a_pRender, D3D11_CULL_MODE a_eCullMode )
{
	TUINT8& uiFlags = *TREINTERPRETCAST( TUINT8*, &a_pRender->GetCurrentRasterizer().uiFlags );
	uiFlags         = TUINT8( ( uiFlags & ~0xC ) | ( a_eCullMode << 2 ) );
}

//-----------------------------------------------------------------------------
// TWorldShaderHAL
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00700ab0
// $deBlob: FUNCTION 00700ac0
// $deBlob: FUNCTION 00700ae0
// $deBlob: FUNCTION 00700b00
// $deBlob: FUNCTION 00700b10
// $deBlob: FUNCTION 0053fa90
// $deBlob: FUNCTION 00859510
TDEFINE_CLASS( TWorldShaderHAL );

// $deBlob: FUNCTION 00700b20
TWorldShaderHAL::TWorldShaderHAL()
{
	m_pWaterNoiseTex      = TNULL;
	m_pWaterTransitionTex = TNULL;
	m_pMemWater           = TNULL;
	m_iNumOrderTables     = 0;
	m_uiMaxInstances      = s_uiMaxInstances;
	m_uiNumInstances      = 0;
	m_pInstanceData       = new TWorldMeshInstanceData[ m_uiMaxInstances ];
	TUtil::MemClear( m_pInstanceData, m_uiMaxInstances * sizeof( TWorldMeshInstanceData ) );
	m_vWaterUVOffset1 = TVector2::VEC_ZERO;
	m_vWaterUVOffset2 = TVector2::VEC_ZERO;
	m_pReflectSRV     = TNULL;
	m_pReflectTexture = TNULL;
}

// $deBlob: FUNCTION 00700c40
// $deBlob: FUNCTION 00705a10
TWorldShaderHAL::~TWorldShaderHAL()
{
	delete[] m_pInstanceData;

	m_pNoMat->OnDestroy();
	delete m_pNoMat;
	m_pNoMat = TNULL;

	m_pNoMatHeat->OnDestroy();
	delete m_pNoMatHeat;
	m_pNoMatHeat = TNULL;

	m_pNoMatWater->OnDestroy();
	delete m_pNoMatWater;
	m_pNoMatWater = TNULL;

	if ( m_pReflectSRV )
	{
		m_pReflectSRV->Release();
		m_pReflectSRV = TNULL;
	}

	if ( m_pReflectTexture )
	{
		m_pReflectTexture->Release();
		m_pReflectTexture = TNULL;
	}
}

// $deBlob: FUNCTION 00700d60
void TWorldShaderHAL::InitWaterTexture()
{
	m_pWaterNoiseTex      = TTextureManager::GetSingleton()->FindTexture( "water_noise.tga" );
	m_pWaterTransitionTex = TTextureManager::GetSingleton()->FindTexture( "watertransition.tga" );
}

// $deBlob: FUNCTION 00700da0
void TWorldShaderHAL::DeinitWaterTexture()
{
	m_pWaterNoiseTex      = TNULL;
	m_pWaterTransitionTex = TNULL;
}

// $deBlob: FUNCTION 00700dc0
void TWorldShaderHAL::MakeReflectTexture()
{
	TRenderDX11*     pRender     = TRenderDX11::Interface();
	ID3D11Texture2D* pMainTexture = pRender->GetMainTexture();

	if ( m_pReflectTexture == TNULL )
	{
		D3D11_TEXTURE2D_DESC oTextureDesc;
		pMainTexture->GetDesc( &oTextureDesc );
		oTextureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		oTextureDesc.SampleDesc.Count = 1;
		pRender->GetDevice()->CreateTexture2D( &oTextureDesc, TNULL, &m_pReflectTexture );

		if ( m_pReflectTexture != TNULL )
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC oViewDesc;
			TUtil::MemClear( &oViewDesc, sizeof( oViewDesc ) );
			oViewDesc.Format                    = oTextureDesc.Format;
			oViewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
			oViewDesc.Texture2D.MostDetailedMip = 0;
			oViewDesc.Texture2D.MipLevels       = 1;
			pRender->GetDevice()->CreateShaderResourceView( m_pReflectTexture, &oViewDesc, &m_pReflectSRV );
		}
	}

	pRender->GetDeviceContext()->ResolveSubresource( m_pReflectTexture, 0, pMainTexture, 0, DXGI_FORMAT_R8G8B8A8_UNORM );
}

// $deBlob: FUNCTION 00700e90
void TWorldShaderHAL::BindReflectTexture( UINT a_uiSlot )
{
	TRenderDX11::Interface()->GetDeviceContext()->PSSetShaderResources( a_uiSlot, 1, &m_pReflectSRV );
	TRenderDX11::Interface()->SetSamplerState( a_uiSlot, 3, TRUE );
}

// $deBlob: FUNCTION 00700ed0
void TWorldShaderHAL::BindNoiseTexture( UINT a_uiSlot )
{
	if ( m_pWaterNoiseTex != TNULL )
		m_pWaterNoiseTex->Bind( a_uiSlot );
}

// $deBlob: FUNCTION 00700ef0
void TWorldShaderHAL::UpdateWater( TFLOAT a_fDeltaTime )
{
	m_vWaterUVOffset1.x += 0.03f * a_fDeltaTime;
	m_vWaterUVOffset1.y += 0.03f * a_fDeltaTime;

	if ( m_vWaterUVOffset1.x > 1.0f )
		m_vWaterUVOffset1.x -= 1.0f;

	if ( m_vWaterUVOffset1.y > 1.0f )
		m_vWaterUVOffset1.y -= 1.0f;

	m_vWaterUVOffset2.x += 0.3f * a_fDeltaTime;
	m_vWaterUVOffset2.y += 0.3f * a_fDeltaTime;

	if ( m_vWaterUVOffset2.x > 1.0f )
		m_vWaterUVOffset2.x -= 1.0f;

	if ( m_vWaterUVOffset2.y > 1.0f )
		m_vWaterUVOffset2.y -= 1.0f;
}

// $deBlob: FUNCTION 00700ff0
void TWorldShaderHAL::SetMemWater( void* a_pMem )
{
	TASSERT( TFALSE );
	TASSERT( a_pMem );
	m_pMemWater = a_pMem;
}

// $deBlob: FUNCTION 00701080
void TWorldShaderHAL::ResetWaterFrame()
{
	s_bWaterFrameReset1 = TTRUE;
	s_bWaterFrameReset2 = TTRUE;
}

// $deBlob: FUNCTION 007010a0
TBOOL TWorldShaderHAL::Create()
{
	TASSERT( TFALSE == IsCreated() );

	TBOOL bResult = m_aOrderTables[ 0 ].Create( this, 5190, 0 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 1 ].Create( this, 5210, 1 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 2 ].Create( this, -5000, 2 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 3 ].Create( this, -4800, 3 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 4 ].Create( this, 800, 4 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 5 ].Create( this, 820, 5 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 6 ].Create( this, 840, 6 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 7 ].Create( this, 860, 7 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 8 ].Create( this, 5300, 8 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 9 ].Create( this, -350, 9 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 10 ].Create( this, 5200, 10 );
	TASSERT( bResult );
	bResult = m_aOrderTables[ 11 ].Create( this, -4900, 11 );
	TASSERT( bResult );

	m_iNumOrderTables = NUM_ORDERTABLES;
	m_pUnk204         = TNULL;

	Validate();
	TShader::Create();

	TTextureManager* pTextureManager = TTextureManager::GetSingleton();

	m_pNoMat = TSTATICCAST( TWorldMaterialHAL, CreateMaterial( "no_mat" ) );
	m_pNoMat->Create( 0 );
	m_pNoMat->SetTexture( 0, pTextureManager->GetWhiteTexture() );
	m_pNoMat->SetTexture( 1, pTextureManager->GetWhiteTexture() );
	m_pNoMat->SetTextureNum( 2 );

	m_pNoMatWater = TSTATICCAST( TWorldMaterialHAL, CreateMaterial( "no_mat_water" ) );
	m_pNoMatWater->Create( 9 );
	m_pNoMatWater->SetOrderTable( &m_aOrderTables[ 1 ], 0, TTRUE );
	m_pNoMatWater->SetTexture( 0, pTextureManager->GetWhiteTexture() );
	m_pNoMatWater->SetTextureNum( 1 );

	m_pNoMatHeat = TSTATICCAST( TWorldMaterialHAL, CreateMaterial( "no_mat_heat" ) );
	m_pNoMatHeat->Create( 23 );
	m_pNoMatHeat->SetOrderTable( &m_aOrderTables[ 1 ], 0, TTRUE );
	m_pNoMatHeat->SetTexture( 0, pTextureManager->GetWhiteTexture() );
	m_pNoMatHeat->SetTextureNum( 1 );

	for ( TINT i = 0; i < m_iNumOrderTables; i++ )
	{
		// NOTE: the materials keep a pointer to this buffer as their name (CreateMaterial doesn't copy it), see
		//; nothing looks them up by name
		TCHAR szName[ 16 ];
		TStringManager::String8Format( szName, sizeof( szName ), "inst_mat_%d", i );

		m_pInstanceRenderMats[ i ] = TSTATICCAST( TWorldMaterialHAL, CreateMaterial( szName ) );
		m_pInstanceRenderMats[ i ]->Create( 0 );
		m_pInstanceRenderMats[ i ]->SetOrderTable( &m_aOrderTables[ i ], 0, TTRUE );
		m_pInstanceRenderMats[ i ]->SetFlags( 0x100, TTRUE );
	}

	m_oShaderCache.Create(
	    "Data\\Shaders\\World\\",
	    s_pszWorldShaderVS,
	    s_pszWorldShaderPS,
	    TShaderCache_DX11::t_CompileVS( CompileVertexShader ),
	    TShaderCache_DX11::t_CreateInputLayout( CreateInputLayout ),
	    TShaderCache_DX11::t_CompilePS( CompilePixelShader )
	);

	return TTRUE;
}

// $deBlob: FUNCTION 007016f0
void TWorldShaderHAL::OnDestroy()
{
	TASSERT( TTRUE == IsCreated() );
	TShader::OnDestroy();

	for ( TINT i = 0; i < m_iNumOrderTables; i++ )
	{
		m_pInstanceRenderMats[ i ]->OnDestroy();
		delete m_pInstanceRenderMats[ i ];
	}
}

// $deBlob: FUNCTION 00701780
TWorldMesh* TWorldShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	TWorldMeshHAL* pMesh = ( s_pWorldMemHeap != TNULL ) ? new ( s_pWorldMemHeap ) TWorldMeshHAL() : new TWorldMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}

// $deBlob: FUNCTION 00701840
void TWorldShaderHAL::StartFlush()
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	s_NumTevStages = 1;
	pRender->SetVec4InVSBuffer( 8, &TMatrix44::IDENTITY, 4 );
	pRender->SetColorUpdate( TTRUE );
	pRender->SetAlphaUpdate( TTRUE );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
	s_bLazyFog = TFALSE;
	s_vFogStartEnd.z = 0.0f;
	pRender->SetDstAlpha( 0.0f );

	for ( TINT i = 0; i < 8; i++ )
	{
		s_aiLazyMultiPassBlendTypes[ i ] = -1;
		s_apLazyTextures[ i ] = TNULL;
	}

	for ( TINT i = 0; i < 8; i++ )
		s_auiLazyUserData2[ i ] = 0xFF;

	s_iLazyBlendMode = -1;
	s_iLazyNumTevStages = 1;
	s_bLightingSet = TFALSE;
	s_uiLazyAlpha    = 0xFF;

	TVector4 vOne( 1.0f, 1.0f, 1.0f, 1.0f );
	pRender->SetVec4InPSBuffer( 8, &vOne, 1 );
	pRender->SetVec4InPSBuffer( 10, &vOne, 1 );

	for ( TINT i = 0; i < 7; i++ )
		s_aiLazyUnk98246C[ i ] = -1;

	s_iReflectionSet = 0;
	s_iLighting = 0;
}

// $deBlob: FUNCTION 00701a70
void TWorldShaderHAL::EndFlush()
{
	TRenderDX11* pRender = TRenderDX11::Interface();
	pRender->SetDstAlpha( 0.0f );
	s_vFogStartEnd.z = 0.0f;
	pRender->SetColorUpdate( TTRUE );

	TUINT uiNumInstances = m_uiNumInstances;
	m_uiNumInstances     = 0;

	if ( uiNumInstances > s_iMaxOfCurNumInstances )
		s_iMaxOfCurNumInstances = uiNumInstances;
}

// $deBlob: FUNCTION 00701ac0
void TWorldShaderHAL::Render( TRenderPacket* a_pRenderPacket )
{
	if ( s_fnRenderCallback != TNULL && s_fnRenderCallback( a_pRenderPacket ) )
		return;

	TASSERT( a_pRenderPacket->GetMesh()->IsA( &TGetClass( TWorldMeshHAL ) ) );

	TMaterial*     pPacketMaterial = a_pRenderPacket->GetMaterial();
	TWorldMeshHAL* pMesh           = TSTATICCAST( TWorldMeshHAL, a_pRenderPacket->GetMesh() );
	TRenderDX11*   pRender         = TRenderDX11::Interface();
	TTRBWorldMesh* pTRBMesh        = pMesh->m_pTRBMesh;

	TASSERT( ( pPacketMaterial == TNULL ) || pPacketMaterial->IsA( &TGetClass( TWorldMaterialHAL ) ) );
	TWorldMaterialHAL* pMaterial = TSTATICCAST( TWorldMaterialHAL, pPacketMaterial );

	TRenderPacket::t_RenderCallback fnCallback = a_pRenderPacket->GetRenderCallback();
	TUINT8                          uiFade     = a_pRenderPacket->GetFade();
	void*                           pUserData  = a_pRenderPacket->GetRenderUserData();

	if ( fnCallback != TNULL )
		fnCallback( a_pRenderPacket, pUserData, TTRUE );

	// The fade of the packet
	if ( s_abLazyForce[ 8 ] || uiFade != s_uiLazyAlpha )
	{
		TVector4 vFade( 1.0f, 1.0f, 1.0f, TFLOAT( uiFade ) / 255.0f );
		pRender->SetVec4InPSBuffer( 8, &vFade, 1 );
		pRender->SetVec4InPSBuffer( 10, &vFade, 1 );
		s_uiLazyAlpha = uiFade;
	}

	TWorldMaterial::BLENDMODE eBlendMode = pMaterial->m_eBlendMode;

	if ( eBlendMode == TWorldMaterial::BLENDMODE_OPAQUE )
	{
		s_vAlphaTest.x = 1.0f;

		if ( uiFade >= 0xFE )
		{
			s_vAlphaTest.y = TFLOAT( pMaterial->m_uiAlpha ) / 255.0f;
			pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
		}
		else
		{
			s_vAlphaTest.y = ( TFLOAT( uiFade ) * 0.5f ) / 255.0f;
			pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
		}
	}
	else if ( eBlendMode != TWorldMaterial::BLENDMODE_WATER && eBlendMode != TWorldMaterial::BLENDMODE_SHOCKWAVE )
	{
		s_vAlphaTest.x = 1.0f;
		s_vAlphaTest.y = TFLOAT( pMaterial->m_uiAlpha ) / 255.0f;
	}

	TVector4 vColour( 1.0f, 1.0f, 1.0f, 1.0f );
	TFLOAT   fShade = a_pRenderPacket->GetAlpha();
	TVector4 vShade( fShade, fShade, fShade, 1.0f );

	TWorldMeshInstanceData* pInstance = pMesh->m_prev;

	if ( pInstance == TNULL )
		pInstance = pMesh->m_current;

	TBOOL bHasCurrent = pMesh->m_current != TNULL;
	TBOOL bListed1    = TFALSE;

	if ( pInstance != TNULL )
	{
		// The instances of the mesh
		TTRBWorldMesh oMesh;
		TUtil::MemCopy( &oMesh, pTRBMesh, sizeof( TTRBWorldMesh ) );

		TBOOL bFirst = TTRUE;

		do
		{
			if ( pMesh->m_current == TNULL && ( pInstance->m_uiFlags & TWorldMeshInstanceData::FLAGS_OPT_RENDERLIST_2 ) )
			{
				pMesh->m_current = pInstance;

				if ( bListed1 )
					break;

				bHasCurrent = TTRUE;
			}

			if ( pInstance->m_uiFlags & TWorldMeshInstanceData::FLAGS_OPT_RENDERLIST_1 )
				bListed1 = TTRUE;

			TWorldMeshInstanceData::Source* pSource           = pInstance->m_pSource;
			TWorldMaterialHAL*              pInstanceMaterial = pSource->m_pMaterial;
			pInstanceMaterial->PreRender();

			vColour = pInstance->m_vAmbient;

			if ( !s_bUnk9B241D && ( pInstanceMaterial->m_aMultiPassBlendTypes[ 0 ] == TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPCOLOUR2 || pInstanceMaterial->m_aMultiPassBlendTypes[ 0 ] == TWorldMaterialHAL::MULTIPASSBLENDTYPE_WINDOWS ) )
			{
				vColour.w = 1.0f;

				TVector4 vColour5;
				vColour5.Multiply( vColour, TWorldShaderHAL::s_vLightingScale5 );
				TVector4 vColour6;
				vColour6.Multiply( vColour, TWorldShaderHAL::s_vLightingScale6 );

				pRender->SetVec4InPSBuffer( 1, &vShade, 1 );
				pRender->SetVec4InPSBuffer( 5, &vColour5, 1 );
				pRender->SetVec4InPSBuffer( 6, &vColour6, 1 );
			}
			else
			{
				pRender->SetVec4InPSBuffer( 1, &vColour, 1 );
			}

			Render( &oMesh, pMaterial, &pInstance->m_oModelView, bFirst, pTRBMesh, pInstanceMaterial, pSource->m_pVertices, pSource->m_pIndices, pSource->m_uiCount );
			pInstanceMaterial->PostRender();

			pInstance->m_uiFlags &= ~( TWorldMeshInstanceData::FLAGS_OPT_RENDERLIST_1 | TWorldMeshInstanceData::FLAGS_OPT_RENDERLIST_2 );

			TWorldMeshInstanceData* pNext = pInstance->m_prev;
			s_bLightingSet                = TTRUE;
			pInstance->m_prev             = TNULL;
			bFirst                        = TFALSE;
			pInstance                     = pNext;
		} while ( pInstance != TNULL );

		pMesh->m_next = TNULL;
		pMesh->m_prev = TNULL;

		if ( !bHasCurrent )
		{
			pMesh->m_pTRBMesh->m_uiFlags &= ~TTRBWorldMesh::Flags_QueuedOpaque;
		}
		else
		{
			pMesh->m_current = TNULL;
			pMesh->m_pTRBMesh->m_uiFlags &= ~TTRBWorldMesh::Flags_QueuedOpaque;
			pMesh->m_pTRBMesh->m_uiFlags &= ~TTRBWorldMesh::Flags_QueuedInstance;
		}
	}
	else
	{
		if ( s_bUnk9B241D || pMaterial->m_aMultiPassBlendTypes[ 0 ] != TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPCOLOUR2 )
		{
			vColour.x = a_pRenderPacket->GetAmbientColour().x;
			vColour.y = a_pRenderPacket->GetAmbientColour().y;
			vColour.z = a_pRenderPacket->GetAmbientColour().z;
			pRender->SetVec4InPSBuffer( 1, &vColour, 1 );
		}
		else
		{
			vColour.w = 1.0f;

			TVector4 vColour5;
			vColour5.Multiply( vColour, TWorldShaderHAL::s_vLightingScale5 );
			TVector4 vColour6;
			vColour6.Multiply( vColour, TWorldShaderHAL::s_vLightingScale6 );

			pRender->SetVec4InPSBuffer( 1, &vShade, 1 );
			pRender->SetVec4InPSBuffer( 5, &vColour5, 1 );
			pRender->SetVec4InPSBuffer( 6, &vColour6, 1 );
		}

		Render( pTRBMesh, pMaterial, &a_pRenderPacket->GetModelViewMatrix(), TTRUE, TNULL, TNULL, TNULL, TNULL, 0 );
		s_bLightingSet = TTRUE;
	}

	if ( fnCallback != TNULL )
		fnCallback( a_pRenderPacket, pUserData, TFALSE );
}

// $deBlob: FUNCTION 00701fb0
TWorldMaterial* TWorldShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	TASSERT( TTRUE == IsCreated() );

	TWorldMaterialHAL* pMaterial = ( s_pWorldMemHeap != TNULL ) ? new ( s_pWorldMemHeap ) TWorldMaterialHAL() : new TWorldMaterialHAL();
	pMaterial->SetShader( this );
	pMaterial->SetMaterialName( a_szName );

	return pMaterial;
}

// $deBlob: FUNCTION 00702080
void TWorldShaderHAL::SetMaxInstances( TUINT a_uiMaxInstances )
{
	s_uiMaxInstances = a_uiMaxInstances;
}

// $deBlob: FUNCTION 00702090
void TWorldShaderHAL::Render( TTRBWorldMesh* a_pMesh, TWorldMaterialHAL* a_pMaterial, const TMatrix44* a_pModelView, TBOOL a_bFirst, TTRBWorldMesh* a_pOrigMesh, TWorldMaterialHAL* a_pInstanceMaterial, const void* a_pVertices, const void* a_pIndices, TUINT16 a_uiCount )
{
	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	// The vertex shader permutation: the texture coordinate generators PreRender picked
	VertexShaderId oVSId;
	oVSId.uiFlags = s_auiTexGenFlagsLo[ s_aiTexCoordGen[ 0 ] ] | s_auiTexGenFlagsHi[ s_aiTexCoordGen[ 1 ] ];
	oVSId.uiTexGen23 = s_auiTexGenFlagsLo[ s_aiTexCoordGen[ 2 ] ] | s_auiTexGenFlagsHi[ s_aiTexCoordGen[ 3 ] ];
	oVSId.uiTexGen4  = s_auiTexGenFlagsLo[ s_aiTexCoordGen[ 4 ] ];

	if ( a_pVertices != TNULL )
		oVSId.uiTexGen4 |= VertexShaderId::Flags_HasVertices;

	if ( s_vFogStartEnd.z > 0.0f )
		oVSId.uiTexGen4 |= VertexShaderId::Flags_Fog;

	ID3D11VertexShader* pVertexShader = TNULL;
	ID3D11InputLayout*  pInputLayout  = TNULL;

	if ( !m_oShaderCache.FindVertexShader( oVSId, pVertexShader, pInputLayout ) )
	{
		ID3DBlob* pBlob = CompileVertexShader( oVSId );
		pInputLayout    = CreateInputLayout( oVSId, pBlob );
		pRender->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pVertexShader );
		m_oShaderCache.AddVertexShader( oVSId, pVertexShader, pInputLayout, pBlob->GetBufferPointer(), pBlob->GetBufferSize() );
		pBlob->Release();
	}

	TASSERT( pInputLayout );
	pDeviceContext->IASetInputLayout( pInputLayout );
	TASSERT( pVertexShader );
	pDeviceContext->VSSetShader( pVertexShader, TNULL, 0 );

	// The pixel shader permutation: the blend types of the passes (of the instance's material if there's one)
	TWorldMaterialHAL* pStagesMaterial = ( a_pInstanceMaterial != TNULL ) ? a_pInstanceMaterial : a_pMaterial;

	for ( TINT i = 0; i < s_NumTevStages; i++ )
	{
		if ( pStagesMaterial->m_aMultiPassBlendTypes[ i ] == TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPTEXLM2 || pStagesMaterial->m_aMultiPassBlendTypes[ i ] == TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPTEX )
		{
			s_NumTevStages--;
			break;
		}
	}

	PixelShaderId oPSId;
	oPSId.aStages[ 0 ] = -1;
	oPSId.aStages[ 1 ] = -1;
	oPSId.aStages[ 2 ] = -1;
	oPSId.aStages[ 3 ] = -1;
	oPSId.aStages[ 4 ] = -1;
	oPSId.uiFlags      = 0;

	TASSERT( s_NumTevStages < 6 );

	for ( TINT i = 0; i < s_NumTevStages; i++ )
		oPSId.aStages[ i ] = TINT8( pStagesMaterial->m_aMultiPassBlendTypes[ i ] );

	oPSId.uiFlags |= s_auiLightingFlags[ s_iLighting ] | s_auiIndirectFlags[ s_iIndirect ];

	if ( s_vFogStartEnd.z > 0.0f )
		oPSId.uiFlags |= PixelShaderId::Flags_Fog;

	ID3D11PixelShader* pPixelShader = TNULL;

	if ( !m_oShaderCache.FindPixelShader( oPSId, pPixelShader ) )
	{
		ID3DBlob* pBlob = CompilePixelShader( oPSId );
		pRender->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pPixelShader );
		m_oShaderCache.AddPixelShader( oPSId, pPixelShader, pBlob->GetBufferPointer(), pBlob->GetBufferSize() );
		pBlob->Release();
	}

	TASSERT( pPixelShader );
	pDeviceContext->PSSetShader( pPixelShader, TNULL, 0 );

	pRender->SetVec4InPSBuffer( 0, &s_vAlphaTest, 1 );

	if ( s_vFogStartEnd.z > 0.0f )
	{
		pRender->SetVec4InVSBuffer( 36, &s_vFogStartEnd, 1 );
		pRender->SetVec4InPSBuffer( 22, &s_vFogColour, 1 );
	}

	if ( s_abLazyForce[ 12 ] || a_bFirst || !s_oLazyModelView.IsEqual( *a_pModelView ) )
	{
		s_oLazyModelView = *a_pModelView;

		// World view projection and model view
		TRenderContextDX11* pContext = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );
		TMatrix44           oWVP     = MultiplyMatrices( *a_pModelView, pContext->GetProjectionMatrix() );
		pRender->SetVec4InVSBuffer( 0, &oWVP, 4 );
		pRender->SetVec4InVSBuffer( 4, a_pModelView, 4 );

		// texMtx7: the environment map coordinates (the view space normals scaled into [0;1])
		TMatrix44 oNormalMatrix = *a_pModelView;
		oNormalMatrix.m_f41     = 0.0f;
		oNormalMatrix.m_f42     = 0.0f;
		oNormalMatrix.m_f43     = 0.0f;

		TMatrix44 oScale;
		oScale.Set( 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f );

		TMatrix44 oBias;
		oBias.Set( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f );

		TMatrix44 oEnvMatrix = MultiplyMatrices( MultiplyMatrices( oNormalMatrix, oScale ), oBias );
		pRender->SetVec4InVSBuffer( 32, &oEnvMatrix, 4 );

		if ( ( s_abLazyForce[ 9 ] || s_iReflectionSet == 0 ) && ( a_pMaterial->m_eBlendMode == TWorldMaterial::BLENDMODE_REFLECTIVEWATER || a_pMaterial->m_eBlendMode == 23 ) )
		{
			// texMtx3 (c16): the screen space coordinates of the reflection texture
			s_iReflectionSet = 1;

			TRenderContext*         pCurrentContext = pRender->GetCurrentContext();
			TRender::DISPLAYPARAMS* pDisplayParams  = pRender->GetCurrentDisplayParams();
			TFLOAT                  fWidth          = TFLOAT( TINT( pDisplayParams->uiWidth ) );
			TFLOAT                  fHeight         = TFLOAT( TINT( pDisplayParams->uiHeight ) );

			// TMath::ATan2 in double precision
			TFLOAT fFov = TFLOAT( atan2( double( pCurrentContext->GetViewportParameters().fHeight ), double( pCurrentContext->GetProjectionParameters().m_Proj.y * 2.0f ) ) );
			TASSERT( TMath::IsFinite( fFov ) );
			fFov *= 2.0f;

			TFLOAT fAspect = ( pRender->GetAspectRatio() == TRender::ASPECT_RATIO_16_9 ) ? 1.7777778f : 1.3333334f;

			TMatrix44 oProjection = PerspectiveFovLH( fFov, fAspect, 0.0f, 1.0f );

			TMatrix44 oScreenScale;
			oScreenScale.Set( 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f );

			TMatrix44 oScreenBias;
			oScreenBias.Set( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f );

			TMatrix44 oReflection = MultiplyMatrices( *a_pModelView, MultiplyMatrices( MultiplyMatrices( oProjection, oScreenScale ), oScreenBias ) );
			pRender->SetVec4InVSBuffer( 16, &oReflection, 4 );

			TWorldShaderHAL* pWorldShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
			pRender->GetDeviceContext()->PSSetShaderResources( 0, 1, &pWorldShader->m_pReflectSRV );
			pRender->SetSamplerState( 0, 3, TRUE );

			if ( pWorldShader->m_pWaterNoiseTex != TNULL )
				pWorldShader->m_pWaterNoiseTex->Bind( 1 );

			TVector4 vScreenSize( fWidth, fHeight, 0.0f, 0.0f );
			pRender->SetVec4InPSBuffer( 26, &vScreenSize, 1 );
		}
	}

	// texMtx6: the projected texture of the material (of the instance's if it has one)
	TWorldMaterialHAL* pProjMat = a_pInstanceMaterial;

	if ( a_pInstanceMaterial == TNULL || !( a_pInstanceMaterial->GetFlags() & TMaterial::FLAGS_PROJECTEDTEXTURE ) )
	{
		pProjMat = TNULL;

		if ( a_pMaterial->GetFlags() & TMaterial::FLAGS_PROJECTEDTEXTURE )
			pProjMat = a_pMaterial;
	}

	if ( pProjMat != TNULL )
	{
		TASSERT( pProjMat->m_iUserDataIndices[ TWorldMaterialHAL::UData_0 ] > -1 );
		ProjTexture& rProjTexture = GetProjTexture( pProjMat->m_iUserDataIndices[ TWorldMaterialHAL::UData_0 ] );

		TMatrix44 oProjMatrix = MultiplyMatrices( rProjTexture.m_oMatrix1, rProjTexture.m_oMatrix2 );
		pRender->SetVec4InVSBuffer( 28, &oProjMatrix, 4 );
	}

	// NOTE: TTRBWorldMesh::RenderMesh inlined
	a_pMesh->RenderMesh( a_pVertices, a_pIndices, a_uiCount );
}

// Index of the first entry of a_puiFlags[ 0..a_iLast ] that has a bit of a_uiFlags, 0 if none has
static TINT FindFlagIndex( const TUINT32* a_puiFlags, TINT a_iLast, TUINT32 a_uiFlags )
{
	for ( TINT i = 0; i <= a_iLast; i++ )
	{
		if ( a_puiFlags[ i ] & a_uiFlags )
			return i;
	}

	return 0;
}

// $deBlob: FUNCTION 00705880
static TINT GetTexGenIndexLo( TUINT32 a_uiFlags )
{
	return FindFlagIndex( s_auiTexGenFlagsLo, TARRAYSIZE( s_auiTexGenFlagsLo ) - 1, a_uiFlags );
}

// $deBlob: FUNCTION 007058b0
static TINT GetTexGenIndexHi( TUINT32 a_uiFlags )
{
	return FindFlagIndex( s_auiTexGenFlagsHi, TARRAYSIZE( s_auiTexGenFlagsHi ) - 1, a_uiFlags );
}

// $deBlob: FUNCTION 007058e0
static TINT GetIndirectIndex( TUINT32 a_uiFlags )
{
	return FindFlagIndex( s_auiIndirectFlags, TARRAYSIZE( s_auiIndirectFlags ) - 1, a_uiFlags );
}

// Index of the first of a_puiFlags[ 0..a_iLast ] that has a bit of a_uiFlags, else of the last zero entry
static TINT FindFlagIndexOrZero( TUINT32 a_uiFlags, TINT a_iLast, const TUINT32* a_puiFlags )
{
	TINT iZeroFlag = -1;

	for ( TINT i = 0; i <= a_iLast; i++ )
	{
		if ( a_puiFlags[ i ] & a_uiFlags )
			return i;

		if ( a_puiFlags[ i ] == 0 )
			iZeroFlag = i;
	}

	TASSERT( iZeroFlag >= 0 );
	return iZeroFlag;
}

// $deBlob: FUNCTION 00702c10
ID3DBlob* __stdcall TWorldShaderHAL::CompileVertexShader( const VertexShaderId& a_rId )
{
	TINT iTexGen0 = GetTexGenIndexLo( a_rId.uiFlags );
	TINT iTexGen1 = GetTexGenIndexHi( a_rId.uiFlags );
	TINT iTexGen2 = GetTexGenIndexLo( a_rId.uiTexGen23 );
	TINT iTexGen3 = GetTexGenIndexHi( a_rId.uiTexGen23 );
	TINT iTexGen4 = GetTexGenIndexLo( a_rId.uiTexGen4 );

	D3D_SHADER_MACRO aMacros[ 7 ];
	aMacros[ 0 ].Definition = s_aszTexGenFunctions[ iTexGen0 ];
	aMacros[ 1 ].Definition = s_aszTexGenFunctions[ iTexGen1 ];
	aMacros[ 2 ].Definition = s_aszTexGenFunctions[ iTexGen2 ];
	aMacros[ 3 ].Definition = s_aszTexGenFunctions[ iTexGen3 ];
	aMacros[ 4 ].Definition = s_aszTexGenFunctions[ iTexGen4 ];
	aMacros[ 5 ].Definition = "FogFactorEmpty";

	if ( a_rId.uiTexGen4 & VertexShaderId::Flags_Fog )
		aMacros[ 5 ].Definition = s_pszFogFactorExponentialSquared;

	aMacros[ 0 ].Name       = "TexCoordGen0";
	aMacros[ 1 ].Name       = "TexCoordGen1";
	aMacros[ 2 ].Name       = "TexCoordGen2";
	aMacros[ 3 ].Name       = "TexCoordGen3";
	aMacros[ 4 ].Name       = "TexCoordGen4";
	aMacros[ 5 ].Name       = "ComputeFogFactor";
	aMacros[ 6 ].Name       = TNULL;
	aMacros[ 6 ].Definition = TNULL;

	return TRenderDX11::CompileShader( s_pszWorldShaderVS, "main", "vs_4_0_level_9_3", aMacros );
}

// $deBlob: FUNCTION 00702d60
ID3D11InputLayout* __stdcall TWorldShaderHAL::CreateInputLayout( const VertexShaderId& a_rId, ID3DBlob* a_pShaderBlob )
{
	D3D11_INPUT_ELEMENT_DESC aInputDescs[ 5 ];
	aInputDescs[ 0 ].SemanticName         = "POSITION";
	aInputDescs[ 0 ].SemanticIndex        = 0;
	aInputDescs[ 0 ].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
	aInputDescs[ 0 ].InputSlot            = 0;
	aInputDescs[ 0 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDescs[ 0 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDescs[ 0 ].InstanceDataStepRate = 0;
	aInputDescs[ 1 ].SemanticName         = "NORMAL";
	aInputDescs[ 1 ].SemanticIndex        = 0;
	aInputDescs[ 1 ].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
	aInputDescs[ 1 ].InputSlot            = 0;
	aInputDescs[ 1 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDescs[ 1 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDescs[ 1 ].InstanceDataStepRate = 0;
	aInputDescs[ 2 ].SemanticName         = "COLOR";
	aInputDescs[ 2 ].SemanticIndex        = 0;
	aInputDescs[ 2 ].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
	aInputDescs[ 2 ].InputSlot            = 0;
	aInputDescs[ 2 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDescs[ 2 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDescs[ 2 ].InstanceDataStepRate = 0;
	aInputDescs[ 3 ].SemanticName         = "TEXCOORD";
	aInputDescs[ 3 ].SemanticIndex        = 0;
	aInputDescs[ 3 ].Format               = DXGI_FORMAT_R32G32_FLOAT;
	aInputDescs[ 3 ].InputSlot            = 0;
	aInputDescs[ 3 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDescs[ 3 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDescs[ 3 ].InstanceDataStepRate = 0;
	aInputDescs[ 4 ].SemanticName         = "TEXCOORD";
	aInputDescs[ 4 ].SemanticIndex        = 1;
	aInputDescs[ 4 ].Format               = DXGI_FORMAT_R32G32_FLOAT;
	aInputDescs[ 4 ].InputSlot            = 0;
	aInputDescs[ 4 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDescs[ 4 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDescs[ 4 ].InstanceDataStepRate = 0;

	ID3D11InputLayout* pInputLayout = TNULL;
	TRenderDX11::Interface()->GetDevice()->CreateInputLayout( aInputDescs, 5, a_pShaderBlob->GetBufferPointer(), a_pShaderBlob->GetBufferSize(), &pInputLayout );

	return pInputLayout;
}

// $deBlob: FUNCTION 00702ed0
ID3DBlob* __stdcall TWorldShaderHAL::CompilePixelShader( const PixelShaderId& a_rId )
{
	D3D_SHADER_MACRO aMacros[ 9 ];
	aMacros[ 0 ].Name       = "Pass0";
	aMacros[ 1 ].Name       = "Pass1";
	aMacros[ 1 ].Definition = "PassEmpty";
	aMacros[ 2 ].Name       = "Pass2";
	aMacros[ 2 ].Definition = "PassEmpty";
	aMacros[ 0 ].Definition = s_aszPassFunctions[ a_rId.aStages[ 0 ] ];
	aMacros[ 3 ].Name       = "Pass3";
	aMacros[ 3 ].Definition = "PassEmpty";
	aMacros[ 4 ].Name       = "Pass4";
	aMacros[ 4 ].Definition = "PassEmpty";
	aMacros[ 5 ].Name       = "Lighting0";
	aMacros[ 5 ].Definition = TNULL;
	aMacros[ 6 ].Name       = "ComputeFog";
	aMacros[ 6 ].Definition = "FogEmpty";
	aMacros[ 7 ].Name       = "IndirectLookup0";
	aMacros[ 7 ].Definition = "IndirectLookupEmpty";
	aMacros[ 8 ].Name       = TNULL;
	aMacros[ 8 ].Definition = TNULL;

	TINT iIndirect = GetIndirectIndex( a_rId.uiFlags );

	TINT numTevStages = ( a_rId.aStages[ 0 ] != -1 ) ? 1 : 0;

	if ( a_rId.aStages[ 1 ] != -1 )
		numTevStages = 2;

	if ( a_rId.aStages[ 2 ] != -1 )
		numTevStages = 3;

	if ( a_rId.aStages[ 3 ] != -1 )
		numTevStages = 4;

	if ( a_rId.aStages[ 4 ] != -1 )
		numTevStages = 5;

	for ( TINT i = 1; i < numTevStages; i++ )
		aMacros[ i ].Definition = s_aszPassFunctions[ a_rId.aStages[ i ] ];

	if ( iIndirect != 0 )
	{
		if ( iIndirect == 2 || iIndirect == 3 )
		{
			aMacros[ 7 ].Definition = s_aszIndirectFunctions[ iIndirect ];
		}
		else if ( iIndirect == 1 )
		{
			TASSERT( numTevStages == 2 );
			aMacros[ 1 ].Definition = s_aszIndirectFunctions[ 1 ];
		}
		else
		{
			aMacros[ 0 ].Definition = s_aszIndirectFunctions[ iIndirect ];
		}
	}

	aMacros[ 5 ].Definition = s_aszLightingFunctions[ FindFlagIndexOrZero( a_rId.uiFlags, 1, s_auiLightingFlags ) ];

	if ( TUINT8( a_rId.uiFlags ) & PixelShaderId::Flags_Fog )
		aMacros[ 6 ].Definition = s_pszFogLinear;

	return TRenderDX11::CompileShader( s_pszWorldShaderPS, "main", "ps_4_0", aMacros );
}

//-----------------------------------------------------------------------------
// TWorldMaterialHAL
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 007030b0
// $deBlob: FUNCTION 007030c0
// $deBlob: FUNCTION 007030e0
// $deBlob: FUNCTION 00703100
// $deBlob: FUNCTION 00703110
// $deBlob: FUNCTION 0053fa10
// $deBlob: FUNCTION 008594f0
TDEFINE_CLASS( TWorldMaterialHAL );

// $deBlob: FUNCTION 00703120
TWorldMaterialHAL::TWorldMaterialHAL()
{
	m_pOrderTable = TNULL;
	m_uiPass      = 0;

	for ( TINT i = 0; i < s_iMaxPasses; i++ )
	{
		SetUVMode( i, UVMODE_STREAM );
		m_aMultiPassBlendTypes[ i ] = MULTIPASSBLENDTYPE_MODULATE;
		m_iUVStream[ i ] = 0;
		m_afUOffset[ i ] = 0.0f;
		m_afVOffset[ i ] = 0.0f;
	}

	m_aMultiPassBlendTypes[ 0 ] = MULTIPASSBLENDTYPE_OVERRIDE;
	m_uiAlpha      = 0x80;

	m_iUserDataIndices[ UData_0 ] = -1;
	m_iUserDataIndices[ UData_1 ] = -1;
	m_iUserDataIndices[ UData_2 ] = -1;
}

// $deBlob: FUNCTION 007059b0
TWorldMaterialHAL::~TWorldMaterialHAL()
{
}

// $deBlob: FUNCTION 00703220
TBOOL TWorldMaterialHAL::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );

	// NOTE: TWorldMaterial::Create( a_eBlendMode ) inlined
	SetBlendMode( a_eBlendMode );
	return TMaterial::Create();
}

// $deBlob: FUNCTION 00703250
void TWorldMaterialHAL::OnDestroy()
{
	TWorldShader::t_MaterialDestroyedCallback fnCallback = TWorldShader::GetSingleton()->GetMaterialDestroyedCallback();

	if ( fnCallback != TNULL )
		fnCallback( this );

	if ( m_pOrderTable != TNULL )
	{
		if ( m_pRegMaterial != TNULL )
			TOrderTable::DeregisterMaterial( m_pRegMaterial );

		m_pOrderTable = TNULL;
	}

	TMaterial::OnDestroy();
}

// $deBlob: FUNCTION 00703290
void TWorldMaterialHAL::PreRender()
{
	if ( s_fnPreRenderCallback != TNULL && s_fnPreRenderCallback( this ) )
		return;

	// The instance materials are set up by their instances' materials (TWorldShaderHAL::Render)
	if ( TWorldShaderHAL::s_bInstancing && ( m_Flags & FLAGS_INSTANCE ) )
		return;

	TRenderDX11*     pRender      = TRenderDX11::Interface();
	TWorldShaderHAL* pWorldShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );

	s_iIndirect = 0;

	TINT  iTevStagesUsed   = 0;
	TINT  iCurrentTexStage = 0;
	TINT  iCurrentTexMap   = 0;
	TBOOL bAlphaTexture    = TFALSE;

	for ( TINT iIndex = 0; iIndex < TINT( m_iNumTex ); iIndex++ )
	{
		TASSERT( iIndex < MAXTEXTURES );
		TTexture* pTex = m_pTextures[ iIndex ];

		if ( pTex != TNULL )
		{
			if ( bAlphaTexture || ( pTex->GetFlags1() & TTexture::Flags1_Alpha ) )
				bAlphaTexture = TTRUE;

			if ( s_abLazyForce[ 0 ] || s_apLazyTextures[ iIndex ] != pTex )
			{
				pTex->Bind( iCurrentTexMap );
				s_apLazyTextures[ iIndex ] = pTex;
			}

			UVMODE eUVMode = m_aUVModes[ iIndex ];
			iCurrentTexMap++;

			// The texture coordinates
			switch ( eUVMode )
			{
				case UVMODE_STREAM:
					TASSERT( iIndex < 5 && m_iUVStream[ iIndex ] < 2 );
					s_aiTexCoordGen[ iIndex ] = s_aiTexGenIdentity[ m_iUVStream[ iIndex ] ];
					break;
				case UVMODE_OFFSET:
				{
					TASSERT( iIndex < 2 );

					TMatrix44 oMatrix = Translation( m_afUOffset[ iIndex ], m_afVOffset[ iIndex ], 0.0f );
					pRender->SetVec4InVSBuffer( s_aiTexMtxRegisters[ iIndex ], &oMatrix, 4 );
					s_aiTexCoordGen[ iIndex ] = s_aiTexGenMatrix[ 4 * m_iUVStream[ iIndex ] + iIndex ];
					break;
				}
				case UVMODE_ENVMAP:
					TASSERT( iIndex < 5 );
					s_aiTexCoordGen[ iIndex ] = 16;
					break;
				case UVMODE_PROJECTED:
				{
					TASSERT( iIndex < 5 );
					s_aiTexCoordGen[ iIndex ] = 15;
					TASSERT( D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP == pTex->GetWrapS() && D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP == pTex->GetWrapT() );
					TASSERT( m_iUserDataIndices[ UData_0 ] > -1 );

					TVector4 vColour;
					UnpackColour( pWorldShader->GetProjTexture( m_iUserDataIndices[ UData_0 ] ).m_uiColour, vColour );
					pRender->SetVec4InPSBuffer( 2, &vColour, 1 );
					break;
				}
				case UVMODE_TEXSCALE:
				{
					TASSERT( m_iUserDataIndices[ UData_1 ] > -1 );
					TWorldShaderHAL::TexScale* pTexScale = pWorldShader->GetTexScale( m_iUserDataIndices[ UData_1 ] );
					TASSERT( iIndex < 4 && m_iUVStream[ iIndex ] < 2 );

					TMatrix44 oMatrix;
					oMatrix.Set( pTexScale->m_fScaleU, 0.0f, 0.0f, 0.0f, 0.0f, pTexScale->m_fScaleV, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f );
					pRender->SetVec4InVSBuffer( s_aiTexMtxRegisters[ iIndex ], &oMatrix, 4 );
					s_aiTexCoordGen[ iIndex ] = s_aiTexGenMatrix[ 4 * m_iUVStream[ iIndex ] + iIndex ];
					break;
				}
				default:
					break;
			}

			TASSERT( iIndex == iCurrentTexStage && iIndex == ( iCurrentTexMap - 1 ) );

			// The pass's constants (only when it changed)
			MULTIPASSBLENDTYPE eType = m_aMultiPassBlendTypes[ iIndex ];

			if ( !s_abLazyForce[ 5 ] && TUINT16( m_iUserDataIndices[ UData_2 ] ) == s_auiLazyUserData2[ iIndex ] && eType == s_aiLazyMultiPassBlendTypes[ iIndex ] )
			{
				iCurrentTexStage++;
				iTevStagesUsed++;

				if ( eType == MULTIPASSBLENDTYPE_ADDHALF || eType == MULTIPASSBLENDTYPE_LERPTEX || eType == MULTIPASSBLENDTYPE_LERPTEXLM2 )
				{
					iTevStagesUsed++;
					iCurrentTexStage++;
				}
			}
			else
			{
				s_aiLazyMultiPassBlendTypes[ iIndex ] = eType;
				s_auiLazyUserData2[ iIndex ]          = TUINT8( m_iUserDataIndices[ UData_2 ] );

				switch ( eType )
				{
					case MULTIPASSBLENDTYPE_OVERRIDE:
					case MULTIPASSBLENDTYPE_MODULATE:
					case MULTIPASSBLENDTYPE_ADD:
					case MULTIPASSBLENDTYPE_SUBTRACTIVE:
					case MULTIPASSBLENDTYPE_SHADOW:
					case MULTIPASSBLENDTYPE_PROJECTEDTEX:
					case MULTIPASSBLENDTYPE_WINDOWS:
					case MULTIPASSBLENDTYPE_ADDWRITEALPHA:
					case MULTIPASSBLENDTYPE_WRITEALPHA:
					case MULTIPASSBLENDTYPE_ADDALPHA:
					case MULTIPASSBLENDTYPE_SUBTRACTIVEALPHA:
					case MULTIPASSBLENDTYPE_MODULATEALPHA:
					case MULTIPASSBLENDTYPE_LERPCOLOUR2:
					case MULTIPASSBLENDTYPE_BLENDWRITEALPHA:
						iTevStagesUsed++;
						iCurrentTexStage++;
						break;
					case MULTIPASSBLENDTYPE_BLEND:
						TASSERT( TFALSE );
						iCurrentTexStage++;
						iTevStagesUsed++;
						break;
					case MULTIPASSBLENDTYPE_ADDHALF:
						TASSERT( TFALSE );
						iCurrentTexStage++;
						iTevStagesUsed++;
						break;
					case MULTIPASSBLENDTYPE_WINDOWS2:
					{
						TVector4 vConstant( s_fWindows2Constant, s_fWindows2Constant, s_fWindows2Constant, s_fWindows2Constant );
						pRender->SetVec4InPSBuffer( 4, &vConstant, 1 );
						iCurrentTexStage++;
						iTevStagesUsed++;
						break;
					}
					case MULTIPASSBLENDTYPE_LERPCOLOUR:
					{
						TASSERT( m_iUserDataIndices[ UData_2 ] > -1 );
						TWorldShaderHAL::Colour* pColour = pWorldShader->GetColour( m_iUserDataIndices[ UData_2 ] );

						TVector4 vColour;
						UnpackColour( pColour->m_uiColour1, vColour );
						pRender->SetVec4InPSBuffer( 5, &vColour, 1 );
						UnpackColour( pColour->m_uiColour2, vColour );
						pRender->SetVec4InPSBuffer( 6, &vColour, 1 );
						iCurrentTexStage++;
						iTevStagesUsed++;
						break;
					}
					case MULTIPASSBLENDTYPE_WINDOWSPROJ:
						TASSERT( TFALSE );
						iCurrentTexStage++;
						iTevStagesUsed++;
						break;
					case MULTIPASSBLENDTYPE_LERPCONST:
						pRender->SetVec4InPSBuffer( 2, &TWorldMaterialHAL::s_vMultiPassColour, 1 );
						pRender->SetVec4InPSBuffer( 7, &TWorldMaterialHAL::s_vLerpConstColour, 1 );
						iCurrentTexStage++;
						iTevStagesUsed++;
						break;
					case MULTIPASSBLENDTYPE_LERPTEX:
					{
						TASSERT( 1 == iIndex );
						TINT16 iUserIndex = m_iUserDataIndices[ UData_2 ];
						TASSERT( iUserIndex > -1 );

						TVector4 vColour;
						UnpackColour( pWorldShader->GetCrossfade( iUserIndex ), vColour );
						pRender->SetVec4InPSBuffer( 4, &vColour, 1 );
						iCurrentTexStage += 2;
						iTevStagesUsed += 2;
						break;
					}
					case MULTIPASSBLENDTYPE_LERPTEXLM1:
						TASSERT( MULTIPASSBLENDTYPE_LERPCOLOUR2 == m_aMultiPassBlendTypes[ 0 ] );
						iCurrentTexStage++;
						iTevStagesUsed++;
						break;
					case MULTIPASSBLENDTYPE_LERPTEXLM2:
					{
						TASSERT( iIndex > 1 && MULTIPASSBLENDTYPE_LERPTEXLM1 == m_aMultiPassBlendTypes[ iIndex - 1 ] );
						TINT16 iUserIndex = m_iUserDataIndices[ UData_2 ];
						TASSERT( iUserIndex > -1 );

						TVector4 vColour;
						UnpackColour( pWorldShader->GetCrossfade( iUserIndex ), vColour );
						pRender->SetVec4InPSBuffer( 4, &vColour, 1 );
						iCurrentTexStage += 2;
						iTevStagesUsed += 2;
						break;
					}
					default:
						break;
				}
			}
		}
		else
		{
			iTevStagesUsed++;
			TASSERT( TFALSE );
		}
	}

	if ( s_abLazyForce[ 1 ] || !s_bLightingSet )
		s_iLighting = 0;

	if ( s_abLazyForce[ 2 ] || s_iLazyNumTevStages != iTevStagesUsed )
	{
		s_NumTevStages      = iTevStagesUsed;
		s_iLazyNumTevStages = iTevStagesUsed;
	}

	if ( m_Flags & FLAGS_NO_CULL )
		SetCullMode( pRender, D3D11_CULL_NONE );
	else if ( m_Flags & FLAGS_UNK2 )
		SetCullMode( pRender, D3D11_CULL_BACK );
	else
		SetCullMode( pRender, D3D11_CULL_FRONT );

	TRenderContext* pContext    = pRender->GetCurrentContext();
	TVector4        vFogColour  = pContext->GetFogColour();
	TBOOL           bFog        = ( pContext->GetFlags() & TRenderContext::FLAG_FOG ) != 0;

	// The render states of the blend mode (only when it changed)
	if ( s_abLazyForce[ 6 ] || m_eBlendMode != BLENDMODE( s_iLazyBlendMode ) )
	{
		s_iLazyBlendMode = m_eBlendMode;

		switch ( m_eBlendMode )
		{
			case BLENDMODE_OPAQUE:
			case 13:
			case 25:
				m_uiAlpha = 0x80;

				if ( m_eBlendMode != BLENDMODE_OPAQUE )
				{
					pRender->SetDstAlpha( -1.0f );
					pRender->SetAlphaUpdate( TTRUE );
					m_uiAlpha = m_eBlendMode == 25;
				}

				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );

				if ( bAlphaTexture )
					pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );

				s_vAlphaTest.x = 1.0f;
				s_vAlphaTest.y = TFLOAT( m_uiAlpha ) / 255.0f;
				break;
			case 1:
			case 24:
				m_uiAlpha      = 1;
				s_vAlphaTest.x = 1.0f;
				s_vAlphaTest.y = TFLOAT( m_uiAlpha ) / 255.0f;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );

				if ( m_eBlendMode == 24 )
				{
					pRender->SetAlphaUpdate( TTRUE );
					pRender->SetDstAlpha( -1.0f );
				}

				break;
			case 2:
				s_vAlphaTest.y = 0.0f;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
				break;
			case 3:
				SetCullMode( pRender, D3D11_CULL_NONE );
				m_uiAlpha = 1;
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE );
				break;
			case 4:
				SetCullMode( pRender, D3D11_CULL_NONE );
				m_uiAlpha = 1;
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE );
				pRender->SetAlphaUpdate( TFALSE );
				s_vAlphaTest.x = 1.0f;
				s_vAlphaTest.y = TFLOAT( m_uiAlpha ) / 255.0f;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
				break;
			case 5:
				bFog           = TFALSE;
				s_vAlphaTest.x = 1.0f;
				s_vAlphaTest.y = 1.0f / 255.0f;
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
				break;
			case BLENDMODE_ZPASS:
				SetCullMode( pRender, D3D11_CULL_NONE );
				s_vAlphaTest.x = 0.0f;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
				pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
				pRender->SetColorUpdate( TFALSE );
				break;
			case 7:
				s_vAlphaTest.x = 1.0f;
				s_vAlphaTest.y = 200.0f / 255.0f;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
				break;
			case BLENDMODE_WATER:
			{
				// Water: scrolled by the first water UV offset
				bFog = TTRUE;

				TMatrix44 oMatrix = Translation( pWorldShader->m_vWaterUVOffset1.x, pWorldShader->m_vWaterUVOffset1.y, 0.0f );
				pRender->SetVec4InVSBuffer( 12, &oMatrix, 4 );
				s_aiTexCoordGen[ 0 ] = 12;
				s_vAlphaTest.x       = 1.0f;
				s_vAlphaTest.y       = 1.0f / 255.0f;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
				pRender->SetDstAlpha( 1.0f );

				// The transition to ink (UData_0 < -1: the amount, else the index of the transition)
				TINT16 iWaterUData = m_iUserDataIndices[ UData_0 ];
				TBOOL  bAmount     = iWaterUData < -1;

				if ( bAmount || iWaterUData > -1 )
				{
					TWorldShaderHAL* pShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
					s_aiTexCoordGen[ 1 ]     = 0;
					s_NumTevStages           = 2;
					s_iLazyNumTevStages      = 2;
					pShader->m_pWaterTransitionTex->Bind( 1 );

					TFLOAT fTransition;

					if ( bAmount )
						fTransition = 1.0f - TFLOAT( -m_iUserDataIndices[ UData_0 ] ) / 32767.0f;
					else
						fTransition = TFLOAT( -pShader->GetTransitionRef( m_iUserDataIndices[ UData_0 ] ).m_pWaterMat->m_iUserDataIndices[ UData_0 ] ) / 32767.0f;

					TVector4 vTransition( fTransition, fTransition, fTransition, fTransition );
					pRender->SetVec4InPSBuffer( 7, &vTransition, 1 );
					s_iIndirect = 1;
				}

				break;
			}
			case BLENDMODE_REFLECTIVEWATER:
			{
				// Reflective water: scrolled by the second water UV offset, the reflection and the noise textures
				bFog = TFALSE;

				if ( s_bWaterFrameReset1 == TTRUE )
				{
					pWorldShader->MakeReflectTexture();
					s_bWaterFrameReset1 = TFALSE;
				}

				TMatrix44 oMatrix = Translation( pWorldShader->m_vWaterUVOffset2.x, pWorldShader->m_vWaterUVOffset2.y, 0.0f );
				pRender->SetVec4InVSBuffer( 12, &oMatrix, 4 );
				pWorldShader->BindReflectTexture( 0 );

				if ( pWorldShader->m_pWaterNoiseTex != TNULL )
					pWorldShader->m_pWaterNoiseTex->Bind( 1 );

				s_aiTexCoordGen[ 0 ] = 14;
				s_aiTexCoordGen[ 1 ] = 12;
				s_NumTevStages       = 1;
				s_iLazyNumTevStages  = 1;

				TVector4 vIndirect( 0.5f, TFLOAT( pow( 2.0, -3.0 ) ), 0.0f, 0.0f );
				pRender->SetVec4InPSBuffer( 18, &vIndirect, 1 );
				s_iIndirect = 2;
				m_uiAlpha   = 0;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
				pRender->SetAlphaUpdate( TTRUE );
				pRender->SetDstAlpha( 0.0f );
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
				break;
			}
			case BLENDMODE_SHOCKWAVE:
			case BLENDMODE_HEAT:
			{
				bFog       = m_eBlendMode != BLENDMODE_HEAT;
				vFogColour = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );

				TMatrix44 oMatrix = Translation( pWorldShader->m_vWaterUVOffset1.x, pWorldShader->m_vWaterUVOffset1.y, 0.0f );
				pRender->SetVec4InVSBuffer( 12, &oMatrix, 4 );
				s_aiTexCoordGen[ 0 ] = 12;
				s_vAlphaTest.x       = 1.0f;
				s_vAlphaTest.y       = 1.0f / 255.0f;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK_ZERO );
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE );
				pRender->SetDstAlpha( 1.0f );
				break;
			}
			case 11:
				m_uiAlpha      = 0;
				s_vAlphaTest.x = 1.0f;
				s_vAlphaTest.y = TFLOAT( m_uiAlpha ) / 255.0f;
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO );
				break;
			case 12:
				m_uiAlpha      = 1;
				s_vAlphaTest.x = 1.0f;
				s_vAlphaTest.y = TFLOAT( m_uiAlpha ) / 255.0f;
				pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
				pRender->SetDstAlpha( -1.0f );
				pRender->SetAlphaUpdate( TTRUE );
				break;
			case 19:
				bFog = TFALSE;
				pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK( s_bUnk9B25EC ) );
				pRender->SetDepthBias( 0.0f, s_fDepthBias );
				pRender->SetDstAlpha( -1.0f );
				pRender->SetAlphaUpdate( TTRUE );
				m_uiAlpha = 0;
				break;
			case 20:
				m_uiAlpha      = 0x40;
				s_vAlphaTest.x = 1.0f;
				s_vAlphaTest.y = TFLOAT( m_uiAlpha ) / 255.0f;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
				SetCullMode( pRender, D3D11_CULL_NONE );
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE );
				pRender->SetAlphaUpdate( TTRUE );
				pRender->SetDstAlpha( -1.0f );
				pRender->SetDepthBias( 0.0f, 1.0f );
				break;
			case 21:
				m_uiAlpha      = 1;
				s_vAlphaTest.x = 1.0f;
				s_vAlphaTest.y = TFLOAT( m_uiAlpha ) / 255.0f;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
				SetCullMode( pRender, D3D11_CULL_NONE );
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE );
				pRender->SetAlphaUpdate( TTRUE );
				pRender->SetDstAlpha( -1.0f );
				pRender->SetDepthBias( 0.0f, 1.0f );
				break;
			case 23:
			{
				// Lightmapped reflective water
				if ( s_bWaterFrameReset2 == TTRUE )
				{
					pWorldShader->MakeReflectTexture();
					s_bWaterFrameReset2 = TFALSE;
				}

				TMatrix44 oMatrix = Translation( pWorldShader->m_vWaterUVOffset2.x, pWorldShader->m_vWaterUVOffset2.y, 0.0f );
				pRender->SetVec4InVSBuffer( 12, &oMatrix, 4 );
				pWorldShader->BindReflectTexture( 0 );

				if ( pWorldShader->m_pWaterNoiseTex != TNULL )
					pWorldShader->m_pWaterNoiseTex->Bind( 1 );

				s_aiTexCoordGen[ 0 ] = 14;
				s_aiTexCoordGen[ 1 ] = 12;
				s_NumTevStages       = 1;
				s_iLazyNumTevStages  = 1;

				TVector4 vIndirect( s_fLightmapScale, TFLOAT( pow( 2.0, double( TFLOAT( s_iLightmapExponent ) ) ) ), 0.0f, 0.0f );
				pRender->SetVec4InPSBuffer( 18, &vIndirect, 1 );
				s_iIndirect    = 3;
				s_vAlphaTest.x = 1.0f;
				s_vAlphaTest.y = 2.0f / 255.0f;
				pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
				bFog = TFALSE;
				break;
			}
			default:
				if ( s_fnPreRenderCallback == TNULL )
				{
					s_vAlphaTest.x = 1.0f;
					s_vAlphaTest.y = 0.0f;
					pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
				}

				break;
		}
	}

	// The fog (only when it changed)
	if ( s_abLazyForce[ 7 ] || bFog != s_bLazyFog || !TRenderDX11::IsColorEqual( vFogColour.AsArray(), s_vLazyFogColour.AsArray() ) )
	{
		if ( bFog == TTRUE )
		{
			s_bLazyFog         = TTRUE;
			s_vLazyFogColour   = vFogColour;
			s_vFogStartEnd.x   = pContext->GetFogDistanceStart();
			s_vFogStartEnd.y   = pContext->GetFogDistanceEnd();
			s_vFogStartEnd.z   = 1.0f;
			s_vFogColour       = TVector4( vFogColour.x, vFogColour.y, vFogColour.z, 1.0f );
		}
		else
		{
			s_bLazyFog       = TFALSE;
			s_vFogStartEnd.z = 0.0f;
		}
	}

	if ( s_bUnk9B241D )
	{
		TASSERT( TFALSE );
		s_NumTevStages = 1;
	}

	TASSERT( iCurrentTexStage == iTevStagesUsed );
}

// $deBlob: FUNCTION 00704920
void TWorldMaterialHAL::PostRender()
{
	if ( s_fnPostRenderCallback != TNULL && s_fnPostRenderCallback( this ) )
		return;

	TRenderDX11* pRender = TRenderDX11::Interface();

	switch ( m_eBlendMode )
	{
		case BLENDMODE_REFLECTIVEWATER:
		case 23:
			s_iLazyBlendMode = -1;
			return;
		case 19:
			pRender->SetDepthBias( 1.0f, 0.0f );
			TFALLTHROUGH;
		case BLENDMODE_WATER:
		case BLENDMODE_SHOCKWAVE:
		case 12:
		case 13:
		case BLENDMODE_HEAT:
		case 25:
			pRender->SetDstAlpha( 0.0f );
			s_iLazyBlendMode = -1;
			return;
		case 20:
		case 21:
			pRender->SetDepthBias( 1.0f, 0.0f );
			pRender->SetDstAlpha( 0.0f );
			pRender->SetAlphaUpdate( TTRUE );
			s_iLazyBlendMode = -1;
			return;
		case 24:
			pRender->SetAlphaUpdate( TTRUE );
			pRender->SetDstAlpha( 0.0f );
			break;
		default:
			break;
	}
}

// $deBlob: FUNCTION 00704a20
void TWorldMaterialHAL::SetBlendMode( BLENDMODE a_eBlendMode )
{
	if ( s_fnSetBlendModeCallback != TNULL && s_fnSetBlendModeCallback( this, a_eBlendMode ) )
		return;

	TWorldMaterial::SetBlendMode( a_eBlendMode );
}

// $deBlob: FUNCTION 00704a50
void TWorldMaterialHAL::SetBlendMode( BLENDMODE a_eBlendMode, TUINT8 a_uiPass )
{
	m_uiPass = a_uiPass;
	SetBlendMode( a_eBlendMode );
}

// $deBlob: FUNCTION 00704a70
void TWorldMaterialHAL::SetOrderTable( TOrderTable* a_pOrderTable, TINT a_iUnused, TBOOL a_bRegister )
{
	if ( a_pOrderTable != m_pOrderTable )
	{
		if ( m_pOrderTable != TNULL && a_bRegister && m_pRegMaterial != TNULL )
			TOrderTable::DeregisterMaterial( m_pRegMaterial );

		if ( a_pOrderTable != TNULL && a_bRegister )
			a_pOrderTable->RegisterMaterial( this );

		m_pOrderTable = a_pOrderTable;
	}
}

// $deBlob: FUNCTION 00704ac0
void TWorldMaterialHAL::SetUVMode( TINT a_iPass, TINT a_eMode )
{
	TASSERT( a_iPass < s_iMaxPasses );

	// TMaterial flag 0x200: a pass has projected texture coordinates
	if ( a_eMode == UVMODE_PROJECTED )
	{
		m_aUVModes[ a_iPass ] = UVMODE( a_eMode );
		SetFlags( 0x200, TTRUE );
	}
	else
	{
		UVMODE eOldMode       = m_aUVModes[ a_iPass ];
		m_aUVModes[ a_iPass ] = UVMODE( a_eMode );

		if ( eOldMode == UVMODE_PROJECTED )
		{
			for ( TINT i = 0; i < s_iMaxPasses; i++ )
			{
				if ( m_aUVModes[ i ] == UVMODE_PROJECTED )
					return;
			}

			SetFlags( 0x200, TFALSE );
		}
	}
}

// $deBlob: FUNCTION 00704b60
void TWorldMaterialHAL::SetUVOffset( TFLOAT a_fUOffset, TFLOAT a_fVOffset, TINT a_Pass )
{
	TASSERT( -1.0f <= a_fUOffset );
	TASSERT( 1.0f >= a_fUOffset );
	TASSERT( -1.0f <= a_fVOffset );
	TASSERT( 1.0f >= a_fVOffset );
	TASSERT( a_Pass >= 0 && a_Pass < s_iMaxPasses );

	m_afUOffset[ a_Pass ] = a_fUOffset;
	m_afVOffset[ a_Pass ] = a_fVOffset;

	// NOTE: retail writes the mode directly (TMaterial flag 0x200 isn't updated when it was UVMODE_PROJECTED)
	m_aUVModes[ a_Pass ] = UVMODE_OFFSET;
}

//-----------------------------------------------------------------------------
// TTRBWorldMesh
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00704cd0
void TTRBWorldMesh::RenderMesh( const void* a_pVertices, const void* a_pIndices, TUINT16 a_uiCount )
{
	if ( a_pVertices != TNULL && a_uiCount != 0 )
	{
		TRenderDX11::Interface()->DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, a_pVertices, a_uiCount, sizeof( Vertex ), 0 );
	}
	else if ( a_pIndices != TNULL && a_uiCount != 0 )
	{
		TRenderDX11::Interface()->DrawIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, a_uiCount, a_pIndices, DXGI_FORMAT_R16_UINT, m_pMesh->GetVertexBuffer(), sizeof( Vertex ), 0 );
	}
	else
	{
		TRenderDX11::Interface()->DrawIndexed( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pMesh->GetTRBMesh()->m_uiIndexCount, m_pMesh->GetIndexBuffer(), 0, DXGI_FORMAT_R16_UINT, m_pMesh->GetVertexBuffer(), sizeof( Vertex ), 0 );
	}
}

//-----------------------------------------------------------------------------
// TWorldMeshHAL
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00704d60
// $deBlob: FUNCTION 00704d70
// $deBlob: FUNCTION 00704dc0
// $deBlob: FUNCTION 00704e10
// $deBlob: FUNCTION 00704e20
// $deBlob: FUNCTION 0053fa50
// $deBlob: FUNCTION 00859500
TDEFINE_CLASS( TWorldMeshHAL );

// $deBlob: FUNCTION 00704e30
TWorldMeshHAL::TWorldMeshHAL()
{
	m_pTRBMesh      = TNULL;
	m_pIndexBuffer  = TNULL;
	m_pVertexBuffer = TNULL;
	m_next          = TNULL;
	m_prev          = TNULL;
	m_current       = TNULL;
}

// $deBlob: FUNCTION 007059e0
TWorldMeshHAL::~TWorldMeshHAL()
{
}

// $deBlob: FUNCTION 00704e70
TBOOL TWorldMeshHAL::Create( TTRBWorldMesh* a_pTRBMesh )
{
	Validate();
	m_pTRBMesh = a_pTRBMesh;

	TRenderDX11* pRender = TRenderDX11::Interface();

	TASSERT( a_pTRBMesh->m_uiIndexCount > 0 );
	// Type 1: index buffer
	m_pIndexBuffer = pRender->CreateBuffer( TRenderDX11::BUFFERTYPE_INDEX, a_pTRBMesh->m_uiIndexCount * sizeof( TUINT16 ), a_pTRBMesh->m_pIndices, D3D11_USAGE_IMMUTABLE, 0 );

	TASSERT( a_pTRBMesh->m_uiVertexCount > 0 );
	// Type 0: vertex buffer
	m_pVertexBuffer = pRender->CreateBuffer( TRenderDX11::BUFFERTYPE_VERTEX, a_pTRBMesh->m_uiVertexCount * sizeof( TTRBWorldMesh::Vertex ), a_pTRBMesh->m_pVertices, D3D11_USAGE_IMMUTABLE, 0 );

	m_State |= State_Created;
	return TTRUE;
}

// $deBlob: FUNCTION 00704f40
void TWorldMeshHAL::OnDestroy()
{
	if ( m_pIndexBuffer )
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = TNULL;
	}

	if ( m_pVertexBuffer )
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = TNULL;
	}

	TWorldMesh::OnDestroy();
}

// $deBlob: FUNCTION 00704f90
TBOOL TWorldMeshHAL::Render()
{
	return RenderWithCallback( TNULL, TNULL );
}

// $deBlob: FUNCTION 00705dd0
TBOOL TWorldMeshHAL::RenderWithCallback( t_RenderCallback a_fnCallback, void* a_pUserData )
{
	return RenderWithCallback( a_fnCallback, a_pUserData, -1 );
}

// Queues a render packet of a_pMesh in a_pRegMaterial with the current render context's state
static TRenderPacket* AddWorldRenderPacket( TRegMaterial* a_pRegMaterial, TWorldMeshHAL* a_pMesh, TRenderContext* a_pContext )
{
	TRenderPacket* pPacket = a_pRegMaterial->AddRenderPacket( a_pMesh );
	pPacket->SetModelViewMatrix( a_pContext->GetModelViewMatrix() );
	pPacket->SetSkeletonInstance( a_pContext->GetSkeletonInstance() );
	pPacket->SetFade( TUINT8( TINT( a_pContext->GetAlphaBlend() * 255.0f ) ) );

	return pPacket;
}

// $deBlob: FUNCTION 00704fa0
TBOOL TWorldMeshHAL::RenderWithCallback( t_RenderCallback a_fnCallback, void* a_pUserData, TINT16 a_iOrderTableIndex )
{
	TRenderContext* pContext = TRenderDX11::Interface()->GetCurrentContext();
	TVector3        vAmbient( pContext->GetAmbientColour().x, pContext->GetAmbientColour().y, pContext->GetAmbientColour().z );
	TVector3        vWhite( 1.0f, 1.0f, 1.0f );

	if ( TWorldShaderHAL::s_bInstancing && a_iOrderTableIndex > -1 )
	{
		if ( m_prev != TNULL || m_current != TNULL )
		{
			// An instanced mesh: queued once in the opaque or in an instance order table
			TWorldShaderHAL*   pWorldShaderHAL = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
			TWorldMaterialHAL* pRenderMaterial;
			TUINT32            uiFlags = m_pTRBMesh->m_uiFlags;

			if ( !( uiFlags & TTRBWorldMesh::Flags_QueuedOpaque ) && a_iOrderTableIndex == TWorldShaderHAL::OTABLE_ORDERING_OPT_OPAQUE )
			{
				m_pTRBMesh->m_uiFlags = uiFlags | TTRBWorldMesh::Flags_QueuedOpaque;
				pRenderMaterial       = pWorldShaderHAL->m_pInstanceRenderMats[ TWorldShaderHAL::OTABLE_ORDERING_OPT_OPAQUE ];
			}
			else if ( !( uiFlags & TTRBWorldMesh::Flags_QueuedInstance ) && a_iOrderTableIndex != TWorldShaderHAL::OTABLE_ORDERING_OPT_OPAQUE )
			{
				m_pTRBMesh->m_uiFlags = uiFlags | TTRBWorldMesh::Flags_QueuedInstance;
				TASSERT( pWorldShaderHAL->m_pInstanceRenderMats[ a_iOrderTableIndex ]->GetOrderTable()->GetPriority() >= pWorldShaderHAL->GetOrderTable( TWorldShaderHAL::OTABLE_ORDERING_OPT_OPAQUE )->GetPriority() );
				pRenderMaterial = pWorldShaderHAL->m_pInstanceRenderMats[ a_iOrderTableIndex ];
			}
			else
			{
				return TTRUE;
			}

			TRenderPacket* pPacket = AddWorldRenderPacket( pRenderMaterial->GetRegMaterial(), this, pContext );
			pPacket->SetAmbientColour( vAmbient );
			pPacket->SetAlpha( pContext->GetShadeCoeff() );
			pPacket->SetRenderCallback( a_fnCallback, a_pUserData );
			return TTRUE;
		}
	}
	else if ( m_prev != TNULL )
	{
		// An instanced mesh without the instance order tables: queued once in the material's
		if ( m_pTRBMesh->m_uiFlags & TTRBWorldMesh::Flags_QueuedOpaque )
			return TTRUE;

		m_pTRBMesh->m_uiFlags |= TTRBWorldMesh::Flags_QueuedOpaque;
	}

	TRenderPacket* pPacket = AddWorldRenderPacket( GetMaterial()->GetRegMaterial(), this, pContext );
	TINT16         iWaterUData = 0;
	pPacket->SetAmbientColour( vAmbient );
	pPacket->SetAlpha( pContext->GetShadeCoeff() );
	pPacket->SetRenderCallback( a_fnCallback, a_pUserData );

	TWorldShaderHAL* pWorldShaderHAL = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );

	if ( GetMaterial()->GetBlendMode() == TWorldMaterial::BLENDMODE_WATER || GetMaterial()->GetBlendMode() == TWorldMaterial::BLENDMODE_SHOCKWAVE )
	{
		// Water: also queued in the water order table
		iWaterUData = TSTATICCAST( TWorldMaterialHAL, GetMaterial() )->m_iUserDataIndices[ TWorldMaterialHAL::UData_0 ];
		pPacket->SetAmbientColour( vWhite );

		TRenderPacket* pWaterPacket = AddWorldRenderPacket( pWorldShaderHAL->m_pNoMatWater->GetRegMaterial(), this, pContext );
		pWaterPacket->SetMaterial( pWorldShaderHAL->m_pNoMatWater );
		pWaterPacket->SetRenderCallback( a_fnCallback, a_pUserData );
		pWaterPacket->SetAmbientColour( vWhite );

		if ( iWaterUData <= -1 )
			return TTRUE;
	}
	else
	{
		if ( GetMaterial()->GetBlendMode() == TWorldMaterial::BLENDMODE_HEAT )
		{
			// Heat haze: also queued with the heat material
			pPacket->SetAmbientColour( vWhite );

			TRenderPacket* pHeatPacket = AddWorldRenderPacket( pWorldShaderHAL->m_pNoMatHeat->GetRegMaterial(), this, pContext );
			pHeatPacket->SetMaterial( pWorldShaderHAL->m_pNoMatHeat );
			pHeatPacket->SetRenderCallback( a_fnCallback, a_pUserData );
			pHeatPacket->SetAmbientColour( vWhite );
		}

		iWaterUData = 0;
	}

	// The transition materials of the water
	TWorldMaterialHAL* pTransitionMaterial;

	if ( GetMaterial()->GetBlendMode() == TWorldMaterial::BLENDMODE_WATER )
	{
		TASSERT( pWorldShaderHAL->GetTransitionRef( iWaterUData ).m_pInkMat == GetMaterial() );
		pTransitionMaterial = pWorldShaderHAL->GetTransitionRef( iWaterUData ).m_pWaterMat;
	}
	else
	{
		if ( GetMaterial()->GetBlendMode() != TWorldMaterial::BLENDMODE_SHOCKWAVE )
			return TTRUE;

		pTransitionMaterial = pWorldShaderHAL->GetTransitionRef( iWaterUData ).m_pInkMat;
	}

	TRenderPacket* pTransitionPacket = AddWorldRenderPacket( pTransitionMaterial->GetRegMaterial(), this, pContext );
	pTransitionPacket->SetAmbientColour( vWhite );
	pTransitionPacket->SetAlpha( pContext->GetShadeCoeff() );
	pTransitionPacket->SetRenderCallback( a_fnCallback, a_pUserData );

	return TTRUE;
}

// $deBlob: FUNCTION 00705520
void TWorldMeshHAL::AddInstance( TWorldMeshInstanceData::Source* a_pSource, TUINT32 a_uiFlags )
{
	TShader* pOwnerShader = GetOwnerShader();
	TASSERT( ( pOwnerShader == TNULL ) || pOwnerShader->IsA( &TGetClass( TWorldShader ) ) );
	TWorldShaderHAL* pShader = TSTATICCAST( TWorldShaderHAL, pOwnerShader );

	if ( pShader->m_uiNumInstances < pShader->m_uiMaxInstances )
	{
		TWorldMeshInstanceData* curInstanceData = &pShader->m_pInstanceData[ pShader->m_uiNumInstances ];
		TASSERT( curInstanceData->m_prev == TNULL );

		TRenderContext* pRenderContext = TRenderDX11::Interface()->GetCurrentContext();
		TASSERT( 0 == ( TWorldMeshInstanceData::FLAGS_OPT_RENDERLIST_1 & curInstanceData->m_uiFlags ) );
		TASSERT( 0 == ( TWorldMeshInstanceData::FLAGS_OPT_RENDERLIST_2 & curInstanceData->m_uiFlags ) );

		curInstanceData->m_pSource    = a_pSource;
		curInstanceData->m_oModelView = TRenderDX11::Interface()->GetCurrentContext()->GetModelViewMatrix();
		curInstanceData->m_uiFlags    = a_uiFlags;
		curInstanceData->m_vAmbient   = pRenderContext->GetAmbientColour();

		TASSERT( TNULL == m_current );

		if ( m_next != TNULL && ( a_uiFlags & TWorldMeshInstanceData::FLAGS_OPT_RENDERLIST_1 ) )
		{
			TASSERT( m_prev != TNULL );
			TASSERT( m_prev->m_next == TNULL );
			m_prev->m_next          = curInstanceData;
			curInstanceData->m_prev = m_prev;
			curInstanceData->m_next = TNULL;
			m_prev                  = curInstanceData;
			pShader->m_uiNumInstances++;
		}
		else if ( m_prev != TNULL && ( a_uiFlags & TWorldMeshInstanceData::FLAGS_OPT_RENDERLIST_2 ) )
		{
			TASSERT( m_next != TNULL );
			TASSERT( m_next->m_prev == TNULL );
			m_next->m_prev          = curInstanceData;
			curInstanceData->m_prev = TNULL;
			curInstanceData->m_next = m_next;
			m_next                  = curInstanceData;
			pShader->m_uiNumInstances++;
		}
		else
		{
			curInstanceData->m_prev = TNULL;
			curInstanceData->m_next = TNULL;
			m_next                  = curInstanceData;
			m_prev                  = curInstanceData;
			pShader->m_uiNumInstances++;
		}
	}
	else
	{
		TASSERT( TFALSE, "Need to increase WORLD_SHADER_MAX_INSTANCES" );
	}
}

TOSHI_NAMESPACE_END
