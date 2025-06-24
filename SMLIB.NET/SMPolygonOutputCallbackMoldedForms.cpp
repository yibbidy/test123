#include "StdAfx.h"
#include "SMPolygonOutputCallbackMoldedForms.h"

namespace PESMLIB
{
	PolygonOutputCallbackMoldedForms::PolygonOutputCallbackMoldedForms(void)
	{
	  s_lNumberPolygons = 0;
	  m_nodeID=0;
	  m_hoopsShellFaceLength=0;
	  m_tol=1.0E-6;
      m_pIwObj = 0;
      m_pContext = 0;
	}

	PolygonOutputCallbackMoldedForms::~PolygonOutputCallbackMoldedForms(void)
	{
		std::multimap<double,SMPoint*>::iterator pos;
		for (pos  = m_mMapX.begin(); pos != m_mMapX.end();pos++)
		{
			SMPoint* pNode = pos->second;
			delete pNode;
		}
		m_mMapX.erase(m_mMapX.begin(),m_mMapX.end());

		std::set<SMElem*>::iterator it;
		for (it  = m_setElem.begin(); it != m_setElem.end();it++)
		{
			SMElem* pElm = *it;
			delete pElm;
		}
		m_setElem.erase(m_setElem.begin(),m_setElem.end());
	}

	IwStatus PolygonOutputCallbackMoldedForms::OutputPolygon(ULONG s_lPolygonType, ULONG lNumPoints, IwVector3d *sPoints, IwVector3d *sNormals, 
		IwPoint2d *sUVPoints, IwSurface *pSurface)
	{
	// called by the smlib tess object to output Tris, Quads, Tristrips and TriFans
	// using the hoops shell entity
	// The precomputed normals are passed to the shell for Tristrips and TriFans

		if (m_eOutputType == IW_PO_TRIANGLES || m_eOutputType == IW_PO_QUADRALATERALS)
		{
			// got a tri or quad
			if (lNumPoints>=3)
			{
				SMElem* pElm=new SMElem;
				pElm->m_Nd[3]=-1;
				for (int i=0; i<(int)lNumPoints; i++)
				{
					pElm->m_Nd[i] = GetNodeID(sPoints[i].x,sPoints[i].y,sPoints[i].z);					
				}
				s_lNumberPolygons++;
				pElm->m_Id=s_lNumberPolygons;
				m_setElem.insert(pElm);
				m_hoopsShellFaceLength=m_hoopsShellFaceLength+(int)lNumPoints+1;
			}
		}
		return IW_SUCCESS;
	}
	long PolygonOutputCallbackMoldedForms::GetNodeID(const double x,const double y,const double z)
	{
		long ID=-1;
		std::multimap<double,SMPoint*>::iterator pos;
		pos=m_mMapX.find(x);
		bool bFound=false;
		if (pos != m_mMapX.end()) // found
		{
			for (pos=m_mMapX.lower_bound(x);pos!=m_mMapX.upper_bound(x) && !bFound;++pos)
			{
				SMPoint* pNode=pos->second;
				if (   fabs(pNode->m_y-y)<=m_tol
					&& fabs(pNode->m_z-z)<=m_tol )
				{
					bFound=true;
					ID=pNode->m_Id;
				}
			}
		}
		if (!bFound)
		{
			SMPoint* pNode=new SMPoint;
			ID=m_nodeID++;
			pNode->m_Id=ID;
			pNode->m_x=x;
			pNode->m_y=y;
			pNode->m_z=z;
			m_mMapX.insert(std::make_pair(x,pNode));
		}
		//ASSERT(ID>=0);
		return ID;
	}

}