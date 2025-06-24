#pragma once
#include "smobject.h"
using namespace System::ComponentModel;
using namespace VEDM::Windows;
using namespace Utilities;

//using PEHoops::MVO::Transformation;
//using HC::KEY;
//using HC::POINT;

namespace PESMLIB
{
	[TypeConverterAttribute(__typeof(Utilities::PropertiesDeluxeTypeConverter))]
	public ref class NurbsSurface : public SMObject, public VEDM::Windows::IGeometry
	{
	public:
		NurbsSurface(void);
		NurbsSurface (Context*, XML::XmlElement* pElem);
		virtual ~NurbsSurface(void) { }

		void NurbsSurface::SetCanonical (ULONG, ULONG, const IwTArray<IwPoint3d>&,
			IwBSplineSurfaceForm, const IwTArray<ULONG>&, const IwTArray<ULONG>&,
			const IwTArray<double>&, const IwTArray<double>&, IwKnotType,
			const IwTArray<double> *, const IwExtent2d *);
		void Copy (NurbsSurface^  srcNurbs);
		//void Transform (PEHoops::MVO::Transformation^  oTransformation);
		void InsertGraphics (bool bDrawDetailed, int handle);
		//void RemoveGraphics (KEY keyGeom);
		void Highlight (HC::KEY);
		void UnHighlight (HC::KEY);
		bool ComputeBoundingBox (HC::NL_POINT^  ptMin, HC::NL_POINT^  ptMax);
		XML::XmlElement^  GetXmlElement (int iFaceOffset) { return m_pXMLElem; }
		System::Object * GetReferencableObject ();

      // Object Overridables
		bool Equals (System::Object^ obj);

      // IComparable interface
		int CompareTo (System::Object^ );

		// Properties
		[Category("Organization"), Description("Order of NURB surface in u-direction"),
		DisplayName("Order in U"),PropertyOrder(0)]
		__property int get_UOrder();
		[Category("Organization"), Description("Order of NURB surface in v-direction"),
		DisplayName("Order in V"),PropertyOrder(1)]
		__property int get_VOrder();
		[Category("Organization"), Description("Number of rows in control net."),
		DisplayName("Row Count"),PropertyOrder(2)]
		__property int get_RowCount();
		[Category("Organization"), Description("Number of columns in control net."),
		DisplayName("Column Count"),PropertyOrder(3)]
		__property int get_ColCount();

	protected private:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	public private:
		virtual void AttachIwObj (Context *pContext, IwObject *pIwObj);
	};
}