#pragma once
#include "smobject.h"
#include "smvector3d.h"

namespace PESMLIB
{
	public ref class Axis2Placement :
	public PersistObject
	{
	public:
		Axis2Placement(void);
		void GetCannonical(Vector3d *pvecOrigin, Vector3d *pvecXaxis, Vector3d *pvecYaxis);
		void SetCannonical(Vector3d *pvecOrigin, Vector3d *pvecXaxis, Vector3d *pvecYaxis);
		Vector3d* GetOrigin(void);
		Vector3d* GetXAxis(void);
		Vector3d* GetYAxis(void);
		Vector3d* GetZAxis(void);
		Axis2Placement* Invert(void);
		Vector3d* MirrorPoint(Vector3d *pPointToMirror);
		void RotateAboutAxis(double dAngRadians, Vector3d *pAxis);
		void RotateAboutAxisAtPoint(double dAngRadians, Vector3d *pCenter, Vector3d *pAxis);
		Axis2Placement* TransformAxis2Placement(Axis2Placement *pInput);
		Vector3d* TransformPoint(Vector3d *pvecInput);
		Vector3d* TransformVector(Vector3d *pvecInput);
		void Translate(Vector3d *pvecTranslation);
		virtual ~Axis2Placement(void);
	public protected:
		virtual IwAxis2Placement *ExtractObj ();
		virtual const IwAxis2Placement *GetIwObj ();

	protected:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	protected:
		IwAxis2Placement *m_pIwAxis2Placement;
		XML::XmlElement* m_pXMLElem;
	};
}