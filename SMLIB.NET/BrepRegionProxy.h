#pragma once
#include "SMObject.h"
#include "SMBrep.h"

using namespace System::IO;
using namespace System::Collections;
using namespace System::Text;
using namespace System::ComponentModel;
//using namespace PEHoops::Geometry;
//using namespace PEHoops::ThreeDGS;
using namespace Utilities;
//using PEHoops::ThreeDGS::HC::POINT;
//using PEHoops::MVO::Transformation;
//using HC::KEY;
namespace XML = System::Xml;


namespace PESMLIB
{
	[TypeConverterAttribute(__typeof(Utilities::PropertiesDeluxeTypeConverter))]
	public ref class BrepRegionProxy : public VEDM::Windows::IGeometry, public IComparable
	{
	public:
		BrepRegionProxy(void);
		BrepRegionProxy (Context^ pContext, Brep^ pBrep, long lRegionID);
		virtual ~BrepRegionProxy(void);

		bool Equals (System::Object^  obj);

	public:
		System::Collections::ArrayList^  GetFaces();
		void SetAttribute (AttributeID ulAttributeID, System::Object^ obj, AttributeBehavior behavior);
		System::Object^  FindAttribute (AttributeID ulAttributeID);
		bool IsInfiniteRegion () { return (m_lRegionID == -1);}

		// Dependency functions
		void AddObjectDependency (IPersistentObject^ pObj);
		void RemoveObjectDependency (IPersistentObject^ pObj);
		System::Collections::ArrayList^  GetObjectDependencies ();
		bool IsDependentOn (IPersistentObject^ pObj);

		// IGeometry implementation
		void InsertGraphics (bool bDrawDetailed, int handle);
		//void RemoveGraphics (HC::KEY keyGeom);
		//void Transform (Transformation * oTransformation);
		bool ComputeBoundingBox (HC::NL_POINT^  ptMin, HC::NL_POINT^  ptMax);
		XML::XmlElement^  GetXmlElement (int  iFaceOffset) { return NULL;};
		void Highlight (HC::KEY);
		void UnHighlight (HC::KEY);
		System::Object^  GetReferencableObject ();

		// IComparable interface
		int CompareTo(System::Object^ obj);

		bool ComputePreciseProperties (
			double% dArea, double% dVolume, Vector3d^ centroid, ArrayList^ arrMoments);
		bool ComputeProperties (
			double% dArea, double% dVolume, Vector3d^ centroid, ArrayList^ arrMoments);
		void SetProperties();

		bool RayIntersect(PESMLIB::Vector3d^ start, PESMLIB::Vector3d^ dir);

		[Category("Identity"), Description("Region identifier"), 
			PropertyOrder(0), DisplayName("Region ID"), ReadOnly(true)]
		__property long get_RegionID () { return m_lRegionID; };
		//[ReadOnly(true)]
		__property void set_RegionID (long value) { m_lRegionID = value; };
		__property PESMLIB::Brep^  get_Brep () { return m_pBrep; };
		[Browsable(false)]
		__property void set_Brep (PESMLIB::Brep^ pBrep) { m_pBrep = pBrep; };
		[Category("Properties"), Description("Volume of region, m^3"),
			PropertyOrder(1),TypeConverter(__typeof(UnitsTypeConverter)), UnitsAttribute(Units::UnitBasis::Category::VOLUME)]
		__property double get_Volume();
		[Category("Properties"), Description("Surface area of region, m^2"),
			PropertyOrder(3), DisplayName("Surface Area"),TypeConverter(__typeof(UnitsTypeConverter)), UnitsAttribute(Units::UnitBasis::Category::AREA)]
		__property double get_Area();
		[Category("Properties"), Description("Centroid of region, {m,m,m}"),PropertyOrder(2),
			TypeConverter(__typeof(Vector3dConverter)), UnitsAttribute(Units::UnitBasis::Category::LENGTH)]
		__property PESMLIB::Vector3d^  get_Centroid();

	private:
		long m_lRegionID; // a value of -1 signifies the infinite region
		PESMLIB::Brep^ m_pBrep;
		PESMLIB::Context^ m_pContext;
		IwRegion * GetIwRegion ();
		PESMLIB::Vector3d^ m_pvecCentroid;
		double m_dVolume;
		double m_dArea;
		System::Collections::ArrayList^  m_arrProperties;
	};
}