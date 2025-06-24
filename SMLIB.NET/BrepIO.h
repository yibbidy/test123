#pragma once
#include "iwdatabaseio.h"
#include <strstream>
#include <string>

namespace PESMLIB
{
	__nogc class BrepIO :   public IwDatabaseIO
	{
	private:
		std::string  m_strOut;              // attempting to fix leak in MS << operator
		istrstream * m_pStrStreamIn;
        bool         initializedAsOut;

	public:
        BrepIO() : IwDatabaseIO(IW_BINARY), m_pStrStreamIn(NULL), initializedAsOut(false) {};
        BrepIO(istrstream *pStreamIn) : IwDatabaseIO(IW_BINARY), m_pStrStreamIn(pStreamIn), initializedAsOut(false) {};
        BrepIO(ostrstream *pStreamOut) : IwDatabaseIO(IW_BINARY), m_pStrStreamIn(NULL), initializedAsOut(true) {};
		virtual ~BrepIO(void);

		const char * GetStringBuf ();
        int GetStringBufLength();

		istrstream * GetInStreamPtr() { return m_pStrStreamIn; }

		virtual IwStatus BeginWriting() 
		{
			return IW_SUCCESS;
		};

		virtual IwStatus EndWriting() 
		{
			return IW_SUCCESS;
		};

		virtual IwStatus BeginReading() {return IW_SUCCESS;};
		virtual IwStatus EndReading() {return IW_SUCCESS;};

		virtual IwStatus WriteLongs(const ULONG * pLongs, ULONG lNumLongs);
		virtual IwStatus WriteDoubles(const double * pDoubles, ULONG lNumDoubles);
		virtual IwStatus WriteCharacters(const char * pChars, ULONG lNumChars);
		virtual IwStatus WriteLong(long pLong);
		virtual IwStatus WriteBoolean(IwBoolean bBoolean);
		virtual IwStatus WriteDouble(double dDouble);
		virtual IwStatus WriteShort(short pInt);

		virtual IwStatus ReadLongs(ULONG * pLongs, ULONG lNumLongs);
		virtual IwStatus ReadDoubles(double * pDoubles, ULONG lNumDoubles);
		virtual IwStatus ReadCharacters(char * pChars, ULONG lNumChars);
		virtual IwStatus ReadLong(long & rLong);
		virtual IwStatus ReadLong(ULONG & rLong);
		virtual IwStatus ReadBoolean(IwBoolean & rBoolean);
		virtual IwStatus ReadDouble(double & rDouble);
		virtual IwStatus ReadShort(short & rInt);
	};

}