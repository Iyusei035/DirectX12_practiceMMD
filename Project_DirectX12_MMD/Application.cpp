#include "Application.h"
#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;
using namespace std;
using namespace Microsoft::WRL;

void DebugOutputFormatString(const char* format, ...)
{
	///@brief コンソール画面にフォーマット付き文字列を表示
	///@param format フォーマット(%dとか%fとかの)
	///@param 可変長引数
	///@remarksこの関数はデバッグ用です。デバッグ時にしか動作しません
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY) {//ウィンドウが破棄されたら呼ばれます
		PostQuitMessage(0);//OSに対して「終了」と伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//規定の処理を行う
}

const unsigned int window_width = 1280;
const unsigned int window_height = 720;

std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath)
{
	int pathIndex1 = modelPath.rfind('/');
	int pathIndex2 = modelPath.rfind('\\');

	auto pathIndex = max(pathIndex1, pathIndex2);
	auto folderPath = modelPath.substr(0, pathIndex);
	return folderPath + texPath;
}

std::string GetExtension(const std::string& path)
{
	int idx = path.rfind('.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

wstring GetExtension(const std::wstring& path)
{
	auto idx = path.rfind(L'.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

std::pair<std::string, std::string>SplitFileName(const std::string& path, const char spliter = '*')
{
	int idx = path.find(spliter);
	pair<std::string, std::string> ret;

	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx + 1, path.length() - idx - 1);
	return ret;
}

std::wstring GetWideStringFromString(const std::string& str)
{
	auto num1 = MultiByteToWideChar
	(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		nullptr,
		0
	);

	std::wstring wstr;
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar
	(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		&wstr[0],
		num1
	);
	assert(num1 == num2);
	return wstr;
}

ID3D12Resource* Application::CreateGrayGradationTexture()
{
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 4, 256);
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	ID3D12Resource* gradBuff = nullptr;
	auto result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&gradBuff)
	);
	if (FAILED(result)) {
		return nullptr;
	}

	//上が白くて下が黒いテクスチャデータを作成
	std::vector<unsigned int> data(4 * 256);
	auto it = data.begin();
	unsigned int c = 0xff;
	for (; it != data.end(); it += 4) {
		auto col = (0xff << 24) | RGB(c, c, c);//RGBAが逆並びしているためRGBマクロと0xff<<24を用いて表す。
		//auto col = (0xff << 24) | (c<<16)|(c<<8)|c;//これでもOK
		std::fill(it, it + 4, col);
		--c;
	}

	result = gradBuff->WriteToSubresource(0, nullptr, data.data(), 4 * sizeof(unsigned int), sizeof(unsigned int) * static_cast<UINT>(data.size()));
	return gradBuff;
}
ID3D12Resource* Application::CreateWhiteTexture()
{
	D3D12_HEAP_PROPERTIES texHeapProp = {};

	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;
	resDesc.Height = 4;
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.MipLevels = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* whiteBuff = nullptr;
	auto result = _dev->CreateCommittedResource
	(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&whiteBuff)
	);
	if (FAILED(result))
	{
		return nullptr;
	}

	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);
	result = whiteBuff->WriteToSubresource
	(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size()
	);
	return whiteBuff;

}
ID3D12Resource* Application::CreateBlackTexture()
{
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//幅
	resDesc.Height = 4;//高さ
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	ID3D12Resource* blackBuff = nullptr;
	auto result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&blackBuff)
	);
	if (FAILED(result)) {
		return nullptr;
	}
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0x00);

	result = blackBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, static_cast<UINT>(data.size()));
	return blackBuff;
}

