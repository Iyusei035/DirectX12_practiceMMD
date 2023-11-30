#pragma once
#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<vector>
#include<map>
#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<d3dx12.h>
#include<dxgidebug.h>
#include<wrl.h>
class Application
{
private:
	//ウィンドウ回り
	WNDCLASSEX _windowClass;
	HWND _hwnd;
	//DXGI
	Microsoft::WRL::ComPtr<IDXGIFactory6> _dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> _swapchain = nullptr;
	//Direct12
	Microsoft::WRL::ComPtr<ID3D12Device>_dev = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>_cmdAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>_cmdList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>_cmdQueue = nullptr;
	//バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource>_depthBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>_vertBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>_idxBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>_constBuff = nullptr;

	//load
	using LoadLamba_t = std::function<HRESULT(const std::wstring& path, DirectX::TexMetadata*, DirectX::ScratchImage&)>;
	std::map<std::string, LoadLamba_t>_loadLambdaTable;
	//material
	unsigned int _materialNum;
	Microsoft::WRL::ComPtr<ID3D12Resource>_materialBuff = nullptr;
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	//Default Texture
	Microsoft::WRL::ComPtr<ID3D12Resource>_whiteTex = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>_blackTex = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>_gradTex = nullptr;
	//座標変換行列
	DirectX::XMMATRIX _worldMat;
	DirectX::XMMATRIX _viewMat;
	DirectX::XMMATRIX _projMat;
	//シェーダへのマテリアルデータ
	struct MaterialForHlsl
	{
		DirectX::XMFLOAT3	diffuse;		//ディヒューズ色
		float				alpha;			//ディヒューズα
		DirectX::XMFLOAT3	specular;		//スペキュラ色
		float				specularity;	//スペキュラの強さ(乗算値)
		DirectX::XMFLOAT3	ambient;		//アンビエント色
	};
	//マテリアルデータ
	struct AdditionalMaterial
	{
		std::string	texPath;	//テクスチャファイルパス
		int			toonIdx;	//トゥーン番号
		bool		edgeFlg;	//マテリアルごとの輪郭線フラグ
	};
	//まとめ
	struct Material
	{
		unsigned int		indicesNum;		//インデックス数
		MaterialForHlsl		material;		
		AdditionalMaterial	additional;		
	};
	std::vector<Material>_materials;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>_textureResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>_sphResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>_spaResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>_toonResources;
	//環境データ
	struct SceneData {
		DirectX::XMMATRIX world;//ワールド行列
		DirectX::XMMATRIX view;//ビュープロジェクション行列
		DirectX::XMMATRIX proj;//
		DirectX::XMFLOAT3 eye;//視点座標
	};
	SceneData* _mapScene;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>_basicDescHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>_materialDescHeap = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Fence>_fence = nullptr;
	UINT64 _fenceVal = 0;

	//頂点バッファ、インデックスバッファコピー
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	D3D12_INDEX_BUFFER_VIEW _ibView = {};

	//マップテーブル
	std::map<std::string, ID3D12Resource*>_resourceTable;

	//パイプライン、ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12PipelineState>_pipelinestate = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>_rootsignature = nullptr;

	std::vector<ID3D12Resource*>_backBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>_rtvHeaps = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>_dsvHeap = nullptr;
	CD3DX12_VIEWPORT _viewport;
	CD3DX12_RECT _scissorrect;

	//テクスチャバッファ
	ID3D12Resource* CreateWhiteTexture();
	ID3D12Resource* CreateBlackTexture();
	ID3D12Resource* CreateGrayGradationTexture();
	ID3D12Resource* LoadTextureFromFile(std::string&texpath);

	//最終的なレンダーターゲットの生成
	HRESULT	CreateFinalRenderTarget(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& rtvHeaps, std::vector<ID3D12Resource*>& backBuffers);

	//スワップチェインの生成
	HRESULT CreateSwapChain(const HWND& hwnd, Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory);

	//ゲーム用ウィンドウの生成
	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass);

	//DXGIまわり初期化
	HRESULT InitializeDXGIDevice();

	//コマンドまわり初期化
	HRESULT InitializeCommand();

	//パイプライン初期化
	HRESULT CreateBasicGraphicsPipeline();
	//ルートシグネチャ初期化
	HRESULT CreateRootSignature();

	//テクスチャローダテーブルの作成
	void CreateTextureLoaderTable();

	//デプスステンシルビューの生成
	HRESULT CreateDepthStencilView();

	//PMDファイルのロード
	HRESULT LoadPMDFile(const char* path);

	//GPU側のマテリアルデータの作成
	HRESULT CreateMaterialData();

	//座標変換用ビューの生成
	HRESULT CreateSceneTransformView();

	//マテリアル＆テクスチャのビューを作成
	void CreateMaterialAndTextureView();

	//↓シングルトンのためにコンストラクタをprivateに
	Application();
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;
public:
	///Applicationのシングルトンインスタンスを得る
	static Application& Instance();

	///初期化
	bool Init();

	///ループ起動
	void Run();

	///後処理
	void Terminate();

	~Application();
};
