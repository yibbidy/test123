#pragma once
#include "smobject.h"
using namespace System::ComponentModel;
using namespace Utilities;
using namespace Units;
//using namespace PEHoops::ThreeDGS;
using namespace VEDM::Windows;

namespace PESMLIB
{
	__gc public class Vector3d: public PersistObject
	{
	public:
		Vector3d(void);
		Vector3d(double x, double y, double z);
		Vector3d(double* pVec);
		Vector3d(Vector3d &vecOther);
		Vector3d(XML::XmlElement* pElem);
		virtual ~Vector3d(void);

		// Managed operators
		static Vector3d __gc * op_Addition(Vector3d *vec1, Vector3d *vec2);
		static Vector3d __gc * op_Subtraction(Vector3d *vec1, Vector3d *vec2);
		static Vector3d __gc * op_Multiply(Vector3d *vec1, Vector3d *vec2);
		static Vector3d __gc * op_Assign(Vector3d *vec1, Vector3d *pt2);

		virtual HC::NL_POINT GetHoopsPoint();
		virtual void SetCanonical (double x, double y, double z);
		virtual double AngleBetween(Vector3d &vecOther);
		virtual double DistanceBetween(Vector3d &vecOther);
		virtual double Dot(Vector3d &vecOther);
		virtual bool IsParallelTo(Vector3d &vecOther, double dAngTolDeg);
		virtual bool IsPerpendicularTo(Vector3d &vecOther, double dAngTolDeg);
		virtual double Length();
		virtual void MakeUnitOrthoVectors(Vector3d *pvecYRef, Vector3d &vecX, Vector3d &vecY, Vector3d &vecZ);
		virtual Vector3d __gc * ProjectPointToPlane(Vector3d &ptPlane, Vector3d &vecPlaneNormal);
		virtual Vector3d __gc * ProjectVectorToPlane(Vector3d &vecPlaneNormal);
		virtual void Unitize();
		virtual void Scale(double dScale);

		static Vector3d __gc * Parse (System::String __gc *sVector);
		System::String __gc * ToString ();

		// Public properties
		__property virtual double get_X () { return m_pIwVector3d->x; };
		[NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
			Description("X coordinate of vector"), 
			TypeConverter(__typeof(UnitsTypeConverter))/*, UnitsAttribute(1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,false)*/]
		__property virtual void set_X (double value) ;
		__property virtual double get_Y () { return m_pIwVector3d->y; };
		[NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
			Description("Y coordinate of vector"), 
			TypeConverter(__typeof(UnitsTypeConverter))/*, UnitsAttribute(1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,false)*/]
		__property virtual void set_Y (double value);
		__property virtual double get_Z () { return m_pIwVector3d->z; };
		[NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
			Description("Z coordinate of vector"), 
			TypeConverter(__typeof(UnitsTypeConverter))/*, UnitsAttribute(1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,false)*/]
		__property virtual void set_Z (double value);
		__event System::EventHandler *Changed;
	public protected:
		virtual IwVector3d * ExtractObj ();
		virtual const IwVector3d *GetIwObj ();

	protected:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	protected:
		IwVector3d *m_pIwVector3d;
		XML::XmlElement* m_pXMLElem;
	};

	__gc public class Vector3dConverter: public Utilities::PropertiesDeluxeTypeConverter
	{
	public:
		bool CanConvertFrom(ITypeDescriptorContext *context, Type *sourceType);
		System::Object __gc * ConvertFrom(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value);
		bool CanConvertTo(ITypeDescriptorContext *context, Type *destinationType);
		System::Object __gc * ConvertTo(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value, Type *destinationType);
		bool GetPropertiesSupported(ITypeDescriptorContext *context);
		PropertyDescriptorCollection* GetProperties(ITypeDescriptorContext* context, Object* value, Attribute* attributes[]);
	};
}
