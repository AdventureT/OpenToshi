// Generated from the HLSL embedded in retail deBlob.exe. Do not edit.
// clang-format off

// ScreenSpace vertex shader (618 bytes)
static const TCHAR* s_szScreenSpaceVS =
	" struct VS_OUT                                 {                                                 float4 Position : POSITION;                   float2 UV : TEXCOORD0;                    };                                                                                          VS_OUT main( float2 Position : POSITION )     {                                                 VS_OUT Out;                                   Out.Position = float4(Position, 0.0f, 1.0f);       Out.UV = Position.xy * float2(0.5f, -0.5f) + 0.5f;           return Out;                               }                                            "
;

// GaussBlur5x5 pixel shader (603 bytes)
static const TCHAR* s_szGaussBlur5x5PS =
	" struct PS_IN\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 Position\t: SV_POSITION;\t\t\t\t\tfloat2 Tex0\t\t: TEXCOORD0;\t\t\t\t };\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t static const int\tMAX_SAMPLES = 16;\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t uniform float4\tg_avSampleOffsets[MAX_SAMPLES] : register(c29);  uniform float4\tg_avSampleWeights[MAX_SAMPLES] : register(c45); \t\t\t\t\t\t\t\t\t\t\t\t sampler s0  : register(s0);\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t float4 main( PS_IN In )  : COLOR\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t    float4 vColor = 0.0f;\t\t\t\t\t\t    for (int i = 0; i < 13; i++)\t\t\t\t    \tvColor += g_avSampleWeights[i] * tex2D(s0, In.Tex0 + g_avSampleOffsets[i].xy);    return vColor;\t\t\t\t\t\t\t\t }\t\t\t\t\t\t\t\t\t\t\t\t"
;

// GaussBlurMultByAlpha pixel shader (688 bytes)
static const TCHAR* s_szGaussBlurMultByAlphaPS =
	" struct PS_IN\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 Position\t: SV_POSITION;\t\t\t\t\tfloat2 Tex0\t\t: TEXCOORD0;\t\t\t\t };\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t static const int\tMAX_SAMPLES = 16;\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t uniform float4\tg_avSampleOffsets[MAX_SAMPLES] : register(c29);  uniform float4\tg_avSampleWeights[MAX_SAMPLES] : register(c45); \t\t\t\t\t\t\t\t\t\t\t\t sampler s0  : register(s0);\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t float4 main( PS_IN In )  : COLOR\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t    float4 vColor = 0.0f;\t\t\t\t\t\t    for (int i = 0; i < 13; i++)\t\t\t\t\t{\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 colour = tex2D(s0, In.Tex0 + g_avSampleOffsets[i].xy); \t\tvColor += g_avSampleWeights[i] * colour * colour.w; \t}\t\t\t\t\t\t\t\t\t\t\t\t vColor.w = 0.0;\t\t\t\t\t\t\t    return vColor;\t\t\t\t\t\t\t\t }\t\t\t\t\t\t\t\t\t\t\t\t"
;

// GaussBlurMultByAlphaStencil pixel shader (923 bytes)
static const TCHAR* s_szGaussBlurMultByAlphaStencilPS =
	" struct PS_IN\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 Position\t: SV_POSITION;\t\t\t\t\tfloat2 Tex0\t\t: TEXCOORD0;\t\t\t\t };\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t static const int\tMAX_SAMPLES = 16;\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t uniform float4\tg_avSampleOffsets[MAX_SAMPLES] : register(c29);  uniform float4\tg_avSampleWeights[MAX_SAMPLES] : register(c45);  uniform float4\tg_stencilSize : register(c61);\t\t\t\t\t\t\t\t\t\t\t\t Texture2D t0\t\t\t: register(t0);\t\t\t sampler s0\t\t\t: register(s0);\t\t\t Texture2D<uint2> t1\t: register(t1);\t\t\t sampler s1\t\t\t: register(s1);\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t float4 main( PS_IN In )  : SV_TARGET\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 vColor = 0.0f;\t\t\t\t\t\t\tfor (int i = 0; i < 13; i++)\t\t\t\t\t{\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 colour = t0.Sample(s0, In.Tex0 + g_avSampleOffsets[i].xy); \t\tcolour *= t1.Load(int3((In.Tex0 + g_avSampleOffsets[i].xy) * g_stencilSize.xy, 0)).g;\t\tvColor += g_avSampleWeights[i] * colour * colour.w; \t}\t\t\t\t\t\t\t\t\t\t\t\tvColor.w = 1.0;\t\t\t\t\t\t\t\t\treturn vColor;\t\t\t\t\t\t\t\t }\t\t\t\t\t\t\t\t\t\t\t\t"
;

// clang-format on
