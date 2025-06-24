#pragma once

#include <vector>
#include <list>
#include <algorithm>
#include "iwtslib_all.h"
#include "IwTess.h"

using namespace System;
namespace XML = System::Xml;
using namespace System::ComponentModel;
using namespace VEDM::Documents;
using namespace SHELL;

namespace PESMLIB
{
       public enum class AttributeID
	{
		AttributeID_IDSELF = 10001,
		AttributeID_HKEY = 10002,
		AttributeID_BREPFACECOUNTER = 10003,
		AttributeID_BREPREGIONCOUNTER = 10004,
		AttributeID_BREPEDGECOUNTER = 10005,
		AttributeID_IDOBJ = 10006,
		AttributeID_USER = 10007,
		//Molded Form Attribute
		AttributeID_PROPERTY = 10008,
		AttributeID_CORROSION = 10009,
		AttributeID_PLATEMASK = 10010
	};

       public enum class AttributeBehavior
	{
		AttributeBehavior_ABCopy = IW_AB_COPY,
		AttributeBehavior_ABReference = IW_AB_REFERENCE,
		AttributeBehavior_ABStandaloneCopy = IW_AB_STANDALONE_COPY,
		AttributeBehavior_ABStandaloneReference = IW_AB_STANDALONE_REFERENCE
	};

	public ref class Context  
	{
	public:
		Context();
		virtual ~Context();

	public private:
		IwContext& GetIwContext() { return *m_pIwContext; }
		IwContext* GetIwContextPtr() { return m_pIwContext; }

	protected private:
		IwContext *m_pIwContext;
	};

	public ref class abstract PersistObject : public IPersistentObject
	{
	public:
		PersistObject(void);
		virtual ~PersistObject(void) { }
		virtual String^  CreateId();
		virtual void SetId(String^  sId);
		virtual String^  GetId();
		virtual bool HasId() { return (NULL != m_sId); }
		virtual void IncrementRefCount();
		virtual void DecrementRefCount();
		[BrowsableAttribute(false)]
		__property virtual System::Xml::XmlElement^ get_XmlElement (void);
		[BrowsableAttribute(false)]
		__property virtual void set_XmlElement (System::Xml::XmlElement^ pElem);
		[BrowsableAttribute(false)]
		__property virtual System::String^ get_IdSelf (void);

		//private:
		String^  m_sId;
		//protected private:
		XML::XmlElement^  m_pXMLElem;
	};

	public ref class abstract SMObject : public PersistObject
	{
	public:
		SMObject(void);
		virtual ~SMObject(void);
		Context^  GetContext () {return m_pContext;};
		virtual String^  GetIwObjAttribute();

	public private:
		virtual IwObject * ExtractIwObj ();
		virtual const IwObject * GetIwObj ();
		virtual void AttachIwObj (Context^ pContext, IwObject *pIwObj) = 0;

	protected private:
		Context^ m_pContext;
		IwObject *m_pIwObj;

		virtual void AddToDOM () = 0;
		virtual void GetFromDOM () = 0;
		virtual void SetIwObjAttribute ();
		static IwAttribute * CreateStringAttribute(IwContext& context, AttributeID idType, String^ pValue);

		virtual IwContext& GetIwContext() { return m_pContext->GetIwContext(); }
		virtual IwContext* GetIwContextPtr() { return m_pContext->GetIwContextPtr(); }
		virtual bool HasIwContext() { return NULL != m_pContext; }
	};
}