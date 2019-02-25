#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

struct VERTEX {
	float x;
	float y;
	float z;
};

#pragma region Global Variables:
bool isRunning = false;
constexpr int windowWidth = 800;
constexpr int windowHeight = 600;
VERTEX verticies[] = { { 0.0f, 0.5f, 0.0f }, { 0.45f, -0.5f, 0.0f }, { -0.45f, -0.5f, 0.0f }, };
#pragma endregion

LRESULT CALLBACK WindowProcedure(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message)
	{
	case WM_CLOSE: {
		DestroyWindow(windowHandle);
		isRunning = false;
		break;
	}
	default:
		break;
	}

	return DefWindowProc(windowHandle, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInst, char* cmdArgs, int cmdShow) {

	#pragma region Declerations:
	HWND						mainWindowHandle;
	ID3D11Device*					device = nullptr;
	ID3D11DeviceContext*				deviceContext = nullptr;
	IDXGISwapChain*					swapChain = nullptr;
	ID3D11RenderTargetView*				renderTarget = nullptr;
	ID3D11Buffer*					vertexBuffer = nullptr;
	ID3D10Blob*					vertexShaderData = nullptr;
	ID3D10Blob*					pixelShaderData = nullptr;
	ID3D11VertexShader*				vertexShader = nullptr;
	ID3D11PixelShader*				pixelShader = nullptr;
	ID3D11InputLayout*				inputLayout = nullptr;
	#pragma endregion

//========================================================================================================================

	#pragma region Create Window:
	{
		isRunning = true;
		WNDCLASS wc = { 0 };
		wc.hInstance = appInstance;
		wc.lpfnWndProc = WindowProcedure;
		wc.lpszClassName = "ClassName";
		wc.style = CS_OWNDC;
		RegisterClass(&wc);
		mainWindowHandle = CreateWindow("ClassName", "Output Window", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, appInstance, nullptr);
		ShowWindow(mainWindowHandle, SW_SHOW);
	}
	#pragma endregion

	#pragma region Create Device and SwapChain:
	{
		DXGI_SWAP_CHAIN_DESC scd = { 0 };
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.SampleDesc.Count = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.BufferCount = 2;
		scd.OutputWindow = mainWindowHandle;
		scd.Windowed = true;

		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &deviceContext);
	}
	#pragma endregion

	#pragma region Create Render Target:
	{
		ID3D11Texture2D* backBuffer = nullptr;
		swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
		device->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
		deviceContext->OMSetRenderTargets(1, &renderTarget, nullptr);

		backBuffer->Release();
	}
	#pragma endregion

	#pragma region Create Buffer:
	{
		D3D11_BUFFER_DESC bd = { 0 };
		bd.ByteWidth = sizeof(VERTEX) * ARRAYSIZE(verticies);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA srd = { verticies, 0, 0 };

		device->CreateBuffer(&bd, &srd, &vertexBuffer);
	}
	#pragma endregion

	#pragma region Create Shaders and InputLayout:
	{
		D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderData);
		D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderData);

		device->CreateVertexShader(vertexShaderData->GetBufferPointer(), vertexShaderData->GetBufferSize(), nullptr, &vertexShader);
		device->CreatePixelShader(pixelShaderData->GetBufferPointer(), pixelShaderData->GetBufferSize(), nullptr, &pixelShader);

		deviceContext->VSSetShader(vertexShader, nullptr, 0);
		deviceContext->PSSetShader(pixelShader, nullptr, 0);


		D3D11_INPUT_ELEMENT_DESC ied[] = { {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0} };
		device->CreateInputLayout(ied, ARRAYSIZE(ied), vertexShaderData->GetBufferPointer(), vertexShaderData->GetBufferSize(), &inputLayout);
		deviceContext->IASetInputLayout(inputLayout);
	}
	#pragma endregion

	#pragma region Setting up Viewport:
	{
		D3D11_VIEWPORT viewport = { 0 };
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = windowWidth;
		viewport.Height = windowHeight;

		deviceContext->RSSetViewports(1, &viewport);
	}
	#pragma endregion

//========================================================================================================================
	while (isRunning) {

		#pragma region Handle Messages:
		MSG message = { 0 };
		if (PeekMessage(&message, mainWindowHandle, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		#pragma endregion

		#pragma region Clear Viewport:
		{
			float clearColour[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
			deviceContext->ClearRenderTargetView(renderTarget, clearColour);
		}
		#pragma endregion

		#pragma region Transfer Data:
		{
			UINT stride = sizeof(VERTEX);
			UINT offset = 0;

			deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
			deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
		#pragma endregion

		#pragma region Render:
		{
			deviceContext->Draw(3, 0);
			swapChain->Present(1, 0);
		}
		#pragma endregion

	}
	//========================================================================================================================

	#pragma region Clean Up:
	device->Release();
	deviceContext->Release();
	swapChain->Release();
	renderTarget->Release();
	vertexBuffer->Release();
	vertexShaderData->Release();
	pixelShaderData->Release();
	vertexShader->Release();
	pixelShader->Release();
	inputLayout->Release();
	#pragma endregion

	return 0;
}
