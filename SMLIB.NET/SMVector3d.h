#pragma once
#include "smobject.h"
using namespace System::ComponentModel;
using namespace Utilities;
using namespace Units;
//using namespace PEHoops::ThreeDGS;
using namespace VEDM::Windows;

namespace PESMLIB
{
	public ref class Vector3d: public PersistObject
	{
	public:
		Vector3d(void);
		Vector3d(double x, double y, double z);
		Vector3d(double* pVec);
		Vector3d(Vector3d &vecOther);
		Vector3d(XML::XmlElement* pElem);
		virtual ~Vector3d(void);

		// Managed operators
		static Vector3d^  op_Addition(Vector3d *vec1, Vector3d *vec2);
		static Vector3d^  op_Subtraction(Vector3d *vec1, Vector3d *vec2);
		static Vector3d^  op_Multiply(Vector3d *vec1, Vector3d *vec2);
		static Vector3d^  op_Assign(Vector3d *vec1, Vector3d *pt2);

		virtual HC::NL_POINT GetHoopsPoint();
		virtual void SetCanonical (double x, double y, double z);
		virtual double AngleBetween(Vector3d &vecOther);
		virtual double DistanceBetween(Vector3d &vecOther);
		virtual double Dot(Vector3d &vecOther);
		virtual bool IsParallelTo(Vector3d &vecOther, double dAngTolDeg);
		virtual bool IsPerpendicularTo(Vector3d &vecOther, double dAngTolDeg);
		virtual double Length();
		virtual void MakeUnitOrthoVectors(Vector3d *pvecYRef, Vector3d &vecX, Vector3d &vecY, Vector3d &vecZ);
		virtual Vector3d^  ProjectPointToPlane(Vector3d &ptPlane, Vector3d &vecPlaneNormal);
		virtual Vector3d^  ProjectVectorToPlane(Vector3d &vecPlaneNormal);
		virtual void Unitize();
		virtual void Scale(double dScale);

		static Vector3d^  Parse (System::String^ sVector);
		System::String^  ToString ();

		// Public properties
               property double X {
                       virtual double get() { return m_pIwVector3d->x; }
                       [NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
                               Description("X coordinate of vector"),
                               TypeConverter(__typeof(UnitsTypeConverter))/*, UnitsAttribute(1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,false)*/]
                       virtual void set(double value);
               }
               property double Y {
                       virtual double get() { return m_pIwVector3d->y; }
                       [NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
                               Description("Y coordinate of vector"),
                               TypeConverter(__typeof(UnitsTypeConverter))/*, UnitsAttribute(1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,false)*/]
                       virtual void set(double value);
               }
               property double Z {
                       virtual double get() { return m_pIwVector3d->z; }
                       [NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
                               Description("Z coordinate of vector"),
                               TypeConverter(__typeof(UnitsTypeConverter))/*, UnitsAttribute(1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,false)*/]
                       virtual void set(double value);
               }
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

	public ref class Vector3dConverter: public Utilities::PropertiesDeluxeTypeConverter
	{
	public:
		bool CanConvertFrom(ITypeDescriptorContext *context, Type *sourceType);
		System::Object^  ConvertFrom(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value);
		bool CanConvertTo(ITypeDescriptorContext *context, Type *destinationType);
		System::Object^  ConvertTo(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value, Type *destinationType);
		bool GetPropertiesSupported(ITypeDescriptorContext *context);
		PropertyDescriptorCollection* GetProperties(ITypeDescriptorContext* context, Object* value, Attribute* attributes[]);
	};
}
