//--------------------------------------------------------------------------------------
// Shader input / output
//--------------------------------------------------------------------------------------

// The structure below describes the vertex data to be sent into the vertex shader.
struct BasicVertex
{
    float3 position : position;
    float3 normal   : normal;
    float2 uv       : uv;
};


// This structure describes what data the lighting pixel shader receives from the vertex shader.
struct LightingPixelShaderInput
{
    float4 projectedPosition : SV_Position; 

    float3 worldPosition : worldPosition;  
    float3 worldNormal   : worldNormal;     
    
    float2 uv : uv; 
};



// This structure is similar to the one above but for the light models, which aren't themselves lit
struct SimplePixelShaderInput
{
    float4 projectedPosition : SV_Position;
    float2 uv : uv;
};


//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

// Per Frame Buffers
// These variables must match exactly the gPerFrameConstants structure in Scene.cpp
cbuffer PerFrameConstants : register(b0)
{
    float4x4 gViewMatrix;
    float4x4 gProjectionMatrix;
    float4x4 gViewProjectionMatrix; // The above two matrices multiplied together to combine their effects

    float3   gLight1Position; 
    float    padding1;        

    float3   gLight1Colour;
    float    padding2;

    float3   gLight2Position;
    float    padding3;
    float3   gLight2Colour;
    float    padding4;
    float    gLight2Strength;

    float3   gAmbientColour;
    float    gSpecularPower; 

    float3   gCameraPosition;
    float    padding5;
}
// Note constant buffers are not structs: we don't use the name of the constant buffer, these are really just a collection of global variables (hence the 'g')


// Per Model Buffers
// These variables must match exactly the gPerModelConstants structure in Scene.cpp
cbuffer PerModelConstants : register(b1)
{
    float4x4 gWorldMatrix;

    float3   gObjectColour;
    float    padding6; 
    float    textureShiftFactor; // shift factor passed here
}
