#include "StdAfx.h"
#include ".\smvector3d.h"
#using <mscorlib.dll>
using namespace System;
using namespace System::Collections;

namespace PESMLIB
{
	Vector3d::Vector3d(void): PersistObject()
	{
		m_pIwVector3d = NULL;
	}

	Vector3d::Vector3d(double dX, double dY, double dZ): PersistObject()
	{
		m_pIwVector3d = new IwVector3d(dX, dY, dZ);
	}

	void Vector3d::Scale(double dScale)
	{
		if (!m_pIwVector3d)
			m_pIwVector3d = new IwVector3d (0.0, 0.0, 0.0);
		else
		{
			((IwVector3d *) m_pIwVector3d)->Set (((IwVector3d *) m_pIwVector3d)->x * dScale,
				((IwVector3d *) m_pIwVector3d)->y * dScale,
				((IwVector3d *) m_pIwVector3d)->z * dScale);
		}

		if (m_pIwVector3d)
			AddToDOM ();
	}
	Vector3d::Vector3d(XML::XmlElement* pElem) : PersistObject()
	{
		m_pIwVector3d = new IwVector3d ();
		m_pXMLElem = pElem;

		// Now populate the Vector3d via the DOM if there is anything in the DOM.
		// Otherwise it is just the node where we are supposed to store the 
		// state later.

		if (m_pXMLElem != NULL)
		{
			GetFromDOM ();
			CreateId ();
		}
	}

	Vector3d::Vector3d(Vector3d &vecOther)
	{
		m_pIwVector3d = new IwVector3d(*(vecOther.GetIwObj()));
	}

	Vector3d::Vector3d(double *pVec)
	{
		m_pIwVector3d = new IwVector3d(pVec);
	}

	Vector3d::~Vector3d(void)
	{
		if (m_pIwVector3d)
		{
			delete m_pIwVector3d;
			m_pIwVector3d = NULL;
		}
	}

	HC::NL_POINT Vector3d::GetHoopsPoint()
	{
		HC::NL_POINT hpt;
		hpt.x = (float) this->X;
		hpt.y = (float) this->Y;
		hpt.z = (float) this->Z;
		return hpt;
	}

	void Vector3d::SetCanonical (double dX, double dY, double dZ)
	{
		if (!m_pIwVector3d)
			m_pIwVector3d = new IwVector3d (dX, dY, dZ);
		else
			((IwVector3d *) m_pIwVector3d)->Set (dX, dY, dZ);

		if (m_pIwVector3d)
			AddToDOM ();
	}

	IwVector3d* Vector3d::ExtractObj ()
	{
		IwVector3d *pIwObj = m_pIwVector3d;
		m_pIwVector3d = NULL;

		return pIwObj;
	}

	const IwVector3d* Vector3d::GetIwObj ()
	{
		return m_pIwVector3d;
	}

	void Vector3d::AddToDOM ()
	{
		if (m_pIwVector3d != NULL && m_pXMLElem != NULL)
		{
			// Add the persistent ID.
			m_pXMLElem->SetAttribute("idSelf", GetId());
		}
	}

	void Vector3d::GetFromDOM ()
	{
		if (m_pXMLElem != NULL && m_pIwVector3d != NULL)
		{
			// Get the persistent GUID.
			String* sId = m_pXMLElem->GetAttribute("idSelf");
			if (sId->Length > 0)
				SetId (sId);
		}
	}

	double Vector3d::AngleBetween(Vector3d &vecOther)
	{
		double dAngleRadians = 0.0;
		if (IW_SUCCESS != m_pIwVector3d->AngleBetween( *(vecOther.GetIwObj()), dAngleRadians))
			throw new System::ArgumentException();
		return dAngleRadians;
	}

	double Vector3d::DistanceBetween(Vector3d &vecOther)
	{
		return m_pIwVector3d->DistanceBetween( *(vecOther.GetIwObj()));
	}

	double Vector3d::Dot(Vector3d &vecOther)
	{
		return m_pIwVector3d->Dot(*(vecOther.GetIwObj()));
	}

	bool Vector3d::IsParallelTo(Vector3d &vecOther, double dAngTolDeg)
	{
		if (TRUE == m_pIwVector3d->IsParallelTo(*(vecOther.GetIwObj()), dAngTolDeg))
			return true;
		else
			return false;
	}

	bool Vector3d::IsPerpendicularTo(Vector3d & vecOther, double dAngTolDeg)
	{
		if (TRUE == m_pIwVector3d->IsPerpendicularTo(*(vecOther.GetIwObj()), dAngTolDeg))
			return true;
		else
			return false;
	}

