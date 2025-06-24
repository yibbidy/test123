#pragma once

#include "iwgfx_extern.h"
#include "iwtslib_all.h"

namespace PESMLIB
{
	__nogc class PolygonOutputCallback : public IwPolygonOutputCallback
	{
	public:
		ULONG s_lNumberTriangles;
		IwAObject *m_pIwObj;
		IwContext *m_pContext;
		int m_hkShell;

		PolygonOutputCallback(void);
		virtual ~PolygonOutputCallback(void);

		IwStatus OutputMesh(IwTArray<ULONG>& , IwTArray<ULONG> & ,
			IwTArray<IwPoint3d> &, IwTArray<IwVector3d>&, IwTArray<IwPoint2d>&, 
			IwSurface *, IwFace *);
		IwStatus OutputPolygon(ULONG s_lPolygonType, ULONG, IwPoint3d *, 
			IwVector3d *, IwPoint2d *, IwSurface *, IwFace *);
		
		int GetShellKey()
		{
			return m_hkShell;
		}
	};
}