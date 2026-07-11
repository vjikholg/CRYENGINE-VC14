#ifndef __CRYDXPSOGL__
#define __CRYDXPSOGL__

#ifdef PSGL
	#include <PSGL/psgl.h>
	#include <Cg/cg.h>
		#include <sys/process.h>
		#include <sys/spu_initialize.h>
		#include <sys/paths.h>

		#include <PSGL/psgl.h>
		#include <PSGL/psglu.h>

		#include <cell/dbgfont.h>

		#include <stdarg.h>
		#include <cell/dbgfont.h>
		#include <cell/sysmodule.h>
#else
	#ifdef CRY_USE_OGL
		#define	CRY_GL_ERROR
	#endif
	#include <Windows.h>
	#include "glew.h"
	#include "wglew.h"
	#include <gl/GL.h>
	#include <cg/cgGL.h>
#endif


#define GL_ALPHA4_EXT                     0x803B
#define GL_ALPHA8_EXT                     0x803C
#define GL_ALPHA12_EXT                    0x803D
#define GL_ALPHA16_EXT                    0x803E
#define GL_LUMINANCE4_EXT                 0x803F
#define GL_LUMINANCE8_EXT                 0x8040
#define GL_LUMINANCE12_EXT                0x8041
#define GL_LUMINANCE16_EXT                0x8042
#define GL_LUMINANCE4_ALPHA4_EXT          0x8043
#define GL_LUMINANCE6_ALPHA2_EXT          0x8044
#define GL_LUMINANCE8_ALPHA8_EXT          0x8045
#define GL_LUMINANCE12_ALPHA4_EXT         0x8046
#define GL_LUMINANCE12_ALPHA12_EXT        0x8047
#define GL_LUMINANCE16_ALPHA16_EXT        0x8048
#define GL_INTENSITY_EXT                  0x8049
#define GL_INTENSITY4_EXT                 0x804A
#define GL_INTENSITY8_EXT                 0x804B
#define GL_INTENSITY12_EXT                0x804C
#define GL_INTENSITY16_EXT                0x804D
#define GL_RGB2_EXT                       0x804E
#define GL_RGB4_EXT                       0x804F
#define GL_RGB5_EXT                       0x8050
#define GL_RGB8_EXT                       0x8051
#define GL_RGB10_EXT                      0x8052
#define GL_RGB12_EXT                      0x8053
#define GL_RGB16_EXT                      0x8054
#define GL_RGBA2_EXT                      0x8055
#define GL_RGBA4_EXT                      0x8056
#define GL_RGB5_A1_EXT                    0x8057
#define GL_RGBA8_EXT                      0x8058
#define GL_RGB10_A2_EXT                   0x8059
#define GL_RGBA12_EXT                     0x805A
#define GL_RGBA16_EXT                     0x805B
#define GL_TEXTURE_RED_SIZE_EXT           0x805C
#define GL_TEXTURE_GREEN_SIZE_EXT         0x805D
#define GL_TEXTURE_BLUE_SIZE_EXT          0x805E
#define GL_TEXTURE_ALPHA_SIZE_EXT         0x805F
#define GL_TEXTURE_LUMINANCE_SIZE_EXT     0x8060
#define GL_TEXTURE_INTENSITY_SIZE_EXT     0x8061
#define GL_REPLACE_EXT                    0x8062
#define GL_PROXY_TEXTURE_1D_EXT           0x8063
#define GL_PROXY_TEXTURE_2D_EXT           0x8064
#define GL_TEXTURE_TOO_LARGE_EXT          0x8065
#define GL_RGBA_FLOAT32_ATI               0x8814
#define GL_RGB_FLOAT32_ATI                0x8815
#define GL_ALPHA_FLOAT32_ATI              0x8816
#define GL_INTENSITY_FLOAT32_ATI          0x8817
#define GL_LUMINANCE_FLOAT32_ATI          0x8818
#define GL_LUMINANCE_ALPHA_FLOAT32_ATI    0x8819
#define GL_RGBA_FLOAT16_ATI               0x881A
#define GL_RGB_FLOAT16_ATI                0x881B
#define GL_ALPHA_FLOAT16_ATI              0x881C
#define GL_INTENSITY_FLOAT16_ATI          0x881D
#define GL_LUMINANCE_FLOAT16_ATI          0x881E
#define GL_LUMINANCE_ALPHA_FLOAT16_ATI    0x881F

#define GL_R11F_G11F_B10F_EXT             0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV_EXT 0x8C3B
#define GL_RGBA_SIGNED_COMPONENTS_EXT     0x8C3C

#define GL_DEPTH_COMPONENT32F_NV          0x8DAB
#define GL_DEPTH32F_STENCIL8_NV           0x8DAC
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV_NV 0x8DAD
#define GL_DEPTH_BUFFER_FLOAT_MODE_NV     0x8DAF





