// Generated from the HLSL embedded in retail deBlob.exe. Do not edit.
// clang-format off

// glow pixel shader (625 bytes)
static const TCHAR* s_szGlowPS =
	" struct PS_IN\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 Position\t\t\t: SV_POSITION;\t\t\tfloat2 Tex0             : TEXCOORD0;\t\t };\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t sampler2D\tframebuf_texture\t: register(s0) = sampler_state{ MipFilter = NONE; MinFilter = LINEAR; MagFilter = LINEAR; AddressU = WRAP; AddressV = WRAP; };   sampler2D\tglow_texture\t\t: register(s0) = sampler_state{ MipFilter = NONE; MinFilter = LINEAR; MagFilter = LINEAR; AddressU = WRAP; AddressV = WRAP; };   float4\tglowmag\t\t\t\t: register(c28);  float4 main( PS_IN In ) : COLOR\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 glowCol = tex2D(glow_texture, In.Tex0);\t\treturn  glowCol * glowmag;\t\t\t\t\t }\t\t\t\t\t\t\t\t\t\t\t\t"
;

// clang-format on
