/******
gcdkeyc.c
GameSpy CDKey SDK Client Code
  
Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

******

 Please see the GameSpy CDKey SDK documentation for more 
 information

******/
#include "../md5.h"
#include "gcdkeyc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RAWSIZE 512

#ifdef __cplusplus
extern "C" {
#endif

	// method = 0 for normal auth response from game server
	// method = 1 for reauth response originating from keymaster
void gcd_compute_response(char *cdkey, char *challenge, char response[RESPONSE_SIZE], CDResponseMethod method)
{
	char rawout[RAWSIZE];
	unsigned int anyrandom;
	char randstr[9];
	int formatResult;
	size_t cdkeyLength;
	size_t challengeLength;

	cdkeyLength = strlen(cdkey);
	challengeLength = strlen(challenge);

	/* check to make sure we weren't passed a huge cd key/challenge */
	if (challengeLength >= RAWSIZE - 8 ||
		cdkeyLength >= (RAWSIZE - 8 - challengeLength) / 2)
	{
		formatResult = snprintf(response, RESPONSE_SIZE, "%s", "CD Key or challenge too long");
		if (formatResult < 0 || (size_t)formatResult >= RESPONSE_SIZE)
			response[0] = '\0';
		return;
	}

	/* make sure we are randomized */
	srand((unsigned int)time(NULL) ^ 0x33333333u);
	/* Since RAND_MAX is 16 bit on many systems, make sure we get a 32 bit number */
	anyrandom = (rand() << 16 | rand()); 
	formatResult = snprintf(randstr, sizeof(randstr), "%.8x",anyrandom);
	if (formatResult < 0 || (size_t)formatResult >= sizeof(randstr))
	{
		response[0] = '\0';
		return;
	}

	/* auth response   = MD5(cdkey + random mod 0xffff + challenge) */
	/* reauth response = MD5(challenge + random mode 0xffff + cdkey) */ 
	if (method == 0)
		formatResult = snprintf(rawout, RAWSIZE, "%s%d%s",cdkey, anyrandom % 0xFFFF , challenge );
	else
		formatResult = snprintf(rawout, RAWSIZE, "%s%d%s",challenge, anyrandom % 0xFFFF, cdkey);
	if (formatResult < 0 || (size_t)formatResult >= RAWSIZE)
	{
		response[0] = '\0';
		return;
	}

	/* do the cd key md5 */
	MD5Digest((unsigned char *)cdkey, (unsigned int)cdkeyLength, response);
	/* add the random value */
	formatResult = snprintf(&response[32], RESPONSE_SIZE - 32, "%s", randstr);
	if (formatResult < 0 || (size_t)formatResult >= RESPONSE_SIZE - 32)
	{
		response[0] = '\0';
		return;
	}
	/* do the response md5 */
	MD5Digest((unsigned char *)rawout, (unsigned int)strlen(rawout), &response[40]);
}


#ifdef __cplusplus
}
#endif

