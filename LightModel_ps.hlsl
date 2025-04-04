//--------------------------------------------------------------------------------------
// Light Model Pixel Shader
//--------------------------------------------------------------------------------------

#include "Common.hlsli" 


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Texture access from buffer
Texture2D    DiffuseMap : register(t0);

// Texture sampler
SamplerState TexSampler : register(s0); 


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader main function
float4 main(SimplePixelShaderInput input) : SV_Target
{

    // Sample diffuse material colour
    float3 diffuseMapColour = DiffuseMap.Sample(TexSampler, input.uv).rgb;

    // Blend texture colour with fixed per-object colour
    float3 finalColour = gObjectColour * diffuseMapColour;

    return float4(finalColour, 1.0f); 
}