ID3D12Resource* Application::LoadTextureFromFile(std::string& texPath)
{
	auto it = _resourceTable.find(texPath);
	if (it != _resourceTable.end())
	{
		return _resourceTable[texPath];
	}

	//WICテクスチャのロード
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	auto wtexpath = GetWideStringFromString(texPath);
	auto ext = GetExtension(texPath);
	auto result = _loadLambdaTable[ext](wtexpath, &metadata, scratchImg);

	if (FAILED(result))
	{
		return nullptr;
	}

	auto img = scratchImg.GetImage(0, 0, 0);

	//WriteToSubreSourceで転送する用のヒープ設定
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D
	(
		metadata.format,
		static_cast<UINT>(metadata.width),
		static_cast<UINT>(metadata.height),
		static_cast<UINT>(metadata.arraySize),
		static_cast<UINT>(metadata.mipLevels)
	);
	ID3D12Resource* texbuff = nullptr;
	result = _dev->CreateCommittedResource
	(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texbuff)
	);

	if (FAILED(result))
	{
		return nullptr;
	}

	result = texbuff->WriteToSubresource
	(
		0,
		nullptr,
		img->pixels,
		static_cast<UINT>(img->rowPitch),
		static_cast<UINT>(img->slicePitch)
	);

	if (FAILED(result))
	{
		return nullptr;
	}
	_resourceTable[texPath] = texbuff;
	return texbuff;
}

void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	debugLayer->EnableDebugLayer();
	debugLayer->Release();
}

HRESULT Application::CreateSwapChain(const HWND& hwnd, Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory)
{
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	return dxgiFactory->CreateSwapChainForHwnd(_cmdQueue.Get(),
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)_swapchain.ReleaseAndGetAddressOf());
}

void Application::CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass)
{
	HINSTANCE hInst = GetModuleHandle(nullptr);
	//ウィンドウクラス生成＆登録
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;//コールバック関数の指定
	w.lpszClassName = _T("DirectXTest");//アプリケーションクラス名(適当でいいです)
	w.hInstance = GetModuleHandle(0);//ハンドルの取得
	RegisterClassEx(&w);//アプリケーションクラス(こういうの作るからよろしくってOSに予告する)

	RECT wrc = { 0,0, window_width, window_height };//ウィンドウサイズを決める
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);//ウィンドウのサイズはちょっと面倒なので関数を使って補正する
	//ウィンドウオブジェクトの生成
	hwnd = CreateWindow(w.lpszClassName,//クラス名指定
		_T("DX12テスト"),//タイトルバーの文字
		//WS_OVERLAPPEDWINDOW,//タイトルバーと境界線があるウィンドウです
		WS_OVERLAPPEDWINDOW - WS_THICKFRAME,
		CW_USEDEFAULT,//表示X座標はOSにお任せします
		CW_USEDEFAULT,//表示Y座標はOSにお任せします
		wrc.right - wrc.left,//ウィンドウ幅
		wrc.bottom - wrc.top,//ウィンドウ高
		nullptr,//親ウィンドウハンドル
		nullptr,//メニューハンドル
		w.hInstance,//呼び出しアプリケーションハンドル
		nullptr);//追加パラメータ
}

HRESULT Application::InitializeDXGIDevice()
{
	UINT flagsDXGI = 0;
	flagsDXGI |= DXGI_CREATE_FACTORY_DEBUG;
	auto result = CreateDXGIFactory2(flagsDXGI, IID_PPV_ARGS(&_dxgiFactory));
	//フィーチャレベル列挙
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}
	result = S_FALSE;
	//Direct3Dデバイスの初期化
	D3D_FEATURE_LEVEL featureLevel;
	for (auto l : levels) {
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&_dev)) == S_OK) {
			featureLevel = l;
			result = S_OK;
			break;
		}
	}
	return result;
}

HRESULT Application::InitializeCommand()
{
	auto result = _dev->CreateCommandAllocator
	(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	result = _dev->CreateCommandList
	(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
		return result;
	}
	
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//タイムアウトなし
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//プライオリティ特に指定なし
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//ここはコマンドリストと合わせてください
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));//コマンドキュー生成
	if (FAILED(result))
	{
		assert(0);
	}
	return S_OK;
}

