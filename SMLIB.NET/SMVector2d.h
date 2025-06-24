#pragma once
#include "smobject.h"
using namespace System::ComponentModel;
using namespace Utilities;
using namespace Units;
//using namespace PEHoops::ThreeDGS;
using namespace VEDM::Windows;

namespace PESMLIB
{
	__gc public class Vector2d: public PersistObject
	{
	public:
		Vector2d(void);
		Vector2d(double x, double y);
		Vector2d(double* pVec);
		Vector2d(Vector2d &vecOther);
		Vector2d(XML::XmlElement* pElem);
		virtual ~Vector2d(void);

		// Managed operators
		static Vector2d __gc * op_Addition(Vector2d *vec1, Vector2d *vec2);
		static Vector2d __gc * op_Subtraction(Vector2d *vec1, Vector2d *vec2);
		static Vector2d __gc * op_Assign(Vector2d *vec1, Vector2d *pt2);

		virtual HC::NL_POINT GetHoopsPoint();
		virtual void SetCanonical (double x, double y);
		virtual double AngleBetween(Vector2d &vecOther);
		virtual double DistanceBetween(Vector2d &vecOther);
		virtual double Dot(Vector2d &vecOther);
		virtual double Length();
		virtual void Unitize();
		virtual void Scale(double dScale);

		static Vector2d __gc * Parse (System::String __gc *sVector);
		System::String __gc * ToString ();

		// Public properties
		__property virtual double get_X () { return m_pIwVector2d->x; };
		[NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
			Description("X coordinate of vector"), 
			TypeConverter(__typeof(UnitsTypeConverter))/*, UnitsAttribute(1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,false)*/]
		__property virtual void set_X (double value) ;
		__property virtual double get_Y () { return m_pIwVector2d->y; };
		[NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
			Description("Y coordinate of vector"), 
			TypeConverter(__typeof(UnitsTypeConverter))/*, UnitsAttribute(1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,false)*/]
		__property virtual void set_Y (double value);
		__event System::EventHandler *Changed;
	public protected:
		virtual IwVector2d * ExtractObj ();
		virtual const IwVector2d *GetIwObj ();

	protected:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	protected:
		IwVector2d *m_pIwVector2d;
		XML::XmlElement* m_pXMLElem;
	};

	__gc public class Vector2dConverter: public Utilities::PropertiesDeluxeTypeConverter
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
