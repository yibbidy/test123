#include "StdAfx.h"
#include "brepio.h"

char tempStr[32];

namespace PESMLIB
{

	BrepIO::~BrepIO(void)
	{
	}

	IwStatus BrepIO::WriteLongs(const ULONG * pLongs, ULONG lNumLongs) 
	{ 
        for (ULONG lLongs = 0; lLongs < lNumLongs; lLongs++)
        {
            sprintf_s(tempStr, sizeof(tempStr), "%u ", pLongs[lLongs]);
            m_strOut += tempStr;
        }
		
        m_strOut += "\n";

		return IW_SUCCESS;
	}

	IwStatus BrepIO::WriteDoubles(const double * pDoubles, ULONG lNumDoubles) 
	{ 
        for (ULONG lDoubles = 0; lDoubles < lNumDoubles; lDoubles++)
        {
            sprintf_s(tempStr, sizeof(tempStr), "%.15g ", pDoubles[lDoubles]);
            m_strOut += tempStr;
        }

        m_strOut += "\n";

		return IW_SUCCESS;
	}

	IwStatus BrepIO::WriteCharacters(const char * pChars, ULONG lNumChars) 
	{ 
        m_strOut.append(pChars, lNumChars);
        m_strOut += "\n";
		return IW_SUCCESS;
	}

	IwStatus BrepIO::WriteLong(long lLong) 
	{ 
        sprintf_s(tempStr, sizeof(tempStr), "%d\n", lLong);
        m_strOut += tempStr;
		return IW_SUCCESS;
	}

	IwStatus BrepIO::WriteDouble(double dDouble) 
	{ 
        sprintf_s(tempStr, sizeof(tempStr), "%.15g\n", dDouble);
        m_strOut += tempStr;
        return IW_SUCCESS;
	}

	IwStatus BrepIO::WriteBoolean(IwBoolean bBoolean) 
	{ 
        BrepIO::WriteLong(bBoolean);
		return IW_SUCCESS;
	}

	IwStatus BrepIO::WriteShort(short sInt) 
	{ 
        BrepIO::WriteLong(sInt);
		return IW_SUCCESS;
	}

	IwStatus BrepIO::ReadLongs(ULONG * pLongs, ULONG lNumLongs) 
	{
		NER(m_pStrStreamIn);
		for (ULONG lLongs = 0; lLongs < lNumLongs; lLongs++)
			*m_pStrStreamIn >> pLongs[lLongs];
		//    m_pStrStreamIn->read((char*)pLongs,sizeof(ULONG)*lNumLongs);
		return IW_SUCCESS;
	}

	IwStatus BrepIO::ReadDoubles(double * pDoubles, ULONG lNumDoubles) 
	{
		NER(m_pStrStreamIn);
		for (ULONG lDoubles = 0; lDoubles < lNumDoubles; lDoubles++)
			*m_pStrStreamIn >> pDoubles[lDoubles];
		//    m_pStrStreamIn->read((char*)pDoubles,sizeof(double)*lNumDoubles);
		return IW_SUCCESS;
	}

	IwStatus BrepIO::ReadCharacters(char * pChars, ULONG lNumChars) 
	{
		NER(m_pStrStreamIn);

		// If the first character is a newline discard it since it is presumably
		// the ending newline from the previous item. 

		for (ULONG j=0; j<lNumChars; j++) 
		{
			char cChar;
			m_pStrStreamIn->get(cChar);
			if (j == 0 && cChar == '\n' && m_pStrStreamIn->tellg () != (streampos) NULL)
				m_pStrStreamIn->get(cChar);
			pChars[j] = cChar;
		}
		//   m_pStrStreamIn->read((char*)pChars,sizeof(char)*lNumChars);
		return IW_SUCCESS;
	}

	IwStatus BrepIO::ReadLong(long & rLong)
	{
		NER(m_pStrStreamIn);
		*m_pStrStreamIn >> rLong;
		//    m_pStrStreamIn->read((char*)&rLong,sizeof(long));
		return IW_SUCCESS;
	}

	IwStatus BrepIO::ReadLong(ULONG & rLong)
	{
		NER(m_pStrStreamIn);
		*m_pStrStreamIn >> rLong;
		//    m_pStrStreamIn->read((char*)&rLong,sizeof(long));
		return IW_SUCCESS;
	}

	IwStatus BrepIO::ReadBoolean(IwBoolean & bBoolean)
	{
		NER(m_pStrStreamIn);
		*m_pStrStreamIn >> bBoolean;
		//    m_pStrStreamIn->read((char*)&bBoolean,sizeof(IwBoolean));
		return IW_SUCCESS;
	}

	IwStatus BrepIO::ReadDouble(double & rDouble) 
	{
		NER(m_pStrStreamIn);
		*m_pStrStreamIn >> rDouble;
		//    m_pStrStreamIn->read((char*)&rDouble,sizeof(double));
		return IW_SUCCESS;
	}

	IwStatus BrepIO::ReadShort(short & rInt) 
	{
		NER(m_pStrStreamIn);
		*m_pStrStreamIn >> rInt;
		//    m_pStrStreamIn->read((char*)&rInt,sizeof(short));
		return IW_SUCCESS;
	}

	const char * BrepIO::GetStringBuf ()
	{
        if (initializedAsOut)
            return m_strOut.c_str();
		else if (m_pStrStreamIn != NULL)
			return m_pStrStreamIn->str ();
		else
			return NULL;
	}

    int BrepIO::GetStringBufLength()
    {
        if (initializedAsOut)
            return (int) m_strOut.length();
        return -1;
    }

}