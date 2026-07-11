// DXPSShaderCompiler.cpp : Defines the entry point for the console application.
//

#include "Core/StdTypes.hpp"
#include "Core/Server/DXPSServer.hpp"

int main(int argc,char* argv[])
{
	CDXPSServer(4455);
	return 0;
}

