//--------------------------------------------------------------------------------------
// Per-Pixel Lighting Pixel Shader
//--------------------------------------------------------------------------------------

#include "Common.hlsli" 


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Texture access from buffer
Texture2D    StoneDiffuseSpecularMap : register(t0); 
Texture2D    WoodDiffuseSpecularMap : register(t1); 

// Texture sampler
SamplerState TexSampler : register(s0);


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader main function
float4 main(LightingPixelShaderInput input) : SV_Target
{

    // General ighting equations
    input.worldNormal = normalize(input.worldNormal); // Normal might have been scaled by model scaling or interpolation so renormalise
    float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

    // Static light - 1 //

    // Diffuse lighting
    float3 light1Vector = gLight1Position - input.worldPosition;
    float3 light1Dist = length(light1Vector);
    float3 light1Direction = light1Vector / light1Dist;
    float3 diffuseLight1 = gLight1Colour * max(dot(input.worldNormal, light1Direction), 0) / light1Dist; // Equations from lighting lecture

    // Specular lighting
    float3 halfway = normalize(light1Direction + cameraDirection);
    float3 specularLight1 =  diffuseLight1 * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower); // Multiplying by diffuseLight instead of light colour


    // Rotating light - 2 //

    // Diffuse lighting
    float3 light2Vector = gLight2Position - input.worldPosition;
    float3 light2Dist = length(light2Vector);
    float3 light2Direction = light2Vector / light2Dist;
    float3 diffuseLight2 = gLight2Strength * gLight2Colour * max(dot(input.worldNormal, light2Direction), 0) / light2Dist;

    // Specular lighting
    halfway = normalize(light2Direction + cameraDirection);
    float3 specularLight2 = gLight2Strength * diffuseLight2 * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower);


    // Textures from two diffuse/specular maps for linear interpolation
    float4 stoneColour = StoneDiffuseSpecularMap.Sample(TexSampler, input.uv);
    float3 diffuseStoneColour = stoneColour.rgb; // Diffuse material colour in texture RGB (base colour of model)
    float specularStoneColour = stoneColour.a;   // Specular material colour in texture A (shininess of the surface)

    float4 WoodColour = WoodDiffuseSpecularMap.Sample(TexSampler, input.uv);
    float3 diffuseWoodColour = WoodColour.rgb; // Diffuse material colour in texture RGB (base colour of model)
    float specularWoodColour = WoodColour.a;   // Specular material colour in texture A (shininess of the surface)

    // Linear interpolation application using texture shift variable
    float t = 0.5f + 0.5f * sin(textureShiftFactor);
    float3 diffuseColour = lerp(diffuseStoneColour, diffuseWoodColour, t);
    float specularColour = lerp(specularStoneColour, specularWoodColour, t);    

    // Combine lighting with texture colours
    float3 finalColour = (gAmbientColour + diffuseLight1 + diffuseLight2) * diffuseColour + 
                         (specularLight1 + specularLight2) * specularColour;

    return float4(finalColour, 1.0f); // final colour
}