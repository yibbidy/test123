#include "StdAfx.h"
#include ".\smextent3d.h"
#using <mscorlib.dll>
using namespace System;
using namespace System::Collections;

namespace PESMLIB
{
	Extent3d::Extent3d(void): PersistObject()
	{
//		m_pIwExtent3d = NULL;
      m_pIwExtent3d = new IwExtent3d ();
	}

	Extent3d::Extent3d(double dX0, double dY0, double dZ0, double dX1, double dY1, double dZ1): PersistObject()
	{
		m_pIwExtent3d = new IwExtent3d (IwPoint3d (dX0, dY0, dZ0), IwPoint3d (dX1, dY1, dZ1));
	}

	Extent3d::Extent3d(XML::XmlElement* pElem) : PersistObject()
	{
		m_pIwExtent3d = new IwExtent3d ();
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

	Extent3d::Extent3d(Extent3d &extOther)
	{
		m_pIwExtent3d = new IwExtent3d(*(extOther.GetIwObj()));
	}

	Extent3d::Extent3d(double *pVec0, double *pVec1)
	{
		m_pIwExtent3d = new IwExtent3d(IwPoint3d (pVec0), IwPoint3d (pVec1));
	}

	Extent3d::~Extent3d(void)
	{
		if (m_pIwExtent3d)
		{
			delete m_pIwExtent3d;
			m_pIwExtent3d = NULL;
		}
	}

	void Extent3d::SetCanonical (double dX0, double dY0, double dZ0, double dX1, double dY1, double dZ1)
	{
		if (!m_pIwExtent3d)
			m_pIwExtent3d = new IwExtent3d (IwPoint3d (dX0, dY0, dZ0), IwPoint3d (dX1, dY1, dZ1));
		else
			((IwExtent3d *) m_pIwExtent3d)->SetMinMax (IwPoint3d (dX0, dY0, dZ0), IwPoint3d (dX1, dY1, dZ1));

		if (m_pIwExtent3d)
			AddToDOM ();
	}

   void Extent3d::Init ()
   {
      if (m_pIwExtent3d)
         m_pIwExtent3d->Init ();
   }

   void Extent3d::Union (Extent3d& extOther, Extent3d& extResult)
   {
      if (m_pIwExtent3d)
      {
         const IwExtent3d *pIwExtentOther = extOther.GetIwObj ();
         m_pIwExtent3d->Union (*pIwExtentOther, *m_pIwExtent3d);
      }
   }

	IwExtent3d* Extent3d::ExtractObj ()
	{
		IwExtent3d *pIwObj = m_pIwExtent3d;
		m_pIwExtent3d = NULL;

		return pIwObj;
	}

	const IwExtent3d* Extent3d::GetIwObj ()
	{
		return m_pIwExtent3d;
	}

	void Extent3d::AddToDOM ()
	{
		if (m_pIwExtent3d != NULL && m_pXMLElem != NULL)
		{
			// Add the persistent ID.
			m_pXMLElem->SetAttribute("idSelf", GetId());
		}
	}

	void Extent3d::GetFromDOM ()
	{
		if (m_pXMLElem != NULL && m_pIwExtent3d != NULL)
		{
			// Get the persistent GUID.
			String* sId = m_pXMLElem->GetAttribute("idSelf");
			if (sId->Length > 0)
				SetId (sId);
		}
	}

	double Extent3d::Diagonal()
	{
		return m_pIwExtent3d->GetSize ().Length ();
	}

   String __gc * Extent3d::ToString ()
   {
      try
      {
         return String::Format ("{0},{1}", 
            this->Min->ToString (), this->Max->ToString ());
      }
	  catch (Exception *)
      {
         return NULL;
      }
   }

   Extent3d __gc * Extent3d::Parse (String __gc *sVector)
   {
      try
      {
         if (NULL != sVector)
         {
            System::Char cTok[] = {','};
            System::String __gc *sSplit[] = sVector->Split (cTok);
            return new Extent3d (
               System::Convert::ToDouble (sSplit[0]), 
               System::Convert::ToDouble (sSplit[1]), 
               System::Convert::ToDouble (sSplit[2]),
               System::Convert::ToDouble (sSplit[3]), 
               System::Convert::ToDouble (sSplit[4]), 
               System::Convert::ToDouble (sSplit[5]));
         }
         else
            return NULL;
      }
	  catch (Exception *)
      {
         return NULL;
      }
   }

   Vector3d __gc * Extent3d::get_Min ()
   {
      try
      {
         if (m_pIwExtent3d != NULL)
         {
            IwPoint3d pt3d = m_pIwExtent3d->GetMin ();
            PESMLIB::Vector3d __gc *pVector = new Vector3d (pt3d.x, pt3d.y, pt3d.z);
            return pVector;
         }
      }
      catch (System::Exception __gc *e)
      {
         Console::WriteLine (e->get_Message ());
      }
      return NULL;
   }

   Vector3d __gc * Extent3d::get_Max ()
   {
      try
      {
         if (m_pIwExtent3d != NULL)
         {
            IwPoint3d pt3d = m_pIwExtent3d->GetMax ();
            PESMLIB::Vector3d __gc *pVector = new Vector3d (pt3d.x, pt3d.y, pt3d.z);
            return pVector;
         }
      }
      catch (System::Exception __gc *e)
      {
         Console::WriteLine (e->get_Message ());
      }
      return NULL;
   }

   void Extent3d::set_Min(Vector3d __gc *pVec)
   {
      try
      {
         if (m_pIwExtent3d != NULL && pVec != NULL)
            m_pIwExtent3d->SetMinMax (*pVec->GetIwObj (), m_pIwExtent3d->GetMax ());

         if (this->Changed != NULL)
            Changed(this, new System::EventArgs());
      }
      catch (System::Exception __gc *e)
      {
         Console::WriteLine (e->get_Message ());
      }
   }

   void Extent3d::set_Max(Vector3d __gc *pVec)
   {
      try
      {
         if (m_pIwExtent3d != NULL && pVec != NULL)
            m_pIwExtent3d->SetMinMax (m_pIwExtent3d->GetMin (), *pVec->GetIwObj ());

         if (this->Changed != NULL)
            Changed(this, new System::EventArgs());
      }
      catch (System::Exception __gc *e)
      {
         Console::WriteLine (e->get_Message ());
      }
   }

   bool Extent3dConverter::CanConvertFrom(ITypeDescriptorContext *context, Type *sourceType)
   {
	   if (sourceType == __typeof(String))
		   return true;
	   return PropertiesDeluxeTypeConverter::CanConvertFrom (context, sourceType);
   }

   Object __gc * Extent3dConverter::ConvertFrom(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, Object *value)
   {
	   if (value->GetType() == __typeof(String))
	   {
		   String __gc* sValue = dynamic_cast< String *>(value);
           System::Char cTok[] = {','};
		   String __gc* v[]  = sValue->Split(cTok);
		   if (v->Length != 6)
			   throw new ArgumentException(
			   "Extent3d string must be in the form <x>,<y>,<z>");
		   PESMLIB::Extent3d *ext = new PESMLIB::Extent3d();
		   ext->SetCanonical(Double::Parse(v[0]), Double::Parse(v[1]), Double::Parse(v[2]), Double::Parse(v[3]), Double::Parse(v[4]), Double::Parse(v[5]));
		   return ext;
	   }
	   return PropertiesDeluxeTypeConverter::ConvertFrom (context, culture, value);
   }

   bool Extent3dConverter::CanConvertTo(ITypeDescriptorContext *context, Type *destinationType)
   {
	   if (destinationType == __typeof(String))
		   return true;
	   if (destinationType == __typeof(System::ComponentModel::Design::Serialization::InstanceDescriptor))
		   return true;
	   return PropertiesDeluxeTypeConverter::CanConvertTo (context, destinationType);
   }

   System::Object __gc * Extent3dConverter::ConvertTo(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value, Type *destinationType)
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

		   PESMLIB::Extent3d *ext = dynamic_cast<PESMLIB::Extent3d*>(value);
		   String* sFormatSpec = String::Format("{{0:{0} }},{{1:{0} }},{{2:{0} },{{3:{0} },{{4:{0} },{{5:{0} }}", sFormat);
         System::Object *objs[] = {__box(ext->Min->X), __box(ext->Min->Y), __box(ext->Min->Z), __box(ext->Max->X), __box(ext->Max->Y), __box(ext->Max->Z)};
		   if (sFormat == String::Empty)
            return String::Format("{0}, {1}, {2}, {3], {4], {5]", objs);
		   else
			   return String::Format(sFormatSpec, objs);
	   }
	   if (destinationType == __typeof(System::ComponentModel::Design::Serialization::InstanceDescriptor))
	   {
		   Extent3d* ext = static_cast<Extent3d*>(value);

		   // Specify that we should use the two-parameter constructor.
		   Type __gc * types[] = {__typeof(double), __typeof(double), __typeof(double)};
		   ArrayList *coords  = new ArrayList();
		   coords->Add(__box(ext->Min->X));
		   coords->Add(__box(ext->Min->Y));
		   coords->Add(__box(ext->Min->Z));
		   coords->Add(__box(ext->Max->X));
		   coords->Add(__box(ext->Max->Y));
		   coords->Add(__box(ext->Max->Z));
		   return new System::ComponentModel::Design::Serialization::InstanceDescriptor(
			   (__typeof(Extent3d)->GetConstructor(types)), coords);
	   }
	   return PropertiesDeluxeTypeConverter::ConvertTo (context, culture, value, destinationType);
   }

   bool Extent3dConverter::GetPropertiesSupported(ITypeDescriptorContext *context)
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