//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"

#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"
#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here

#include "ColourRGBA.h" 

#include <sstream>
#include <memory>
#include <iostream>


//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------

// Constants controlling speed of movement/rotation
const float ROTATION_SPEED = 2.0f;
const float MOVEMENT_SPEED = 50.0f;


// Meshes, models and cameras, same meaning as TL-Engine. Meshes prepared in InitGeometry function, Models & camera in InitScene
Mesh* gTeapotMesh;
Mesh* gCubeMesh;
Mesh* gCrateMesh;
Mesh* gSphereMesh;
Mesh* gGroundMesh;
Mesh* gLightMesh;
Mesh* gPortalMesh;

Model* gTeapot;
Model* gCube;
Model* gCrate;
Model* gSphere;
Model* gGround;
Model* gLight1;
Model* gLight2;
Model* gPortal;

// Two cameras - The main camera, and the view through the portal
Camera* gCamera;
Camera* gPortalCamera;


// Additional light information
CVector3 gLight1Colour = { 0.8f, 0.8f, 1.0f };
float    gLight1Strength = 10;

CVector3 gLight2Colour = { 1.0f, 0.8f, 0.2f };
float    gLight2Strength = 7;
float    gLight2MinStrength = 0;
float    gLight2MaxStrength = 7;
float    gLight2PulseSpeed = 1.5f;

float    textureShiftFactor = 0; // texture effect variable

CVector3 gAmbientColour = { 0.2f, 0.2f, 0.3f }; // Background level of light
float    gSpecularPower = 256; // Specular power controls shininess - same for all models in this app

ColourRGBA gBackgroundColor = { 0.2f, 0.2f, 0.3f , 1.0f };


// Variables controlling light1's orbiting of the cube
const float gLightOrbit = 20.0f;
const float gLightOrbitSpeed = 0.7f;



// Lock FPS to monitor refresh rate, which will typically set it to 60fps.
bool lockFPS = true;


//--------------------------------------------------------------------------------------
//**** Portal Texture  ****//
//--------------------------------------------------------------------------------------
// Dimensions of portal texture - controls quality of rendered scene in portal
int gPortalWidth  = 256;
int gPortalHeight = 256;

// The portal texture - each frame it is rendered to, then it is used as a texture for model
ID3D11Texture2D*          gPortalTexture      = nullptr; 
ID3D11RenderTargetView*   gPortalRenderTarget = nullptr; 
ID3D11ShaderResourceView* gPortalTextureSRV   = nullptr; 

// Also need a depth/stencil buffer for the portal
ID3D11Texture2D*        gPortalDepthStencil     = nullptr;
ID3D11DepthStencilView* gPortalDepthStencilView = nullptr;



//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame

PerFrameConstants gPerFrameConstants;      // The constants that need to be sent to the GPU each frame
ID3D11Buffer*     gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constant that change per-model
ID3D11Buffer*     gPerModelConstantBuffer; 



//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

// DirectX objects controlling textures used
ID3D11Resource*           gTeapotDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gTeapotDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gCubeStoneDiffuseSpecularMap    = nullptr; 
ID3D11ShaderResourceView* gCubeStoneDiffuseSpecularMapSRV = nullptr;
ID3D11Resource*           gCubeWoodDiffuseSpecularMap = nullptr; 
ID3D11ShaderResourceView* gCubeWoodDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gCrateDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gCrateDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gSphereDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gSphereDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gGroundDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gGroundDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gLightDiffuseMap    = nullptr;
ID3D11ShaderResourceView* gLightDiffuseMapSRV = nullptr;


