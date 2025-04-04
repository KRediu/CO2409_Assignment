//--------------------------------------------------------------------------------------
// Light Model Vertex Shader
//--------------------------------------------------------------------------------------

#include "Common.hlsli"


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Vertex shader main function
SimplePixelShaderInput main(BasicVertex modelVertex)
{
    SimplePixelShaderInput output; // This is the data the pixel shader requires from this vertex shader

    // Input position
    float4 modelPosition = float4(modelVertex.position, 1); 

    // Matrices
    float4 worldPosition     = mul(gWorldMatrix,      modelPosition);
    float4 viewPosition      = mul(gViewMatrix,       worldPosition);
    output.projectedPosition = mul(gProjectionMatrix, viewPosition);

    // Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
    output.uv = modelVertex.uv;

    return output; // Output data sent down the pipeline (to the pixel shader)
}
