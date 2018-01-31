////////////////////////////////////////////////////////////////////////////////
// Filename: simpleTexture.hlsl
// http://rastertek.com/dx11tut05.html
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer constants : register(b0)
{
	float brightness;
	float contrast;
	float unused1;
	float unused2;
};


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
	float4 source = shaderTexture.Sample(SampleType, input.tex);
	float4 output;
    //float brightness = 0.1125;
	//float contrast = 1.05;

	
    output.r = source.r;
    output.g = source.g;
    output.b = source.b;
    output.a = source.a;
        
    output.rgba *= input.color;
	
    // Courtesy https://stackoverflow.com/questions/944713/help-with-pixel-shader-effect-for-brightness-and-contrast
    // Apply contrast.
    output.rgb = ((output.rgb - 0.5f) * max(contrast, 0)) + 0.5f;

    // Apply brightness.
    output.rgb += brightness;


    return saturate(output);
}