	double Vector3d::Length()
	{
		return m_pIwVector3d->Length();
	}

	void Vector3d::MakeUnitOrthoVectors(Vector3d *pvecYRef, Vector3d &vecX, Vector3d &vecY, Vector3d &vecZ)
	{
		const IwVector3d *pIwVecYRef = NULL;
		if (pvecYRef != NULL)
			pIwVecYRef = pvecYRef->GetIwObj();
		IwVector3d rVecX, rVecY, rVecZ;
		if (IW_SUCCESS != m_pIwVector3d->MakeUnitOrthoVectors(pIwVecYRef, rVecX, rVecY, rVecZ))
			throw new System::ArithmeticException();
		vecX.SetCanonical(rVecX.x, rVecX.y, rVecX.z);
		vecY.SetCanonical(rVecY.x, rVecY.y, rVecY.z);
		vecZ.SetCanonical(rVecZ.x, rVecZ.y, rVecZ.z);
	}

	Vector3d^  Vector3d::ProjectPointToPlane(Vector3d &ptPlane, Vector3d &vecPlaneNormal)
	{
		IwVector3d ptProject = m_pIwVector3d->ProjectPointToPlane(*(ptPlane.GetIwObj()), *(vecPlaneNormal.GetIwObj()));
		return new Vector3d(ptProject.x, ptProject.y, ptProject.z);
	}

	Vector3d^  Vector3d::ProjectVectorToPlane(Vector3d &vecPlaneNormal)
	{
		IwVector3d vecProject = m_pIwVector3d->ProjectToPlane(*(vecPlaneNormal.GetIwObj()));
		return new Vector3d(vecProject.x, vecProject.y, vecProject.z);
	}

	void Vector3d::Unitize()
	{
		m_pIwVector3d->Unitize();
	}

	Vector3d^  Vector3d::op_Addition(Vector3d *lh, Vector3d *rh)
	{
		return new Vector3d(lh->X + rh->X, lh->Y + rh->Y, lh->Z + rh->Z);
	}

	Vector3d^  Vector3d::op_Subtraction(Vector3d *lh, Vector3d *rh)
	{
		return new Vector3d (lh->X - rh->X, lh->Y -rh->Y, lh->Z - rh->Z);
	}

	Vector3d^  Vector3d::op_Multiply(Vector3d *lh, Vector3d *rh)
	{
		IwVector3d cross = *(lh->GetIwObj()) * (*(rh->GetIwObj()));
		return new Vector3d(cross.x, cross.y, cross.z);
	}

	Vector3d^  Vector3d::op_Assign(Vector3d *lh, Vector3d *rh)
	{
		lh->X = rh->X;
		lh->Y = rh->Y;
		lh->Z = rh->Z;
		return lh;
	}

   String^  Vector3d::ToString ()
   {
      try
      {
         return String::Format ("{0},{1},{2}", 
            this->X.ToString(), this->Y.ToString(), this->Z.ToString());
      }
	  catch (Exception *)
      {
         return NULL;
      }
   }

   Vector3d^  Vector3d::Parse (String^ sVector)
   {
      try
      {
         if (NULL != sVector)
         {
            System::Char cTok[] = {','};
            System::String^ sSplit[] = sVector->Split (cTok);
            return new Vector3d (
               System::Convert::ToDouble (sSplit[0]), 
               System::Convert::ToDouble (sSplit[1]), 
               System::Convert::ToDouble (sSplit[2]));
         }
         else
            return NULL;
      }
	  catch (Exception *)
      {
         return NULL;
      }
   }

   void Vector3d::set_X(double value)
   {
	   m_pIwVector3d->x = value; 
	   if (this->Changed != NULL)
		   Changed(this, new System::EventArgs());
   }

   void Vector3d::set_Y(double value)
   {
	   m_pIwVector3d->y = value; 
	   if (this->Changed != NULL)
		   Changed(this, new System::EventArgs());
   }

   void Vector3d::set_Z(double value)
   {
	   m_pIwVector3d->z = value; 
	   if (this->Changed != NULL)
		   Changed(this, new System::EventArgs());
   }

   bool Vector3dConverter::CanConvertFrom(ITypeDescriptorContext *context, Type *sourceType)
   {
	   if (sourceType == __typeof(String))
		   return true;
	   return PropertiesDeluxeTypeConverter::CanConvertFrom (context, sourceType);
   }

