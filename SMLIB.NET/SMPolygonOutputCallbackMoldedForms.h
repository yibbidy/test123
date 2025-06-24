#pragma once

#include "iwgfx_extern.h"
#include "iwtslib_all.h"
#include <map>
#include <vector>
#include <set>

namespace PESMLIB
{	
	__nogc struct SMPoint 
	{
		long m_Id;
		double m_x,m_y,m_z;
	};

	__nogc struct SMElem 
	{
		long m_Id;
		long m_Nd[4];
	};

	__nogc class PolygonOutputCallbackMoldedForms : public IwPolygonOutputCallback
	{
	public:
		ULONG s_lNumberPolygons;
		ULONG m_nodeID;
		ULONG m_hoopsShellFaceLength;
		double m_tol;
		IwAObject *m_pIwObj;
		IwContext *m_pContext;

		std::multimap<double,SMPoint*> m_mMapX;
		std::set<SMElem*> m_setElem;

		PolygonOutputCallbackMoldedForms(void);
		virtual ~PolygonOutputCallbackMoldedForms(void);

		IwStatus OutputPolygon(ULONG s_lPolygonType, ULONG, IwVector3d *, 
			IwVector3d *, IwPoint2d *, IwSurface *);
	private:
		long GetNodeID(const double x,const double y,const double z);
	};
}