//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
bool InitGeometry()
{
    // Load mesh geometry data
    try 
    {
        gTeapotMesh = new Mesh("Teapot.x");
        gCubeMesh   = new Mesh("Cube.x");
        gCrateMesh  = new Mesh("CargoContainer.x");
        gSphereMesh = new Mesh("Sphere.x");
        gGroundMesh = new Mesh("Hills.x");
        gLightMesh  = new Mesh("Light.x");
        gPortalMesh = new Mesh("Portal.x");
    }
    catch (std::runtime_error e)
    {
        gLastError = e.what();
        return false;
    }


    // Load the shaders required for the geometry used
    if (!LoadShaders())
    {
        gLastError = "Error loading shaders";
        return false;
    }


    // Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
    gPerFrameConstantBuffer = CreateConstantBuffer(sizeof(gPerFrameConstants));
    gPerModelConstantBuffer = CreateConstantBuffer(sizeof(gPerModelConstants));
    if (gPerFrameConstantBuffer == nullptr || gPerModelConstantBuffer == nullptr)
    {
        gLastError = "Error creating constant buffers";
        return false;
    }


    //// Load / prepare textures on the GPU ////

    // Load textures and create DirectX objects for them
    if (!LoadTexture("MetalDiffuseSpecular.dds", &gTeapotDiffuseSpecularMap, &gTeapotDiffuseSpecularMapSRV) ||
        !LoadTexture("StoneDiffuseSpecular.dds", &gCubeStoneDiffuseSpecularMap,   &gCubeStoneDiffuseSpecularMapSRV  ) ||
        !LoadTexture("WoodDiffuseSpecular.dds",  &gCubeWoodDiffuseSpecularMap,    &gCubeWoodDiffuseSpecularMapSRV) ||
        !LoadTexture("CargoA.dds",               &gCrateDiffuseSpecularMap,  &gCrateDiffuseSpecularMapSRV) ||
        !LoadTexture("Brick1.jpg",               &gSphereDiffuseSpecularMap, &gSphereDiffuseSpecularMapSRV) ||
        !LoadTexture("GrassDiffuseSpecular.dds", &gGroundDiffuseSpecularMap, &gGroundDiffuseSpecularMapSRV ) ||
        !LoadTexture("Flare.jpg",                &gLightDiffuseMap,          &gLightDiffuseMapSRV ))
    {
        gLastError = "Error loading textures";
        return false;
    }




    //**** Create Portal Texture ****//

	// Using a helper function to load textures from files above
	D3D11_TEXTURE2D_DESC portalDesc = {};
	portalDesc.Width  = gPortalWidth;  // Size of the portal texture determines its quality
	portalDesc.Height = gPortalHeight;
	portalDesc.MipLevels = 1; // No mip-maps when rendering to textures
	portalDesc.ArraySize = 1;
	portalDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA texture (8-bits each)
	portalDesc.SampleDesc.Count = 1;
	portalDesc.SampleDesc.Quality = 0;
	portalDesc.Usage = D3D11_USAGE_DEFAULT;
	portalDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	portalDesc.CPUAccessFlags = 0;
	portalDesc.MiscFlags = 0;
	if (FAILED( gD3DDevice->CreateTexture2D(&portalDesc, NULL, &gPortalTexture) ))
	{
		gLastError = "Error creating portal texture";
		return false;
	}

	if (FAILED( gD3DDevice->CreateRenderTargetView(gPortalTexture, NULL, &gPortalRenderTarget) ))
	{
		gLastError = "Error creating portal render target view";
		return false;
	}

	// Create a shader-resource "view"
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc = {};
	srDesc.Format = portalDesc.Format;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	if (FAILED( gD3DDevice->CreateShaderResourceView(gPortalTexture, &srDesc, &gPortalTextureSRV) ))
	{
		gLastError = "Error creating portal shader resource view";
		return false;
	}


	//**** Create Portal Depth Buffer ****//

    portalDesc = {};
	portalDesc.Width  = gPortalWidth;
	portalDesc.Height = gPortalHeight;
	portalDesc.MipLevels = 1;
	portalDesc.ArraySize = 1;
	portalDesc.Format = DXGI_FORMAT_D32_FLOAT;
	portalDesc.SampleDesc.Count = 1;
	portalDesc.SampleDesc.Quality = 0;
	portalDesc.Usage = D3D11_USAGE_DEFAULT;
	portalDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	portalDesc.CPUAccessFlags = 0;
	portalDesc.MiscFlags = 0;
	if (FAILED(gD3DDevice->CreateTexture2D(&portalDesc, NULL, &gPortalDepthStencil) ))
	{
		gLastError = "Error creating portal depth stencil texture";
		return false;
	}

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC portalDescDSV = {};
	portalDescDSV.Format = portalDesc.Format;
	portalDescDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	portalDescDSV.Texture2D.MipSlice = 0;
    portalDescDSV.Flags = 0;
	if (FAILED(gD3DDevice->CreateDepthStencilView(gPortalDepthStencil, &portalDescDSV, &gPortalDepthStencilView) ))
	{
		gLastError = "Error creating portal depth stencil view";
		return false;
	}

    
    //*****************************//
    


  	// Create all filtering modes, blending modes etc. used by the app
	if (!CreateStates())
	{
		gLastError = "Error creating states";
		return false;
	}

	return true;
}


