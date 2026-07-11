#ifndef __DXPSERROR__
#define __DXPSERROR__

#include <string>

#define DXPS_ERROR(X) throw std::string(X)

#define DXPS_SECURE_START try{
#define DXPS_SECURE_END }catch(const std::string Value){printf(Value.c_str());}

#endif
