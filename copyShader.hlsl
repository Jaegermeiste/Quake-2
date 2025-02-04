Texture2D<float4> inputTexture : register(t0);
SamplerState samplerState : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
	float4 color : COLOR;
    float2 texCoord : TEXCOORD;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR, float2 uv : TEXCOORD)
{
    PSInput output;
    output.position = position;
    output.texCoord = uv;
	output.color = color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float4 textureColor = inputTexture.Sample(samplerState, input.texCoord);
    return textureColor * input.color; // Modulate texture color with input
}