// Prepare the scene
bool InitScene()
{
    //// Set up scene ////

    gTeapot = new Model(gTeapotMesh);
    gCube   = new Model(gCubeMesh);
    gCrate  = new Model(gCrateMesh);
    gSphere = new Model(gSphereMesh);
    gGround = new Model(gGroundMesh);
    gLight1 = new Model(gLightMesh);
    gLight2 = new Model(gLightMesh);
    gPortal = new Model(gPortalMesh);

	// Initial positions
    gTeapot->SetPosition({ 10,  0, 40 });
	gCube->  SetPosition({  0, 15,  0 });
	gSphere->SetPosition({ 30, 10,  0 });
	gCrate-> SetPosition({-10,  0, 90 });
	gCrate-> SetScale( 6.0f );
	gCrate-> SetRotation({ 0.0f, ToRadians(40.0f), 0.0f });
	gPortal->SetPosition({ 40, 20, 40 });
	gPortal->SetRotation({ 0.0f, ToRadians(-130.0f), 0.0f });

    gLight1->SetPosition({ 30, 10, 0 });
    gLight1->SetScale(pow(gLight1Strength, 0.7f));
    gLight2->SetPosition({ -20, 30, 40 });
    gLight2->SetScale(pow(gLight2MaxStrength, 0.7f));

    //// Set up cameras ////

    gCamera = new Camera();
    gCamera->SetPosition({ 40, 30, -90 });
    gCamera->SetRotation({ ToRadians(8.0f), ToRadians(-18.0f), 0.0f });
	gCamera->SetNearClip( 1.0f );
	gCamera->SetFarClip( 1000.0f );


  	// Set up portal
	gPortalCamera = new Camera();
	gPortalCamera->SetPosition({ 45, 45, 85 });
	gPortalCamera->SetRotation({ ToRadians(20.0f), ToRadians(215.0f), 0 });


    return true;
}


// Release the geometry and scene resources created above
void ReleaseResources()
{
    ReleaseStates();

    if (gPortalDepthStencilView)  gPortalDepthStencilView->Release();
    if (gPortalDepthStencil)      gPortalDepthStencil->Release();
    if (gPortalTextureSRV)        gPortalTextureSRV->Release();
    if (gPortalRenderTarget)      gPortalRenderTarget->Release();
    if (gPortalTexture)           gPortalTexture->Release();

    if (gLightDiffuseMapSRV)           gLightDiffuseMapSRV->Release();
    if (gLightDiffuseMap)              gLightDiffuseMap->Release();
    if (gGroundDiffuseSpecularMapSRV)  gGroundDiffuseSpecularMapSRV->Release();
    if (gGroundDiffuseSpecularMap)     gGroundDiffuseSpecularMap->Release();
    if (gSphereDiffuseSpecularMapSRV)  gSphereDiffuseSpecularMapSRV->Release();
    if (gSphereDiffuseSpecularMap)     gSphereDiffuseSpecularMap->Release();
    if (gCrateDiffuseSpecularMapSRV)   gCrateDiffuseSpecularMapSRV->Release();
    if (gCrateDiffuseSpecularMap)      gCrateDiffuseSpecularMap->Release();
    if (gTeapotDiffuseSpecularMapSRV)   gTeapotDiffuseSpecularMapSRV->Release();
    if (gTeapotDiffuseSpecularMap)      gTeapotDiffuseSpecularMap->Release();
    if (gCubeStoneDiffuseSpecularMapSRV)    gCubeStoneDiffuseSpecularMapSRV->Release();
    if (gCubeStoneDiffuseSpecularMap)       gCubeStoneDiffuseSpecularMap->Release();
    if (gCubeWoodDiffuseSpecularMapSRV)    gCubeWoodDiffuseSpecularMapSRV->Release();
    if (gCubeWoodDiffuseSpecularMap)       gCubeWoodDiffuseSpecularMap->Release();

    if (gPerModelConstantBuffer)  gPerModelConstantBuffer->Release();
    if (gPerFrameConstantBuffer)  gPerFrameConstantBuffer->Release();

    ReleaseShaders();

    // Delete dynamically allocated objects not using unique_ptr
    delete gCamera;        gCamera       = nullptr;
    delete gPortalCamera;  gPortalCamera = nullptr;

    delete gPortal;  gPortal = nullptr;
    delete gLight1;  gLight1 = nullptr;
    delete gLight2;  gLight2 = nullptr;
    delete gGround;  gGround = nullptr;
    delete gSphere;  gSphere = nullptr;
    delete gCrate;   gCrate  = nullptr;
    delete gCube;    gCube   = nullptr;
    delete gTeapot;  gTeapot = nullptr;

    delete gPortalMesh;  gPortalMesh = nullptr;
    delete gLightMesh;   gLightMesh  = nullptr;
    delete gGroundMesh;  gGroundMesh = nullptr;
    delete gSphereMesh;  gSphereMesh = nullptr;
    delete gCrateMesh;   gCrateMesh  = nullptr;
    delete gCubeMesh;    gCubeMesh   = nullptr;
    delete gTeapotMesh;  gTeapotMesh = nullptr;
}



