#include "StdAfx.h"
#include ".\smvector2d.h"
#using <mscorlib.dll>
using namespace System;
using namespace System::Collections;

namespace PESMLIB
{
	Vector2d::Vector2d(void): PersistObject()
	{
		m_pIwVector2d = NULL;
	}

	Vector2d::Vector2d(double dX, double dY): PersistObject()
	{
		m_pIwVector2d = new IwVector2d(dX, dY);
	}

	void Vector2d::Scale(double dScale)
	{
		if (!m_pIwVector2d)
			m_pIwVector2d = new IwVector2d (0.0, 0.0);
		else
		{
			((IwVector2d *) m_pIwVector2d)->Set (((IwVector2d *) m_pIwVector2d)->x * dScale,
				((IwVector2d *) m_pIwVector2d)->y * dScale);
		}

		if (m_pIwVector2d)
			AddToDOM ();
	}
	Vector2d::Vector2d(XML::XmlElement* pElem) : PersistObject()
	{
		m_pIwVector2d = new IwVector2d ();
		m_pXMLElem = pElem;

		// Now populate the Vector2d via the DOM if there is anything in the DOM.
		// Otherwise it is just the node where we are supposed to store the 
		// state later.

		if (m_pXMLElem != NULL)
		{
			GetFromDOM ();
			CreateId ();
		}
	}

	Vector2d::Vector2d(Vector2d &vecOther)
	{
		m_pIwVector2d = new IwVector2d(*(vecOther.GetIwObj()));
	}

	Vector2d::Vector2d(double *pVec)
	{
		m_pIwVector2d = new IwVector2d(pVec);
	}

	Vector2d::~Vector2d(void)
	{
		if (m_pIwVector2d)
		{
			delete m_pIwVector2d;
			m_pIwVector2d = NULL;
		}
	}

	HC::NL_POINT Vector2d::GetHoopsPoint()
	{
		HC::NL_POINT hpt;
		hpt.x = (float) this->X;
		hpt.y = (float) this->Y;
		hpt.z = 0.0;
		return hpt;
	}

	void Vector2d::SetCanonical (double dX, double dY)
	{
		if (!m_pIwVector2d)
			m_pIwVector2d = new IwVector2d (dX, dY);
		else
			((IwVector2d *) m_pIwVector2d)->Set (dX, dY);

		if (m_pIwVector2d)
			AddToDOM ();
	}

	IwVector2d* Vector2d::ExtractObj ()
	{
		IwVector2d *pIwObj = m_pIwVector2d;
		m_pIwVector2d = NULL;

		return pIwObj;
	}

	const IwVector2d* Vector2d::GetIwObj ()
	{
		return m_pIwVector2d;
	}

	void Vector2d::AddToDOM ()
	{
		if (m_pIwVector2d != NULL && m_pXMLElem != NULL)
		{
			// Add the persistent ID.
			m_pXMLElem->SetAttribute("idSelf", GetId());
		}
	}

	void Vector2d::GetFromDOM ()
	{
		if (m_pXMLElem != NULL && m_pIwVector2d != NULL)
		{
			// Get the persistent GUID.
			String* sId = m_pXMLElem->GetAttribute("idSelf");
			if (sId->Length > 0)
				SetId (sId);
		}
	}

	double Vector2d::AngleBetween(Vector2d &vecOther)
	{
		double dAngleRadians = 0.0;
		if (IW_SUCCESS != m_pIwVector2d->AngleBetween( *(vecOther.GetIwObj()), dAngleRadians))
			throw new System::ArgumentException();
		return dAngleRadians;
	}

	double Vector2d::DistanceBetween(Vector2d &vecOther)
	{
		return m_pIwVector2d->DistanceBetween( *(vecOther.GetIwObj()));
	}

	double Vector2d::Dot(Vector2d &vecOther)
	{
		return m_pIwVector2d->Dot(*(vecOther.GetIwObj()));
	}

	double Vector2d::Length()
	{
		return m_pIwVector2d->Length();
	}

	void Vector2d::Unitize()
	{
		m_pIwVector2d->Unitize();
	}

	Vector2d^  Vector2d::op_Addition(Vector2d *lh, Vector2d *rh)
	{
		return new Vector2d(lh->X + rh->X, lh->Y + rh->Y);
	}

	Vector2d^  Vector2d::op_Subtraction(Vector2d *lh, Vector2d *rh)
	{
		return new Vector2d (lh->X - rh->X, lh->Y -rh->Y);
	}