HRESULT Application::CreateFinalRenderTarget(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& rtvHeaps, std::vector<ID3D12Resource*>& backBuffers)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンダーターゲットビューなので当然RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;//表裏の２つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//特に指定なし

	//ID3D12DescriptorHeap* rtvHeaps = nullptr;
	auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(rtvHeaps.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
		return result;
	}
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	backBuffers.resize(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	//SRGBレンダーターゲットビュー設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;


	for (size_t i = 0; i < swcDesc.BufferCount; ++i)
	{
		result = _swapchain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&backBuffers[i]));
		assert(SUCCEEDED(result));
		rtvDesc.Format = backBuffers[i]->GetDesc().Format;
		_dev->CreateRenderTargetView(backBuffers[i], &rtvDesc, handle);
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	_viewport = CD3DX12_VIEWPORT(backBuffers[0]);
	_scissorrect = CD3DX12_RECT(0, 0, window_width, window_height);
	return result;
}

HRESULT Application::CreateBasicGraphicsPipeline()
{
	ComPtr<ID3DBlob> _vsBlob = nullptr;
	ComPtr<ID3DBlob> _psBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	auto result = D3DCompileFromFile(L"BasicVertexShader.hlsl",
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &_vsBlob, &errorBlob);
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}
	result = D3DCompileFromFile(L"BasicPixelShader.hlsl",
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &_psBlob, &errorBlob);
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "BONE_NO",0,DXGI_FORMAT_R16G16_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{ "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{ "EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = _rootsignature.Get();
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(_vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(_psBlob.Get());

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//中身は0xffffffff

	gpipeline.BlendState=CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//カリングしない

	//深度ステンシル
	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.StencilEnable = false;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;//レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements = _countof(inputLayout);//レイアウト配列数

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//ストリップ時のカットなし
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形で構成

	gpipeline.NumRenderTargets = 1;//今は１つのみ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0～1に正規化されたRGBA

	gpipeline.SampleDesc.Count = 1;//サンプリングは1ピクセルにつき１
	gpipeline.SampleDesc.Quality = 0;//クオリティは最低	

	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(_pipelinestate.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
	}
	return result;
}

HRESULT Application::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE descTblRanges[3] = {};
	descTblRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);//定数(座標)
	descTblRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);//定数(マテリアル)
	descTblRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);//テクスチャ(基本、sph.spa.toon)

	CD3DX12_ROOT_PARAMETER rootParam[2] = {};
	rootParam[0].InitAsDescriptorTable(1, &descTblRanges[0]);//座標変換
	rootParam[1].InitAsDescriptorTable(2, &descTblRanges[1]);//マテリアル

	CD3DX12_STATIC_SAMPLER_DESC samplerDescs[2] = {};
	samplerDescs[0].Init(0);
	samplerDescs[1].Init(1, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(2, rootParam, 2, samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	if (FAILED(result)) {
		assert(0);
		return result;
	}
	result = _dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(_rootsignature.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
		return result;
	}
	return result;
}

void Application::Run()
{
	ShowWindow(_hwnd, SW_SHOW);//ウィンドウ表示
	float angle = 0.0f;
	MSG msg = {};
	unsigned int frame = 0;
	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	while (true) {
		_worldMat = XMMatrixRotationY(angle);
		_mapScene->world = _worldMat;
		_mapScene->view = _viewMat;
		_mapScene->proj = _projMat;
		angle += 0.01f;

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//もうアプリケーションが終わるって時にmessageがWM_QUITになる
		if (msg.message == WM_QUIT) {
			break;
		}

		//DirectX処理
		//バックバッファのインデックスを取得
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(_backBuffer[bbIdx],
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		_cmdList->ResourceBarrier(1, &barrier);

		_cmdList->SetPipelineState(_pipelinestate.Get());


		//レンダーターゲットを指定
		auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
		_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		//画面クリア

		float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };//白色
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		_cmdList->RSSetViewports(1, &_viewport);
		_cmdList->RSSetScissorRects(1, &_scissorrect);
		_cmdList->SetGraphicsRootSignature(_rootsignature.Get());

		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->IASetVertexBuffers(0, 1, &_vbView);
		_cmdList->IASetIndexBuffer(&_ibView);

		_cmdList->SetGraphicsRootSignature(_rootsignature.Get());

		//WVP変換行列
		ID3D12DescriptorHeap* bdh[] = { _basicDescHeap.Get() };
		_cmdList->SetDescriptorHeaps(1, bdh);
		_cmdList->SetGraphicsRootDescriptorTable(0, _basicDescHeap->GetGPUDescriptorHandleForHeapStart());

		ID3D12DescriptorHeap* mdh[] = { _materialDescHeap.Get() };
		//マテリアル
		_cmdList->SetDescriptorHeaps(1, mdh);

		auto materialH = _materialDescHeap->GetGPUDescriptorHandleForHeapStart();
		unsigned int idxOffset = 0;

		auto cbvsrvIncSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 5;
		for (auto& m : _materials) {
			_cmdList->SetGraphicsRootDescriptorTable(1, materialH);
			_cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
			materialH.ptr += cbvsrvIncSize;
			idxOffset += m.indicesNum;
		}
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(_backBuffer[bbIdx],
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		_cmdList->ResourceBarrier(1, &barrier);

		//命令のクローズ
		_cmdList->Close();



		//コマンドリストの実行
		ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);
		////待ち
		_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		_cmdAllocator->Reset();//キューをクリア
		_cmdList->Reset(_cmdAllocator.Get(), _pipelinestate.Get());//再びコマンドリストをためる準備


		//フリップ
		_swapchain->Present(1, 0);

	}
}

void Application::CreateTextureLoaderTable()
{
	_loadLambdaTable["sph"] = _loadLambdaTable["spa"] = _loadLambdaTable["bmp"] = _loadLambdaTable["png"] = _loadLambdaTable["jpg"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT {
		return LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, meta, img);
		};

	_loadLambdaTable["tga"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT {
		return LoadFromTGAFile(path.c_str(), meta, img);
		};

	_loadLambdaTable["dds"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT {
		return LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, meta, img);
		};
}

HRESULT Application::CreateDepthStencilView()
{
	//深度バッファ作成
	//深度バッファの仕様
	auto depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
		window_width, window_height,
		1, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	//デプス用ヒーププロパティ
	auto depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	CD3DX12_CLEAR_VALUE depthClearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	float clrColor[4] = { 1.0f,1.0f,1.0f,1.0f };
	CD3DX12_CLEAR_VALUE rtClearValue(DXGI_FORMAT_R8G8B8A8_UINT, clrColor);

	auto result = _dev->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, //デプス書き込みに使用
		&depthClearValue,
		IID_PPV_ARGS(_depthBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		//エラー処理
		return result;
	}

	//深度のためのデスクリプタヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};//深度に使うよという事がわかればいい
	dsvHeapDesc.NumDescriptors = 1;//深度ビュー1つのみ
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//デプスステンシルビューとして使う
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;


	result = _dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(_dsvHeap.ReleaseAndGetAddressOf()));

	//深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//デプス値に32bit使用
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//フラグは特になし
	_dev->CreateDepthStencilView(_depthBuffer.Get(), &dsvDesc, _dsvHeap->GetCPUDescriptorHandleForHeapStart());
	return result;
}

HRESULT Application::LoadPMDFile(const char* path)
{
	//PMDヘッダ構造体
	struct PMDHeader {
		float version; //例：00 00 80 3F == 1.00
		char model_name[20];//モデル名
		char comment[256];//モデルコメント
	};
	char signature[3];
	PMDHeader pmdheader = {};

	string strModelPath = path;
	FILE* fp;
	fopen_s(&fp, strModelPath.c_str(), "rb");
	if (fp == nullptr) {
		//エラー処理
		assert(0);
		return ERROR_FILE_NOT_FOUND;
	}
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	unsigned int vertNum;//頂点数
	fread(&vertNum, sizeof(vertNum), 1, fp);


#pragma pack(1)//ここから1バイトパッキング…アライメントは発生しない
	//PMDマテリアル構造体
	struct PMDMaterial {
		XMFLOAT3 diffuse; //ディフューズ色
		float alpha; // ディフューズα
		float specularity;//スペキュラの強さ(乗算値)
		XMFLOAT3 specular; //スペキュラ色
		XMFLOAT3 ambient; //アンビエント色
		unsigned char toonIdx; //トゥーン番号(後述)
		unsigned char edgeFlg;//マテリアル毎の輪郭線フラグ
		//2バイトのパディングが発生！！
		unsigned int indicesNum; //このマテリアルが割り当たるインデックス数
		char texFilePath[20]; //テクスチャファイル名(プラスアルファ…後述)
	};//70バイトのはず…でもパディングが発生するため72バイト
#pragma pack()//1バイトパッキング解除

	constexpr unsigned int pmdvertex_size = 38;//頂点1つあたりのサイズ
	std::vector<unsigned char> vertices(vertNum * pmdvertex_size);//バッファ確保
	fread(vertices.data(), vertices.size(), 1, fp);//一気に読み込み

	unsigned int indicesNum;//インデックス数
	fread(&indicesNum, sizeof(indicesNum), 1, fp);//

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size());
	//UPLOAD(確保は可能)
	auto result = _dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_vertBuff.ReleaseAndGetAddressOf()));

	unsigned char* vertMap = nullptr;
	result = _vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(vertices.begin(), vertices.end(), vertMap);
	_vertBuff->Unmap(0, nullptr);


	_vbView.BufferLocation = _vertBuff->GetGPUVirtualAddress();//バッファの仮想アドレス
	_vbView.SizeInBytes = static_cast<UINT>(vertices.size());//全バイト数
	_vbView.StrideInBytes = static_cast<UINT>(pmdvertex_size);//1頂点あたりのバイト数

	std::vector<unsigned short> indices(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);//一気に読み込み

	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]));
	//設定は、バッファのサイズ以外頂点バッファの設定を使いまわして
	//OKだと思います。
	result = _dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_idxBuff.ReleaseAndGetAddressOf()));

	//作ったバッファにインデックスデータをコピー
	unsigned short* mappedIdx = nullptr;
	_idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(indices.begin(), indices.end(), mappedIdx);
	_idxBuff->Unmap(0, nullptr);


	//インデックスバッファビューを作成
	_ibView.BufferLocation = _idxBuff->GetGPUVirtualAddress();
	_ibView.Format = DXGI_FORMAT_R16_UINT;
	_ibView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(indices[0]));


	fread(&_materialNum, sizeof(_materialNum), 1, fp);
	_materials.resize(_materialNum);
	_textureResources.resize(_materialNum);
	_sphResources.resize(_materialNum);
	_spaResources.resize(_materialNum);
	_toonResources.resize(_materialNum);

	std::vector<PMDMaterial> pmdMaterials(_materialNum);
	fread(pmdMaterials.data(), pmdMaterials.size() * sizeof(PMDMaterial), 1, fp);
	//コピー
	for (int i = 0; i < pmdMaterials.size(); ++i) {
		_materials[i].indicesNum = pmdMaterials[i].indicesNum;
		_materials[i].material.diffuse = pmdMaterials[i].diffuse;
		_materials[i].material.alpha = pmdMaterials[i].alpha;
		_materials[i].material.specular = pmdMaterials[i].specular;
		_materials[i].material.specularity = pmdMaterials[i].specularity;
		_materials[i].material.ambient = pmdMaterials[i].ambient;
		_materials[i].additional.toonIdx = pmdMaterials[i].toonIdx;
	}

	for (int i = 0; i < pmdMaterials.size(); ++i) {
		//トゥーンリソースの読み込み
		string toonFilePath = "toon/";
		char toonFileName[16];
		sprintf_s(toonFileName, 16, "toon%02d.bmp", pmdMaterials[i].toonIdx + 1);
		toonFilePath += toonFileName;
		_toonResources[i] = LoadTextureFromFile(toonFilePath);

		if (strlen(pmdMaterials[i].texFilePath) == 0) {
			_textureResources[i] = nullptr;
			continue;
		}

		string texFileName = pmdMaterials[i].texFilePath;
		string sphFileName = "";
		string spaFileName = "";
		if (count(texFileName.begin(), texFileName.end(), '*') > 0) {//スプリッタがある
			auto namepair = SplitFileName(texFileName);
			if (GetExtension(namepair.first) == "sph") {
				texFileName = namepair.second;
				sphFileName = namepair.first;
			}
			else if (GetExtension(namepair.first) == "spa") {
				texFileName = namepair.second;
				spaFileName = namepair.first;
			}
			else {
				texFileName = namepair.first;
				if (GetExtension(namepair.second) == "sph") {
					sphFileName = namepair.second;
				}
				else if (GetExtension(namepair.second) == "spa") {
					spaFileName = namepair.second;
				}
			}
		}
		else {
			if (GetExtension(pmdMaterials[i].texFilePath) == "sph") {
				sphFileName = pmdMaterials[i].texFilePath;
				texFileName = "";
			}
			else if (GetExtension(pmdMaterials[i].texFilePath) == "spa") {
				spaFileName = pmdMaterials[i].texFilePath;
				texFileName = "";
			}
			else {
				texFileName = pmdMaterials[i].texFilePath;
			}
		}
		//モデルとテクスチャパスからアプリケーションからのテクスチャパスを得る
		if (texFileName != "") {
			auto texFilePath = GetTexturePathFromModelAndTexPath(strModelPath, texFileName.c_str());
			_textureResources[i] = LoadTextureFromFile(texFilePath);
		}
		if (sphFileName != "") {
			auto sphFilePath = GetTexturePathFromModelAndTexPath(strModelPath, sphFileName.c_str());
			_sphResources[i] = LoadTextureFromFile(sphFilePath);
		}
		if (spaFileName != "") {
			auto spaFilePath = GetTexturePathFromModelAndTexPath(strModelPath, spaFileName.c_str());
			_spaResources[i] = LoadTextureFromFile(spaFilePath);
		}


	}
	fclose(fp);
	return result;
}