//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------


// Render everything in the scene from the given camera
void RenderSceneFromCamera(Camera* camera)
{
    // Set camera matrices in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix           = camera->ViewMatrix();
    gPerFrameConstants.projectionMatrix     = camera->ProjectionMatrix();
    gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);


    //// Render lit models ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gPixelLightingPixelShader,  nullptr, 0);
    
    // States for non-unique objects
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullBackState);

    // Select the approriate textures and sampler to use in the pixel shader
    gD3DContext->PSSetShaderResources(0, 1, &gGroundDiffuseSpecularMapSRV);
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // Render model
    gGround->Render();

    // Container render
    gD3DContext->PSSetShaderResources(0, 1, &gCrateDiffuseSpecularMapSRV);
    gCrate->Render();

    // Teapot render
    gD3DContext->PSSetShaderResources(0, 1, &gTeapotDiffuseSpecularMapSRV);
    gTeapot->Render();

    // Portal render
    gD3DContext->PSSetShaderResources(0, 1, &gPortalTextureSRV);
    gPortal->Render();

    // Sphere render - change in shaders
    gD3DContext->VSSetShader(gSphereModelVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gSphereModelPixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &gSphereDiffuseSpecularMapSRV);
    gSphere->Render();
    
    // Cube render - change in shaders, and two textures sent to buffers
    gD3DContext->VSSetShader(gCubeModelVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gCubeModelPixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &gCubeStoneDiffuseSpecularMapSRV); // Send two textures to the buffers for linear interpolation
    gD3DContext->PSSetShaderResources(1, 1, &gCubeWoodDiffuseSpecularMapSRV);
    gCube->Render();

    //// Render lights ////
    // Rendered with different shaders, textures, states from other models

    gD3DContext->VSSetShader(gLightModelVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gLightModelPixelShader,  nullptr, 0);

    // Select the texture and sampler to use in the pixel shader
    gD3DContext->PSSetShaderResources(0, 1, &gLightDiffuseMapSRV); 
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // States - additive blending, read-only depth buffer and no culling 
    gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);

    // Render model, sets world matrix, vertex and index buffer and calls Draw on the GPU
    gPerModelConstants.objectColour = gLight1Colour; // Set any per-model constants apart from the world matrix just before calling render
    gLight1->Render();

    gPerModelConstants.objectColour = gLight2Colour;
    gLight2->Render();
}




