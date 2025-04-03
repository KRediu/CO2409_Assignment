//--------------------------------------------------------------------------------------
// Per-Pixel Lighting Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader receives position and normal from the vertex shader and uses them to calculate
// lighting per pixel. Also samples a samples a diffuse + specular texture map and combines with light colour.

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.
Texture2D    StoneDiffuseSpecularMap : register(t0); 
Texture2D    WoodDiffuseSpecularMap : register(t1); 
SamplerState TexSampler : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader entry point - each shader has a "main" function
// This shader just samples a diffuse texture map
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

    return float4(finalColour, 1.0f); // Always use 1.0f for output alpha - no alpha blending in this lab
}