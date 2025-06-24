#pragma once

#include "SMObject.h"
#include "SMVector3d.h"
#include "SMBrep.h"

using namespace System::IO;
using namespace System::Collections;
using namespace System::ComponentModel;
using namespace System::Text;
//using namespace PEHoops::Geometry;
//using namespace PEHoops::ThreeDGS;
//using PEHoops::ThreeDGS::HC::POINT;
//using PEHoops::MVO::Transformation;
//using HC::KEY;
namespace XML = System::Xml;

namespace PESMLIB
{

	public ref class BrepEdgeProxy: public VEDM::Windows::IGeometry, public IComparable
	{
	public:
		BrepEdgeProxy(void);
		BrepEdgeProxy (PESMLIB::Context^ pContext, PESMLIB::Brep^ pBrep, long lEdgeID);
		~BrepEdgeProxy(void);

		bool Equals (System::Object^  obj);

	public:
		double GetLength();
		Vector3d * GetMidPoint();
		Vector3d * GetPoint(double percentage);
		bool IsLine();
		System::Collections::ArrayList^  GetFaces();
		// IGeometry implementation
		void InsertGraphics (bool bDrawDetailed, int handle);
//		void RemoveGraphics (HC::KEY keyGeom);
		//void Transform (Transformation * oTransformation);
		bool ComputeBoundingBox (HC::NL_POINT^  ptMin, HC::NL_POINT^  ptMax);
		XML::XmlElement^  GetXmlElement (int  iFaceOffset) { return NULL;};
		void Highlight (HC::KEY);
		void UnHighlight (HC::KEY);
		System::Object^  FindAttribute (AttributeID ulAttributeID);
		System::Object^  GetReferencableObject ();
		void AddObjectDependency (IPersistentObject^ pIPersistentObject);
		System::Collections::ArrayList^  GetObjectDependencies ();
		bool IsDependentOn (IPersistentObject^ pObj);
		void RemoveObjectDependency (IPersistentObject^ pIPersistentObject);
		void ComputeProperties(double *pdLength);

		// IComparable interface
		int CompareTo(System::Object^ obj);

		__property long get_EdgeID () { return m_lEdgeID; };
		__property void set_EdgeID (long value) { m_lEdgeID = value; };
		[BrowsableAttribute(false)]
		__property PESMLIB::Brep^  get_Brep () { return m_pBrep; };
		__property void set_Brep (PESMLIB::Brep^ pBrep) { m_pBrep = pBrep; };
		__property int get_EdgeUseCount ();
		[Browsable(false)]
		__property bool get_IsManifold() { return (GetIwEdge()->IsManifold() != 0); };
		[Browsable(false)]
		__property bool get_IsLamina(){return (GetIwEdge()->IsLamina() != 0); };
		[TypeConverter(__typeof(UnitsTypeConverter)), UnitsAttribute(Units::UnitBasis::Category::LENGTH)]
		__property double get_EdgeLength(){return GetLength(); };

	private:
		IwEdge * GetIwEdge ();
		long m_lEdgeID;
		PESMLIB::Brep^ m_pBrep;
		PESMLIB::Context^ m_pContext;
	};
}
