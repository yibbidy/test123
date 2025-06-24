#include "StdAfx.h"
#include "smextent1d.h"
#using <mscorlib.dll>
using namespace System;
using namespace System::Collections;

namespace PESMLIB
{
	Extent1d::Extent1d(void): PersistObject()
	{
      m_pIwExtent1d = new IwExtent1d ();
	}

	Extent1d::Extent1d(double dMin, double dMax): PersistObject()
	{
		m_pIwExtent1d = new IwExtent1d (dMin, dMax);
	}

	Extent1d::Extent1d(XML::XmlElement* pElem) : PersistObject()
	{
		m_pIwExtent1d = new IwExtent1d ();
		m_pXMLElem = pElem;

		// Now populate the Extent3d via the DOM if there is anything in the DOM.
		// Otherwise it is just the node where we are supposed to store the 
		// state later.

		if (m_pXMLElem != NULL)
		{
			GetFromDOM ();
			CreateId ();
		}
	}

	Extent1d::Extent1d(Extent1d &extOther)
	{
		m_pIwExtent1d = new IwExtent1d(*(extOther.GetIwObj()));
	}

	Extent1d::~Extent1d(void)
	{
		if (m_pIwExtent1d)
		{
			delete m_pIwExtent1d;
			m_pIwExtent1d = NULL;
		}
	}

	void Extent1d::SetCanonical (double dMin, double dMax)
	{
		if (!m_pIwExtent1d)
			m_pIwExtent1d = new IwExtent1d (dMin, dMax);
		else
			((IwExtent1d *) m_pIwExtent1d)->SetMinMax (dMin, dMax);

		if (m_pIwExtent1d)
			AddToDOM ();
	}

   void Extent1d::Init ()
   {
      if (m_pIwExtent1d)
         m_pIwExtent1d->Init ();
   }

   void Extent1d::Union (Extent1d& extOther, Extent1d& extResult)
   {
      if (m_pIwExtent1d)
      {
         const IwExtent1d *pIwExtentOther = extOther.GetIwObj ();
         m_pIwExtent1d->Union (*pIwExtentOther, *m_pIwExtent1d);
      }
   }

	IwExtent1d* Extent1d::ExtractObj ()
	{
		IwExtent1d *pIwObj = m_pIwExtent1d;
		m_pIwExtent1d = NULL;

		return pIwObj;
	}

	const IwExtent1d* Extent1d::GetIwObj ()
	{
		return m_pIwExtent1d;
	}

	void Extent1d::AddToDOM ()
	{
		if (m_pIwExtent1d != NULL && m_pXMLElem != NULL)
		{
			// Add the persistent ID.
			m_pXMLElem->SetAttribute("idSelf", GetId());
		}
	}

	void Extent1d::GetFromDOM ()
	{
		if (m_pXMLElem != NULL && m_pIwExtent1d != NULL)
		{
			// Get the persistent GUID.
			String* sId = m_pXMLElem->GetAttribute("idSelf");
			if (sId->Length > 0)
				SetId (sId);
		}
	}

   String __gc * Extent1d::ToString ()
   {
      try
      {
         return String::Format ("{0},{1}", 
            this->Min.ToString (), this->Max.ToString ());
      }
	  catch (Exception *)
      {
         return NULL;
      }
   }

   Extent1d __gc * Extent1d::Parse (String __gc *sVector)
   {
      try
      {
         if (NULL != sVector)
         {
            System::Char cTok[] = {','};
            System::String __gc *sSplit[] = sVector->Split (cTok);
            return new Extent1d (
               System::Convert::ToDouble (sSplit[0]), 
               System::Convert::ToDouble (sSplit[1]));
         }
         else
            return NULL;
      }
	  catch (Exception *)
      {
         return NULL;
      }
   }

   double Extent1d::get_Min ()
   {
      try
      {
         if (m_pIwExtent1d != NULL)
         {
            return m_pIwExtent1d->GetMin ();
         }
      }
      catch (System::Exception __gc *e)
      {
         Console::WriteLine (e->get_Message ());
      }
      return NULL;
   }

   double Extent1d::get_Max ()
   {
      try
      {
         if (m_pIwExtent1d != NULL)
         {
            return m_pIwExtent1d->GetMax ();
         }
      }
      catch (System::Exception __gc *e)
      {
         Console::WriteLine (e->get_Message ());
      }
      return NULL;
   }

   void Extent1d::set_Min(double dMin)
   {
      try
      {
         if (m_pIwExtent1d != NULL)
            m_pIwExtent1d->SetMinMax (dMin, m_pIwExtent1d->GetMax ());

//         if (this->Changed != NULL)
//            Changed(this, new System::EventArgs());
      }
      catch (System::Exception __gc *e)
      {
         Console::WriteLine (e->get_Message ());
      }
   }