	Vector2d^  Vector2d::op_Assign(Vector2d *lh, Vector2d *rh)
	{
		lh->X = rh->X;
		lh->Y = rh->Y;
		return lh;
	}

   String^  Vector2d::ToString ()
   {
      try
      {
         return String::Format ("{0},{1}", 
            this->X.ToString(), this->Y.ToString());
      }
	  catch (Exception *)
      {
         return NULL;
      }
   }

   Vector2d^  Vector2d::Parse (String^ sVector)
   {
      try
      {
         if (NULL != sVector)
         {
            System::Char cTok[] = {','};
            System::String^ sSplit[] = sVector->Split (cTok);
            return new Vector2d (
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

   void Vector2d::set_X(double value)
   {
	   m_pIwVector2d->x = value; 
	   if (this->Changed != NULL)
		   Changed(this, new System::EventArgs());
   }

   void Vector2d::set_Y(double value)
   {
	   m_pIwVector2d->y = value; 
	   if (this->Changed != NULL)
		   Changed(this, new System::EventArgs());
   }

   bool Vector2dConverter::CanConvertFrom(ITypeDescriptorContext *context, Type *sourceType)
   {
	   if (sourceType == __typeof(String))
		   return true;
	   return PropertiesDeluxeTypeConverter::CanConvertFrom (context, sourceType);
   }

   Object^  Vector2dConverter::ConvertFrom(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, Object *value)
   {
	   Utilities::UnitsAttribute* unitsAttrib = NULL;
	   Utilities::UnitScaleManager* scaleMgr = Utilities::UnitScaleManager::Current;
       System::Char cTok[] = {','};
	   String* sValues = String::Empty;
	   String* sAbbrev = String::Empty;
	   double dX, dY;

	   if (value->GetType() == __typeof(String))
	   {
		   PropertyDescriptor *propertyDescriptor = context->PropertyDescriptor;
		   Attribute *attrib;
		   IEnumerator *attribEnum = propertyDescriptor->Attributes->GetEnumerator();
		   while (attribEnum->MoveNext())
		   {
			   attrib = dynamic_cast<Attribute *>(attribEnum->Current);
			   if (attrib->GetType() == __typeof(Utilities::UnitsAttribute))
			   {
				   unitsAttrib  = dynamic_cast<Utilities::UnitsAttribute*>(attrib);
				   break;
			   }
		   }
		   String^  sValue = dynamic_cast< String *>(value);
		   // If we have a UnitsAttribute, parse out value and abbreviation
		   if (unitsAttrib != NULL)
		   {
			   int iOpenBrace = sValue->IndexOf("{");
			   int iCloseBrace = sValue->IndexOf("}");
			   sValues = sValue->Substring(iOpenBrace+1,iCloseBrace-iOpenBrace-1);
			   sAbbrev = sValue->Substring(iCloseBrace+1)->Trim();
		   }
		   else
			   sValues = sValue;
		   // Parse the vector values
		   String^  v[]  = sValues->Split(cTok);
		   if (v->Length != 2)
			   throw new ArgumentException(
			   "Vector2d string must be in the form <x>,<y>");
		   PESMLIB::Vector2d *point = new PESMLIB::Vector2d();
		   dX = Double::Parse(v[0]);
		   dY = Double::Parse(v[1]);
		   // If we have a UnitsAttribute, convert values using unit scale factors
		   if (unitsAttrib != NULL)
		   {
			   double dScale = 1.;
			   bool bMatch = false;
			   for (int i = 0; i < scaleMgr->Count; i++)
			   {
				   UnitPreference* pref = scaleMgr->GetUnitPreference(i);
				   if (pref->Abbreviation->Equals(sAbbrev))
				   {
					   dScale = pref->Scale;
					   bMatch = true;
					   continue;
				   }
			   }
			   if (!bMatch)
			   {
				   String* sException = String::Format("No match found for unit abbreviation {0}.", sAbbrev);
				   throw new ArgumentException(sException);
			   }
			   double dInternalScale = unitsAttrib->InternalScale;
			   dX *= dScale/dInternalScale;
			   dY *= dScale/dInternalScale;
		   }
		   point->SetCanonical(dX, dY);
		   return point;
	   }
	   return PropertiesDeluxeTypeConverter::ConvertFrom (context, culture, value);
   }

   bool Vector2dConverter::CanConvertTo(ITypeDescriptorContext *context, Type *destinationType)
   {
	   if (destinationType == __typeof(String))
		   return true;
	   if (destinationType == __typeof(System::ComponentModel::Design::Serialization::InstanceDescriptor))
		   return true;
	   return PropertiesDeluxeTypeConverter::CanConvertTo (context, destinationType);
   }

   System::Object^  Vector2dConverter::ConvertTo(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value, Type *destinationType)
   {
	   String* sFormat = String::Empty;
	   String* sFormatSpec = String::Empty;
	   Utilities::UnitsAttribute* unitsAttrib = NULL;
	   Utilities::UnitScaleManager* scaleMgr = Utilities::UnitScaleManager::Current;

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
			   attrib = dynamic_cast<Attribute *>(attribEnum->Current);
			   if (attrib->GetType() == __typeof(Utilities::UnitsAttribute))
			   {
				   unitsAttrib  = dynamic_cast<Utilities::UnitsAttribute*>(attrib);
				   break;
			   }
		   }

		   PESMLIB::Vector2d *point = dynamic_cast<PESMLIB::Vector2d*>(value);
		   if (unitsAttrib != NULL)
		   {
			   // Convert point coordinate values to SI equivalent
			   double dSiX = point->X*unitsAttrib->InternalScale;
			   double dSiY = point->Y*unitsAttrib->InternalScale;
			   // Get string used as units abbreviation - use maximum dimension for selection of unit scale
			   double dSiMax = Math::Max(Math::Abs(dSiX), Math::Abs(dSiY));
			   String* sAbbrev = scaleMgr->GetDisplayedUnitAbbreviation(unitsAttrib, dSiMax);
			   // Get associated Scale factor
			   double dScale = scaleMgr->GetPreferredScale(unitsAttrib, dSiMax);
			   // Get associate format string
			   String* sNumFormat = scaleMgr->GetDisplayedFormat(unitsAttrib, dSiMax);
			   // Convert display value to preferred Unit scale
			   double dX = dSiX/dScale;
			   double dY = dSiY/dScale;
			   sFormatSpec = String::Format("{{{{ {{0:{0} }},{{1:{0} }}}}}} {1}", sNumFormat, sAbbrev);
			   return String::Format(sFormatSpec, __box(dX), __box(dY));
		   }
		   else if (sFormat != String::Empty)
		   {
			   sFormatSpec = String::Format("{{0:{0} }},{{1:{0} }}", sFormat);
			   return String::Format(sFormatSpec, __box(point->X), __box(point->Y));
		   }
		   else
			   return String::Format("{0}, {1}", __box(point->X), __box(point->Y));
	   }
	   if (destinationType == __typeof(System::ComponentModel::Design::Serialization::InstanceDescriptor))
	   {
		   Vector2d* point = static_cast<Vector2d*>(value);

		   // Specify that we should use the two-parameter constructor.
		   Type^  types[] = {__typeof(double), __typeof(double)};
		   ArrayList *coords  = new ArrayList();
		   coords->Add(__box(point->X));
		   coords->Add(__box(point->Y));
		   return new System::ComponentModel::Design::Serialization::InstanceDescriptor(
			   (__typeof(Vector2d)->GetConstructor(types)), coords);
	   }
	   return PropertiesDeluxeTypeConverter::ConvertTo (context, culture, value, destinationType);
   }

   bool Vector2dConverter::GetPropertiesSupported(ITypeDescriptorContext *context)
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

   PropertyDescriptorCollection* Vector2dConverter::GetProperties(ITypeDescriptorContext *context, Object *value, Attribute *attributes[])
		{
			ArrayList *addAttributes = new ArrayList();
			Type *type = __typeof(UnitsAttribute);
			UnitsAttribute *unitsAttribute = dynamic_cast<UnitsAttribute *>(context->PropertyDescriptor->Attributes->get_Item(type));
			PropertyDescriptorCollection *props = TypeDescriptor::GetProperties(value, attributes, true);
			PropertyDescriptorCollection *newProps = new PropertyDescriptorCollection(NULL);
			for(int i = 0; i < props->Count; i++)
			{
				PropertyDescriptor *pd = dynamic_cast<PropertyDescriptor *>(props->get_Item(i));
				addAttributes->Clear();
				if (unitsAttribute != NULL)
					addAttributes->Add(unitsAttribute);
				Object *initialValue = value->GetType()->GetProperty(pd->Name)->GetValue(value, NULL);
				DefaultValueAttribute *defaultAttribute = new DefaultValueAttribute(initialValue);
				addAttributes->Add(defaultAttribute);
				PropertyDescriptor *newPd = TypeDescriptor::CreateProperty(value->GetType(), pd, dynamic_cast<Attribute *[]>(addAttributes->ToArray(__typeof(Attribute))));
				newProps->Add(newPd);
			}
			return newProps;
		}

}