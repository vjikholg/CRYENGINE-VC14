/*=============================================================================
  Parser.h : Script parser declarations.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honich Andrey

=============================================================================*/

#ifndef PARSER_H
#define PARSER_H

struct STokenDesc
{
  int  id;
  const char  *token;
};
int shGetObject(char **buf, STokenDesc *tokens, char **name, char **data);


extern const char *kWhiteSpace;
extern FXMacro sStaticMacros;
extern byte sSkipChars[256];

void fxParserInit(void);

void SkipCharacters(const char*& buf, const char* toSkip);
void SkipCharacters(char*& buf, const char* toSkip);
void RemoveCR(char *buf);
void SkipComments(char **buf, bool bSkipWhiteSpace);

bool fxIsFirstPass(const char* buf);
void fxRegisterEnv(const char *szStr);

int shFill(const char*& buf, char* dst, int nSize=-1);
int shFill(char*& buf, char* dst, int nSize=-1);
int fxFill(const char*& buf, char* dst, int nSize=-1);
int fxFill(char*& buf, char* dst, int nSize=-1);
const char* fxFillPr(const char*& buf, char* dst);
char* fxFillPr(char*& buf, char* dst);
const char* fxFillPrC(const char*& buf, char* dst);
char* fxFillPrC(char*& buf, char* dst);
const char* fxFillParam(const char*& buf, char* dst);
char* fxFillParam(char*& buf, char* dst);
int fxFillCR(const char*& buf, char* dst);
int fxFillCR(char*& buf, char* dst);

bool shGetBool(char *buf);
float shGetFloat(const char *buf);
void shGetFloat(const char *buf, float *v1, float *v2);
int shGetInt(const char *buf);
int shGetHex(const char *buf);
uint64 shGetHex64(const char *buf);
void shGetVector(char *buf, Vec3& v);
void shGetVector(char *buf, float v[3]);
void shGetVector4(char *buf, vec4_t& v);
void shGetColor(const char *buf, ColorF& v);
void shGetColor(char *buf, float v[4]);
int shGetVar (char **buf, char **vr, char **val);

#endif

