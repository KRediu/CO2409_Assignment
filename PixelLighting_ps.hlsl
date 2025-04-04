//--------------------------------------------------------------------------------------
// Per-Pixel Lighting Pixel Shader
//--------------------------------------------------------------------------------------


#include "Common.hlsli"


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Texture access from buffer
Texture2D    DiffuseSpecularMap : register(t0);
                                      
// Texture sampler          
SamplerState TexSampler : register(s0);


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader main function
float4 main(LightingPixelShaderInput input) : SV_Target
{
    // Lighting equations
    input.worldNormal = normalize(input.worldNormal); // Normal might have been scaled by model scaling or interpolation so renormalise
    float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

    // Light 1
    float3 light1Vector = gLight1Position - input.worldPosition;
    float3 light1Dist = length(light1Vector);
    float3 light1Direction = light1Vector / light1Dist;
    float3 diffuseLight1 = gLight1Colour * max(dot(input.worldNormal, light1Direction), 0) / light1Dist; // Equations from lighting lecture

    float3 halfway = normalize(light1Direction + cameraDirection);
    float3 specularLight1 =  diffuseLight1 * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower); // Multiplying by diffuseLight instead of light colour - my own personal preference


    // Light 2
    float3 light2Vector = gLight2Position - input.worldPosition;
    float3 light2Dist = length(light2Vector);
    float3 light2Direction = light2Vector / light2Dist;
    float3 diffuseLight2 = gLight2Strength * gLight2Colour * max(dot(input.worldNormal, light2Direction), 0) / light2Dist;

    
    halfway = normalize(light2Direction + cameraDirection);
    float3 specularLight2 = gLight2Strength * diffuseLight2 * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower);



    // Sample diffuse material and specular material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
    float3 diffuseMaterialColour = textureColour.rgb; // Diffuse material colour in texture RGB (base colour of model)
    float specularMaterialColour = textureColour.a;   // Specular material colour in texture A (shininess of the surface)

    // Combine lighting with texture colours
    float3 finalColour = (gAmbientColour + diffuseLight1 + diffuseLight2) * diffuseMaterialColour + 
                         (specularLight1 + specularLight2) * specularMaterialColour;

    return float4(finalColour, 1.0f); // Always use 1.0f for output alpha
}