HRESULT Application::CreateMaterialData()
{
	//マテリアルバッファを作成
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * _materialNum);//勿体ないけど仕方ないですね
	auto result = _dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_materialBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	//マップマテリアルにコピー
	char* mapMaterial = nullptr;
	result = _materialBuff->Map(0, nullptr, (void**)&mapMaterial);
	if (FAILED(result)) {
		assert(0);
		return result;
	}
	for (auto& m : _materials) {
		*((MaterialForHlsl*)mapMaterial) = m.material;//データコピー
		mapMaterial += materialBuffSize;//次のアライメント位置まで進める
	}
	_materialBuff->Unmap(0, nullptr);


	D3D12_DESCRIPTOR_HEAP_DESC materialDescHeapDesc = {};
	materialDescHeapDesc.NumDescriptors = _materialNum * 5;//マテリアル数ぶん(定数1つ、テクスチャ3つ)
	materialDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	materialDescHeapDesc.NodeMask = 0;

	materialDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	result = _dev->CreateDescriptorHeap(&materialDescHeapDesc, IID_PPV_ARGS(_materialDescHeap.ReleaseAndGetAddressOf()));//生成
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	matCBVDesc.BufferLocation = _materialBuff->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = static_cast<UINT>(materialBuffSize);
	return result;
}

