//--------------------------------------------------------------------------------------
// Commonly used definitions across entire project
//--------------------------------------------------------------------------------------
#ifndef _COMMON_H_INCLUDED_
#define _COMMON_H_INCLUDED_

#define NOMINMAX 

#include <windows.h>
#include <d3d11.h>
#include <string>

#include "CVector3.h"
#include "CMatrix4x4.h"


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Make global Variables from various files available to other files.

// Windows variables
extern HWND gHWnd;

// Viewport size
extern int gViewportWidth;
extern int gViewportHeight;


// Important DirectX variables
extern ID3D11Device*           gD3DDevice;
extern ID3D11DeviceContext*    gD3DContext;
extern IDXGISwapChain*         gSwapChain;
extern ID3D11RenderTargetView* gBackBufferRenderTarget;  
extern ID3D11DepthStencilView* gDepthStencil;            

// Input constsnts
extern const float ROTATION_SPEED;
extern const float MOVEMENT_SPEED;


// A global error message to help track down fatal errors
extern std::string gLastError;



//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame

// Per Frame Buffers
// Must match the hlsli file
struct PerFrameConstants
{
    // These are the matrices used to position the camera
    CMatrix4x4 viewMatrix;
    CMatrix4x4 projectionMatrix;
    CMatrix4x4 viewProjectionMatrix; // The above two matrices multiplied together to combine their effects

    CVector3   light1Position; 
    float      padding1;      
    CVector3   light1Colour;
    float      padding2;

    CVector3   light2Position;
    float      padding3;
    CVector3   light2Colour;
    float      padding4;
    float      light2Strength;

    CVector3   ambientColour;
    float      specularPower; 

    CVector3   cameraPosition;
    float      padding5;
};

extern PerFrameConstants gPerFrameConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*     gPerFrameConstantBuffer; // This variable controls the GPU-side constant buffer matching to the above structure



// Per Model Buffers
// Must match the hlsli file
struct PerModelConstants
{
    CMatrix4x4 worldMatrix;
    CVector3   objectColour; // Allows each light model to be tinted to match the light colour they cast
    float      padding6;
    float      textureShiftFactor;
};
extern PerModelConstants gPerModelConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*     gPerModelConstantBuffer; // This variable controls the GPU-side constant buffer related to the above structure


#endif //_COMMON_H_INCLUDED_
