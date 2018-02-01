////////////////////////////////////////////////////////////////////////////////
// Filename: vertexTexture2D.hlsl
// http://rastertek.com/dx11tut05.html
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType VS_Entry(VertexInputType input)
{
    PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    //output.position = mul(input.position, worldMatrix);
    //output.position = mul(output.position, viewMatrix);
    output.position = mul(input.position, projectionMatrix);

    output.color = input.color;
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
    return output;
}