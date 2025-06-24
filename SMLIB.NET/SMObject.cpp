#include "stdafx.h"
#include "smobject.h"

namespace PESMLIB
{
	Context::Context()
	{
		m_pIwContext = new IwContext();
	}

	Context::~Context()
	{
		if (m_pIwContext != NULL)
		{
			m_pIwContext->~IwContext();
			m_pIwContext = NULL;
		}
	}

	PersistObject::PersistObject()
	{
		m_sId = NULL;
		m_pXMLElem = NULL;
	}
	String __gc * PersistObject::GetId()
	{
		return m_sId;
	}
	void PersistObject::SetId(String* sId)
	{
		m_sId = sId;
	}
	String __gc * PersistObject::CreateId()
	{

		// If m_sId is not empty, then create one. Otherwise, return
		// return existing value

		if (m_sId != NULL)
			return m_sId;

		m_sId = Guid::NewGuid().ToString("B");
		return m_sId;
	}

	void PersistObject::IncrementRefCount()
	{

		int iRefCnt = 0;
		System::String __gc * sRefCnt;
		sRefCnt = m_pXMLElem->GetAttribute("iRefCnt");
		if (sRefCnt->Length > 0)
			iRefCnt = Convert::ToInt32(sRefCnt);
		iRefCnt++;
		m_pXMLElem->SetAttribute("iRefCnt", iRefCnt.ToString());
	}

	void PersistObject::DecrementRefCount()
	{

		int iRefCnt = 0;
		System::String __gc * sRefCnt;
		sRefCnt = m_pXMLElem->GetAttribute("iRefCnt");
		if (sRefCnt->Length > 0)
			iRefCnt = Convert::ToInt32(sRefCnt);
		iRefCnt--;
		if (0 <= iRefCnt)
		{
			if (m_pXMLElem->ParentNode != NULL)
				m_pXMLElem->ParentNode->RemoveChild (m_pXMLElem);
			m_pXMLElem = NULL;
		}
		else
			m_pXMLElem->SetAttribute("iRefCnt", iRefCnt.ToString());
	}
	SMObject::SMObject(void) : PersistObject()
	{

		m_pContext = NULL;
		m_pIwObj = NULL;
	}

	SMObject::~SMObject()
	{

		if (m_pIwObj)
		{
			//something needs to be fixed here, this try catch is just temporary
			try
			{
			//m_pIwObj->~IwObject();
			}
			catch (...)
			{
			}
			m_pIwObj = NULL;
		}
		m_pContext = NULL;
	}


	void SMObject::SetIwObjAttribute ()
	{

		try
		{
			// If created set an attribute on this surface which is the GUID of the
			// persistent instance.

			String __gc *sId = GetId ();

			if (m_pIwObj != NULL && sId != NULL)
			{
				IwTArray<long> arrLongEl;
				IwTArray<double> arrDoubleEl;
				IwTArray<char> arrCharEl;

				int lSize = sId->Length;
				if (lSize > 0)
				{
					for (int iGUID = 0; iGUID < lSize; iGUID++)
						arrCharEl.Add(Convert::ToByte(sId->Chars[iGUID]));

					IwGenericAttribute *pAttribute = new (GetIwContext()) IwGenericAttribute (
						AttributeID_IDSELF, IW_AB_COPY, arrLongEl, arrDoubleEl, arrCharEl);
					IwAttribute *pOldAttribute = 0;

					if (NULL != (pOldAttribute = ((IwAObject *) m_pIwObj)->FindAttribute (AttributeID_IDSELF)))
						((IwAObject *) m_pIwObj)->RemoveAttribute (pOldAttribute, TRUE);

					((IwAObject *) m_pIwObj)->AddAttribute (pAttribute);
				}
			}
		}
		catch (...)
		{
		}
	}

	String __gc * SMObject::GetIwObjAttribute()
	{
		String __gc *sId = String::Empty;
		IwAttribute *pAttribute = ((IwAObject *)m_pIwObj)->FindAttribute(AttributeID_IDSELF);
		if (pAttribute != NULL)
				sId = new String(pAttribute->GetCharacterElementsAddress());
		return sId;
	}


	IwObject* SMObject::ExtractIwObj ()
	{

		IwObject *pIwObj = m_pIwObj;
		m_pIwObj = NULL;
		return pIwObj;
	}

	const IwObject * SMObject::GetIwObj ()
	{
		return m_pIwObj;
	}

   System::Xml::XmlElement __gc *PESMLIB::PersistObject::get_XmlElement (void)
   {
      return m_pXMLElem;
   }

   void PESMLIB::PersistObject::set_XmlElement (System::Xml::XmlElement __gc *pElem)
   {
      m_pXMLElem = pElem;
      //TODO: update graphics or whatever else is needed
   }

   System::String __gc *PESMLIB::PersistObject::get_IdSelf (void)
   {
      return GetId ();
   }

   IwAttribute * SMObject::CreateStringAttribute(IwContext &context, PESMLIB::AttributeID idType, System::String __gc *pValue)
   {
	   IwTArray<long> arrLongEl;
	   IwTArray<double> arrDoubleEl;
	   IwTArray<char> arrCharEl;
	   IwGenericAttribute *pAttribute = NULL;

	   int lSize = pValue->Length;
	   if (lSize > 0)
	   {
		   for (int i = 0; i < lSize; i++)
			   arrCharEl.Add(Convert::ToByte(pValue->Chars[i]));

		   pAttribute = new (context) IwGenericAttribute (
			   idType, IW_AB_COPY, arrLongEl, arrDoubleEl, arrCharEl);
	   }
	   return pAttribute;
   }

}
