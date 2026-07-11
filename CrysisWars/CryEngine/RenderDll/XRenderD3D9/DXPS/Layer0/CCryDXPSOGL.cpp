#include "RenderPCH.h"
#ifdef CRY_USE_OGL
#include "CCryDXPSOGL.hpp"
//#include <assert.h>

CCryLayer0OGL::CCryLayer0OGL()
{

}
CCryLayer0OGL::~CCryLayer0OGL()
{

}

CCryLayer0OGL& CCryLayer0OGL::Instance()
{
	static CCryLayer0OGL g_CryLayer0;
	return g_CryLayer0;
}
#endif

#ifdef CRY_GL_ERROR
void CCryLayer0OGL::StreamError(const char* pFileName,const char* pFunction,GLenum	Err)
{
	if(Err==GL_NO_ERROR)
		return OutputDebugString("No error has been recorded.\n The value of this symbolic constant is guaranteed to be 0.\n");

	OutputDebugString("An error occured");
//	OutputDebugString(" in File:");
//	OutputDebugString(pFileName);
	OutputDebugString(" in Function:");
	OutputDebugString(pFunction);
	OutputDebugString("\n");

	if(Err==GL_INVALID_ENUM)
		OutputDebugString("An unacceptable value is specified for an enumerated argument.\n\
The offending command is ignored, and has no other side effect than to set the error flag.\n");
	else
		if(Err==GL_INVALID_VALUE)
			OutputDebugString("A numeric argument is out of range. The offending command is ignored,\n\
and has no other side effect than to set the error flag.\n");
	else
	if(Err==GL_INVALID_OPERATION)
		OutputDebugString("The specified operation is not allowed in the current state.\n\
The offending command is ignored, and has no other side effect than to set the error flag.\n");
	else
	if(Err==GL_STACK_OVERFLOW)
		OutputDebugString("This command would cause a stack overflow.\n\
The offending command is ignored, and has no other side effect than to set the error flag.\n");
	else
	if(Err==GL_STACK_UNDERFLOW)
		OutputDebugString("This command would cause a stack underflow.\n\
The offending command is ignored, and has no other side effect than to set the error flag.\n");
	else
	if(Err==GL_OUT_OF_MEMORY)
		OutputDebugString("There is not enough memory left to execute the command.\n\
The state of the GL is undefined, except for the state of the error flags, after this error is recorded.\n");
	else
	if(Err==GL_TABLE_TOO_LARGE)
		OutputDebugString("The specified table exceeds the implementation's maximum supported table size.\n\
The offending command is ignored, and has no other side effect than to set the error flag.\n");
	else
	if(Err==GL_INVALID_FRAMEBUFFER_OPERATION_EXT)
		OutputDebugString("Invalid Framebuffer operations/FBO\n");
	assert(Err==GL_NO_ERROR);
}
#endif
