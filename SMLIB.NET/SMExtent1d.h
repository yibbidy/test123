#pragma once
#include "smobject.h"
using namespace System::ComponentModel;
using namespace Utilities;
//using namespace PEHoops::ThreeDGS;

namespace PESMLIB
{
	public ref class Extent1d: public PersistObject
	{
	public:
		Extent1d(void);
		Extent1d(double dMin, double dMax);
		Extent1d(Extent1d &extOther);
		Extent1d(XML::XmlElement* pElem);
		virtual ~Extent1d(void);

		virtual void SetCanonical (double dMin, double dMax);
      void Init ();
      void Union (Extent1d& extOther, Extent1d& extResult);

		static Extent1d^  Parse (System::String^ sExtent);
		System::String^  ToString ();

		// Public properties
      __property virtual double get_Min ();
      [NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
			Description("Minimum value"), FormatStringAttribute("#0.000")]
		__property virtual void set_Min (double dMin) ;
      __property virtual double get_Max ();
		[NotifyParentPropertyAttribute(true), RefreshPropertiesAttribute(RefreshProperties::Repaint),
			Description("Maximum value"), FormatStringAttribute("#0.000")]
		__property virtual void set_Max (double dMax);
//		__event System::EventHandler^ Changed;
	public protected:
		virtual IwExtent1d * ExtractObj ();
		virtual const IwExtent1d *GetIwObj ();

	protected:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	protected:
		IwExtent1d *m_pIwExtent1d;
	};

	public ref class Extent1dConverter: public Utilities::PropertiesDeluxeTypeConverter
	{
	public:
		bool CanConvertFrom(ITypeDescriptorContext *context, Type *sourceType);
		System::Object^  ConvertFrom(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value);
		bool CanConvertTo(ITypeDescriptorContext *context, Type *destinationType);
		System::Object^  ConvertTo(ITypeDescriptorContext *context, System::Globalization::CultureInfo *culture, System::Object *value, Type *destinationType);
		bool GetPropertiesSupported(ITypeDescriptorContext *context);
	};
}
