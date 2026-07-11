#include "StdTypes.hpp"
#include "Error.hpp"
#include "DXPSHelper.hpp"
#include <algorithm>
#ifdef _MSC_VER
#include <io.h>
#include <windows.h>
#endif

void CDXPSHelper::Log(const std::string& rLog)
{
	const std::string Output=rLog+"\n";
#ifdef _DEBUG
	OutputDebugString(Output.c_str());
#endif
}

void CDXPSHelper::Tokenize(tdEntryVec& rRet,const std::string& Tokens,const std::string& Separator)
{
		rRet.clear();
		std::string::size_type Pt;
		std::string::size_type Start	= 0;
		std::string::size_type SSize	=	Separator.size();

		while((Pt = Tokens.find(Separator,Start)) != std::string::npos)
		{
			std::string  SubStr	=	Tokens.substr(Start,Pt-Start);
//			if(!SubStr.empty())
				rRet.push_back(SubStr);
			Start = Pt + SSize;
		}

		rRet.push_back(Tokens.substr(Start));
}

tdToken	CDXPSHelper::SplitToken(const std::string& rToken,const std::string& rSeparator)
{
#undef min
	using namespace std;
	string Token;
//	remove_copy_if(rToken.begin(), rToken.end(), back_inserter(Token), bind2nd(equal_to<char>(),' ') ) ;
	Remove(Token,rToken,' ');

	string::size_type Pt=Token.find(rSeparator);
	return tdToken(Token.substr(0,Pt),Token.substr(std::min(Pt+1,Token.size())));
}

void CDXPSHelper::Splitizer(tdTokenList& rTokenList,const tdEntryVec& rFilter,const std::string& rSeparator)
{
	rTokenList.clear();
	for(size_t a=0,Size=rFilter.size();a<Size;a++)
		rTokenList.push_back(SplitToken(rFilter[a],rSeparator));
}

void CDXPSHelper::Trim(std::string& rStr,char C)
{
	std::string::size_type Pt1 = rStr.find_first_not_of(C);
	if(Pt1==std::string::npos)
		return;

	std::string::size_type Pt2 = rStr.find_last_not_of(C) + 1;

	Pt2		=	Pt2-Pt1;
	rStr	=	rStr.substr(Pt1,Pt2);
}

void CDXPSHelper::Remove(std::string& rTokenDst,const std::string& rTokenSrc,const char C)
{
	using namespace std;
	remove_copy_if(rTokenSrc.begin(), rTokenSrc.end(), back_inserter(rTokenDst), bind2nd(equal_to<char>(),C) ) ;
}

void CDXPSHelper::ToFile(const std::string& rFileName,const std::vector<uint8_t>&	rOut)
{
	FILE* pFile	=	fopen(rFileName.c_str(),"wb");
	if(!pFile)
	{
		DXPS_ERROR(std::string("Could not create: ")+rFileName);
		return;
	}
	fwrite(&rOut[0],1,rOut.size(),pFile);
	fclose(pFile);
}

bool CDXPSHelper::FromFile(const std::string& rFileName,std::vector<uint8_t>&	rIn)
{
	FILE* pFile	=	fopen(rFileName.c_str(),"rb");
	if(!pFile)
	{
		DXPS_ERROR(std::string("Could not read: ")+rFileName);
		return false;
	}
	uint32_t FileLen	=	_filelength(_fileno(pFile));
	if(FileLen<0)
	{
		DXPS_ERROR(std::string("Error getting file-size of ")+rFileName);
		return false;
	}
	rIn.resize(FileLen);
	if(FileLen>0)
		fread(&rIn[0],1,FileLen,pFile);
	fclose(pFile);
	return true;
}
