#pragma once
#include "smobject.h"
#include "smvector3d.h"

namespace PESMLIB
{
	__gc public class Point3d : public Vector3d, public IPersistentObject
	{
	//public:
	//	Point3d(void);
	//	//Point3d(double dX, double dY, double dZ);
	//	//Point3d(XML::XmlElement* pElem);
	//	virtual ~Point3d(void);
	};
}