HRESULT Application::CreateSceneTransformView()
{
	//定数バッファ作成
	_worldMat = XMMatrixIdentity();
	XMFLOAT3 eye(0, 15, -15);
	XMFLOAT3 target(0, 15, 0);
	XMFLOAT3 up(0, 1, 0);
	_viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	_projMat = XMMatrixPerspectiveFovLH(XM_PIDIV4,//画角は45°
		static_cast<float>(window_width) / static_cast<float>(window_height),//アス比
		1.0f,//近い方
		100.0f//遠い方
	);

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + 0xff) & ~0xff);
	auto result = _dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_constBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		assert(0);
		return result;
	}

	_mapScene = nullptr;//マップ先を示すポインタ
	result = _constBuff->Map(0, nullptr, (void**)&_mapScene);//マップ
	//行列の内容をコピー
	_mapScene->world = _worldMat;
	_mapScene->view = _viewMat;
	_mapScene->proj = _projMat;
	_mapScene->eye = eye;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	descHeapDesc.NodeMask = 0;//マスクは0
	descHeapDesc.NumDescriptors = 1;//
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_basicDescHeap.ReleaseAndGetAddressOf()));//生成

	////デスクリプタの先頭ハンドルを取得しておく
	auto basicHeapHandle = _basicDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_constBuff->GetDesc().Width);
	//定数バッファビューの作成
	_dev->CreateConstantBufferView(&cbvDesc, basicHeapHandle);
	return result;
}

