#ifndef __CRYDXPSRENDERDEVICE__
#define __CRYDXPSRENDERDEVICE__

#include "../CCryDXPSDummy.hpp"
#include "../CCryDXPSMisc.hpp"

#include "../Layer0/CCryDXPS.hpp"
#include "Resource/Buffer/CCryDXPSBuffer.hpp"
#include "Resource/Buffer/CCryDXPSSamplerState.hpp"
#include "Resource/Misc/CCryDXPSShader.hpp"
#include "Resource/Misc/CCryDXPSInputLayout.hpp"
#include "Resource/Misc/CCryDXPSRenderTargetView.hpp"
#include "Resource/Misc/CCryDXPSDepthStencilView.hpp"
#include "Resource/Misc/CCryDXPSShaderResourceView.hpp"
#include "Resource/Misc/CCryDXPSShaderReflection.hpp"
#include "Resource/Textures/CCryDXPSTexture1D.hpp"
#include "Resource/Textures/CCryDXPSTexture2D.hpp"
#include "Resource/Textures/CCryDXPSTexture3D.hpp"
#include "Resource/States/CCryDXPSBlendState.hpp"
#include "Resource/States/CCryDXPSRasterizerState.hpp"
#include "Resource/States/CCryDXPSDepthStencilState.hpp"

const uint32 CRYDXPSDEVICE_MAXRENDERTARGETS	=	4;

class CCryDXPSSwapChain;

class CCryDXPSRenderDevice;
class CCryDXPSRenderDevice		:		public	CCryRefAndWeak<CCryDXPSRenderDevice>
{
	friend class CCryDXPSSwapChain;
private:

	CellGcmConfig									m_GCMConfig;
	D3D10_VIEWPORT								m_GCMViewPort;
	APWeakTexture2D								m_pRenderTargetViews[CRYDXPSDEVICE_MAXRENDERTARGETS];
	APWeakTexture2D								m_pDepthStencilView;
	APWeakInputLayout							m_pInputLayout;
	APWeakShader									m_pVShader;
	APWeakShader									m_pPShader;
	APWeakBuffer									m_pVSConstBuffer[16];
	APWeakBuffer									m_pPSConstBuffer[16];
//	uint32												m_VSCBOffset;
//	uint32												m_PSCBOffset;
//	float													m_PSConstBuffer[256*4];
	APWeakBuffer									m_pIndexBuffer;
	APWeakBuffer									m_pVertexBuffer[16];//max 16 input slots
	uint32												m_VBStride[16];
	uint32												m_VBOffset[16];
	uint32												m_IBFormat;
	uint32												m_IBOffset;

	bool													m_ClearableFormat;


	D3D10_PRIMITIVE_TOPOLOGY			m_Topology;
	bool													m_Rendering;

			long						CreateSwapChain(			CCryDXPSSwapChain&		rSwapChain,
																						DXGI_SWAP_CHAIN_DESC*	pSwapChainDesc);

#if _DEBUG
public:
	uint32												m_BreakDrawCall;
	uint32												m_DrawCounter;
#endif

public:
											CCryDXPSRenderDevice();
											CCryDXPSRenderDevice(	IDXGIAdapter*			pAdapter,
																						D3D10_DRIVER_TYPE DriverType,
																						int								Software,
																						uint32			Flags,
																						uint32			SDKVersion);
											~CCryDXPSRenderDevice();

			unsigned long		AddRef(){return IncRef();}
			unsigned long		Release(){return DecRef();}
			HRESULT					QueryInterface(REFIID riid,void **ppvObject);
 
			void						BeginScene();
			void						EndScene();