// Main render function
void RenderScene()
{
    //// Common settings for both main scene and portal scene ////

    // Set up the light information in the constant buffer - this is the same for portal and main render
    gPerFrameConstants.light1Colour   = gLight1Colour * gLight1Strength;
    gPerFrameConstants.light1Position = gLight1->Position();
    gPerFrameConstants.light2Colour   = gLight2Colour * gLight2Strength;
    gPerFrameConstants.light2Strength = gLight2Strength;
    gPerFrameConstants.light2Position = gLight2->Position();
    gPerFrameConstants.ambientColour  = gAmbientColour;
    gPerFrameConstants.specularPower  = gSpecularPower;
    gPerFrameConstants.cameraPosition = gCamera->Position();

    // Send time-based variable to constant buffer for use in pixel shader
    gPerModelConstants.textureShiftFactor = textureShiftFactor;

    //-------------------------------------------------------------------------


    //// Portal scene rendering ////

    // Set the portal texture and portal depth buffer as the targets for rendering
    gD3DContext->OMSetRenderTargets(1, &gPortalRenderTarget, gPortalDepthStencilView);

    // Clear the portal texture to a fixed colour and the portal depth buffer to the far distance
    gD3DContext->ClearRenderTargetView(gPortalRenderTarget, &gBackgroundColor.r);
    gD3DContext->ClearDepthStencilView(gPortalDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Setup the viewport for the portal texture size
    D3D11_VIEWPORT vp;
    vp.Width  = static_cast<FLOAT>(gPortalWidth);
    vp.Height = static_cast<FLOAT>(gPortalHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Render the scene for the portal
    RenderSceneFromCamera(gPortalCamera);


    //-------------------------------------------------------------------------


    //// Main scene rendering ////

    // Set the back buffer as the target for rendering and select the main depth buffer.
    gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

    // Clear the back buffer to a fixed colour and the depth buffer to the far distance
    gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &gBackgroundColor.r);
    gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Setup the viewport to the size of the main window
    vp.Width  = static_cast<FLOAT>(gViewportWidth);
    vp.Height = static_cast<FLOAT>(gViewportHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Render the scene for the main window
    RenderSceneFromCamera(gCamera);


    //-------------------------------------------------------------------------


    //// Scene completion ////

    // When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
    // Set first parameter to 1 to lock to vsync (typically 60fps)
    gSwapChain->Present(lockFPS ? 1 : 0, 0);
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------

// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
	// Control sphere (will update its world matrix)
	gSphere->Control(frameTime, Key_I, Key_K, Key_J, Key_L, Key_U, Key_O, Key_Period, Key_Comma );

    // Orbit the light
	static float rotate = 0.0f;
	gLight1->SetPosition( gCube->Position() + CVector3{ cos(rotate) * gLightOrbit, 0.0f, sin(rotate) * gLightOrbit } );
	rotate -= gLightOrbitSpeed * frameTime;


	// Control camera 
	gCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D );


    // Toggle FPS limiting
    if (KeyHit(Key_P))  lockFPS = !lockFPS;

    // Show frame time / FPS in the window title //
    const float fpsUpdateTime = 0.5f; // How long between updates (in seconds)
    static float fpsFrameTime = 0;
    static float effectTime = 0;
    static int frameCount = 0;
    fpsFrameTime += frameTime;
    ++frameCount;

    // local texture shift factor - separate frame time variable for effects, does not reset
    effectTime += frameTime; 

    // global factor passed into buffer to be used in pixel shaders
    textureShiftFactor = 2 * effectTime;

    // Rotating light color values
    float r = 0.5f + 0.5f * sinf(effectTime * 0.8f);
    float g = 0.5f + 0.5f * sinf(effectTime * 0.8f);
    float b = 0.5f + 0.5f * sinf(effectTime * 0.8f);

    gLight1Colour = { r, g, b }; // assign values to vector before sending to GPU

    // Static light pulsate on/off
    float gLight2PulseFactor = 0.5f + 0.5f * sinf(effectTime * gLight2PulseSpeed);
    gLight2Strength = gLight2MinStrength + (gLight2MaxStrength - gLight2MinStrength) * gLight2PulseFactor;

    if (fpsFrameTime > fpsUpdateTime)
    {
        // Displays FPS rounded to nearest int, and frame time in milliseconds to 2 decimal places
        float avgFrameTime = fpsFrameTime / frameCount;
        std::ostringstream frameTimeMs;
        frameTimeMs.precision(2);
        frameTimeMs << std::fixed << avgFrameTime * 1000;
        std::string windowTitle = "CO2409 Assignment / Kyriacos Rediu - Frame Time: " + frameTimeMs.str() +
                                  "ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
        SetWindowTextA(gHWnd, windowTitle.c_str());
        fpsFrameTime = 0;
        frameCount = 0;
    }
}
