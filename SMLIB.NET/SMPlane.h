#pragma once
#include "smobject.h"
#include "smvector3d.h"
#include "smnurbssurface.h"
#include "axis2placement.h"

namespace PESMLIB
{
	__gc public class Plane : public SMObject, public VEDM::Windows::IGeometry
	{
	public:
		Plane(void);
		Plane(Context __gc * oContext, XML::XmlElement __gc * pElem);
		virtual ~Plane(void) { }

		void SetCanonical (Vector3d* origin, Vector3d* normal);
		void SetCanonical (Axis2Placement __gc *axis2p);
		void GetCanonical (Vector3d __gc * oOrigin, Vector3d __gc * oNormal);
		void GetCanonical (Axis2Placement __gc * axis2p);
		void Copy (Plane __gc * srcPlane);
      NurbsSurface __gc * GetNurbsSurface ();

      // Object Overridables
		bool Equals (System::Object __gc *obj);

      // IComparable interface
		int CompareTo (System::Object __gc *);

      // IGeometry interface
      //void Transform (PEHoops::MVO::Transformation __gc * oTransformation);
   	bool ComputeBoundingBox (HC::NL_POINT __gc * ptMin, HC::NL_POINT __gc * ptMax);
   	void Highlight (HC::KEY keySeg);
	   void UnHighlight (HC::KEY keySeg);
	   System::Object __gc * GetReferencableObject ();
   	void InsertGraphics (bool bDrawDetailed, int handle);
   	//void RemoveGraphics (KEY keyGeom);
		XML::XmlElement __gc * GetXmlElement (int iFaceOffset) { return m_pXMLElem; }
   
	protected private:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	public private:
		virtual void AttachIwObj (Context *pContext, IwObject *pIwObj);
	};

}