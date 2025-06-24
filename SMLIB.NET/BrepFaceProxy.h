#pragma once

#include "SMObject.h"
#include "SMVector3d.h"
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
	[TypeConverterAttribute(__typeof(PropertiesDeluxeTypeConverter))]
	public ref class BrepFaceProxy : public VEDM::Windows::IGeometry, public IComparable
	{
	public:
		BrepFaceProxy(void);
		BrepFaceProxy (PESMLIB::Context^ pContext, PESMLIB::Brep^ pBrep, long lFaceID);
		virtual ~BrepFaceProxy(void);

		bool Equals (System::Object^  obj);

	public:
		double GetArea();
		double GetVolume ();
		Vector3d * GetCentroid();
		Vector3d * GetNormal();
		double GetMyArea();
		double getVertexMin();
		double getVertexMax();
		Vector3d * GetMyCentroid();
		System::Collections::ArrayList^  GetEdges();
		// IGeometry implementation
		void InsertGraphics (bool bDrawDetailed, int handle);
		//void RemoveGraphics (HC::KEY keyGeom);
		//void Transform (Transformation * oTransformation);
		bool ComputeBoundingBox (HC::NL_POINT^  ptMin, HC::NL_POINT^  ptMax);
		XML::XmlElement^  GetXmlElement (int  iFaceOffset) { return NULL;};
		void Highlight (HC::KEY);
		void UnHighlight (HC::KEY);
		System::Object^  GetReferencableObject ();
		void AddObjectDependency (IPersistentObject^ pIPersistentObject);
		System::Collections::ArrayList^  GetObjectDependencies ();
		bool IsDependentOn (IPersistentObject^ pObj);
		void RemoveObjectDependency (IPersistentObject^ pIPersistentObject);
		void SetAttribute (AttributeID, System::Object^ , AttributeBehavior);
		System::Object^  FindAttribute (AttributeID);
		void RemoveAttribute (AttributeID);
		System::Collections::ArrayList^  GetRegions ();
		//      System::Object^  GetSurface ();
		bool HasSameSurface (BrepFaceProxy^ srcFace);
		System::Object^  GetSurfaceCopy ();
		void ComputeProperties (double% dArea, double% dVolume, System::Collections::ArrayList^ arrMoments);
		void ComputeMyProperties (double% dArea, double% dVolume, System::Collections::ArrayList^ arrMoments);
		Vector3d^  DropPoint(Vector3d^  ptToDrop, Vector3d^ vecNormal);
		Vector3d^  DropPointAlongLine(Vector3d^ ptToDrop, Vector3d^ vecDropDirection, Vector3d^ vecNormal);

		//Molded Forms
		void AssignPropertyAttribute (String^ sName);

		// IComparable interface
		int CompareTo(System::Object^ obj);

		[Category("Identity"), Description("Face identifier"), PropertyOrder(0),
			DisplayName("Face ID"), ReadOnly(true)]
		property long FaceID { long get() { return m_lFaceID; }
	//	[ReadOnly(true)]
		 void set(long value) { m_lFaceID = value; } }
		[Browsable(false)]
		property PESMLIB::Brep^ Brep { PESMLIB::Brep^ get() { return m_pBrep; }
		 void set(PESMLIB::Brep^ pBrep) { m_pBrep = pBrep; } }
		[Category("Properties"), Description("Area of face, m^2"), PropertyOrder(1),
			TypeConverter(__typeof(UnitsTypeConverter)), UnitsAttribute(Units::UnitBasis::Category::AREA)]
		property double Area { double get() { return GetArea(); } }
		[Category("Properties"), Description("Centroid of face, {m,m,m}"), PropertyOrder(2),
			TypeConverter(__typeof(Vector3dConverter)),UnitsAttribute(Units::UnitBasis::Category::LENGTH)]
		property PESMLIB::Vector3d^ Centroid { PESMLIB::Vector3d^ get() { return GetCentroid(); } }
		[Category("Properties"), Description("Delta Volume of face, m^3"), PropertyOrder(3),
			TypeConverter(__typeof(NumericFormatConverter)),FormatString("#0.000")]
		[Browsable(false)]
		property double Volume { double get() { return GetVolume(); } }
		[Browsable(false)]
		System::String^ get_PropertyName()
		{
			try
			{
				IwFace * pIwFace=GetIwFace ();
				if (pIwFace)
				{
					IwAttribute *pExistingAttribute = pIwFace->FindAttribute (AttributeID_PROPERTY);
					if (pExistingAttribute && pExistingAttribute->GetNumCharacterElements()>0)//GetNumLongElements () > 0)
					{
						const char *lAttributes = pExistingAttribute->GetCharacterElementsAddress ();
						return lAttributes;
					}
				}
				return get_Brep()->get_PropertyName();
			}
			catch (...) {}
			return "";
		}
		IwFace * GetIwFace ();
	private:
		long m_lFaceID;
		double m_dArea;
		PESMLIB::Brep^ m_pBrep;
		PESMLIB::Context^ m_pContext;
	};
}
