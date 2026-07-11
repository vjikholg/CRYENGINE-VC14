#ifndef __DXPSHELPER__
#define __DXPSHELPER__

#include <vector>
#include <string>


typedef std::vector<std::string>						tdEntryVec;
typedef	std::pair<std::string,std::string>	tdToken;
typedef std::vector<tdToken >								tdTokenList;

class CDXPSHelper
{
public:
	static	void		Tokenize(tdEntryVec& rRet,const std::string& Tokens,const std::string& Separator);
	static	tdToken	SplitToken(const std::string& rToken,const std::string& rSeparator);
	static	void		Splitizer(tdTokenList& rTokenList,const tdEntryVec& rFilter,const std::string& rSeparator);
	static	void		Trim(std::string& rStr,char C);
	static	void		Remove(std::string& rTokenDst,const std::string& rTokenSrc,const char C);


	static	void		ToFile(const std::string& rFileName,const std::vector<uint8_t>&	rOut);
	static	bool		FromFile(const std::string& rFileName,std::vector<uint8_t>&	rIn);


	static	void		EndianSwizzleU32(uint32_t& S)
									{
										uint8_t* pT=reinterpret_cast<uint8_t*>(&S);
										uint8_t T;
										T=pT[0];pT[0]=pT[3];pT[3]=T;
										T=pT[1];pT[1]=pT[2];pT[2]=T;
									}
	static	void		EndianSwizzleU16(uint16_t& S)
									{
										uint8_t* pT=reinterpret_cast<uint8_t*>(&S);
										uint8_t T;
										T=pT[0];pT[0]=pT[1];pT[1]=T;
									}




	static	void		Log(const std::string& rLog);
};

#define DXPS_LOG(X) CDXPSHelper::Log(X)
#endif

