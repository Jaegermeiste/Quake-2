////////////////////////////////////////////////////////////////////////////////
// Filename: simpleTexture.hlsl
// http://rastertek.com/dx11tut05.html
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture;
SamplerState SampleType;


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS_Entry(PixelInputType input) : SV_TARGET
{
	float4 source = shaderTexture.Sample(SampleType, input.tex);;
	float4 destination;
	
	destination.r = source.r;
	destination.g = source.g;
	destination.b = source.b;
	destination.a = source.a;
	
	destination.rgb *= source.a;

	return destination;
}