   Object^  Vector3dConverter::ConvertFrom(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, Object *value)
   {
	   Utilities::UnitsAttribute* unitsAttrib = NULL;
	   Utilities::UnitScaleManager* scaleMgr = Utilities::UnitScaleManager::Current;
       System::Char cTok[] = {','};
	   String* sValues = String::Empty;
	   String* sAbbrev = String::Empty;
	   double dX, dY, dZ;

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
		   if (v->Length != 3)
			   throw new ArgumentException(
			   "Vector3d string must be in the form <x>,<y>,<z>");
		   PESMLIB::Vector3d *point = new PESMLIB::Vector3d();
		   dX = Double::Parse(v[0]);
		   dY = Double::Parse(v[1]);
		   dZ = Double::Parse(v[2]);
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
			   dZ *= dScale/dInternalScale;
		   }
		   point->SetCanonical(dX, dY, dZ);
		   return point;
	   }
	   return PropertiesDeluxeTypeConverter::ConvertFrom (context, culture, value);
   }

   bool Vector3dConverter::CanConvertTo(ITypeDescriptorContext *context, Type *destinationType)
   {
	   if (destinationType == __typeof(String))
		   return true;
	   if (destinationType == __typeof(System::ComponentModel::Design::Serialization::InstanceDescriptor))
		   return true;
	   return PropertiesDeluxeTypeConverter::CanConvertTo (context, destinationType);
   }

   System::Object^  Vector3dConverter::ConvertTo(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value, Type *destinationType)
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

		   PESMLIB::Vector3d *point = dynamic_cast<PESMLIB::Vector3d*>(value);
		   if (unitsAttrib != NULL)
		   {
			   // Convert point coordinate values to SI equivalent
			   double dSiX = point->X*unitsAttrib->InternalScale;
			   double dSiY = point->Y*unitsAttrib->InternalScale;
			   double dSiZ = point->Z*unitsAttrib->InternalScale;
			   // Get string used as units abbreviation - use maximum dimension for selection of unit scale
			   double dSiMax = Math::Max(Math::Abs(dSiX), Math::Abs(dSiY));
			   dSiMax = Math::Max(dSiMax, Math::Abs(dSiZ));
			   String* sAbbrev = scaleMgr->GetDisplayedUnitAbbreviation(unitsAttrib, dSiMax);
			   // Get associated Scale factor
			   double dScale = scaleMgr->GetPreferredScale(unitsAttrib, dSiMax);
			   // Get associate format string
			   String* sNumFormat = scaleMgr->GetDisplayedFormat(unitsAttrib, dSiMax);
			   // Convert display value to preferred Unit scale
			   double dX = dSiX/dScale;
			   double dY = dSiY/dScale;
			   double dZ = dSiZ/dScale;
			   sFormatSpec = String::Format("{{{{ {{0:{0} }},{{1:{0} }},{{2:{0} }} }}}} {1}", sNumFormat, sAbbrev);
			   return String::Format(sFormatSpec, __box(dX), __box(dY), __box(dZ));
		   }
		   else if (sFormat != String::Empty)
		   {
			   sFormatSpec = String::Format("{{0:{0} }},{{1:{0} }},{{2:{0} }}", sFormat);
			   return String::Format(sFormatSpec, __box(point->X), __box(point->Y), __box(point->Z));
		   }
		   else
			   return String::Format("{0}, {1}, {2}", __box(point->X), __box(point->Y), __box(point->Z));
	   }
	   if (destinationType == __typeof(System::ComponentModel::Design::Serialization::InstanceDescriptor))
	   {
		   Vector3d* point = static_cast<Vector3d*>(value);

		   // Specify that we should use the two-parameter constructor.
		   Type^  types[] = {__typeof(double), __typeof(double), __typeof(double)};
		   ArrayList *coords  = new ArrayList();
		   coords->Add(__box(point->X));
		   coords->Add(__box(point->Y));
		   coords->Add(__box(point->Z));
		   return new System::ComponentModel::Design::Serialization::InstanceDescriptor(
			   (__typeof(Vector3d)->GetConstructor(types)), coords);
	   }
	   return PropertiesDeluxeTypeConverter::ConvertTo (context, culture, value, destinationType);
   }

   bool Vector3dConverter::GetPropertiesSupported(ITypeDescriptorContext *context)
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

   PropertyDescriptorCollection* Vector3dConverter::GetProperties(ITypeDescriptorContext *context, Object *value, Attribute *attributes[])
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