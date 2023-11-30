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
	//�E�B���h�E���
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
	//�o�b�t�@
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
	//���W�ϊ��s��
	DirectX::XMMATRIX _worldMat;
	DirectX::XMMATRIX _viewMat;
	DirectX::XMMATRIX _projMat;
	//�V�F�[�_�ւ̃}�e���A���f�[�^
	struct MaterialForHlsl
	{
		DirectX::XMFLOAT3	diffuse;		//�f�B�q���[�Y�F
		float				alpha;			//�f�B�q���[�Y��
		DirectX::XMFLOAT3	specular;		//�X�y�L�����F
		float				specularity;	//�X�y�L�����̋���(��Z�l)
		DirectX::XMFLOAT3	ambient;		//�A���r�G���g�F
	};
	//�}�e���A���f�[�^
	struct AdditionalMaterial
	{
		std::string	texPath;	//�e�N�X�`���t�@�C���p�X
		int			toonIdx;	//�g�D�[���ԍ�
		bool		edgeFlg;	//�}�e���A�����Ƃ̗֊s���t���O
	};
	//�܂Ƃ�
	struct Material
	{
		unsigned int		indicesNum;		//�C���f�b�N�X��
		MaterialForHlsl		material;		
		AdditionalMaterial	additional;		
	};
	std::vector<Material>_materials;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>_textureResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>_sphResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>_spaResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>_toonResources;
	//���f�[�^
	struct SceneData {
		DirectX::XMMATRIX world;//���[���h�s��
		DirectX::XMMATRIX view;//�r���[�v���W�F�N�V�����s��
		DirectX::XMMATRIX proj;//
		DirectX::XMFLOAT3 eye;//���_���W
	};
	SceneData* _mapScene;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>_basicDescHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>_materialDescHeap = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Fence>_fence = nullptr;
	UINT64 _fenceVal = 0;

	//���_�o�b�t�@�A�C���f�b�N�X�o�b�t�@�R�s�[
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	D3D12_INDEX_BUFFER_VIEW _ibView = {};

	//�}�b�v�e�[�u��
	std::map<std::string, ID3D12Resource*>_resourceTable;

	//�p�C�v���C���A���[�g�V�O�l�`��
	Microsoft::WRL::ComPtr<ID3D12PipelineState>_pipelinestate = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>_rootsignature = nullptr;

	std::vector<ID3D12Resource*>_backBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>_rtvHeaps = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>_dsvHeap = nullptr;
	CD3DX12_VIEWPORT _viewport;
	CD3DX12_RECT _scissorrect;

	//�e�N�X�`���o�b�t�@
	ID3D12Resource* CreateWhiteTexture();
	ID3D12Resource* CreateBlackTexture();
	ID3D12Resource* CreateGrayGradationTexture();
	ID3D12Resource* LoadTextureFromFile(std::string&texpath);

	//�ŏI�I�ȃ����_�[�^�[�Q�b�g�̐���
	HRESULT	CreateFinalRenderTarget(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& rtvHeaps, std::vector<ID3D12Resource*>& backBuffers);

	//�X���b�v�`�F�C���̐���
	HRESULT CreateSwapChain(const HWND& hwnd, Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory);

	//�Q�[���p�E�B���h�E�̐���
	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass);

	//DXGI�܂�菉����
	HRESULT InitializeDXGIDevice();

	//�R�}���h�܂�菉����
	HRESULT InitializeCommand();

	//�p�C�v���C��������
	HRESULT CreateBasicGraphicsPipeline();
	//���[�g�V�O�l�`��������
	HRESULT CreateRootSignature();

	//�e�N�X�`�����[�_�e�[�u���̍쐬
	void CreateTextureLoaderTable();

	//�f�v�X�X�e���V���r���[�̐���
	HRESULT CreateDepthStencilView();

	//PMD�t�@�C���̃��[�h
	HRESULT LoadPMDFile(const char* path);

	//GPU���̃}�e���A���f�[�^�̍쐬
	HRESULT CreateMaterialData();

	//���W�ϊ��p�r���[�̐���
	HRESULT CreateSceneTransformView();

	//�}�e���A�����e�N�X�`���̃r���[���쐬
	void CreateMaterialAndTextureView();

	//���V���O���g���̂��߂ɃR���X�g���N�^��private��
	Application();
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;
public:
	///Application�̃V���O���g���C���X�^���X�𓾂�
	static Application& Instance();

	///������
	bool Init();

	///���[�v�N��
	void Run();

	///�㏈��
	void Terminate();

	~Application();
};
