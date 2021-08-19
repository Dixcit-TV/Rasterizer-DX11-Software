//-----------------------------------
// GLOBAL
//-----------------------------------

float4x4 gWorldViewProjection : WorldViewProjection;
float4x4 gCameraMatrix : CameraMatrix;
float4x4 gWorldMatrix : WorldMatrix;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;
float3 gLightDir = float3(0.577f, -0.577f, 0.577f);
float gShininess = 25.f;
float gLightIntensity = 6.f;
float gPi = 3.14159265358979323846f;


//-----------------------------------
// Vertex Structs
//-----------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 Uv : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : WORLDPOS;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
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
float3 FilteredDiffuseShading(SamplerState sState, float3 normal, float2 uv)
{
	float specStrength = saturate(dot(gLightDir, -normal)) / gPi * gLightIntensity;
	return gDiffuseMap.Sample(sState, uv).rgb * specStrength;
}

//-----------------------------------
// NormalMapping
//-----------------------------------
float3 FilteredNormalMapping(SamplerState sState, float3 normal, float3 tangent, float2 uv)
{
	normal = normalize(normal);
	tangent = normalize(tangent);

	float3 normalSample = gNormalMap.Sample(sState, uv).rgb;
	normalSample = normalSample * 2 - 1;
	float3x3 tangentSpaceMatrix = float3x3(tangent, cross(normal, tangent), normal);

	return normalize(mul(normalSample, tangentSpaceMatrix));
}

//-----------------------------------
// PhongShading
//-----------------------------------
float3 FilteredPhongShading(SamplerState sState, float3 normal, float4 worldPosition, float2 uv)
{
	float3 r = reflect(gLightDir, normal);
	float3 viewVector = normalize(gCameraMatrix[3].xyz - worldPosition.xyz);
	float3 rDvv = saturate(dot(r, viewVector));
	return gSpecularMap.Sample(sState, uv).rgb * pow(rDvv, gShininess * gGlossinessMap.Sample(sState, uv).r);
}

//-----------------------------------
// Filtered Shading
//-----------------------------------
float4 FilteredShading(SamplerState sState, VS_OUTPUT input)
{
	float3 normal = FilteredNormalMapping(sState, input.Normal, input.Tangent, input.Uv);
	float3 diffuseColor = FilteredDiffuseShading(sState, normal, input.Uv);
	float3 specularColor = FilteredPhongShading(sState, normal, input.WorldPosition, input.Uv);

	float3 finalShading = (diffuseColor + specularColor);
	return float4(finalShading, 1.f);
}

//-----------------------------------
// Vertex Shader
//-----------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	//invert z component due to left handed system
	input.Position.z = -input.Position.z;
	input.Normal.z = -input.Normal.z;
	input.Tangent.z = -input.Tangent.z;

	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProjection);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	output.Normal = mul(input.Normal, (float3x3)gWorldMatrix);
	output.Tangent = mul(input.Tangent, (float3x3)gWorldMatrix);
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