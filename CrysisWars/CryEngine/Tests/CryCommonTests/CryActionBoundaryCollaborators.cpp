#include "StdAfx.h"
#include "CryActionBoundaryCollaborators.h"
#include <cstdio>
#include <cstdlib>
#include <algorithm>

void CryActionRegressionRenderer::AddListener(IRendererEventListener *pRendererEventListener)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::AddListener\n"); std::abort();
}
void CryActionRegressionRenderer::RemoveListener(IRendererEventListener *pRendererEventListener)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::RemoveListener\n"); std::abort();
}
ERenderType CryActionRegressionRenderer::GetRenderType() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetRenderType\n"); std::abort();
}
WIN_HWND CryActionRegressionRenderer::Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd, bool bReInit, const SCustomRenderInitArgs* pCustomArgs)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::Init\n"); std::abort();
}
bool CryActionRegressionRenderer::SetCurrentContext(WIN_HWND hWnd)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetCurrentContext\n"); std::abort();
}
bool CryActionRegressionRenderer::CreateContext(WIN_HWND hWnd, bool bAllowFSAA)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::CreateContext\n"); std::abort();
}
bool CryActionRegressionRenderer::DeleteContext(WIN_HWND hWnd)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DeleteContext\n"); std::abort();
}
int CryActionRegressionRenderer::GetFeatures()
{
    return RFT_RGBA;
}
void CryActionRegressionRenderer::GetVideoMemoryUsageStats( size_t& vidMemUsedThisFrame, size_t& vidMemUsedRecently )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetVideoMemoryUsageStats\n"); std::abort();
}
int CryActionRegressionRenderer::GetCurrentNumberOfDrawCalls()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetCurrentNumberOfDrawCalls\n"); std::abort();
}
void CryActionRegressionRenderer::ShutDown(bool bReInit)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ShutDown\n"); std::abort();
}
void CryActionRegressionRenderer::ShutDownFast()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ShutDownFast\n"); std::abort();
}
int CryActionRegressionRenderer::EnumDisplayFormats(SDispFormat *Formats)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EnumDisplayFormats\n"); std::abort();
}
int CryActionRegressionRenderer::EnumAAFormats(const SDispFormat &rDispFmt, SAAFormat *Formats)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EnumAAFormats\n"); std::abort();
}
bool CryActionRegressionRenderer::ChangeResolution(int nNewWidth, int nNewHeight, int nNewColDepth, int nNewRefreshHZ, bool bFullScreen)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ChangeResolution\n"); std::abort();
}
void CryActionRegressionRenderer::Release()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::Release\n"); std::abort();
}
void CryActionRegressionRenderer::FreeResources(int nFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FreeResources\n"); std::abort();
}
void CryActionRegressionRenderer::BeginFrame()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::BeginFrame\n"); std::abort();
}
void CryActionRegressionRenderer::RenderDebug()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::RenderDebug\n"); std::abort();
}
void CryActionRegressionRenderer::EndFrame()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EndFrame\n"); std::abort();
}
void CryActionRegressionRenderer::GetViewport(int *x, int *y, int *width, int *height)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetViewport\n"); std::abort();
}
void CryActionRegressionRenderer::SetViewport(int x, int y, int width, int height)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetViewport\n"); std::abort();
}
void CryActionRegressionRenderer::SetRenderTile(f32 nTilesPosX, f32 nTilesPosY, f32 nTilesGridSizeX, f32 nTilesGridSizeY)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetRenderTile\n"); std::abort();
}
void CryActionRegressionRenderer::SetScissor(int x, int y, int width, int height)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetScissor\n"); std::abort();
}
EScreenAspectRatio CryActionRegressionRenderer::GetScreenAspect(int nWidth, int nHeight)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetScreenAspect\n"); std::abort();
}
void CryActionRegressionRenderer::MakeCurrent()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::MakeCurrent\n"); std::abort();
}
void CryActionRegressionRenderer::DrawTriStrip(CVertexBuffer *src, int vert_num)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawTriStrip\n"); std::abort();
}
int CryActionRegressionRenderer::GetDynVBSize( int vertexType)
{
    return vertexCapacity;
}
void *CryActionRegressionRenderer::GetDynVBPtr(int nVerts, int &nOffs, int Pool)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetDynVBPtr\n"); std::abort();
}
void CryActionRegressionRenderer::DrawDynVB(int nOffs, int Pool, int nVerts)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawDynVB\n"); std::abort();
}
void CryActionRegressionRenderer::DrawDynVB(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F* vertices, ushort* indices, int vertexCount, int indexCount, int primitive)
{
    CryActionRegressionDraw draw;
    draw.vertices.assign(vertices, vertices + vertexCount);
    draw.indices.assign(indices, indices + indexCount);
    draw.primitive = primitive; draw.texture = texture; draw.state = state;
    draws.push_back(std::move(draw));
}
CVertexBuffer *CryActionRegressionRenderer::CreateBuffer(int vertexcount,int vertexformat, const char *szSource, bool bDynamic)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::CreateBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::CreateBuffer(int size, int vertexformat, CVertexBuffer *buf, int Type, const char *szSource, bool bDynamic)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::CreateBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::ReleaseBuffer(CVertexBuffer *bufptr, int nVerts)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ReleaseBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::DrawBuffer(CVertexBuffer *src,SVertexStream *indicies,int numindices, int offsindex, int prmode,int vert_start,int vert_stop, CRenderChunk *pChunk)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::UpdateBuffer(CVertexBuffer *dest,const void *src,int vertexcount, bool bUnLock, int nOffs, int Type)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::UpdateBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::UnlockBuffer(CVertexBuffer *dest, int Type, int nVerts)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::UnlockBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::CreateIndexBuffer(SVertexStream *dest,const void *src,int indexcount, int oldIndexCount)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::CreateIndexBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::UpdateIndexBuffer(SVertexStream *dest,const void *src, int indexcount, int oldIndexCount, bool bUnLock, bool bDynamic)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::UpdateIndexBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::ReleaseIndexBuffer(SVertexStream *dest, int nIndices)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ReleaseIndexBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::CheckError(const char *comment)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::CheckError\n"); std::abort();
}
void CryActionRegressionRenderer::Draw3dBBox(const Vec3 &mins,const Vec3 &maxs, int nPrimType)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::Draw3dBBox\n"); std::abort();
}
void CryActionRegressionRenderer::SetCamera(const CCamera &cam)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetCamera\n"); std::abort();
}
const CCamera& CryActionRegressionRenderer::GetCamera()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetCamera\n"); std::abort();
}
bool CryActionRegressionRenderer::SetGammaDelta(const float fGamma)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetGammaDelta\n"); std::abort();
}
void CryActionRegressionRenderer::RestoreGamma(void)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::RestoreGamma\n"); std::abort();
}
bool CryActionRegressionRenderer::ChangeDisplay(unsigned int width,unsigned int height,unsigned int cbpp)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ChangeDisplay\n"); std::abort();
}
void CryActionRegressionRenderer::ChangeViewport(unsigned int x,unsigned int y,unsigned int width,unsigned int height)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ChangeViewport\n"); std::abort();
}
bool CryActionRegressionRenderer::SaveTga(unsigned char *sourcedata,int sourceformat,int w,int h,const char *filename,bool flip)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SaveTga\n"); std::abort();
}
void CryActionRegressionRenderer::SetTexture(int value)
{
    texture = value;
}
void CryActionRegressionRenderer::SetWhiteTexture()
{
    texture = -1;
}
void CryActionRegressionRenderer::WriteXY( int x,int y, float xscale,float yscale,float r,float g,float b,float a,const char *message, ...) PRINTF_PARAMS(10, 11)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::WriteXY\n"); std::abort();
}
void CryActionRegressionRenderer::Draw2dText( float posX,float posY,const char *szText,SDrawTextInfo &info )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::Draw2dText\n"); std::abort();
}
void CryActionRegressionRenderer::Draw2dImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float angle,float r,float g,float b,float a,float z)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::Draw2dImage\n"); std::abort();
}
void CryActionRegressionRenderer::DrawImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float r,float g,float b,float a)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawImage\n"); std::abort();
}
void CryActionRegressionRenderer::DrawImageWithUV(float xpos,float ypos,float z,float width,float height,int texture_id,float s[4],float t[4],float r,float g,float b,float a)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawImageWithUV\n"); std::abort();
}
int CryActionRegressionRenderer::SetPolygonMode(int mode)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetPolygonMode\n"); std::abort();
}
int CryActionRegressionRenderer::GetWidth()
{
    return 640;
}
int CryActionRegressionRenderer::GetHeight()
{
    return 480;
}
void CryActionRegressionRenderer::GetMemoryUsage(ICrySizer* Sizer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetMemoryUsage\n"); std::abort();
}
bool CryActionRegressionRenderer::ScreenShot(const char *filename, int width)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ScreenShot\n"); std::abort();
}
int CryActionRegressionRenderer::GetColorBpp()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetColorBpp\n"); std::abort();
}
int CryActionRegressionRenderer::GetDepthBpp()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetDepthBpp\n"); std::abort();
}
int CryActionRegressionRenderer::GetStencilBpp()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetStencilBpp\n"); std::abort();
}
void CryActionRegressionRenderer::ProjectToScreen( float ptx, float pty, float ptz, float *sx, float *sy, float *sz )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ProjectToScreen\n"); std::abort();
}
int CryActionRegressionRenderer::UnProject(float sx, float sy, float sz, float *px, float *py, float *pz, const float modelMatrix[16], const float projMatrix[16], const int viewport[4])
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::UnProject\n"); std::abort();
}
int CryActionRegressionRenderer::UnProjectFromScreen( float sx, float sy, float sz, float *px, float *py, float *pz)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::UnProjectFromScreen\n"); std::abort();
}
void CryActionRegressionRenderer::GetModelViewMatrix(float *mat)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetModelViewMatrix\n"); std::abort();
}
void CryActionRegressionRenderer::GetProjectionMatrix(float *mat)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetProjectionMatrix\n"); std::abort();
}
Vec3 CryActionRegressionRenderer::GetUnProject(const Vec3 &WindowCoords,const CCamera &cam)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetUnProject\n"); std::abort();
}
bool CryActionRegressionRenderer::WriteDDS(byte *dat, int wdt, int hgt, int Size, const char *name, ETEX_Format eF, int NumMips)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::WriteDDS\n"); std::abort();
}
bool CryActionRegressionRenderer::WriteTGA(byte *dat, int wdt, int hgt, const char *name, int src_bits_per_pixel, int dest_bits_per_pixel )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::WriteTGA\n"); std::abort();
}
bool CryActionRegressionRenderer::WriteJPG(byte *dat, int wdt, int hgt, char *name, int src_bits_per_pixel )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::WriteJPG\n"); std::abort();
}
bool CryActionRegressionRenderer::FontUploadTexture(class CFBitmap*, ETEX_Format eTF)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FontUploadTexture\n"); std::abort();
}
int CryActionRegressionRenderer::FontCreateTexture(int Width, int Height, byte *pData, ETEX_Format eTF, bool genMips)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FontCreateTexture\n"); std::abort();
}
bool CryActionRegressionRenderer::FontUpdateTexture(int nTexId, int X, int Y, int USize, int VSize, byte *pData)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FontUpdateTexture\n"); std::abort();
}
void CryActionRegressionRenderer::FontReleaseTexture(class CFBitmap *pBmp)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FontReleaseTexture\n"); std::abort();
}
void CryActionRegressionRenderer::FontSetTexture(class CFBitmap*, int nFilterMode)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FontSetTexture\n"); std::abort();
}
void CryActionRegressionRenderer::FontSetTexture(int nTexId, int nFilterMode)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FontSetTexture\n"); std::abort();
}
void CryActionRegressionRenderer::FontSetRenderingState(unsigned int nVirtualScreenWidth, unsigned int nVirtualScreenHeight)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FontSetRenderingState\n"); std::abort();
}
void CryActionRegressionRenderer::FontSetBlending(int src, int dst)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FontSetBlending\n"); std::abort();
}
void CryActionRegressionRenderer::FontRestoreRenderingState()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FontRestoreRenderingState\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_PrecacheResource(IShader *pSH, float fDist, float fTimeToReady, int Flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_PrecacheResource\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_PrecacheResource(ITexture *pTP, float fDist, float fTimeToReady, int Flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_PrecacheResource\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_PrecacheResource(IRenderMesh *pPB, float fDist, float fTimeToReady, int Flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_PrecacheResource\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_PrecacheResource(CDLight *pLS, float fDist, float fTimeToReady, int Flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_PrecacheResource\n"); std::abort();
}
CRenderObject* CryActionRegressionRenderer::EF_AddParticlesToScene(const SShaderItem& si, CRenderObject* pRO, IParticleVertexCreator* pPVC, SParticleRenderInfo const& RenInfo, int nAW, bool& canUseGS)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_AddParticlesToScene\n"); std::abort();
}
void CryActionRegressionRenderer::EF_AddGPUParticlesToScene( int32 nGPUParticleIdx, AABB const& bb, const SShaderItem& shaderItem, CRenderObject* pRO, bool nAW, bool canUseGS )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_AddGPUParticlesToScene\n"); std::abort();
}
void CryActionRegressionRenderer::EF_AddPolyToScene2D(int Ef, int numPts, const struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *verts)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_AddPolyToScene2D\n"); std::abort();
}
void CryActionRegressionRenderer::EF_AddPolyToScene2D(const SShaderItem& si, int nTempl, int numPts, const struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *verts)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_AddPolyToScene2D\n"); std::abort();
}
CRenderObject* CryActionRegressionRenderer::EF_AddPolygonToScene(const SShaderItem& si, int numPts, const struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *verts, const SPipTangents *tangs, CRenderObject *obj, uint16 *inds, int ninds, int nAW, bool bMerge)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_AddPolygonToScene\n"); std::abort();
}
CRenderObject* CryActionRegressionRenderer::EF_AddPolygonToScene(const SShaderItem& si, CRenderObject* obj, int numPts, int ninds, struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F*& verts, SPipTangents*& tangs, uint16*& inds, int nAW, bool bMerge)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_AddPolygonToScene\n"); std::abort();
}
string *CryActionRegressionRenderer::EF_GetShaderNames(int& nNumShaders)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_GetShaderNames\n"); std::abort();
}
IShader *CryActionRegressionRenderer::EF_LoadShader(const char *name, int flags, uint nMaskGen)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_LoadShader\n"); std::abort();
}
const SShaderProfile &CryActionRegressionRenderer::GetShaderProfile(EShaderType eST) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetShaderProfile\n"); std::abort();
}
void CryActionRegressionRenderer::SetShaderQuality(EShaderType eST, EShaderQuality eSQ)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetShaderQuality\n"); std::abort();
}
ERenderQuality CryActionRegressionRenderer::EF_GetRenderQuality() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_GetRenderQuality\n"); std::abort();
}
EShaderQuality CryActionRegressionRenderer::EF_GetShaderQuality(EShaderType eST)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_GetShaderQuality\n"); std::abort();
}
SShaderItem CryActionRegressionRenderer::EF_LoadShaderItem(const char *szName, bool bShare, int flags, SInputShaderResources *Res, uint nMaskGen)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_LoadShaderItem\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_ReloadFile(const char *szFileName)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_ReloadFile\n"); std::abort();
}
void CryActionRegressionRenderer::EF_ReloadShaderFiles(int nCategory)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_ReloadShaderFiles\n"); std::abort();
}
void CryActionRegressionRenderer::EF_ReloadTextures()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_ReloadTextures\n"); std::abort();
}
ITexture* CryActionRegressionRenderer::EF_GetTextureByID(int id)
{
    auto it = textures.find(id); return it == textures.end() ? nullptr : it->second;
}
ITexture *CryActionRegressionRenderer::EF_LoadTexture(const char* nameTex, uint flags, byte eTT, float fAmount1, float fAmount2)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_LoadTexture\n"); std::abort();
}
int CryActionRegressionRenderer::EF_LoadLightmap(const char *name)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_LoadLightmap\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_ScanEnvironmentCM(const char *name, int size, Vec3& Pos)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_ScanEnvironmentCM\n"); std::abort();
}
void CryActionRegressionRenderer::EF_AddSplash(Vec3 Pos, eSplashType eST, float fForce, int Id)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_AddSplash\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_HideTemplate(const char *name)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_HideTemplate\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_UnhideTemplate(const char *name)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_UnhideTemplate\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_UnhideAllTemplates()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_UnhideAllTemplates\n"); std::abort();
}
CRendElement *CryActionRegressionRenderer::EF_CreateRE(EDataType edt)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_CreateRE\n"); std::abort();
}
void CryActionRegressionRenderer::EF_StartEf()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_StartEf\n"); std::abort();
}
CRenderObject *CryActionRegressionRenderer::EF_GetObject(bool bTemp, int num)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_GetObject\n"); std::abort();
}
void CryActionRegressionRenderer::EF_AddEf(CRendElement *pRE, SShaderItem& pSH, CRenderObject *pObj, int nList, int nAW)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_AddEf\n"); std::abort();
}
void CryActionRegressionRenderer::EF_EndEf3D(int nFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_EndEf3D\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_IsFakeDLight(CDLight *Source)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_IsFakeDLight\n"); std::abort();
}
void CryActionRegressionRenderer::EF_ADDDlight(CDLight *Source)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_ADDDlight\n"); std::abort();
}
bool CryActionRegressionRenderer::EF_UpdateDLight(CDLight *pDL)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_UpdateDLight\n"); std::abort();
}
void CryActionRegressionRenderer::EF_EndEf2D(bool bSort)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_EndEf2D\n"); std::abort();
}
void *CryActionRegressionRenderer::EF_Query(int Query, INT_PTR Param)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_Query\n"); std::abort();
}
int CryActionRegressionRenderer::GetPolyCount()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetPolyCount\n"); std::abort();
}
void CryActionRegressionRenderer::GetPolyCount(int &nPolygons,int &nShadowVolPolys)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetPolyCount\n"); std::abort();
}
void CryActionRegressionRenderer::SetClearColor(const Vec3 & vColor)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetClearColor\n"); std::abort();
}
IRenderMesh * CryActionRegressionRenderer::CreateRenderMesh(bool bDynamic, const char *szType,const char *szSourceName)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::CreateRenderMesh\n"); std::abort();
}
struct IRenderMesh * CryActionRegressionRenderer::CreateRenderMeshInitialized( void * pVertBuffer, int nVertCount, int nVertFormat, ushort* pIndices, int nIndices, int nPrimetiveType, const char *szType,const char *szSourceName, EBufferType eBufType, int nMatInfoCount, int nClientTextureBindID, bool (*PrepareBufferCallback)(IRenderMesh *, bool), void *CustomData, bool bOnlyVideoBuffer, bool bPrecache, SPipTangents * pTangents)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::CreateRenderMeshInitialized\n"); std::abort();
}
void CryActionRegressionRenderer::DeleteRenderMesh(struct IRenderMesh * pLBuffer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DeleteRenderMesh\n"); std::abort();
}
int CryActionRegressionRenderer::GetFrameID(bool bIncludeRecursiveCalls)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetFrameID\n"); std::abort();
}
void CryActionRegressionRenderer::MakeMatrix(const Vec3 & pos, const Vec3 & angles,const Vec3 & scale, Matrix34* mat)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::MakeMatrix\n"); std::abort();
}
void CryActionRegressionRenderer::DrawLabelImage(const Vec3 &vPos,float fSize,int nTextureId)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawLabelImage\n"); std::abort();
}
void CryActionRegressionRenderer::DrawLabel(Vec3 pos, float font_size, const char * label_text, ...) PRINTF_PARAMS(4, 5)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawLabel\n"); std::abort();
}
void CryActionRegressionRenderer::DrawLabelEx(Vec3 pos, float font_size, float * pfColor, bool bFixedSize, bool bCenter, const char * label_text, ...) PRINTF_PARAMS(7, 8)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawLabelEx\n"); std::abort();
}
void CryActionRegressionRenderer::Draw2dLabel(float x, float y, float scale, float* color, bool center, const char* format, ...)
{
    char text[1024]{}; va_list args; va_start(args, format);
    vsnprintf(text, sizeof(text), format, args); va_end(args);
    labels.push_back({x, y, scale, ColorF(color[0], color[1], color[2], color[3]), text});
}
float CryActionRegressionRenderer::ScaleCoordX(float value)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ScaleCoordX\n"); std::abort();
}
float CryActionRegressionRenderer::ScaleCoordY(float value)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ScaleCoordY\n"); std::abort();
}
void CryActionRegressionRenderer::SetState(int value, int alpha)
{
    state = value; states.push_back(value);
}
void CryActionRegressionRenderer::SetCullMode(int)
{
    
}
bool CryActionRegressionRenderer::EnableFog(bool enable)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EnableFog\n"); std::abort();
}
void CryActionRegressionRenderer::SetFog(float density,float fogstart,float fogend,const float *color,int fogmode)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetFog\n"); std::abort();
}
void CryActionRegressionRenderer::SetFogColor(float * color)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetFogColor\n"); std::abort();
}
void CryActionRegressionRenderer::SetColorOp(byte, byte, byte, byte)
{
    
}
void CryActionRegressionRenderer::SetTerrainAONodes(PodArray<SSectorTextureSet> * terrainAONodes)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetTerrainAONodes\n"); std::abort();
}
IRenderAuxGeom* CryActionRegressionRenderer::GetIRenderAuxGeom()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetIRenderAuxGeom\n"); std::abort();
}
void CryActionRegressionRenderer::EnableVSync(bool enable)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EnableVSync\n"); std::abort();
}
void CryActionRegressionRenderer::PushMatrix()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::PushMatrix\n"); std::abort();
}
void CryActionRegressionRenderer::PopMatrix()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::PopMatrix\n"); std::abort();
}
void CryActionRegressionRenderer::EnableTMU(bool)
{
    
}
void CryActionRegressionRenderer::SelectTMU(int)
{
    
}
unsigned int CryActionRegressionRenderer::DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat, int filter, int Id, char *szCacheName, int flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DownLoadToVideoMemory\n"); std::abort();
}
void CryActionRegressionRenderer::UpdateTextureInVideoMemory(uint tnum, unsigned char *newdata,int posx,int posy,int w,int h,ETEX_Format eTFSrc)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::UpdateTextureInVideoMemory\n"); std::abort();
}
bool CryActionRegressionRenderer::DXTCompress( byte *raw_data,int nWidth,int nHeight,ETEX_Format eTF, bool bUseHW, bool bGenMips, int nSrcBytesPerPix, const Vec3 vLumWeight, MIPDXTcallback callback )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DXTCompress\n"); std::abort();
}
bool CryActionRegressionRenderer::DXTDecompress(byte *srcData, const size_t srcFileSize, byte *dstData, int nWidth,int nHeight,int nMips,ETEX_Format eSrcTF, bool bUseHW, int nDstBytesPerPix)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DXTDecompress\n"); std::abort();
}
void CryActionRegressionRenderer::RemoveTexture(unsigned int TextureId)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::RemoveTexture\n"); std::abort();
}
void CryActionRegressionRenderer::TextToScreen(float x, float y, const char * format, ...) PRINTF_PARAMS(4, 5)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::TextToScreen\n"); std::abort();
}
void CryActionRegressionRenderer::TextToScreenColor(int x, int y, float r, float g, float b, float a, const char * format, ...) PRINTF_PARAMS(8, 9)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::TextToScreenColor\n"); std::abort();
}
void CryActionRegressionRenderer::ResetToDefault()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ResetToDefault\n"); std::abort();
}
void CryActionRegressionRenderer::SetMaterialColor(float r, float g, float b, float a)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetMaterialColor\n"); std::abort();
}
void CryActionRegressionRenderer::DrawLine(const Vec3 & vPos1, const Vec3 & vPos2)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawLine\n"); std::abort();
}
void CryActionRegressionRenderer::Graph(byte *g, int x, int y, int wdt, int hgt, int nC, int type, const char *text, ColorF& color, float fScale)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::Graph\n"); std::abort();
}
void CryActionRegressionRenderer::FlushTextMessages()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::FlushTextMessages\n"); std::abort();
}
void CryActionRegressionRenderer::DrawObjSprites(PodArray<struct SVegetationSpriteInfo> *pList, float fMaxViewDist, CObjManager *pObjMan, float fZoomFactor)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawObjSprites\n"); std::abort();
}
void CryActionRegressionRenderer::GenerateObjSprites(PodArray<struct SVegetationSpriteInfo> *pList, float fMaxViewDist, CObjManager *pObjMan, float fZoomFactor)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GenerateObjSprites\n"); std::abort();
}
void CryActionRegressionRenderer::DrawQuad(const Vec3 &right, const Vec3 &up, const Vec3 &origin,int nFlipMode)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawQuad\n"); std::abort();
}
void CryActionRegressionRenderer::DrawQuad(float dy,float dx, float dz, float x, float y, float z)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DrawQuad\n"); std::abort();
}
void CryActionRegressionRenderer::ClearBuffer(uint nFlags, ColorF *vColor, float depth)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ClearBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::ReadFrameBuffer(unsigned char * pRGB, int nImageX, int nSizeX, int nSizeY, ERB_Type eRBType, bool bRGBA, int nScaledX, int nScaledY)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ReadFrameBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::ReadFrameBufferFast(unsigned int* pDstARGBA8, int dstWidth, int dstHeight)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ReadFrameBufferFast\n"); std::abort();
}
const char* CryActionRegressionRenderer::GetVertexProfile(bool bSupportedProfile)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetVertexProfile\n"); std::abort();
}
const char* CryActionRegressionRenderer::GetPixelProfile(bool bSupportedProfile)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetPixelProfile\n"); std::abort();
}
uint CryActionRegressionRenderer::RenderOccludersIntoBuffer(const CCamera & viewCam, int nTexSize, PodArray<struct IRenderNode*> & lstOccluders, float * pBuffer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::RenderOccludersIntoBuffer\n"); std::abort();
}
void CryActionRegressionRenderer::Set2DMode(bool, int, int, float, float)
{
    
}
int CryActionRegressionRenderer::ScreenToTexture()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::ScreenToTexture\n"); std::abort();
}
void CryActionRegressionRenderer::EnableSwapBuffers(bool bEnable)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EnableSwapBuffers\n"); std::abort();
}
WIN_HWND CryActionRegressionRenderer::GetHWND()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetHWND\n"); std::abort();
}
WIN_HWND CryActionRegressionRenderer::GetCurrentContextHWND()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetCurrentContextHWND\n"); std::abort();
}
void CryActionRegressionRenderer::OnEntityDeleted(struct IRenderNode * pRenderNode)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::OnEntityDeleted\n"); std::abort();
}
int CryActionRegressionRenderer::CreateRenderTarget(int nWidth, int nHeight, ETEX_Format eTF)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::CreateRenderTarget\n"); std::abort();
}
bool CryActionRegressionRenderer::DestroyRenderTarget(int nHandle)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::DestroyRenderTarget\n"); std::abort();
}
bool CryActionRegressionRenderer::SetRenderTarget(int nHandle)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetRenderTarget\n"); std::abort();
}
float CryActionRegressionRenderer::EF_GetWaterZElevation(float fX, float fY)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::EF_GetWaterZElevation\n"); std::abort();
}
void CryActionRegressionRenderer::PauseTimer(bool bPause)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::PauseTimer\n"); std::abort();
}
IShaderPublicParams* CryActionRegressionRenderer::CreateShaderPublicParams()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::CreateShaderPublicParams\n"); std::abort();
}
void CryActionRegressionRenderer::SF_ConfigMask(ESFMaskOp maskOp, unsigned int stencilRef)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SF_ConfigMask\n"); std::abort();
}
void CryActionRegressionRenderer::SF_DrawIndexedTriList(int baseVertexIndex, int minVertexIndex, int numVertices, int startIndex, int triangleCount, const SSF_GlobalDrawParams& params)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SF_DrawIndexedTriList\n"); std::abort();
}
void CryActionRegressionRenderer::SF_DrawLineStrip(int baseVertexIndex, int lineCount, const SSF_GlobalDrawParams& params)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SF_DrawLineStrip\n"); std::abort();
}
void CryActionRegressionRenderer::SF_DrawGlyphClear(const SSF_GlobalDrawParams& params)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SF_DrawGlyphClear\n"); std::abort();
}
void CryActionRegressionRenderer::SF_Flush()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SF_Flush\n"); std::abort();
}
int CryActionRegressionRenderer::SF_CreateTexture(int width, int height, int numMips, unsigned char* pData, ETEX_Format eTF)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SF_CreateTexture\n"); std::abort();
}
bool CryActionRegressionRenderer::SF_UpdateTexture(int texId, int mipLevel, int numRects, const SUpdateRect* pRects, unsigned char* pData, size_t pitch, ETEX_Format eTF)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SF_UpdateTexture\n"); std::abort();
}
void CryActionRegressionRenderer::SF_GetMeshMaxSize(int& numVertices, int& numIndices) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SF_GetMeshMaxSize\n"); std::abort();
}
IVideoPlayer* CryActionRegressionRenderer::CreateVideoPlayerInstance() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::CreateVideoPlayerInstance\n"); std::abort();
}
void CryActionRegressionRenderer::SetCloudShadowTextureId( int id, const Vec3 & vSpeed )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetCloudShadowTextureId\n"); std::abort();
}
void CryActionRegressionRenderer::SetSkyLightRenderParams( const SSkyLightRenderParams* pSkyLightRenderParams )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetSkyLightRenderParams\n"); std::abort();
}
uint16 CryActionRegressionRenderer::PushFogVolumeContribution( const ColorF& fogVolumeContrib )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::PushFogVolumeContribution\n"); std::abort();
}
int CryActionRegressionRenderer::GetMaxTextureSize()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetMaxTextureSize\n"); std::abort();
}
const char * CryActionRegressionRenderer::GetTextureFormatName(ETEX_Format eTF)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetTextureFormatName\n"); std::abort();
}
int CryActionRegressionRenderer::GetTextureFormatDataSize(int nWidth, int nHeight, int nDepth, int nMips, ETEX_Format eTF)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::GetTextureFormatDataSize\n"); std::abort();
}
void CryActionRegressionRenderer::SetDefaultMaterials(IMaterial * pDefMat, IMaterial * pTerrainDefMat)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::SetDefaultMaterials\n"); std::abort();
}
bool CryActionRegressionRenderer::IsMultiGPUModeActive() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionRenderer::IsMultiGPUModeActive\n"); std::abort();
}
int CryActionRegressionTexture::AddRef()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::AddRef\n"); std::abort();
}
int CryActionRegressionTexture::Release()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::Release\n"); std::abort();
}
int CryActionRegressionTexture::ReleaseForce()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::ReleaseForce\n"); std::abort();
}
const char *CryActionRegressionTexture::GetName()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetName\n"); std::abort();
}
int CryActionRegressionTexture::GetWidth()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetWidth\n"); std::abort();
}
int CryActionRegressionTexture::GetHeight()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetHeight\n"); std::abort();
}
int CryActionRegressionTexture::GetSourceWidth()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetSourceWidth\n"); std::abort();
}
int CryActionRegressionTexture::GetSourceHeight()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetSourceHeight\n"); std::abort();
}
int CryActionRegressionTexture::GetTextureID()
{
    return id;
}
uint CryActionRegressionTexture::GetFlags()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetFlags\n"); std::abort();
}
int CryActionRegressionTexture::GetNumMips()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetNumMips\n"); std::abort();
}
int CryActionRegressionTexture::GetDeviceDataSize()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetDeviceDataSize\n"); std::abort();
}
ETEX_Type CryActionRegressionTexture::GetTextureType()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetTextureType\n"); std::abort();
}
bool CryActionRegressionTexture::IsTextureLoaded()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::IsTextureLoaded\n"); std::abort();
}
void CryActionRegressionTexture::PrecacheAsynchronously(float fDist, int nFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::PrecacheAsynchronously\n"); std::abort();
}
void CryActionRegressionTexture::Preload(int nFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::Preload\n"); std::abort();
}
byte *CryActionRegressionTexture::GetData32(int nSide, int nLevel)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetData32\n"); std::abort();
}
byte *CryActionRegressionTexture::LockData(int& nPitch, int nSide, int nLevel)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::LockData\n"); std::abort();
}
void CryActionRegressionTexture::UnlockData(int nSide, int nLevel)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::UnlockData\n"); std::abort();
}
bool CryActionRegressionTexture::SaveTGA(const char *szName, bool bMips)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::SaveTGA\n"); std::abort();
}
bool CryActionRegressionTexture::SaveJPG(const char *szName, bool bMips)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::SaveJPG\n"); std::abort();
}
bool CryActionRegressionTexture::SetFilter(int nFilter)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::SetFilter\n"); std::abort();
}
void CryActionRegressionTexture::SetClamp(bool bEnable)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::SetClamp\n"); std::abort();
}
ColorF CryActionRegressionTexture::GetAvgColor()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetAvgColor\n"); std::abort();
}
const char* CryActionRegressionTexture::GetFormatName()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetFormatName\n"); std::abort();
}
const char* CryActionRegressionTexture::GetTypeName()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::GetTypeName\n"); std::abort();
}
bool CryActionRegressionTexture::IsShared() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionTexture::IsShared\n"); std::abort();
}
void CryActionRegressionInput::AddEventListener(IInputEventListener* listener)
{
    listeners.push_back(listener); ++adds;
}
void CryActionRegressionInput::RemoveEventListener(IInputEventListener* listener)
{
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end()); ++removes;
}
void CryActionRegressionInput::AddConsoleEventListener( IInputEventListener *pListener )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::AddConsoleEventListener\n"); std::abort();
}
void CryActionRegressionInput::RemoveConsoleEventListener( IInputEventListener *pListener )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::RemoveConsoleEventListener\n"); std::abort();
}
void CryActionRegressionInput::SetExclusiveListener( IInputEventListener *pListener )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::SetExclusiveListener\n"); std::abort();
}
IInputEventListener *CryActionRegressionInput::GetExclusiveListener()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::GetExclusiveListener\n"); std::abort();
}
void CryActionRegressionInput::EnableEventPosting( bool bEnable )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::EnableEventPosting\n"); std::abort();
}
void CryActionRegressionInput::PostInputEvent( const SInputEvent &event )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::PostInputEvent\n"); std::abort();
}
void CryActionRegressionInput::ForceFeedbackEvent( const SFFOutputEvent &event )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::ForceFeedbackEvent\n"); std::abort();
}
bool CryActionRegressionInput::Init()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::Init\n"); std::abort();
}
void CryActionRegressionInput::Update(bool bFocus)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::Update\n"); std::abort();
}
void CryActionRegressionInput::ShutDown()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::ShutDown\n"); std::abort();
}
void CryActionRegressionInput::SetExclusiveMode(EDeviceId deviceId, bool exclusive,void *hwnd)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::SetExclusiveMode\n"); std::abort();
}
bool CryActionRegressionInput::InputState(const TKeyName& key, EInputState state)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::InputState\n"); std::abort();
}
const char *CryActionRegressionInput::GetKeyName(const SInputEvent& event, bool bGUI)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::GetKeyName\n"); std::abort();
}
SInputSymbol* CryActionRegressionInput::LookupSymbol( EDeviceId deviceId, EKeyId keyId )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::LookupSymbol\n"); std::abort();
}
const wchar_t* CryActionRegressionInput::GetOSKeyName(const SInputEvent& event)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::GetOSKeyName\n"); std::abort();
}
void CryActionRegressionInput::ClearKeyState()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::ClearKeyState\n"); std::abort();
}
void CryActionRegressionInput::RetriggerKeyState()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::RetriggerKeyState\n"); std::abort();
}
bool CryActionRegressionInput::Retriggering()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::Retriggering\n"); std::abort();
}
bool CryActionRegressionInput::HasInputDeviceOfType( EInputDeviceType type )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::HasInputDeviceOfType\n"); std::abort();
}
int CryActionRegressionInput::GetModifiers() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::GetModifiers\n"); std::abort();
}
void CryActionRegressionInput::EnableDevice( EDeviceId deviceId, bool enable)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionInput::EnableDevice\n"); std::abort();
}
void CryActionRegressionFont::Reset()
{
    ++resets;
}
void CryActionRegressionFont::Release()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::Release\n"); std::abort();
}
bool CryActionRegressionFont::Load(const char *szFile, unsigned long nWidth, unsigned long nHeight, unsigned long nTTFFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::Load\n"); std::abort();
}
bool CryActionRegressionFont::Load(const char* file)
{
    return true;
}
void CryActionRegressionFont::Free()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::Free\n"); std::abort();
}
void CryActionRegressionFont::SetEffect(const char* value)
{
    effect = value;
}
void CryActionRegressionFont::SetClippingRect(float x, float y, float w, float h)
{
    clip = {x,y,w,h};
}
void CryActionRegressionFont::EnableClipping(bool value)
{
    clipping = value;
}
void CryActionRegressionFont::SetColor(const ColorF& value, int pass)
{
    color = value;
}
void CryActionRegressionFont::UseRealPixels(bool value)
{
    pixels = value;
}
bool CryActionRegressionFont::UsingRealPixels()
{
    return pixels;
}
void CryActionRegressionFont::SetSize(const vector2f& value)
{
    size = value;
}
vector2f &CryActionRegressionFont::GetSize()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetSize\n"); std::abort();
}
float CryActionRegressionFont::GetCharWidth()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetCharWidth\n"); std::abort();
}
float CryActionRegressionFont::GetCharHeight()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetCharHeight\n"); std::abort();
}
void CryActionRegressionFont::SetSameSize(bool bSameSize)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::SetSameSize\n"); std::abort();
}
bool CryActionRegressionFont::GetSameSize()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetSameSize\n"); std::abort();
}
void CryActionRegressionFont::SetCharWidthScale(float fScale)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::SetCharWidthScale\n"); std::abort();
}
float CryActionRegressionFont::GetCharWidthScale()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetCharWidthScale\n"); std::abort();
}
void CryActionRegressionFont::DrawString( float x, float y, const char *szMsg, const bool bASCIIMultiLine)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::DrawString\n"); std::abort();
}
void CryActionRegressionFont::DrawString( float x, float y, float z, const char *szMsg, const bool bASCIIMultiLine)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::DrawString\n"); std::abort();
}
vector2f CryActionRegressionFont::GetTextSize(const char *szMsg, const bool bASCIIMultiLine)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetTextSize\n"); std::abort();
}
void CryActionRegressionFont::DrawStringW(float x, float y, const wchar_t* value, const bool multiline)
{
    borrowed = value; text = value; ++draws;
}
void CryActionRegressionFont::DrawStringW( float x, float y, float z, const wchar_t *swStr, const bool bASCIIMultiLine)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::DrawStringW\n"); std::abort();
}
void CryActionRegressionFont::DrawWrappedStringW( float x, float y, float w, const wchar_t *swStr, const bool bASCIIMultiLine)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::DrawWrappedStringW\n"); std::abort();
}
vector2f CryActionRegressionFont::GetTextSizeW(const wchar_t *swStr, const bool bASCIIMultiLine)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetTextSizeW\n"); std::abort();
}
vector2f CryActionRegressionFont::GetWrappedTextSizeW(const wchar_t *swStr, float w, const bool bASCIIMultiLine)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetWrappedTextSizeW\n"); std::abort();
}
int CryActionRegressionFont::GetTextLength(const char *szMsg, const bool bASCIIMultiLine)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetTextLength\n"); std::abort();
}
int CryActionRegressionFont::GetTextLengthW(const wchar_t *szwMsg, const bool bASCIIMultiLine)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetTextLengthW\n"); std::abort();
}
void CryActionRegressionFont::GetMemoryUsage(class ICrySizer* pSizer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFont::GetMemoryUsage\n"); std::abort();
}
void CryActionRegressionFonts::Release()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFonts::Release\n"); std::abort();
}
IFFont* CryActionRegressionFonts::NewFont(const char* name)
{
    return font;
}
IFFont* CryActionRegressionFonts::GetFont(const char* name)
{
    return font;
}
void CryActionRegressionFonts::GetMemoryUsage(class ICrySizer* pSizer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionFonts::GetMemoryUsage\n"); std::abort();
}
EntityId CryActionRegressionEntity::GetId() const
{
    return id;
}
EntityGUID CryActionRegressionEntity::GetGuid() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetGuid\n"); std::abort();
}
IEntityClass* CryActionRegressionEntity::GetClass() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetClass\n"); std::abort();
}
IEntityArchetype* CryActionRegressionEntity::GetArchetype()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetArchetype\n"); std::abort();
}
void CryActionRegressionEntity::SetFlags( uint32 flags )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetFlags\n"); std::abort();
}
uint32 CryActionRegressionEntity::GetFlags() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetFlags\n"); std::abort();
}
void CryActionRegressionEntity::AddFlags( uint32 flagsToAdd )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::AddFlags\n"); std::abort();
}
void CryActionRegressionEntity::ClearFlags( uint32 flagsToClear )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::ClearFlags\n"); std::abort();
}
bool CryActionRegressionEntity::CheckFlags( uint32 flagsToCheck ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::CheckFlags\n"); std::abort();
}
bool CryActionRegressionEntity::IsGarbage() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::IsGarbage\n"); std::abort();
}
void CryActionRegressionEntity::SetName( const char *sName )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetName\n"); std::abort();
}
const char* CryActionRegressionEntity::GetName() const
{
    return name.c_str();
}
const char* CryActionRegressionEntity::GetEntityTextDescription() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetEntityTextDescription\n"); std::abort();
}
void CryActionRegressionEntity::SerializeXML( XmlNodeRef &entityNode,bool bLoading )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SerializeXML\n"); std::abort();
}
void CryActionRegressionEntity::AttachChild( IEntity *pChildEntity,int nAttachFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::AttachChild\n"); std::abort();
}
void CryActionRegressionEntity::DetachAll( int nDetachFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::DetachAll\n"); std::abort();
}
void CryActionRegressionEntity::DetachThis( int nDetachFlags,int nWhyFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::DetachThis\n"); std::abort();
}
int CryActionRegressionEntity::GetChildCount() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetChildCount\n"); std::abort();
}
IEntity* CryActionRegressionEntity::GetChild( int nIndex ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetChild\n"); std::abort();
}
IEntity* CryActionRegressionEntity::GetParent() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetParent\n"); std::abort();
}
void CryActionRegressionEntity::SetWorldTM( const Matrix34 &tm,int nWhyFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetWorldTM\n"); std::abort();
}
void CryActionRegressionEntity::SetLocalTM( const Matrix34 &tm,int nWhyFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetLocalTM\n"); std::abort();
}
const Matrix34& CryActionRegressionEntity::GetWorldTM() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetWorldTM\n"); std::abort();
}
Matrix34 CryActionRegressionEntity::GetLocalTM() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetLocalTM\n"); std::abort();
}
void CryActionRegressionEntity::GetWorldBounds( AABB &bbox )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetWorldBounds\n"); std::abort();
}
void CryActionRegressionEntity::GetLocalBounds( AABB &bbox )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetLocalBounds\n"); std::abort();
}
void CryActionRegressionEntity::SetPos( const Vec3 &vPos,int nWhyFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetPos\n"); std::abort();
}
const Vec3& CryActionRegressionEntity::GetPos() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetPos\n"); std::abort();
}
void CryActionRegressionEntity::SetRotation( const Quat &qRotation,int nWhyFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetRotation\n"); std::abort();
}
const Quat& CryActionRegressionEntity::GetRotation() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetRotation\n"); std::abort();
}
void CryActionRegressionEntity::SetScale( const Vec3 &vScale,int nWhyFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetScale\n"); std::abort();
}
const Vec3& CryActionRegressionEntity::GetScale() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetScale\n"); std::abort();
}
void CryActionRegressionEntity::SetPosRotScale( const Vec3 &vPos,const Quat &qRotation,const Vec3 &vScale,int nWhyFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetPosRotScale\n"); std::abort();
}
Vec3 CryActionRegressionEntity::GetWorldPos() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetWorldPos\n"); std::abort();
}
Ang3 CryActionRegressionEntity::GetWorldAngles() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetWorldAngles\n"); std::abort();
}
Quat CryActionRegressionEntity::GetWorldRotation() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetWorldRotation\n"); std::abort();
}
const Vec3& CryActionRegressionEntity::GetForwardDir() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetForwardDir\n"); std::abort();
}
void CryActionRegressionEntity::Activate( bool bActive )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::Activate\n"); std::abort();
}
bool CryActionRegressionEntity::IsActive() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::IsActive\n"); std::abort();
}
void CryActionRegressionEntity::PrePhysicsActivate( bool bActive )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::PrePhysicsActivate\n"); std::abort();
}
bool CryActionRegressionEntity::IsPrePhysicsActive()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::IsPrePhysicsActive\n"); std::abort();
}
void CryActionRegressionEntity::Serialize( TSerialize serializer, int nFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::Serialize\n"); std::abort();
}
bool CryActionRegressionEntity::SendEvent( SEntityEvent &event )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SendEvent\n"); std::abort();
}
void CryActionRegressionEntity::SetTimer( int nTimerId,int nMilliSeconds )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetTimer\n"); std::abort();
}
void CryActionRegressionEntity::KillTimer( int nTimerId )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::KillTimer\n"); std::abort();
}
void CryActionRegressionEntity::Hide( bool bHide )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::Hide\n"); std::abort();
}
bool CryActionRegressionEntity::IsHidden() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::IsHidden\n"); std::abort();
}
void CryActionRegressionEntity::Invisible( bool bInvisible )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::Invisible\n"); std::abort();
}
bool CryActionRegressionEntity::IsInvisible() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::IsInvisible\n"); std::abort();
}
CSmartObject* CryActionRegressionEntity::GetSmartObject() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetSmartObject\n"); std::abort();
}
void CryActionRegressionEntity::SetSmartObject( CSmartObject* pSmartObject )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetSmartObject\n"); std::abort();
}
IAIObject* CryActionRegressionEntity::GetAI()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetAI\n"); std::abort();
}
bool CryActionRegressionEntity::RegisterInAISystem(unsigned short type, const AIObjectParameters &params)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::RegisterInAISystem\n"); std::abort();
}
void CryActionRegressionEntity::SetUpdatePolicy( EEntityUpdatePolicy eUpdatePolicy )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetUpdatePolicy\n"); std::abort();
}
EEntityUpdatePolicy CryActionRegressionEntity::GetUpdatePolicy() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetUpdatePolicy\n"); std::abort();
}
IEntityProxy* CryActionRegressionEntity::GetProxy( EEntityProxy proxy ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetProxy\n"); std::abort();
}
void CryActionRegressionEntity::SetProxy(EEntityProxy proxy, IEntityProxy *pProxy)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetProxy\n"); std::abort();
}
IEntityProxy* CryActionRegressionEntity::CreateProxy( EEntityProxy proxy )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::CreateProxy\n"); std::abort();
}
void CryActionRegressionEntity::Physicalize( SEntityPhysicalizeParams &params )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::Physicalize\n"); std::abort();
}
IPhysicalEntity* CryActionRegressionEntity::GetPhysics() const
{
    return physics;
}
int CryActionRegressionEntity::PhysicalizeSlot(int slot, SEntityPhysicalizeParams &params)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::PhysicalizeSlot\n"); std::abort();
}
void CryActionRegressionEntity::UnphysicalizeSlot(int slot)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::UnphysicalizeSlot\n"); std::abort();
}
void CryActionRegressionEntity::UpdateSlotPhysics(int slot)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::UpdateSlotPhysics\n"); std::abort();
}
void CryActionRegressionEntity::SetPhysicsState( XmlNodeRef & physicsState )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetPhysicsState\n"); std::abort();
}
void CryActionRegressionEntity::SetMaterial( IMaterial *pMaterial )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetMaterial\n"); std::abort();
}
IMaterial* CryActionRegressionEntity::GetMaterial()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetMaterial\n"); std::abort();
}
bool CryActionRegressionEntity::IsSlotValid( int nIndex ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::IsSlotValid\n"); std::abort();
}
void CryActionRegressionEntity::FreeSlot( int nIndex )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::FreeSlot\n"); std::abort();
}
int CryActionRegressionEntity::GetSlotCount() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetSlotCount\n"); std::abort();
}
bool CryActionRegressionEntity::GetSlotInfo( int nIndex,SEntitySlotInfo &slotInfo ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetSlotInfo\n"); std::abort();
}
const Matrix34& CryActionRegressionEntity::GetSlotWorldTM( int nSlot ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetSlotWorldTM\n"); std::abort();
}
const Matrix34& CryActionRegressionEntity::GetSlotLocalTM( int nSlot, bool bRelativeToParent) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetSlotLocalTM\n"); std::abort();
}
void CryActionRegressionEntity::SetSlotLocalTM( int nSlot,const Matrix34 &localTM,int nWhyFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetSlotLocalTM\n"); std::abort();
}
bool CryActionRegressionEntity::SetParentSlot( int nParentIndex,int nChildIndex )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetParentSlot\n"); std::abort();
}
void CryActionRegressionEntity::SetSlotMaterial( int nSlot,IMaterial *pMaterial )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetSlotMaterial\n"); std::abort();
}
void CryActionRegressionEntity::SetSlotFlags( int nSlot,uint32 nFlags )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetSlotFlags\n"); std::abort();
}
uint32 CryActionRegressionEntity::GetSlotFlags( int nSlot ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetSlotFlags\n"); std::abort();
}
bool CryActionRegressionEntity::ShouldUpdateCharacter( int nSlot ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::ShouldUpdateCharacter\n"); std::abort();
}
ICharacterInstance* CryActionRegressionEntity::GetCharacter(int slot)
{
    ++characterReads; return slot == 0 ? character : nullptr;
}
int CryActionRegressionEntity::SetCharacter( ICharacterInstance *pCharacter, int nSlot )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetCharacter\n"); std::abort();
}
IStatObj* CryActionRegressionEntity::GetStatObj( int nSlot )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetStatObj\n"); std::abort();
}
IParticleEmitter* CryActionRegressionEntity::GetParticleEmitter( int nSlot )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetParticleEmitter\n"); std::abort();
}
int CryActionRegressionEntity::SetStatObj( IStatObj *pStatObj, int nSlot,bool bUpdatePhysics, float mass)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetStatObj\n"); std::abort();
}
int CryActionRegressionEntity::LoadGeometry( int nSlot,const char *sFilename,const char *sGeomName,int nLoadFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::LoadGeometry\n"); std::abort();
}
int CryActionRegressionEntity::LoadCharacter( int nSlot,const char *sFilename,int nLoadFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::LoadCharacter\n"); std::abort();
}
int CryActionRegressionEntity::LoadParticleEmitter( int nSlot, IParticleEffect* pEffect, SpawnParams const* params, bool bPrime, bool bSerialize)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::LoadParticleEmitter\n"); std::abort();
}
int CryActionRegressionEntity::SetParticleEmitter( int nSlot, IParticleEmitter* pEmitter, bool bSerialize)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::SetParticleEmitter\n"); std::abort();
}
int CryActionRegressionEntity::LoadLight( int nSlot,CDLight *pLight )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::LoadLight\n"); std::abort();
}
int CryActionRegressionEntity::LoadCloud( int nSlot,const char *sCloudFilename )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::LoadCloud\n"); std::abort();
}
int CryActionRegressionEntity::LoadFogVolume( int nSlot, const SFogVolumeProperties& properties )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::LoadFogVolume\n"); std::abort();
}
void CryActionRegressionEntity::InvalidateTM( int nWhyFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::InvalidateTM\n"); std::abort();
}
void CryActionRegressionEntity::EnablePhysics(bool enable)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::EnablePhysics\n"); std::abort();
}
IEntityLink* CryActionRegressionEntity::GetEntityLinks()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::GetEntityLinks\n"); std::abort();
}
IEntityLink* CryActionRegressionEntity::AddEntityLink( const char *sLinkName,EntityId entityId )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::AddEntityLink\n"); std::abort();
}
void CryActionRegressionEntity::RemoveEntityLink( IEntityLink* pLink )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::RemoveEntityLink\n"); std::abort();
}
void CryActionRegressionEntity::RemoveAllEntityLinks()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::RemoveAllEntityLinks\n"); std::abort();
}
IEntity *CryActionRegressionEntity::UnmapAttachedChild(int &partId)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::UnmapAttachedChild\n"); std::abort();
}
bool CryActionRegressionEntity::IsInitialized() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntity::IsInitialized\n"); std::abort();
}
void CryActionRegressionEntities::Release()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::Release\n"); std::abort();
}
void CryActionRegressionEntities::PrePhysicsUpdate()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::PrePhysicsUpdate\n"); std::abort();
}
void CryActionRegressionEntities::Update()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::Update\n"); std::abort();
}
void CryActionRegressionEntities::Reset()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::Reset\n"); std::abort();
}
void CryActionRegressionEntities::DeletePendingEntities()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::DeletePendingEntities\n"); std::abort();
}
IEntityClassRegistry* CryActionRegressionEntities::GetClassRegistry()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::GetClassRegistry\n"); std::abort();
}
IEntity* CryActionRegressionEntities::SpawnEntity( SEntitySpawnParams &params,bool bAutoInit)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::SpawnEntity\n"); std::abort();
}
bool CryActionRegressionEntities::InitEntity( IEntity* pEntity,SEntitySpawnParams &params )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::InitEntity\n"); std::abort();
}
IEntity* CryActionRegressionEntities::GetEntity(EntityId id) const
{
    auto it = entities.find(id); return it == entities.end() ? nullptr : it->second;
}
IEntity* CryActionRegressionEntities::FindEntityByName( const char *sEntityName ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::FindEntityByName\n"); std::abort();
}
void CryActionRegressionEntities::ReserveEntityId( const EntityId id )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::ReserveEntityId\n"); std::abort();
}
void CryActionRegressionEntities::RemoveEntity( EntityId entity,bool bForceRemoveNow)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::RemoveEntity\n"); std::abort();
}
uint32 CryActionRegressionEntities::GetNumEntities() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::GetNumEntities\n"); std::abort();
}
IEntityIt * CryActionRegressionEntities::GetEntityIterator()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::GetEntityIterator\n"); std::abort();
}
void CryActionRegressionEntities::SendEventToAll( SEntityEvent &event )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::SendEventToAll\n"); std::abort();
}
int CryActionRegressionEntities::QueryProximity( SEntityProximityQuery &query )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::QueryProximity\n"); std::abort();
}
void CryActionRegressionEntities::ResizeProximityGrid( int nWidth,int nHeight )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::ResizeProximityGrid\n"); std::abort();
}
int CryActionRegressionEntities::GetPhysicalEntitiesInBox( const Vec3 &origin, float radius, IPhysicalEntity **&pList, int physFlags) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::GetPhysicalEntitiesInBox\n"); std::abort();
}
IEntity* CryActionRegressionEntities::GetEntityFromPhysics( struct IPhysicalEntity *pPhysEntity ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::GetEntityFromPhysics\n"); std::abort();
}
void CryActionRegressionEntities::AddSink( IEntitySystemSink *sink )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::AddSink\n"); std::abort();
}
void CryActionRegressionEntities::RemoveSink( IEntitySystemSink *sink )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::RemoveSink\n"); std::abort();
}
void CryActionRegressionEntities::PauseTimers( bool bPause,bool bResume)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::PauseTimers\n"); std::abort();
}
bool CryActionRegressionEntities::IsIDUsed( EntityId nID ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::IsIDUsed\n"); std::abort();
}
void CryActionRegressionEntities::GetMemoryStatistics(ICrySizer *pSizer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::GetMemoryStatistics\n"); std::abort();
}
ISystem* CryActionRegressionEntities::GetSystem() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::GetSystem\n"); std::abort();
}
void CryActionRegressionEntities::LoadEntities( XmlNodeRef &objectsNode )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::LoadEntities\n"); std::abort();
}
void CryActionRegressionEntities::AddEntityEventListener( EntityId nEntity,EEntityEvent event,IEntityEventListener *pListener )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::AddEntityEventListener\n"); std::abort();
}
void CryActionRegressionEntities::RemoveEntityEventListener( EntityId nEntity,EEntityEvent event,IEntityEventListener *pListener )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::RemoveEntityEventListener\n"); std::abort();
}
EntityId CryActionRegressionEntities::FindEntityByGuid( const EntityGUID &guid ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::FindEntityByGuid\n"); std::abort();
}
CAreaManager* CryActionRegressionEntities::GetAreaManager() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::GetAreaManager\n"); std::abort();
}
IBreakableManager* CryActionRegressionEntities::GetBreakableManager() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::GetBreakableManager\n"); std::abort();
}
IEntityArchetype* CryActionRegressionEntities::LoadEntityArchetype( const char *sArchetype )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::LoadEntityArchetype\n"); std::abort();
}
IEntityArchetype* CryActionRegressionEntities::CreateEntityArchetype( IEntityClass *pClass,const char *sArchetype )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::CreateEntityArchetype\n"); std::abort();
}
void CryActionRegressionEntities::Serialize(TSerialize ser)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::Serialize\n"); std::abort();
}
void CryActionRegressionEntities::SetNextSpawnId(EntityId id)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::SetNextSpawnId\n"); std::abort();
}
void CryActionRegressionEntities::ResetAreas()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::ResetAreas\n"); std::abort();
}
void CryActionRegressionEntities::DumpEntities()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::DumpEntities\n"); std::abort();
}
void CryActionRegressionEntities::LockSpawning(bool lock)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEntities::LockSpawning\n"); std::abort();
}
ISkeletonAnim* CryActionRegressionCharacter::GetISkeletonAnim()
{
    ++skeletonReads; return skeleton;
}
ISkeletonPose* CryActionRegressionCharacter::GetISkeletonPose()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetISkeletonPose\n"); std::abort();
}
IMorphing* CryActionRegressionCharacter::GetIMorphing()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetIMorphing\n"); std::abort();
}
IAttachmentManager* CryActionRegressionCharacter::GetIAttachmentManager()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetIAttachmentManager\n"); std::abort();
}
ICharacterModel* CryActionRegressionCharacter::GetICharacterModel()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetICharacterModel\n"); std::abort();
}
IAnimationSet* CryActionRegressionCharacter::GetIAnimationSet()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetIAnimationSet\n"); std::abort();
}
const char* CryActionRegressionCharacter::GetModelAnimEventDatabase()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetModelAnimEventDatabase\n"); std::abort();
}
void CryActionRegressionCharacter::AddRef()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::AddRef\n"); std::abort();
}
void CryActionRegressionCharacter::Release()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::Release\n"); std::abort();
}
void CryActionRegressionCharacter::Serialize(TSerialize ser)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::Serialize\n"); std::abort();
}
void CryActionRegressionCharacter::EnableStartAnimation(bool bEnable)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::EnableStartAnimation\n"); std::abort();
}
void CryActionRegressionCharacter::SkeletonPreProcess(const QuatT &rPhysLocationCurr,const QuatTS &rAnimLocationCurr, const CCamera& rCamera, uint32 OnRender )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::SkeletonPreProcess\n"); std::abort();
}
void CryActionRegressionCharacter::SkeletonPostProcess(const QuatT &rPhysLocationNext,const QuatTS &rAnimLocationNext, IAttachment* pIAttachment, float fZoomAdjustedDistanceFromCamera, uint32 OnRender )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::SkeletonPostProcess\n"); std::abort();
}
void CryActionRegressionCharacter::UpdateAttachedObjectsFast(const QuatT& rPhysLocationNext, float fZoomAdjustedDistanceFromCamera, uint32 OnRender )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::UpdateAttachedObjectsFast\n"); std::abort();
}
void CryActionRegressionCharacter::HideMaster(uint32 h)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::HideMaster\n"); std::abort();
}
void CryActionRegressionCharacter::DrawWireframeStatic( const Matrix34& m34, int nLOD, uint32 color)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::DrawWireframeStatic\n"); std::abort();
}
uint32 CryActionRegressionCharacter::GetResetMode()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetResetMode\n"); std::abort();
}
void CryActionRegressionCharacter::SetResetMode(uint32 rm)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::SetResetMode\n"); std::abort();
}
AABB CryActionRegressionCharacter::GetAABB()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetAABB\n"); std::abort();
}
f32 CryActionRegressionCharacter::ComputeExtent(GeomQuery& geo, EGeomForm eForm)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::ComputeExtent\n"); std::abort();
}
void CryActionRegressionCharacter::GetRandomPos(RandomPos& ran, GeomQuery& geo, EGeomForm eForm)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetRandomPos\n"); std::abort();
}
void CryActionRegressionCharacter::Render(const SRendParams& RendParams, const QuatTS& Offset, Matrix34 *pFinalPhysLocation)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::Render\n"); std::abort();
}
void CryActionRegressionCharacter::SetFlags(int nFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::SetFlags\n"); std::abort();
}
int CryActionRegressionCharacter::GetFlags()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetFlags\n"); std::abort();
}
int CryActionRegressionCharacter::GetOjectType()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetOjectType\n"); std::abort();
}
void CryActionRegressionCharacter::SetColor(f32 fR, f32 fG, f32 fB, f32 fA)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::SetColor\n"); std::abort();
}
const char* CryActionRegressionCharacter::GetFilePath()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetFilePath\n"); std::abort();
}
bool CryActionRegressionCharacter::IsModelFileEqual(const char* szFileName)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::IsModelFileEqual\n"); std::abort();
}
void CryActionRegressionCharacter::OnDetach()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::OnDetach\n"); std::abort();
}
void CryActionRegressionCharacter::EnableDecals(uint32 d)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::EnableDecals\n"); std::abort();
}
void CryActionRegressionCharacter::CreateDecal(CryEngineDecalInfo& DecalLCS)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::CreateDecal\n"); std::abort();
}
void CryActionRegressionCharacter::GetMemoryUsage(class ICrySizer* pSizer) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetMemoryUsage\n"); std::abort();
}
void CryActionRegressionCharacter::ReleaseTemporaryResources()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::ReleaseTemporaryResources\n"); std::abort();
}
f32 CryActionRegressionCharacter::GetAverageFrameTime()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetAverageFrameTime\n"); std::abort();
}
void CryActionRegressionCharacter::SetMaterial( IMaterial *pMaterial )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::SetMaterial\n"); std::abort();
}
IMaterial* CryActionRegressionCharacter::GetMaterial()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetMaterial\n"); std::abort();
}
IMaterial* CryActionRegressionCharacter::GetMaterialOverride()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetMaterialOverride\n"); std::abort();
}
uint32 CryActionRegressionCharacter::GetSkeletonPose(int nLod, const Matrix34& RenderMat34, QuatTS*& pBoneQuatsL, QuatTS*& pBoneQuatsS, QuatTS*& pMBBoneQuatsL, QuatTS*& pMBBoneQuatsS, Vec4 shapeDeformationData[], uint32 &DoWeNeedMorphtargets, uint8*& pRemapTable)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetSkeletonPose\n"); std::abort();
}
IFacialInstance* CryActionRegressionCharacter::GetFacialInstance()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetFacialInstance\n"); std::abort();
}
void CryActionRegressionCharacter::EnableFacialAnimation( bool bEnable )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::EnableFacialAnimation\n"); std::abort();
}
void CryActionRegressionCharacter::EnableProceduralFacialAnimation( bool bEnable )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::EnableProceduralFacialAnimation\n"); std::abort();
}
void CryActionRegressionCharacter::LipSyncWithSound( uint32 nSoundId, bool bStop)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::LipSyncWithSound\n"); std::abort();
}
void CryActionRegressionCharacter::SetAnimationSpeed(f32 fSpeed)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::SetAnimationSpeed\n"); std::abort();
}
f32 CryActionRegressionCharacter::GetAnimationSpeed()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetAnimationSpeed\n"); std::abort();
}
uint32 CryActionRegressionCharacter::IsCharacterVisible()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::IsCharacterVisible\n"); std::abort();
}
bool CryActionRegressionCharacter::IsAnimationGraphValid()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::IsAnimationGraphValid\n"); std::abort();
}
void CryActionRegressionCharacter::SetFPWeapon(f32 fp)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::SetFPWeapon\n"); std::abort();
}
void CryActionRegressionCharacter::ProcessSkinAttachment(const QuatT &rPhysLocationNext,const QuatTS &rAnimLocationNext, IAttachment* pIAttachment, float fZoomAdjustedDistanceFromCamera, uint32 OnRender )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::ProcessSkinAttachment\n"); std::abort();
}
size_t CryActionRegressionCharacter::SizeOfThis(ICrySizer * pSizer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::SizeOfThis\n"); std::abort();
}
f32* CryActionRegressionCharacter::GetShapeDeformArray()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::GetShapeDeformArray\n"); std::abort();
}
void CryActionRegressionCharacter::SpawnSkeletonEffect(int animID, const char* animName, const char* effectName, const char* boneName, const Vec3& offset, const Vec3& dir, const Matrix34& entityTM)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::SpawnSkeletonEffect\n"); std::abort();
}
void CryActionRegressionCharacter::KillAllSkeletonEffects()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionCharacter::KillAllSkeletonEffects\n"); std::abort();
}
void CryActionRegressionSkeleton::SetDebugging( uint32 flags )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetDebugging\n"); std::abort();
}
void CryActionRegressionSkeleton::SetAnimationDrivenMotion(uint32 ts)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetAnimationDrivenMotion\n"); std::abort();
}
uint32 CryActionRegressionSkeleton::GetAnimationDrivenMotion()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetAnimationDrivenMotion\n"); std::abort();
}
void CryActionRegressionSkeleton::SetTrackViewExclusive(uint32 i)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetTrackViewExclusive\n"); std::abort();
}
uint32 CryActionRegressionSkeleton::GetTrackViewStatus()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetTrackViewStatus\n"); std::abort();
}
void CryActionRegressionSkeleton::SetFuturePathAnalyser(uint32 a)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetFuturePathAnalyser\n"); std::abort();
}
void CryActionRegressionSkeleton::GetFuturePath( SAnimRoot* pRelFuturePath )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetFuturePath\n"); std::abort();
}
void CryActionRegressionSkeleton::SetCharEditMode( uint32 m )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetCharEditMode\n"); std::abort();
}
bool CryActionRegressionSkeleton::StartAnimation(const char* szAnimName0,const char* szAnimName1, const char* szAim0,const char* szAim1, const struct CryCharAnimationParams& Params)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::StartAnimation\n"); std::abort();
}
bool CryActionRegressionSkeleton::StopAnimationInLayer(int32 nLayer, f32 BlendOutTime)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::StopAnimationInLayer\n"); std::abort();
}
bool CryActionRegressionSkeleton::StopAnimationsAllLayers()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::StopAnimationsAllLayers\n"); std::abort();
}
bool CryActionRegressionSkeleton::RemoveAnimFromFIFO(uint32 nLayer, uint32 num)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::RemoveAnimFromFIFO\n"); std::abort();
}
int CryActionRegressionSkeleton::GetNumAnimsInFIFO(uint32 nLayer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetNumAnimsInFIFO\n"); std::abort();
}
CAnimation& CryActionRegressionSkeleton::GetAnimFromFIFO(uint32 nLayer, uint32 num )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetAnimFromFIFO\n"); std::abort();
}
void CryActionRegressionSkeleton::ManualSeekAnimationInFIFO(uint32 nLayer, uint32 num, float time, bool advance)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::ManualSeekAnimationInFIFO\n"); std::abort();
}
void CryActionRegressionSkeleton::SetLayerUpdateMultiplier(int32 nLayer, f32 fSpeed)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetLayerUpdateMultiplier\n"); std::abort();
}
void CryActionRegressionSkeleton::SetDesiredLocalLocation(const QuatT& desiredLocalLocation, float deltaTime, float frameTime, float turnSpeedMultiplier)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetDesiredLocalLocation\n"); std::abort();
}
void CryActionRegressionSkeleton::SetDesiredMotionParam(EMotionParamID id, f32 value, f32 frametime, bool initOnly)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetDesiredMotionParam\n"); std::abort();
}
float CryActionRegressionSkeleton::GetDesiredMotionParam(EMotionParamID id) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetDesiredMotionParam\n"); std::abort();
}
void CryActionRegressionSkeleton::SetBlendSpaceOverride(EMotionParamID id, float value, bool enable)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetBlendSpaceOverride\n"); std::abort();
}
f32 CryActionRegressionSkeleton::GetAnimationSpeedLayer(uint32 nLayer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetAnimationSpeedLayer\n"); std::abort();
}
void CryActionRegressionSkeleton::SetIWeight(uint32 layer, f32 bw)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetIWeight\n"); std::abort();
}
f32 CryActionRegressionSkeleton::GetIWeight(uint32 nLayer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetIWeight\n"); std::abort();
}
void CryActionRegressionSkeleton::SetLayerTime(uint32 nLayer, f32 fTimeSeconds)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetLayerTime\n"); std::abort();
}
f32 CryActionRegressionSkeleton::GetLayerTime(uint32 nLayer) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetLayerTime\n"); std::abort();
}
Vec3 CryActionRegressionSkeleton::GetCurrentVelocity()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetCurrentVelocity\n"); std::abort();
}
Vec3 CryActionRegressionSkeleton::GetCurrentAimDirection()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetCurrentAimDirection\n"); std::abort();
}
Vec3 CryActionRegressionSkeleton::GetCurrentLookDirection()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetCurrentLookDirection\n"); std::abort();
}
f32 CryActionRegressionSkeleton::GetCurrentSlope()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetCurrentSlope\n"); std::abort();
}
void CryActionRegressionSkeleton::SetEventCallback(int (*func)(ICharacterInstance*,void*), void *pdata)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetEventCallback\n"); std::abort();
}
AnimEventInstance CryActionRegressionSkeleton::GetLastAnimEvent()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetLastAnimEvent\n"); std::abort();
}
void CryActionRegressionSkeleton::SetPreProcessCallback(int (*func)(ICharacterInstance*,void*), void *pdata)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::SetPreProcessCallback\n"); std::abort();
}
AnimTransRotParams CryActionRegressionSkeleton::GetBlendedAnimTransRot(f32 DeltaTime, uint32 clamp)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetBlendedAnimTransRot\n"); std::abort();
}
f32 CryActionRegressionSkeleton::GetFootPlantStatus()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetFootPlantStatus\n"); std::abort();
}
Vec3 CryActionRegressionSkeleton::GetRelTranslation()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetRelTranslation\n"); std::abort();
}
f32 CryActionRegressionSkeleton::GetRelRotationZ()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetRelRotationZ\n"); std::abort();
}
QuatT CryActionRegressionSkeleton::GetRelMovement()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetRelMovement\n"); std::abort();
}
Vec3 CryActionRegressionSkeleton::GetRelFootSlide()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionSkeleton::GetRelFootSlide\n"); std::abort();
}
f32 CryActionRegressionSkeleton::GetUserData(int id)
{
    reads.push_back(id); return movement;
}
bool CryActionRegressionGame::Init(IGameFramework *pFramework)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::Init\n"); std::abort();
}
void CryActionRegressionGame::GetMemoryStatistics(ICrySizer * s)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::GetMemoryStatistics\n"); std::abort();
}
void CryActionRegressionGame::Shutdown()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::Shutdown\n"); std::abort();
}
string CryActionRegressionGame::InitMapReloading()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::InitMapReloading\n"); std::abort();
}
int CryActionRegressionGame::Update(bool haveFocus, unsigned int updateFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::Update\n"); std::abort();
}
void CryActionRegressionGame::ConfigureGameChannel(bool isServer, IProtocolBuilder *pBuilder)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::ConfigureGameChannel\n"); std::abort();
}
void CryActionRegressionGame::EditorResetGame(bool bStart)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::EditorResetGame\n"); std::abort();
}
void CryActionRegressionGame::PlayerIdSet(EntityId playerId)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::PlayerIdSet\n"); std::abort();
}
IGameFramework* CryActionRegressionGame::GetIGameFramework()
{
    return framework;
}
const char *CryActionRegressionGame::GetLongName()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::GetLongName\n"); std::abort();
}
const char *CryActionRegressionGame::GetName()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::GetName\n"); std::abort();
}
void CryActionRegressionGame::LoadActionMaps(const char* filename)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::LoadActionMaps\n"); std::abort();
}
void CryActionRegressionGame::OnClearPlayerIds()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::OnClearPlayerIds\n"); std::abort();
}
bool CryActionRegressionGame::IsReloading()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::IsReloading\n"); std::abort();
}
const char* CryActionRegressionGame::CreateSaveGameName()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::CreateSaveGameName\n"); std::abort();
}
const char* CryActionRegressionGame::GetMappedLevelName(const char *levelName) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionGame::GetMappedLevelName\n"); std::abort();
}
bool CryActionRegressionEngine3D::Init()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::Init\n"); std::abort();
}
void CryActionRegressionEngine3D::SetLevelPath( const char * szFolderName )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetLevelPath\n"); std::abort();
}
bool CryActionRegressionEngine3D::LoadLevel(const char * szFolderName, const char * szMissionName)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::LoadLevel\n"); std::abort();
}
bool CryActionRegressionEngine3D::InitLevelForEditor(const char * szFolderName, const char * szMissionName)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::InitLevelForEditor\n"); std::abort();
}
void CryActionRegressionEngine3D::UnloadLevel( bool bForceClearParticlesAssets)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::UnloadLevel\n"); std::abort();
}
void CryActionRegressionEngine3D::PostLoadLevel()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::PostLoadLevel\n"); std::abort();
}
void CryActionRegressionEngine3D::OnFrameStart()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::OnFrameStart\n"); std::abort();
}
void CryActionRegressionEngine3D::Update()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::Update\n"); std::abort();
}
void CryActionRegressionEngine3D::RenderWorld(const int nRenderFlags, const CCamera &cam, const char *szDebugName, const int dwDrawFlags, const int nFilterFlags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::RenderWorld\n"); std::abort();
}
void CryActionRegressionEngine3D::ShutDown()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::ShutDown\n"); std::abort();
}
void CryActionRegressionEngine3D::Release()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::Release\n"); std::abort();
}
IStatObj* CryActionRegressionEngine3D::LoadStatObj( const char *szFileName,const char *szGeomName,IStatObj::SSubObject **ppSubObject)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::LoadStatObj\n"); std::abort();
}
void CryActionRegressionEngine3D::GetLoadedStatObjArray( IStatObj** pObjectsArray,int &nCount )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetLoadedStatObjArray\n"); std::abort();
}
void CryActionRegressionEngine3D::RegisterEntity( IRenderNode * pEntity )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::RegisterEntity\n"); std::abort();
}
void CryActionRegressionEngine3D::SelectEntity( IRenderNode * pEntity )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SelectEntity\n"); std::abort();
}
bool CryActionRegressionEngine3D::UnRegisterEntity( IRenderNode * pEntity )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::UnRegisterEntity\n"); std::abort();
}
bool CryActionRegressionEngine3D::IsUnderWater( const Vec3& vPos) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::IsUnderWater\n"); std::abort();
}
void CryActionRegressionEngine3D::SetOceanRenderFlags( uint8 nFlags )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetOceanRenderFlags\n"); std::abort();
}
uint8 CryActionRegressionEngine3D::GetOceanRenderFlags() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetOceanRenderFlags\n"); std::abort();
}
uint32 CryActionRegressionEngine3D::GetOceanVisiblePixelsCount() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetOceanVisiblePixelsCount\n"); std::abort();
}
float CryActionRegressionEngine3D::GetBottomLevel(const Vec3 & referencePos, float maxRelevantDepth, int objtypes)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetBottomLevel\n"); std::abort();
}
float CryActionRegressionEngine3D::GetBottomLevel(const Vec3 & referencePos, float maxRelevantDepth)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetBottomLevel\n"); std::abort();
}
float CryActionRegressionEngine3D::GetBottomLevel(const Vec3 & referencePos, int objflags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetBottomLevel\n"); std::abort();
}
float CryActionRegressionEngine3D::GetWaterLevel(const Vec3 * pvPos, Vec3 * pvFlowDir)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetWaterLevel\n"); std::abort();
}
float CryActionRegressionEngine3D::GetOceanWaterLevel( const Vec3 &pCurrPos ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetOceanWaterLevel\n"); std::abort();
}
Vec4 CryActionRegressionEngine3D::GetCausticsParams() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetCausticsParams\n"); std::abort();
}
void CryActionRegressionEngine3D::GetOceanAnimationParams(Vec4 &pParams0, Vec4 &pParams1 ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetOceanAnimationParams\n"); std::abort();
}
void CryActionRegressionEngine3D::ResetParticlesAndDecals( )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::ResetParticlesAndDecals\n"); std::abort();
}
IParticleEmitter* CryActionRegressionEngine3D::CreateParticleEmitter( bool bIndependent, Matrix34 const& mLoc, const ParticleParams& Params )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateParticleEmitter\n"); std::abort();
}
void CryActionRegressionEngine3D::DeleteParticleEmitter(IParticleEmitter * pPartEmitter)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DeleteParticleEmitter\n"); std::abort();
}
IParticleEffect* CryActionRegressionEngine3D::CreateParticleEffect()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateParticleEffect\n"); std::abort();
}
void CryActionRegressionEngine3D::DeleteParticleEffect( IParticleEffect* pEffect )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DeleteParticleEffect\n"); std::abort();
}
IParticleEffect* CryActionRegressionEngine3D::FindParticleEffect( const char *sEffectName, const char* sCallerType, const char* sCallerName, bool bLoad)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::FindParticleEffect\n"); std::abort();
}
void CryActionRegressionEngine3D::CreateDecal( const CryEngineDecalInfo & Decal )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateDecal\n"); std::abort();
}
void CryActionRegressionEngine3D::DeleteDecalsInRange( AABB * pAreaBox, IRenderNode * pEntity )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DeleteDecalsInRange\n"); std::abort();
}
void CryActionRegressionEngine3D::DrawFarTrees()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DrawFarTrees\n"); std::abort();
}
void CryActionRegressionEngine3D::GenerateFarTrees()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GenerateFarTrees\n"); std::abort();
}
void CryActionRegressionEngine3D::SetSkyColor(Vec3 vColor)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetSkyColor\n"); std::abort();
}
void CryActionRegressionEngine3D::SetSunColor(Vec3 vColor)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetSunColor\n"); std::abort();
}
void CryActionRegressionEngine3D::SetSkyBrightness(float fMul)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetSkyBrightness\n"); std::abort();
}
float CryActionRegressionEngine3D::GetSunRel()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetSunRel\n"); std::abort();
}
float CryActionRegressionEngine3D::GetMaxViewDistance()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetMaxViewDistance\n"); std::abort();
}
void CryActionRegressionEngine3D::SetFogColor(const Vec3& vFogColor)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetFogColor\n"); std::abort();
}
Vec3 CryActionRegressionEngine3D::GetFogColor( )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetFogColor\n"); std::abort();
}
void CryActionRegressionEngine3D::GetVolumetricFogSettings( float& globalDensity, float& atmosphereHeight, float& artistTweakDensityOffset, float& globalDensityMultiplierLDR )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetVolumetricFogSettings\n"); std::abort();
}
void CryActionRegressionEngine3D::SetVolumetricFogSettings( float globalDensity, float atmosphereHeight, float artistTweakDensityOffset )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetVolumetricFogSettings\n"); std::abort();
}
void CryActionRegressionEngine3D::GetVolumetricFogModifiers(float& globalDensityModifier, float& atmosphereHeightModifier)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetVolumetricFogModifiers\n"); std::abort();
}
void CryActionRegressionEngine3D::SetVolumetricFogModifiers(float globalDensityModifier, float atmosphereHeightModifier, bool reset)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetVolumetricFogModifiers\n"); std::abort();
}
void CryActionRegressionEngine3D::GetSkyLightParameters( Vec3& sunIntensity, float& Km, float& Kr, float& g, Vec3& rgbWaveLengths )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetSkyLightParameters\n"); std::abort();
}
void CryActionRegressionEngine3D::SetSkyLightParameters( const Vec3& sunIntensity, float Km, float Kr, float g, const Vec3& rgbWaveLengths, bool forceImmediateUpdate)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetSkyLightParameters\n"); std::abort();
}
void CryActionRegressionEngine3D::GetCloudShadingMultiplier( float& sunLightMultiplier, float& skyLightMultiplier )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetCloudShadingMultiplier\n"); std::abort();
}
void CryActionRegressionEngine3D::SetCloudShadingMultiplier( float sunLightMultiplier, float skyLightMultiplier )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetCloudShadingMultiplier\n"); std::abort();
}
IAutoCubeMapRenderNode* CryActionRegressionEngine3D::GetClosestAutoCubeMap(const Vec3& p)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetClosestAutoCubeMap\n"); std::abort();
}
float CryActionRegressionEngine3D::GetHDRDynamicMultiplier() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetHDRDynamicMultiplier\n"); std::abort();
}
void CryActionRegressionEngine3D::SetHDRDynamicMultiplier( const float value )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetHDRDynamicMultiplier\n"); std::abort();
}
void CryActionRegressionEngine3D::TraceFogVolumes( const Vec3& worldPos, ColorF& fogVolumeContrib )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::TraceFogVolumes\n"); std::abort();
}
float CryActionRegressionEngine3D::GetTerrainElevation(float x, float y, bool bIncludeOutdoorVoxles)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetTerrainElevation\n"); std::abort();
}
float CryActionRegressionEngine3D::GetTerrainZ(int x, int y)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetTerrainZ\n"); std::abort();
}
Vec3 CryActionRegressionEngine3D::GetTerrainSurfaceNormal(Vec3 vPos)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetTerrainSurfaceNormal\n"); std::abort();
}
int CryActionRegressionEngine3D::GetHeightMapUnitSize()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetHeightMapUnitSize\n"); std::abort();
}
int CryActionRegressionEngine3D::GetTerrainSize()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetTerrainSize\n"); std::abort();
}
int CryActionRegressionEngine3D::GetTerrainSectorSize()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetTerrainSectorSize\n"); std::abort();
}
void CryActionRegressionEngine3D::RemoveAllStaticObjects()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::RemoveAllStaticObjects\n"); std::abort();
}
void CryActionRegressionEngine3D::SetTerrainSurfaceType(int x, int y, int nType)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetTerrainSurfaceType\n"); std::abort();
}
bool CryActionRegressionEngine3D::IsTerrainHightMapModifiedByGame()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::IsTerrainHightMapModifiedByGame\n"); std::abort();
}
void CryActionRegressionEngine3D::SetHeightMapMaxHeight(float fMaxHeight)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetHeightMapMaxHeight\n"); std::abort();
}
void CryActionRegressionEngine3D::SetTerrainSectorTexture( const int nTexSectorX, const int nTexSectorY, unsigned int textureId )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetTerrainSectorTexture\n"); std::abort();
}
int CryActionRegressionEngine3D::GetTerrainTextureNodeSizeMeters()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetTerrainTextureNodeSizeMeters\n"); std::abort();
}
int CryActionRegressionEngine3D::GetTerrainTextureNodeSizePixels(int nLayer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetTerrainTextureNodeSizePixels\n"); std::abort();
}
bool CryActionRegressionEngine3D::SetStatInstGroup(int nGroupId, const IStatInstGroup & siGroup)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetStatInstGroup\n"); std::abort();
}
bool CryActionRegressionEngine3D::GetStatInstGroup(int nGroupId, IStatInstGroup & siGroup)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetStatInstGroup\n"); std::abort();
}
void CryActionRegressionEngine3D::SetTerrainBurnedOut(int x, int y, bool bBurnedOut)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetTerrainBurnedOut\n"); std::abort();
}
bool CryActionRegressionEngine3D::IsTerrainBurnedOut(int x, int y)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::IsTerrainBurnedOut\n"); std::abort();
}
void CryActionRegressionEngine3D::OnExplosion(Vec3 vPos, float fRadius, bool bDeformTerrain)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::OnExplosion\n"); std::abort();
}
void CryActionRegressionEngine3D::AddWaterSplash(Vec3 vPos, enum eSplashType eST, float fForce, int Id)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::AddWaterSplash\n"); std::abort();
}
void CryActionRegressionEngine3D::SetPhysMaterialEnumerator(IPhysMaterialEnumerator * pPhysMaterialEnumerator)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetPhysMaterialEnumerator\n"); std::abort();
}
IPhysMaterialEnumerator * CryActionRegressionEngine3D::GetPhysMaterialEnumerator()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetPhysMaterialEnumerator\n"); std::abort();
}
void CryActionRegressionEngine3D::SetupDistanceFog()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetupDistanceFog\n"); std::abort();
}
void CryActionRegressionEngine3D::LoadMissionDataFromXMLNode(const char * szMissionName)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::LoadMissionDataFromXMLNode\n"); std::abort();
}
void CryActionRegressionEngine3D::LoadEnvironmentSettingsFromXML(XmlNodeRef pInputNode)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::LoadEnvironmentSettingsFromXML\n"); std::abort();
}
void CryActionRegressionEngine3D::LoadTerrainSurfacesFromXML(XmlNodeRef pDoc, bool bUpdateTerrain)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::LoadTerrainSurfacesFromXML\n"); std::abort();
}
void CryActionRegressionEngine3D::ApplyForceToEnvironment(Vec3 vPos, float fRadius, float fAmountOfForce)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::ApplyForceToEnvironment\n"); std::abort();
}
Vec3 CryActionRegressionEngine3D::GetSunDir()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetSunDir\n"); std::abort();
}
Vec3 CryActionRegressionEngine3D::GetSunDirNormalized()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetSunDirNormalized\n"); std::abort();
}
Vec3 CryActionRegressionEngine3D::GetRealtimeSunDirNormalized()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetRealtimeSunDirNormalized\n"); std::abort();
}
Vec3 CryActionRegressionEngine3D::GetAmbientColorFromPosition(const Vec3 & vPos, float fRadius)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetAmbientColorFromPosition\n"); std::abort();
}
float CryActionRegressionEngine3D::GetDistanceToSectorWithWater()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetDistanceToSectorWithWater\n"); std::abort();
}
Vec3 CryActionRegressionEngine3D::GetSkyColor()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetSkyColor\n"); std::abort();
}
Vec3 CryActionRegressionEngine3D::GetSunColor()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetSunColor\n"); std::abort();
}
float CryActionRegressionEngine3D::GetSkyBrightness()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetSkyBrightness\n"); std::abort();
}
float CryActionRegressionEngine3D::GetSSAOAmount()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetSSAOAmount\n"); std::abort();
}
float CryActionRegressionEngine3D::GetSunShadowIntensity() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetSunShadowIntensity\n"); std::abort();
}
void CryActionRegressionEngine3D::ClearRenderResources()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::ClearRenderResources\n"); std::abort();
}
void CryActionRegressionEngine3D::FreeRenderNodeState(IRenderNode * pEntity)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::FreeRenderNodeState\n"); std::abort();
}
const char * CryActionRegressionEngine3D::GetLevelFilePath(const char * szFileName)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetLevelFilePath\n"); std::abort();
}
void CryActionRegressionEngine3D::DisplayInfo(float & fTextPosX, float & fTextPosY, float & fTextStepY, const bool bEnhanced )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DisplayInfo\n"); std::abort();
}
void CryActionRegressionEngine3D::DrawText(float x, float y, const char * format, ...) PRINTF_PARAMS(4, 5)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DrawText\n"); std::abort();
}
void CryActionRegressionEngine3D::DrawTextRA(float x, float y, const char * format, ...) PRINTF_PARAMS(4, 5)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DrawTextRA\n"); std::abort();
}
void CryActionRegressionEngine3D::ActivatePortal(const Vec3 &vPos, bool bActivate, const char * szEntityName)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::ActivatePortal\n"); std::abort();
}
void CryActionRegressionEngine3D::GetMemoryUsage(class ICrySizer * pSizer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetMemoryUsage\n"); std::abort();
}
IVisArea * CryActionRegressionEngine3D::CreateVisArea()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateVisArea\n"); std::abort();
}
void CryActionRegressionEngine3D::DeleteVisArea(IVisArea * pVisArea)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DeleteVisArea\n"); std::abort();
}
void CryActionRegressionEngine3D::UpdateVisArea(IVisArea * pArea, const Vec3 * pPoints, int nCount, const char * szName, const SVisAreaInfo & info, bool bReregisterObjects)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::UpdateVisArea\n"); std::abort();
}
bool CryActionRegressionEngine3D::IsVisAreasConnected(IVisArea * pArea1, IVisArea * pArea2, int nMaxRecursion, bool bSkipDisabledPortals)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::IsVisAreasConnected\n"); std::abort();
}
IRenderNode* CryActionRegressionEngine3D::CreateRenderNode( EERType type )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateRenderNode\n"); std::abort();
}
void CryActionRegressionEngine3D::DeleteRenderNode(IRenderNode * pRenderNode)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DeleteRenderNode\n"); std::abort();
}
void CryActionRegressionEngine3D::SetWind( const Vec3 & vWind )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetWind\n"); std::abort();
}
Vec3 CryActionRegressionEngine3D::GetWind( const AABB & box, bool bIndoors ) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetWind\n"); std::abort();
}
IVisArea * CryActionRegressionEngine3D::GetVisAreaFromPos(const Vec3 &vPos)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetVisAreaFromPos\n"); std::abort();
}
bool CryActionRegressionEngine3D::IntersectsVisAreas(const AABB& box, void** pNodeCache)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::IntersectsVisAreas\n"); std::abort();
}
bool CryActionRegressionEngine3D::ClipToVisAreas(IVisArea* pInside, Sphere& sphere, Vec3 const& vNormal, void* pNodeCache)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::ClipToVisAreas\n"); std::abort();
}
void CryActionRegressionEngine3D::EnableOceanRendering(bool bOcean)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::EnableOceanRendering\n"); std::abort();
}
IMaterialManager* CryActionRegressionEngine3D::GetMaterialManager()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetMaterialManager\n"); std::abort();
}
struct ILMSerializationManager * CryActionRegressionEngine3D::CreateLMSerializationManager()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateLMSerializationManager\n"); std::abort();
}
struct ILightSource * CryActionRegressionEngine3D::CreateLightSource()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateLightSource\n"); std::abort();
}
void CryActionRegressionEngine3D::DeleteLightSource(ILightSource * pLightSource)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DeleteLightSource\n"); std::abort();
}
const PodArray<CDLight*> * CryActionRegressionEngine3D::GetStaticLightSources()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetStaticLightSources\n"); std::abort();
}
const PodArray<ILightSource*> * CryActionRegressionEngine3D::GetLightEntities()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetLightEntities\n"); std::abort();
}
bool CryActionRegressionEngine3D::RestoreTerrainFromDisk()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::RestoreTerrainFromDisk\n"); std::abort();
}
void CryActionRegressionEngine3D::SetPostEffectParam(const char *pParam, float fValue) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetPostEffectParam\n"); std::abort();
}
void CryActionRegressionEngine3D::SetPostEffectParamVec4(const char *pParam, const Vec4 &pValue) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetPostEffectParamVec4\n"); std::abort();
}
void CryActionRegressionEngine3D::SetPostEffectParamString(const char *pParam, const char *pszArg) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetPostEffectParamString\n"); std::abort();
}
void CryActionRegressionEngine3D::GetPostEffectParam(const char *pParam, float &fValue) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetPostEffectParam\n"); std::abort();
}
void CryActionRegressionEngine3D::GetPostEffectParamVec4(const char *pParam, Vec4 &pValue) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetPostEffectParamVec4\n"); std::abort();
}
void CryActionRegressionEngine3D::GetPostEffectParamString(const char *pParam, const char *pszArg) const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetPostEffectParamString\n"); std::abort();
}
void CryActionRegressionEngine3D::ResetPostEffects() const
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::ResetPostEffects\n"); std::abort();
}
void CryActionRegressionEngine3D::CheckPhysicalized(const Vec3 & vBoxMin, const Vec3 & vBoxMax)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CheckPhysicalized\n"); std::abort();
}
void CryActionRegressionEngine3D::CheckMemoryHeap()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CheckMemoryHeap\n"); std::abort();
}
bool CryActionRegressionEngine3D::SetMaterialFloat( char * szMatName, int nSubMatId, int nTexSlot, char * szParamName, float fValue )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetMaterialFloat\n"); std::abort();
}
void CryActionRegressionEngine3D::CloseTerrainTextureFile()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CloseTerrainTextureFile\n"); std::abort();
}
void CryActionRegressionEngine3D::DeleteEntityDecals(IRenderNode * pEntity)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DeleteEntityDecals\n"); std::abort();
}
void CryActionRegressionEngine3D::LoadLightmaps()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::LoadLightmaps\n"); std::abort();
}
void CryActionRegressionEngine3D::CompleteObjectsGeometry()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CompleteObjectsGeometry\n"); std::abort();
}
void CryActionRegressionEngine3D::LockCGFResources()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::LockCGFResources\n"); std::abort();
}
void CryActionRegressionEngine3D::UnlockCGFResources()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::UnlockCGFResources\n"); std::abort();
}
class CMatMan * CryActionRegressionEngine3D::GetMatMan()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetMatMan\n"); std::abort();
}
IStatObj * CryActionRegressionEngine3D::CreateStatObj()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateStatObj\n"); std::abort();
}
IIndexedMesh* CryActionRegressionEngine3D::CreateIndexedMesh()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateIndexedMesh\n"); std::abort();
}
void CryActionRegressionEngine3D::Voxel_Paint(Vec3 vPos, float fRadius, int nSurfaceTypeId, Vec3 vBaseColor, EVoxelEditOperation eOperation, EVoxelBrushShape eShape, EVoxelEditTarget eTarget)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::Voxel_Paint\n"); std::abort();
}
IMemoryBlock * CryActionRegressionEngine3D::Voxel_GetObjects(Vec3 vPos, float fRadius, int nSurfaceTypeId, EVoxelEditOperation eOperation, EVoxelBrushShape eShape, EVoxelEditTarget eTarget)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::Voxel_GetObjects\n"); std::abort();
}
void CryActionRegressionEngine3D::Voxel_SetFlags(bool bPhysics, bool bSimplify, bool bShadows, bool bMaterials)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::Voxel_SetFlags\n"); std::abort();
}
IStatObj * CryActionRegressionEngine3D::UpdateDeformableStatObj(IGeometry *pPhysGeom, bop_meshupdate *pLastUpdate, IFoliage *pSrcFoliage)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::UpdateDeformableStatObj\n"); std::abort();
}
IStatObj * CryActionRegressionEngine3D::SmearStatObj(IStatObj *pObj, const Vec3 &pthit,const Vec3 &dir, float r,float depth)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SmearStatObj\n"); std::abort();
}
void CryActionRegressionEngine3D::SerializeState( TSerialize ser )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SerializeState\n"); std::abort();
}
void CryActionRegressionEngine3D::PostSerialize( bool bReading )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::PostSerialize\n"); std::abort();
}
CContentCGF* CryActionRegressionEngine3D::LoadChunkFileContent( const char *filename,bool bNoWarningMode)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::LoadChunkFileContent\n"); std::abort();
}
void CryActionRegressionEngine3D::ReleaseChunkFileContent( CContentCGF *pCGF )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::ReleaseChunkFileContent\n"); std::abort();
}
IChunkFile * CryActionRegressionEngine3D::CreateChunkFile( bool bReadOnly)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateChunkFile\n"); std::abort();
}
ITerrain * CryActionRegressionEngine3D::GetITerrain()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetITerrain\n"); std::abort();
}
ITerrain * CryActionRegressionEngine3D::CreateTerrain(const STerrainInfo & TerrainInfo)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CreateTerrain\n"); std::abort();
}
void CryActionRegressionEngine3D::DeleteTerrain()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DeleteTerrain\n"); std::abort();
}
IVisAreaManager * CryActionRegressionEngine3D::GetIVisAreaManager()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetIVisAreaManager\n"); std::abort();
}
void CryActionRegressionEngine3D::GetVoxelRenderNodes(struct IRenderNode**pRenderNodes, int & nCount)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetVoxelRenderNodes\n"); std::abort();
}
float CryActionRegressionEngine3D::GetLightAmountInRange(const Vec3 &pPos, float fRange, bool bAccurate)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetLightAmountInRange\n"); std::abort();
}
void CryActionRegressionEngine3D::PrecacheLevel(bool value, Vec3* points, int count)
{
    ++calls; enabled = value; nullPoints = points == nullptr;
    if (count) positions.assign(points, points + count); else positions.clear();
}
void CryActionRegressionEngine3D::ProposeContentPrecache()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::ProposeContentPrecache\n"); std::abort();
}
ITimeOfDay* CryActionRegressionEngine3D::GetTimeOfDay()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetTimeOfDay\n"); std::abort();
}
void CryActionRegressionEngine3D::SetGlobalParameter( E3DEngineParameter param,const Vec3 &v )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetGlobalParameter\n"); std::abort();
}
void CryActionRegressionEngine3D::GetGlobalParameter( E3DEngineParameter param,Vec3 &v )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetGlobalParameter\n"); std::abort();
}
int CryActionRegressionEngine3D::SaveStatObj(IStatObj *pStatObj, TSerialize ser)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SaveStatObj\n"); std::abort();
}
IStatObj *CryActionRegressionEngine3D::LoadStatObj(TSerialize ser)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::LoadStatObj\n"); std::abort();
}
bool CryActionRegressionEngine3D::CheckIntersectClouds(const Vec3 & p1, const Vec3 & p2)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::CheckIntersectClouds\n"); std::abort();
}
void CryActionRegressionEngine3D::OnRenderMeshDeleted(IRenderMesh * pRenderMesh)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::OnRenderMeshDeleted\n"); std::abort();
}
void CryActionRegressionEngine3D::DebugDraw_PushFrustrum( const char *szName, const CCamera &rCam, const ColorB col, const float fQuadDist)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DebugDraw_PushFrustrum\n"); std::abort();
}
void CryActionRegressionEngine3D::DebugDraw_PushFrustrum( const char *szName, const CRenderCamera &rCam, const ColorB col, const float fQuadDist)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::DebugDraw_PushFrustrum\n"); std::abort();
}
bool CryActionRegressionEngine3D::RayObjectsIntersection( Vec3 vStart, Vec3 vEnd, Vec3 & vHitPoint, EERType eERType )
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::RayObjectsIntersection\n"); std::abort();
}
void CryActionRegressionEngine3D::FreeRNTmpData(struct CRNTmpData ** ppInfo)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::FreeRNTmpData\n"); std::abort();
}
bool CryActionRegressionEngine3D::IsAmbientOcclusionEnabled()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::IsAmbientOcclusionEnabled\n"); std::abort();
}
uint32 CryActionRegressionEngine3D::GetObjectsByType( EERType objType, IRenderNode **pObjects)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetObjectsByType\n"); std::abort();
}
struct CLoadingTimeContainer * CryActionRegressionEngine3D::StartLoadingSectionProfiling(struct CLoadingTimeProfiler * pProfiler, const char * szFuncName)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::StartLoadingSectionProfiling\n"); std::abort();
}
void CryActionRegressionEngine3D::EndLoadingSectionProfiling(struct CLoadingTimeProfiler * pProfiler)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::EndLoadingSectionProfiling\n"); std::abort();
}
void CryActionRegressionEngine3D::OutputLoadingTimeStats()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::OutputLoadingTimeStats\n"); std::abort();
}
void CryActionRegressionEngine3D::FillDebugFPSInfo(SDebugFPSInfo&)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::FillDebugFPSInfo\n"); std::abort();
}
bool CryActionRegressionEngine3D::RefineRayHit(ray_hit *phit, const Vec3 &dir)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::RefineRayHit\n"); std::abort();
}
void CryActionRegressionEngine3D::SetFlags(int flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::SetFlags\n"); std::abort();
}
int CryActionRegressionEngine3D::GetFlags(void)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionEngine3D::GetFlags\n"); std::abort();
}
pe_type CryActionRegressionPhysicalEntity::GetType()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::GetType\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::AddRef()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::AddRef\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::Release()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::Release\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::SetParams(pe_params* params, int threadSafe)
{
    if (params->type != pe_params_structural_joint::type_id) std::abort();
    joints.push_back(*static_cast<pe_params_structural_joint*>(params)); return succeeds ? 1 : 0;
}
int CryActionRegressionPhysicalEntity::GetParams(pe_params* params)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::GetParams\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::GetStatus(pe_status* status)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::GetStatus\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::Action(pe_action*, int bThreadSafe)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::Action\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id, int bThreadSafe)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::AddGeometry\n"); std::abort();
}
void CryActionRegressionPhysicalEntity::RemoveGeometry(int id, int bThreadSafe)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::RemoveGeometry\n"); std::abort();
}
void* CryActionRegressionPhysicalEntity::GetForeignData(int type)
{
    return type == PHYS_FOREIGN_ID_ENTITY ? entity : nullptr;
}
int CryActionRegressionPhysicalEntity::GetiForeignData()
{
    return PHYS_FOREIGN_ID_ENTITY;
}
int CryActionRegressionPhysicalEntity::GetStateSnapshot(class CStream &stm, float time_back, int flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::GetStateSnapshot\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::GetStateSnapshot(TSerialize ser, float time_back, int flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::GetStateSnapshot\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::SetStateFromSnapshot(class CStream &stm, int flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::SetStateFromSnapshot\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::SetStateFromSnapshot(TSerialize ser, int flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::SetStateFromSnapshot\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::SetStateFromTypedSnapshot(TSerialize ser, int type, int flags)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::SetStateFromTypedSnapshot\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::PostSetStateFromSnapshot()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::PostSetStateFromSnapshot\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::GetStateSnapshotTxt(char *txtbuf,int szbuf, float time_back)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::GetStateSnapshotTxt\n"); std::abort();
}
void CryActionRegressionPhysicalEntity::SetStateFromSnapshotTxt(const char *txtbuf,int szbuf)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::SetStateFromSnapshotTxt\n"); std::abort();
}
unsigned int CryActionRegressionPhysicalEntity::GetStateChecksum()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::GetStateChecksum\n"); std::abort();
}
int CryActionRegressionPhysicalEntity::DoStep(float time_interval, int iCaller)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::DoStep\n"); std::abort();
}
void CryActionRegressionPhysicalEntity::StepBack(float time_interval)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::StepBack\n"); std::abort();
}
IPhysicalWorld *CryActionRegressionPhysicalEntity::GetWorld()
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::GetWorld\n"); std::abort();
}
void CryActionRegressionPhysicalEntity::GetMemoryStatistics(ICrySizer *pSizer)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionPhysicalEntity::GetMemoryStatistics\n"); std::abort();
}
void CryActionRegressionReport::AuthPlayer(int playerid, uint ip, const char* challenge, const char* responce)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionReport::AuthPlayer\n"); std::abort();
}
void CryActionRegressionReport::ReAuthPlayer(int playerid, const char* responce)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionReport::ReAuthPlayer\n"); std::abort();
}
void CryActionRegressionReport::SetReportParams(int p, int t)
{
    players = p; teams = t; order.emplace_back("params"); rows.clear();
}
void CryActionRegressionReport::SetServerValue(const char* key, const char* value)
{
    order.emplace_back(std::string("server:") + key); rows.push_back({-1,key,value});
}
void CryActionRegressionReport::SetPlayerValue(int row, const char* key, const char* value)
{
    order.emplace_back(std::string("player:") + std::to_string(row) + ":" + key); rows.push_back({row,key,value});
}
void CryActionRegressionReport::SetTeamValue(int, const char* key, const char*)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionReport::SetTeamValue\n"); std::abort();
}
void CryActionRegressionReport::Update()
{
    ++updates; order.emplace_back("update");
}
void CryActionRegressionReport::StartReporting(INetNub*,IServerReportListener*)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionReport::StartReporting\n"); std::abort();
}
void CryActionRegressionReport::StopReporting()
{
    ++stops; order.emplace_back("stop");
}
void CryActionRegressionReport::ProcessQuery(char* data, int len, struct sockaddr_in* addr)
{
    std::fprintf(stderr, "Unexpected collaborator call: CryActionRegressionReport::ProcessQuery\n"); std::abort();
}
bool CryActionRegressionReport::IsAvailable() const
{
    return true;
}
