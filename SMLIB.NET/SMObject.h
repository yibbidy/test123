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
	__value public enum AttributeID 
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

	__value public enum AttributeBehavior
	{
		AttributeBehavior_ABCopy = IW_AB_COPY,
		AttributeBehavior_ABReference = IW_AB_REFERENCE,
		AttributeBehavior_ABStandaloneCopy = IW_AB_STANDALONE_COPY,
		AttributeBehavior_ABStandaloneReference = IW_AB_STANDALONE_REFERENCE
	};

	__gc public class Context  
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

	__gc __abstract public class PersistObject : public IPersistentObject
	{
	public:
		PersistObject(void);
		virtual ~PersistObject(void) { }
		virtual String __gc * CreateId();
		virtual void SetId(String __gc * sId);
		virtual String __gc * GetId();
		virtual bool HasId() { return (NULL != m_sId); }
		virtual void IncrementRefCount();
		virtual void DecrementRefCount();
		[BrowsableAttribute(false)]
		__property virtual System::Xml::XmlElement __gc *get_XmlElement (void);
		[BrowsableAttribute(false)]
		__property virtual void set_XmlElement (System::Xml::XmlElement __gc *pElem);
		[BrowsableAttribute(false)]
		__property virtual System::String __gc *get_IdSelf (void);

		//private:
		String __gc * m_sId;
		//protected private:
		XML::XmlElement __gc * m_pXMLElem;
	};

	__gc __abstract public class SMObject : public PersistObject
	{
	public:
		SMObject(void);
		virtual ~SMObject(void);
		Context __gc * GetContext () {return m_pContext;};
		virtual String __gc * GetIwObjAttribute();

	public private:
		virtual IwObject * ExtractIwObj ();
		virtual const IwObject * GetIwObj ();
		virtual void AttachIwObj (Context __gc *pContext, IwObject *pIwObj) = 0;

	protected private:
		Context __gc *m_pContext;
		IwObject *m_pIwObj;

		virtual void AddToDOM () = 0;
		virtual void GetFromDOM () = 0;
		virtual void SetIwObjAttribute ();
		static IwAttribute * CreateStringAttribute(IwContext& context, AttributeID idType, String __gc *pValue);

		virtual IwContext& GetIwContext() { return m_pContext->GetIwContext(); }
		virtual IwContext* GetIwContextPtr() { return m_pContext->GetIwContextPtr(); }
		virtual bool HasIwContext() { return NULL != m_pContext; }
	};
}