			inline	void		ReleaseResources(){}
			void  					VSSetConstantBuffers(uint32	Offset,uint32 NumBuffers,ID3D10Buffer *const *ppConstantBuffers);
			void  					PSSetShaderResources(uint32	Offset,uint32 NumViews,ID3D10ShaderResourceView *const *ppShaderResourceViews);
			void  					PSSetShader(ID3D10PixelShader *pPixelShader);
			void  					PSSetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState *const *ppSamplers);
			void  					VSSetShader(ID3D10VertexShader *pVertexShader);
			void  					DrawIndexed(uint32 IndexCount,uint32 StartIndexLocation,int BaseVertexLocation);
			void  					Draw(uint32 VertexCount,uint32 StartVertexLocation);
			void  					PSSetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer *const *ppConstantBuffers);
			void  					IASetInputLayout(ID3D10InputLayout *pInputLayout);
			void  					IASetVertexBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer *const *ppVertexBuffers,const uint32*pStrides,const uint32*pOffsets);
			void  					IASetIndexBuffer(ID3D10Buffer *pIndexBuffer,DXGI_FORMAT Format,uint32 Offset);
			void  					DrawIndexedInstanced( uint32 IndexCountPerInstance,uint32 InstanceCount,uint32 StartIndexLocation,int BaseVertexLocation,uint32 StartInstanceLocation);
			void  					DrawInstanced( uint32 VertexCountPerInstance,uint32 InstanceCount,uint32 StartVertexLocation,uint32 StartInstanceLocation);
			void  					GSSetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer *const *ppConstantBuffers);
			void  					GSSetShader(ID3D10GeometryShader *pShader);
			void  					IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY Topology);
			void  					VSSetShaderResources(uint32 Offset,uint32 NumViews,ID3D10ShaderResourceView *const *ppShaderResourceViews);
			void  					VSSetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState *const *ppSamplers);
			void  					SetPredication(ID3D10Predicate *pPredicate,uint32 PredicateValue);
			void  					GSSetShaderResources(uint32 Offset,uint32 NumViews,ID3D10ShaderResourceView *const *ppShaderResourceViews);
			void  					GSSetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState *const *ppSamplers);
			void  					OMSetRenderTargets(uint32 NumViews,ID3D10RenderTargetView *const *ppRenderTargetViews,ID3D10DepthStencilView *pDepthStencilView);
			void  					OMSetBlendState(ID3D10BlendState *pBlendState,const float BlendFactor[ 4 ],uint32 SampleMask);
			void  					OMSetDepthStencilState(ID3D10DepthStencilState *pDepthStencilState,uint32 StencilRef);
			void  					SOSetTargets(uint32 NumBuffers,ID3D10Buffer *const *ppSOTargets,uint32 *pOffsets);
			void  					DrawAuto( void);
			void  					RSSetState(ID3D10RasterizerState *pRasterizerState);
			void  					RSSetViewports(uint32 NumViewports,const D3D10_VIEWPORT *pViewports);
			void  					RSSetScissorRects(uint32 NumRects,const D3D10_RECT *pRects);
			void  					CopySubresourceRegion(ID3D10Resource *pDstResource,uint32 DstSubresource,uint32 DstX,uint32 DstY,uint32 DstZ,ID3D10Resource *pSrcResource,uint32 SrcSubresource,const D3D10_BOX *pSrcBox);
			void  					CopyResource(ID3D10Resource *pDstResource,ID3D10Resource *pSrcResource);
			void  					UpdateSubresource(ID3D10Resource *pDstResource,uint32 DstSubresource,const D3D10_BOX *pDstBox,const void *pSrcData,uint32 SrcRowPitch,uint32 SrcDepthPitch);
			void  					ClearRenderTargetView( ID3D10RenderTargetView *pRenderTargetView,const float ColorRGBA[ 4 ]);
			void  					ClearDepthStencilView( ID3D10DepthStencilView *pDepthStencilView,uint32 ClearFlags,float Depth,uint8 Stencil);
			void  					GenerateMips(ID3D10ShaderResourceView *pShaderResourceView);
			void  					ResolveSubresource(ID3D10Resource *pDstResource,uint32 DstSubresource,ID3D10Resource *pSrcResource,uint32 SrcSubresource,DXGI_FORMAT Format);
			void  					VSGetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer **ppConstantBuffers);
			void  					PSGetShaderResources(uint32 Offset,uint32 NumViews,ID3D10ShaderResourceView **ppShaderResourceViews);
			void  					PSGetShader(ID3D10PixelShader **ppPixelShader);
			void  					PSGetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState **ppSamplers);
			void  					VSGetShader(ID3D10VertexShader **ppVertexShader);
			void  					PSGetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer **ppConstantBuffers);
			void  					IAGetInputLayout(ID3D10InputLayout **ppInputLayout);
			void  					IAGetVertexBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer **ppVertexBuffers,uint32*pStrides,uint32 *pOffsets);
			void  					IAGetIndexBuffer(ID3D10Buffer **pIndexBuffer,DXGI_FORMAT *Format,uint32*Offset);
			void  					GSGetConstantBuffers(uint32 Offset,uint32 NumBuffers,ID3D10Buffer **ppConstantBuffers);
			void  					GSGetShader(ID3D10GeometryShader **ppGeometryShader);
			void  					IAGetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY *pTopology);
			void  					VSGetShaderResources(uint32 Offset,uint32 NumViews,ID3D10ShaderResourceView **ppShaderResourceViews);
			void  					VSGetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState **ppSamplers);
			void  					GetPredication(ID3D10Predicate **ppPredicate,BOOL *pPredicateValue);
			void  					GSGetShaderResources(uint32 Offset,uint32 NumViews,ID3D10ShaderResourceView **ppShaderResourceViews);
			void  					GSGetSamplers(uint32 Offset,uint32 NumSamplers,ID3D10SamplerState **ppSamplers);
			void  					OMGetRenderTargets(uint32 NumViews,ID3D10RenderTargetView **ppRenderTargetViews,ID3D10DepthStencilView **ppDepthStencilView);
			void  					OMGetBlendState(ID3D10BlendState **ppBlendState,float BlendFactor[ 4 ],uint32*pSampleMask);
			void  					OMGetDepthStencilState(ID3D10DepthStencilState **ppDepthStencilState,uint32*pStencilRef);
			void  					SOGetTargets(uint32 NumBuffers,ID3D10Buffer **ppSOTargets,uint32* pOffsets);
			void  					RSGetState(ID3D10RasterizerState **ppRasterizerState);
			void  					RSGetViewports(uint32* NumViewports,D3D10_VIEWPORT *pViewports);
			void  					RSGetScissorRects(uint32* NumRects,D3D10_RECT *pRects);
			long  					GetDeviceRemovedReason( void);
			long  					SetExceptionMode(uint32 intRaiseFlags);
			uint32					GetExceptionMode( void);
			long  					GetPrivateData(const ECryGUID & guid,uint32*pDataSize,void *pData);
			long  					SetPrivateData(const ECryGUID & guid,uint32 DataSize,const void *pData);
			long  					SetPrivateDataInterface( const ECryGUID & guid,const IUnknown *pData);
			void						SetTextFilterSize(unsigned int Width,unsigned int Height);
			void						GetTextFilterSize(unsigned int* Width,unsigned int* Height);
			void  					ClearState( void);        
			void  					Flush( void);        
			long  					CreateBuffer(const D3D10_BUFFER_DESC *pDesc,const D3D10_SUBRESOURCE_DATA *pInitialData,ID3D10Buffer **ppBuffer);
			long  					CreateTexture1D(const D3D10_TEXTURE1D_DESC *pDesc,const D3D10_SUBRESOURCE_DATA *pInitialData,ID3D10Texture1D **ppTexture1D);
			long  					CreateTexture2D(const D3D10_TEXTURE2D_DESC *pDesc,const D3D10_SUBRESOURCE_DATA *pInitialData,ID3D10Texture2D **ppTexture2D);
			long  					CreateTexture3D(const D3D10_TEXTURE3D_DESC *pDesc,const D3D10_SUBRESOURCE_DATA *pInitialData,ID3D10Texture3D **ppTexture3D);
			long  					CreateShaderResourceView( CCryDXPSResource *pResource,const D3D10_SHADER_RESOURCE_VIEW_DESC *pDesc,ID3D10ShaderResourceView **ppSRView);
			long  					CreateRenderTargetView(ID3D10Resource *pResource,const D3D10_RENDER_TARGET_VIEW_DESC *pDesc,ID3D10RenderTargetView **ppRTView);
			long  					CreateDepthStencilView(ID3D10Resource *pResource,const D3D10_DEPTH_STENCIL_VIEW_DESC *pDesc,ID3D10DepthStencilView **ppDepthStencilView);
			long  					CreateInputLayout(const D3D10_INPUT_ELEMENT_DESC *pInputElementDescs,uint32 NumElements,const void *pShaderBytecodeWithInputSignature,size_t Size,ID3D10InputLayout **ppInputLayout);
			long  					CreateVertexShader(const void *pShaderBytecode,size_t Size,ID3D10VertexShader **ppVertexShader);
			long  					CreateGeometryShader(const void *pShaderBytecode,size_t Size,ID3D10GeometryShader **ppGeometryShader);
			long  					CreateGeometryShaderWithStreamOutput(const void *pShaderBytecode,size_t BytecondeLength,const D3D10_SO_DECLARATION_ENTRY *pSODeclaration,uint32 NumEntries,uint32 OutputStreamStride,ID3D10GeometryShader **ppGeometryShader);
			long  					CreatePixelShader(const void *pShaderBytecode,size_t Size,ID3D10PixelShader **ppPixelShader);
			long  					CreateBlendState(const D3D10_BLEND_DESC *pBlendStateDesc,ID3D10BlendState **ppBlendState);
			long  					CreateDepthStencilState(const D3D10_DEPTH_STENCIL_DESC *pDepthStencilDesc,ID3D10DepthStencilState **ppDepthStencilState);
			long  					CreateRasterizerState(const D3D10_RASTERIZER_DESC *pRasterizerDesc,ID3D10RasterizerState **ppRasterizerState);
			long  					CreateSamplerState(const D3D10_SAMPLER_DESC *pSamplerDesc,ID3D10SamplerState **ppSamplerState);
			long  					CreateQuery(const D3D10_QUERY_DESC *pQueryDesc,ID3D10Query **ppQuery);
			long  					CreatePredicate(const D3D10_QUERY_DESC *pPredicateDesc,ID3D10Predicate **ppPredicate);
			long  					CreateCounter(const D3D10_COUNTER_DESC *pCounterDesc,ID3D10Counter **ppCounter);
			long  					CheckFormatSupport(DXGI_FORMAT Format,uint32*pFormatSupport);
			long  					CheckMultisampleQualityLevels(DXGI_FORMAT Format,uint32 SampleCount,uint32*pNumQualityLevels);
			void  					CheckCounterInfo(D3D10_COUNTER_INFO *pCounterInfo);
			long  					CheckCounter(const D3D10_COUNTER_DESC *pDesc,D3D10_COUNTER_TYPE *pType,uint32*pActiveCounters,const char* szName,uint32 *pNameLength,const char* szUnits,uint32 *pUnitsLength,const char* szDescription,uint32 *pDescriptionLength);
			uint32					GetCreationFlags( void);
			long						OpenSharedResource(int hResource,const CRYIID & ReturnedInterface,void **ppResource);

#ifdef CRY_USE_GCM
			void						UpdateRendertargets();
#endif
};

typedef CCryDXPSRenderDevice ID3D10Device;
typedef CCryAPtrRefCnt<ID3D10Device> LPD3D10DEVICE;

#endif

