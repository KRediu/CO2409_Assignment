//--------------------------------------------------------------------------------------
// Per-Pixel Lighting Vertex Shader
//--------------------------------------------------------------------------------------

#include "Common.hlsli"


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Vertex shader main function
LightingPixelShaderInput main(BasicVertex modelVertex)
{
    LightingPixelShaderInput output; 

    // Input position
    float4 modelPosition = float4(modelVertex.position, 1); 

    // Matrices
    float4 worldPosition     = mul(gWorldMatrix,      modelPosition);
    float4 viewPosition      = mul(gViewMatrix,       worldPosition);
    output.projectedPosition = mul(gProjectionMatrix, viewPosition);

    // Transform model normals into world space using world matrix - lighting will be calculated in world space
    float4 modelNormal = float4(modelVertex.normal, 0);     
    output.worldNormal = mul(gWorldMatrix, modelNormal).xyz; 
                                                           
    output.worldPosition = worldPosition.xyz; // Also pass world position to pixel shader for lighting

    // Pass texture coordinates (UVs) on to the pixel shader
    output.uv = modelVertex.uv;

    return output; // Output data sent down the pipeline (to the pixel shader)
}