   void Extent1d::set_Max(double dMax)
   {
      try
      {
         if (m_pIwExtent1d != NULL)
            m_pIwExtent1d->SetMinMax (m_pIwExtent1d->GetMin (), dMax);

//         if (this->Changed != NULL)
//            Changed(this, new System::EventArgs());
      }
      catch (System::Exception __gc *e)
      {
         Console::WriteLine (e->get_Message ());
      }
   }

   bool Extent1dConverter::CanConvertFrom(ITypeDescriptorContext *context, Type *sourceType)
   {
	   if (sourceType == __typeof(String))
		   return true;
	   return PropertiesDeluxeTypeConverter::CanConvertFrom (context, sourceType);
   }

   Object __gc * Extent1dConverter::ConvertFrom(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, Object *value)
   {
	   if (value->GetType() == __typeof(String))
	   {
		   String __gc* sValue = dynamic_cast< String *>(value);
           System::Char cTok[] = {','};
		   String __gc* v[]  = sValue->Split(cTok);
		   if (v->Length != 2)
			   throw new ArgumentException(
			   "Extent1d string must be in the form <min>,<max>");
		   PESMLIB::Extent1d *ext = new PESMLIB::Extent1d();
		   ext->SetCanonical(Double::Parse(v[0]), Double::Parse(v[1]));
		   return ext;
	   }
	   return PropertiesDeluxeTypeConverter::ConvertFrom (context, culture, value);
   }

   bool Extent1dConverter::CanConvertTo(ITypeDescriptorContext *context, Type *destinationType)
   {
	   if (destinationType == __typeof(String))
		   return true;
	   if (destinationType == __typeof(System::ComponentModel::Design::Serialization::InstanceDescriptor))
		   return true;
	   return PropertiesDeluxeTypeConverter::CanConvertTo (context, destinationType);
   }

   System::Object __gc * Extent1dConverter::ConvertTo(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value, Type *destinationType)
   {
	   String* sFormat = String::Empty;

	   if (destinationType == __typeof(String))
	   {
		   PropertyDescriptor *propertyDescriptor = context->PropertyDescriptor;
		   Attribute *attrib;
		   IEnumerator *attribEnum = propertyDescriptor->Attributes->GetEnumerator();
		   while (attribEnum->MoveNext())
		   {
			   attrib = dynamic_cast<Attribute *>(attribEnum->Current);
			   if (attrib->GetType() == __typeof(Utilities::FormatStringAttribute))
			   {
				   Utilities::FormatStringAttribute* formatAttrib  = dynamic_cast<Utilities::FormatStringAttribute*>(attrib);
				   sFormat = formatAttrib->FormatString;
				   break;
			   }
		   }

		   PESMLIB::Extent1d *ext = dynamic_cast<PESMLIB::Extent1d*>(value);
		   String* sFormatSpec = String::Format("{{0:{0} }},{{1:{0} }}", sFormat);
         System::Object *objs[] = {__box(ext->Min), __box(ext->Max)};
		   if (sFormat == String::Empty)
            return String::Format("{0}, {1}", objs);
		   else
			   return String::Format(sFormatSpec, objs);
	   }
	   if (destinationType == __typeof(System::ComponentModel::Design::Serialization::InstanceDescriptor))
	   {
		   Extent1d* ext = static_cast<Extent1d*>(value);

		   // Specify that we should use the two-parameter constructor.
		   Type __gc * types[] = {__typeof(double), __typeof(double), __typeof(double)};
		   ArrayList *coords  = new ArrayList();
		   coords->Add(__box(ext->Min));
		   coords->Add(__box(ext->Max));
		   return new System::ComponentModel::Design::Serialization::InstanceDescriptor(
			   (__typeof(Extent1d)->GetConstructor(types)), coords);
	   }
	   return PropertiesDeluxeTypeConverter::ConvertTo (context, culture, value, destinationType);
   }

   bool Extent1dConverter::GetPropertiesSupported(ITypeDescriptorContext *context)
   {
	   bool bReadOnly = false;

	   PropertyDescriptor *propertyDescriptor = context->PropertyDescriptor;
	   Attribute *attrib;
	   IEnumerator *attribEnum = propertyDescriptor->Attributes->GetEnumerator();
	   while (attribEnum->MoveNext())
	   {
		   attrib = dynamic_cast<Attribute *>(attribEnum->Current);
		   if (attrib->GetType() == __typeof(ReadOnlyAttribute))
		   {
			   ReadOnlyAttribute *readOnlyAttrib = dynamic_cast<ReadOnlyAttribute*>(attrib);
			   bReadOnly = readOnlyAttrib->IsReadOnly;
			   break;
		   }
	   }
	   if (bReadOnly)
		   return false;
	   else
		   return PropertiesDeluxeTypeConverter::GetPropertiesSupported (context);
   }
}