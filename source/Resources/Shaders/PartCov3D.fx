//-----------------------------------
// GLOBAL
//-----------------------------------

float4x4 gWorldViewProjection : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;


//-----------------------------------
// Vertex Structs
//-----------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float2 Uv : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 Uv : TEXCOORD;
};

//-----------------------------------
// UV Point Sampler
//-----------------------------------
SamplerState samplerPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Border;
	AddressV = Clamp;
	BorderColor = float4(0.f, 0.f, 1.f, 1.f);
};

//-----------------------------------
// UV Linear Sampler
//-----------------------------------
SamplerState samplerLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border;
	AddressV = Clamp;
	BorderColor = float4(0.f, 0.f, 1.f, 1.f);
};

//-----------------------------------
// UV Anisotropic Sampler
//-----------------------------------
SamplerState samplerAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Border;
	AddressV = Clamp;
	BorderColor = float4(0.f, 0.f, 1.f, 1.f);
};


//-----------------------------------
// DiffuseShading
//-----------------------------------
float4 FilteredDiffuseShading(SamplerState sState, float2 uv)
{
	return gDiffuseMap.Sample(sState, uv);
}

//-----------------------------------
// Filtered Shading
//-----------------------------------
float4 FilteredShading(SamplerState sState, VS_OUTPUT input)
{
	return FilteredDiffuseShading(sState, input.Uv);
}

//-----------------------------------
// Vertex Shader
//-----------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	//invert z component due to left handed system
	input.Position.z = -input.Position.z;

	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProjection);
	output.Uv = input.Uv;
	return output;
}

//-----------------------------------
// Pixel Shader
//-----------------------------------
float4 PS_Point(VS_OUTPUT input) : SV_TARGET
{
	return FilteredShading(samplerPoint, input);
}

//-----------------------------------
// Pixel Shader
//-----------------------------------
float4 PS_Linear(VS_OUTPUT input) : SV_TARGET
{
	return FilteredShading(samplerLinear, input);
}

//-----------------------------------
// Pixel Shader
//-----------------------------------
float4 PS_Anisotropic(VS_OUTPUT input) : SV_TARGET
{
	return FilteredShading(samplerAnisotropic, input);
}

//-----------------------------------
// Technique
//-----------------------------------
technique11 PointTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Point()));
	}
}

technique11 LinearTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
	}
}

technique11 AnisotropicTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
	}
}