void Application::CreateMaterialAndTextureView()
{
	auto materialBuffSize = matCBVDesc.SizeInBytes;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//後述
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1
	CD3DX12_CPU_DESCRIPTOR_HANDLE matDescHeapH(_materialDescHeap->GetCPUDescriptorHandleForHeapStart());
	auto incSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (size_t i = 0; i < _materialNum; ++i) {
		//マテリアル固定バッファビュー
		_dev->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
		matDescHeapH.ptr += static_cast<ULONG_PTR>(incSize);
		matCBVDesc.BufferLocation += materialBuffSize;
		if (_textureResources[i] == nullptr) {
			srvDesc.Format = _whiteTex->GetDesc().Format;
			_dev->CreateShaderResourceView(_whiteTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _textureResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(_textureResources[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.Offset(incSize);

		if (_sphResources[i] == nullptr) {
			srvDesc.Format = _whiteTex->GetDesc().Format;
			_dev->CreateShaderResourceView(_whiteTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _sphResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(_sphResources[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (_spaResources[i] == nullptr) {
			srvDesc.Format = _blackTex->GetDesc().Format;
			_dev->CreateShaderResourceView(_blackTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _spaResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(_spaResources[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;


		if (_toonResources[i] == nullptr) {
			srvDesc.Format = _gradTex->GetDesc().Format;
			_dev->CreateShaderResourceView(_gradTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _toonResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(_toonResources[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;
	}
}

bool Application::Init()
{
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);
	CreateGameWindow(_hwnd, _windowClass);
#ifdef _DEBUG
	//デバッグレイヤーをオンに
	EnableDebugLayer();
#endif

	//DirectX12関連初期化
	if (FAILED(InitializeDXGIDevice())) {
		assert(0);
		return false;
	}
	if (FAILED(InitializeCommand())) {
		assert(0);
		return false;
	}
	if (FAILED(CreateSwapChain(_hwnd, _dxgiFactory))) {
		assert(0);
		return false;
	}
	if (FAILED(CreateFinalRenderTarget(_rtvHeaps, _backBuffer))) {
		assert(0);
		return false;
	}

	//テクスチャローダー関連初期化
	CreateTextureLoaderTable();


	//深度バッファ作成
	if (FAILED(CreateDepthStencilView())) {
		assert(0);
		return false;
	}

	//フェンスの作成
	if (FAILED(_dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf())))) {
		assert(0);
		return false;
	}


	_whiteTex = CreateWhiteTexture();
	_blackTex = CreateBlackTexture();
	_gradTex = CreateGrayGradationTexture();

	
	if (FAILED(LoadPMDFile("Model/初音ミク.pmd"))) {
		return false;
	}

	//ロードしたデータをもとにバッファにマテリアルデータを作る
	if (FAILED(CreateMaterialData())) {
		return false;
	}

	//マテリアルまわりのビュー作成およびテクスチャビュー作成
	//同じデスクリプタヒープ内に作っていくためマテリアルと
	//強い結合状態になっている。
	CreateMaterialAndTextureView();

	//ルートシグネチャ作成
	if (FAILED(CreateRootSignature())) {
		return false;
	}

	//パイプライン設定(シェーダもここで設定)
	if (FAILED(CreateBasicGraphicsPipeline())) {
		return false;
	}

	if (FAILED(CreateSceneTransformView())) {
		return false;
	}
	return true;
}

void Application::Terminate()
{
	UnregisterClass(_windowClass.lpszClassName, _windowClass.hInstance);
}
Application::Application()
{
}


Application::~Application()
{
}

Application&
Application::Instance() {
	static Application instance;
	return instance;
}