static uint32 DXGI_FORMAT2OGL[]	=
{
	0										,
	GL_RGBA_FLOAT32_ATI			,
	GL_RGBA_FLOAT32_ATI				,
	GL_RGBA_FLOAT32_ATI					,
	GL_RGBA_FLOAT32_ATI					,
	GL_RGB_FLOAT32_ATI				,
	GL_RGB_FLOAT32_ATI						,
	GL_RGB_FLOAT32_ATI						,
	GL_RGB_FLOAT32_ATI						,
	GL_RGBA_FLOAT16_ATI			,
	GL_RGBA_FLOAT16_ATI				,
	GL_RGBA_FLOAT16_ATI				,
	GL_RGBA_FLOAT16_ATI					,
	GL_RGBA_FLOAT16_ATI				,
	GL_RGBA_FLOAT16_ATI					,
	GL_RGBA_FLOAT32_ATI						,
	GL_RGBA_FLOAT32_ATI							,
	GL_RGBA_FLOAT32_ATI								,
	GL_RGBA_FLOAT32_ATI								,
	GL_RGBA_FLOAT32_ATI					,
	GL_RGBA_FLOAT32_ATI			,
	GL_RGBA_FLOAT32_ATI	,
	GL_RGBA_FLOAT32_ATI		,
	GL_ALPHA_FLOAT16_ATI			,
	GL_ALPHA_FLOAT16_ATI					,
	GL_ALPHA_FLOAT16_ATI					,
	GL_R11F_G11F_B10F_EXT						,
	GL_RGBA8_EXT					,
	GL_RGBA8_EXT						,
	GL_RGBA8_EXT				,
	GL_RGBA8_EXT							,
	GL_RGBA8_EXT						,
	GL_RGBA8_EXT							,
	GL_ALPHA_FLOAT16_ATI						,
	GL_ALPHA_FLOAT16_ATI							,
	GL_ALPHA_FLOAT16_ATI							,
	GL_ALPHA_FLOAT16_ATI								,
	GL_ALPHA_FLOAT16_ATI							,
	GL_ALPHA_FLOAT16_ATI								,
	GL_ALPHA_FLOAT32_ATI							,
	GL_ALPHA_FLOAT32_ATI									,
	GL_ALPHA_FLOAT32_ATI									,
	GL_ALPHA_FLOAT32_ATI									,
	GL_ALPHA_FLOAT32_ATI									,
	0						,
	0					,
	0			,
	0			,
	GL_RGBA8_EXT							,
	GL_RGBA8_EXT								,
	GL_RGBA8_EXT									,
	GL_RGBA8_EXT								,
	GL_RGBA8_EXT									,
	GL_ALPHA_FLOAT16_ATI							,
	GL_ALPHA_FLOAT16_ATI									,
	GL_ALPHA_FLOAT16_ATI									,
	GL_ALPHA_FLOAT16_ATI									,
	GL_ALPHA_FLOAT16_ATI									,
	GL_ALPHA_FLOAT16_ATI									,
	GL_ALPHA_FLOAT16_ATI									,
	GL_INTENSITY8_EXT								,
	GL_INTENSITY8_EXT									,
	GL_INTENSITY8_EXT										,
	GL_INTENSITY8_EXT									,
	GL_INTENSITY8_EXT										,
	GL_INTENSITY8_EXT									,
	0									,
	0				,
	0						,
	0						,
	0							,
	0									,
	0						,
	0							,
	0									,
	0						,
	0							,
	0									,
	0						,
	0							,
	0									,
	0									,
	0							,
	0									,
	0									,
	GL_RGB5_EXT							,
	GL_RGB5_A1_EXT						,
	GL_RGBA8_EXT						,
	GL_RGBA8_EXT						,
	0								};


#ifdef CRY_GL_ERROR
	#define CRY_GL_CHECKERROR \
	{GLenum Err=glGetError();\
	if(Err!=GL_NO_ERROR)\
	CCryLayer0OGL::Instance().StreamError(__FILE__,__FUNCTION__,Err);}

//#define	GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT 0x8CD7

#define CHECK_FRAMEBUFFER_STATUS						\
	{									\
		GLenum status;							\
		status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);	\
		switch (status) {						\
			case GL_FRAMEBUFFER_COMPLETE_EXT:			\
				break;						\
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:\
				OutputDebugString(" missing a required image/buffer attachment!\n");\
				break;\
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:\
				OutputDebugString(" has no images/buffers attached!\n");\
				break;\
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:\
				OutputDebugString(" has mismatched image/buffer dimensions!\n");\
				break;\
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:\
				OutputDebugString("'s colorbuffer attachments have different types!\n");\
				break;\
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:\
				OutputDebugString(" trying to draw to non-attached color buffer!\n");\
				break;\
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:\
				OutputDebugString(" trying to read from a non-attached color buffer!\n");\
				break;\
			case GL_FRAMEBUFFER_UNSUPPORTED_EXT:\
				OutputDebugString(" format is not supported by current graphics card/driver!\n");\
				break;\
			default:						\
				OutputDebugString("unknown FBO error\n");;					\
		}								\
	}
#else
	#define CRY_GL_CHECKERROR 
	#define CHECK_FRAMEBUFFER_STATUS
#endif


class CCryLayer0OGL
{
	CGcontext									m_CGContext;
	CGprofile									m_VertexProfile;
	CGprofile									m_FragmentProfile;


	CCryLayer0OGL();
	~CCryLayer0OGL();
public:
	static CCryLayer0OGL& Instance();

#ifdef CRY_GL_ERROR
	void StreamError(const char* pFileName,const char* pFunction,GLenum	Err);
#endif

	CGcontext& ShaderContext()
	{
		return m_CGContext;
	}
	CGprofile& FragmentProfile()
	{
		return m_FragmentProfile;
	}
	CGprofile& VertexProfile()
	{
		return m_VertexProfile;
	}
};
typedef CCryLayer0OGL				tdLayer0;
#endif

