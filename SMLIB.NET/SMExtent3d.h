#pragma once
#include "smobject.h"
#include "smvector3d.h"
using namespace System::ComponentModel;
using namespace Utilities;
//using namespace PEHoops::ThreeDGS;

namespace PESMLIB
{
	public ref class Extent3d: public PersistObject
	{
	public:
		Extent3d(void);
		Extent3d(double x0, double y0, double z0, double x1, double y1, double z1);
		Extent3d(double* pVec0, double* pVec1);
		Extent3d(Extent3d &extOther);
		Extent3d(XML::XmlElement* pElem);
		virtual ~Extent3d(void);

		virtual void SetCanonical (double x0, double y0, double z0, double x1, double y1, double z1);
		virtual double Diagonal ();
      void Init ();
      void Union (Extent3d& extOther, Extent3d& extResult);

		static Extent3d^  Parse (System::String^ sExtent);
		System::String^  ToString ();

		// Public properties
      __property virtual PESMLIB::Vector3d^  get_Min ();
      [NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
			Description("X coordinate of vector"), FormatStringAttribute("#0.000")]
		__property virtual void set_Min (Vector3d^ pVec) ;
      __property virtual PESMLIB::Vector3d^  get_Max ();
		[NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
			Description("Y coordinate of vector"), FormatStringAttribute("#0.000")]
		__property virtual void set_Max (Vector3d^ pVec);
		__event System::EventHandler *Changed;
	public protected:
		virtual IwExtent3d * ExtractObj ();
		virtual const IwExtent3d *GetIwObj ();

	protected:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	protected:
		IwExtent3d *m_pIwExtent3d;
//		XML::XmlElement* m_pXMLElem;
	};

	public ref class Extent3dConverter: public Utilities::PropertiesDeluxeTypeConverter
	{
	public:
		bool CanConvertFrom(ITypeDescriptorContext *context, Type *sourceType);
		System::Object^  ConvertFrom(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value);
		bool CanConvertTo(ITypeDescriptorContext *context, Type *destinationType);
		System::Object^  ConvertTo(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value, Type *destinationType);
		bool GetPropertiesSupported(ITypeDescriptorContext *context);
	};
}
