// Password Reminder Module 
// Interface
///////////////////////////////////////////////////////////////////////////////

#ifndef __PASSWORD_REMINDER_H__
#define __PASSWORD_REMINDER_H__


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Includes

#include <string>

// not used in this version
//#include "Singleton.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Globals/Types
static const char *EncAspAddress = 
	"http://www.gamespyid.com/password/Enc.aspx?";
static const char *SendAspAddress = 
	"http://www.gamespyid.com/password/Send.aspx?";

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class PasswordReminder
{
	std::string	m_EmailAddr,
			m_EncAddress,
			m_SendAddress,
			m_Cookie,
			m_Key,
			m_Errorstring,
			m_Resultstring,
			m_ProfileId;
	
	int m_EncryptMethod,
		m_EncodeMethod;
		
	bool m_Hardwired;
	bool EncryptEncode();
	bool UrlEscape(std::string &urlPart);
	void SetError(const char *errorStr)
	{
		m_Errorstring = errorStr;
	}
	
public:
	
	PasswordReminder();
	virtual ~PasswordReminder();
	
	bool Initialize(bool hardwire, const std::string &key,
					int encryptingMethod, int encodingMethod);
	
	bool RequestMethod();
	
	bool SendReminderUsingEmail(const std::string &msgTemplate, 
		const std::string &emailAddr);
	
	bool SendReminderUsingId(const std::string &msgTemplate, int profileId);
	
	const char * GetResults()
	{
		return m_Resultstring.c_str();
	}

	const char * GetLastError()
	{
		return m_Errorstring.c_str();
	}
};


#endif
