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
	__gc public class BrepFaceProxy : public VEDM::Windows::IGeometry, public IComparable
	{
	public:
		BrepFaceProxy(void);
		BrepFaceProxy (PESMLIB::Context __gc *pContext, PESMLIB::Brep __gc *pBrep, long lFaceID);
		virtual ~BrepFaceProxy(void);

		bool Equals (System::Object __gc * obj);

	public:
		double GetArea();
		double GetVolume ();
		Vector3d * GetCentroid();
		Vector3d * GetNormal();
		double GetMyArea();
		double getVertexMin();
		double getVertexMax();
		Vector3d * GetMyCentroid();
		System::Collections::ArrayList __gc * GetEdges();
		// IGeometry implementation
		void InsertGraphics (bool bDrawDetailed, int handle);
		//void RemoveGraphics (HC::KEY keyGeom);
		//void Transform (Transformation * oTransformation);
		bool ComputeBoundingBox (HC::NL_POINT __gc * ptMin, HC::NL_POINT __gc * ptMax);
		XML::XmlElement __gc * GetXmlElement (int  iFaceOffset) { return NULL;};
		void Highlight (HC::KEY);
		void UnHighlight (HC::KEY);
		System::Object __gc * GetReferencableObject ();
		void AddObjectDependency (IPersistentObject __gc *pIPersistentObject);
		System::Collections::ArrayList __gc * GetObjectDependencies ();
		bool IsDependentOn (IPersistentObject __gc *pObj);
		void RemoveObjectDependency (IPersistentObject __gc *pIPersistentObject);
		void SetAttribute (AttributeID, System::Object __gc *, AttributeBehavior);
		System::Object __gc * FindAttribute (AttributeID);
		void RemoveAttribute (AttributeID);
		System::Collections::ArrayList __gc * GetRegions ();
		//      System::Object __gc * GetSurface ();
		bool HasSameSurface (BrepFaceProxy __gc *srcFace);
		System::Object __gc * GetSurfaceCopy ();
		void ComputeProperties (double __gc& dArea, double __gc& dVolume, System::Collections::ArrayList __gc *arrMoments);
		void ComputeMyProperties (double __gc& dArea, double __gc& dVolume, System::Collections::ArrayList __gc *arrMoments);
		Vector3d __gc * DropPoint(Vector3d __gc * ptToDrop, Vector3d __gc *vecNormal);
		Vector3d __gc * DropPointAlongLine(Vector3d __gc *ptToDrop, Vector3d __gc *vecDropDirection, Vector3d __gc *vecNormal);

		//Molded Forms
		void AssignPropertyAttribute (String __gc *sName);

		// IComparable interface
		int CompareTo(System::Object __gc *obj);

		[Category("Identity"), Description("Face identifier"), PropertyOrder(0),
			DisplayName("Face ID"), ReadOnly(true)]
		__property long get_FaceID () { return m_lFaceID; };
	//	[ReadOnly(true)]
		__property void set_FaceID (long value) { m_lFaceID = value; };
		[Browsable(false)]
		__property PESMLIB::Brep __gc * get_Brep () { return m_pBrep; };
		__property void set_Brep (PESMLIB::Brep __gc *pBrep) { m_pBrep = pBrep; };
		[Category("Properties"), Description("Area of face, m^2"), PropertyOrder(1),
			TypeConverter(__typeof(UnitsTypeConverter)), UnitsAttribute(Units::UnitBasis::Category::AREA)]
		__property double get_Area() { return GetArea(); };
		[Category("Properties"), Description("Centroid of face, {m,m,m}"), PropertyOrder(2),
			TypeConverter(__typeof(Vector3dConverter)),UnitsAttribute(Units::UnitBasis::Category::LENGTH)]
		__property PESMLIB::Vector3d __gc * get_Centroid() { return GetCentroid(); };
		[Category("Properties"), Description("Delta Volume of face, m^3"), PropertyOrder(3),
			TypeConverter(__typeof(NumericFormatConverter)),FormatString("#0.000")]
		[Browsable(false)]
		__property double get_Volume() { return GetVolume();};
		[Browsable(false)]
		__property System::String* get_PropertyName ()
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
		PESMLIB::Brep __gc *m_pBrep;
		PESMLIB::Context __gc *m_pContext;
	};
}
