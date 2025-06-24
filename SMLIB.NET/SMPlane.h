#pragma once
#include "smobject.h"
#include "smvector3d.h"
#include "smnurbssurface.h"
#include "axis2placement.h"

namespace PESMLIB
{
	public ref class Plane : public SMObject, public VEDM::Windows::IGeometry
	{
	public:
		Plane(void);
		Plane(Context^  oContext, XML::XmlElement^  pElem);
		virtual ~Plane(void) { }

		void SetCanonical (Vector3d* origin, Vector3d* normal);
		void SetCanonical (Axis2Placement^ axis2p);
		void GetCanonical (Vector3d^  oOrigin, Vector3d^  oNormal);
		void GetCanonical (Axis2Placement^  axis2p);
		void Copy (Plane^  srcPlane);
      NurbsSurface^  GetNurbsSurface ();

      // Object Overridables
		bool Equals (System::Object^ obj);

      // IComparable interface
		int CompareTo (System::Object^ );

      // IGeometry interface
      //void Transform (PEHoops::MVO::Transformation^  oTransformation);
   	bool ComputeBoundingBox (HC::NL_POINT^  ptMin, HC::NL_POINT^  ptMax);
   	void Highlight (HC::KEY keySeg);
	   void UnHighlight (HC::KEY keySeg);
	   System::Object^  GetReferencableObject ();
   	void InsertGraphics (bool bDrawDetailed, int handle);
   	//void RemoveGraphics (KEY keyGeom);
		XML::XmlElement^  GetXmlElement (int iFaceOffset) { return m_pXMLElem; }
   
	protected private:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	public private:
		virtual void AttachIwObj (Context *pContext, IwObject *pIwObj);
	};

}