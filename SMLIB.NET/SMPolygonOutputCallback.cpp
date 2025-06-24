#include "stdafx.h"
#include "smpolygonoutputcallback.h"
#include "SMObject.h"

using namespace VEDM::Windows;

namespace PESMLIB
{
	PolygonOutputCallback::PolygonOutputCallback(void)
	{
		s_lNumberTriangles = 0;
		m_pIwObj = 0;
		m_pContext = 0;
		m_hkShell = 0;
	}

	PolygonOutputCallback::~PolygonOutputCallback(void)
	{
	}

	IwStatus PolygonOutputCallback::OutputPolygon(ULONG s_lPolygonType, ULONG lNumPoints, IwPoint3d *sPoints,
		IwVector3d *sNormals, IwPoint2d *sUVPoints, IwSurface *pSurface, IwFace *pFace)
	{
		// called by the smlib tess object to output Tris, Quads, Tristrips and TriFans
		// using the hoops shell entity
		// The precomputed normals are passed to the shell for Tristrips and TriFans

		if (m_eOutputType == IW_PO_TRIANGLES || m_eOutputType == IW_PO_QUADRALATERALS)
		{
			// got a tri or quad

			HC_POINT hpt[100];
			for (int i=0; i<(int)lNumPoints; i++)
			{
				hpt[i].x = (float)sPoints[i].x;
				hpt[i].y = (float)sPoints[i].y;
				hpt[i].z = (float)sPoints[i].z;
			}

			//HC_Insert_Polygon(lNumPoints, hpt);

			s_lNumberTriangles++;
		}
		else if (m_eOutputType == IW_PO_TRIANGLE_STRIPS)
		{
			//got a tristrip
			int polygon_count = lNumPoints-2;
			int total_polygon_points = lNumPoints;
			HC_POINT	*shell_points,*normals;
			int		*face_list;

			// create arrays to hold hoops shell input data
			shell_points = (HC_POINT *) malloc (total_polygon_points * sizeof (HC_POINT));
			normals = (HC_POINT *) malloc (total_polygon_points * sizeof (HC_POINT));

			// copy data from smlib format to hoops format for points and normals
			// I am using register variables here to try and speed up the loops.
			for (register int i=0; i<total_polygon_points; i++)
			{
				shell_points[i].x = (float)sPoints[i].x;
				shell_points[i].y = (float)sPoints[i].y;
				shell_points[i].z = (float)sPoints[i].z;
				if (m_bGenerateNormals)
				{
					normals[i].x = (float)sNormals[i].x;
					normals[i].y = (float)sNormals[i].y;
					normals[i].z = (float)sNormals[i].z;
				}
			}

			// compute the size of the facelist array and malloc memory for it.
			int face_list_length = 4*polygon_count;
			face_list = (int *) malloc (face_list_length * sizeof (int));

			int indx1=0,indx2=1,indx3=2;

			// create hoops face list from the crazy storage format from SMlib
			// I am using register variables here to try and speed up the loops.
			register int count=0,jump=0;;
			for (register int i=0; i<polygon_count; i++)
			{
				face_list[count++] = 3;
				face_list[count++] = indx1+i;
				face_list[count++] = indx2+i;
				face_list[count++] = indx3+i;
			}

			//	WriteFaceList(shell_points, total_polygon_points, face_list, polygon_count);

			// create hoops shell entity 
			//HC_KEY tristrip = HC_KInsert_Shell_By_Tristrips (total_polygon_points, shell_points, 
			//	count, face_list, 
			//	0, NULL);

			// Add normals to the shell entity
			//if (m_bGenerateNormals)
			//	HC_MSet_Vertex_Normals (tristrip, 0, total_polygon_points, normals);

			// clean up malloc'd memory		
			if (shell_points)
				free(shell_points);
			if (normals)
				free(normals);
			if (face_list)
				free(face_list);		

		}
		else if (m_eOutputType == IW_PO_TRIANGLE_FANS)
		{

			int polygon_count = lNumPoints-2;
			int total_polygon_points = lNumPoints;
			HC_POINT	*shell_points,*normals;
			int		*face_list;

			// create arrays to hold hoops shell input data
			shell_points = (HC_POINT *) malloc (total_polygon_points * sizeof (HC_POINT));
			normals = (HC_POINT *) malloc (total_polygon_points * sizeof (HC_POINT));

			// copy data from smlib format to hoops format for points and normals
			// I am using register variables here to try and speed up the loops.
			for (register int i=0; i<total_polygon_points; i++)
			{
				shell_points[i].x = (float)sPoints[i].x;
				shell_points[i].y = (float)sPoints[i].y;
				shell_points[i].z = (float)sPoints[i].z;
				if (m_bGenerateNormals)
				{
					normals[i].x = (float)sNormals[i].x;
					normals[i].y = (float)sNormals[i].y;
					normals[i].z = (float)sNormals[i].z;
				}
			}

			// compute the size of the facelist array and malloc memory for it.
			int face_list_length = 4*polygon_count;
			face_list = (int *) malloc (face_list_length * sizeof (int));

			int indx1=0,indx2=1,indx3=2;

			// create hoops face list from the crazy storage format from SMlib
			// I am using register variables here to try and speed up the loops.
			register int count=0,jump=0;;
			for (register int i=0; i<polygon_count; i++)
			{
				face_list[count++] = 3;
				face_list[count++] = indx1;
				face_list[count++] = indx2+i;
				face_list[count++] = indx3+i;
			}

			//		WriteFaceList(shell_points, total_polygon_points, face_list, polygon_count);

			// create hoops shell entity 
			//HC_KEY trifans = HC_KInsert_Shell_By_Tristrips (total_polygon_points, shell_points, 
			//	count, face_list, 
			//	0, NULL);

			// Add normals to the shell entity		
			//if (m_bGenerateNormals)
			//	HC_MSet_Vertex_Normals (trifans, 0, total_polygon_points, normals);

			// clean up malloc'd memory		
			if (shell_points)
				free(shell_points);
			if (normals)
				free(normals);
			if (face_list)
				free(face_list);		
		}

		return IW_SUCCESS;
	}

	IwStatus PolygonOutputCallback::OutputMesh(IwTArray<ULONG>& rPolygonVertexCounts, IwTArray<ULONG> & rPolygonVertexIndices, 
		IwTArray<IwPoint3d> & rPolygon3DPoints, IwTArray<IwVector3d> & rSurfaceNormals, IwTArray<IwPoint2d> & rPolygonUVPoints, 
		IwSurface *pSurface, IwFace *pFace)
	{
		// called by the smlib tess object to output meshes
		// using the hoops shell entity
		// The precomputed normals are passed to the shell
		if (m_eOutputType == IW_PO_TRIANGLE_MESH)
		{
			int polygon_count = rPolygonVertexCounts.GetSize();
			int total_polygon_points = rPolygon3DPoints.GetSize();
			int face_list __gc[];
			float shell_points __gc[];
			float normals __gc[];

			// create arrays to hold hoops shell input data
			shell_points = new float __gc[3*total_polygon_points];
			normals = new float __gc[3*total_polygon_points];

			// copy data from smlib format to hoops format for points and normals
			// I am using register variables here to try and speed up the loops.
			for (register int i=0; i<total_polygon_points; i++)
			{
				shell_points[3*i] = (float)rPolygon3DPoints[i].x;
				shell_points[3*i+1] = (float)rPolygon3DPoints[i].y;
				shell_points[3*i+2] = (float)rPolygon3DPoints[i].z;
				if (m_bGenerateNormals)
				{
					normals[3*i] = (float)rSurfaceNormals[i].x;
					normals[3*i+1] = (float)rSurfaceNormals[i].y;
					normals[3*i+2] = (float)rSurfaceNormals[i].z;
				}
			}

			// compute the size of the facelist array and malloc memory for it.
			int face_list_length = 4*polygon_count;
			//face_list = (int *) malloc (face_list_length * sizeof (int));
			face_list = new int __gc[face_list_length];
			// create hoops face list from the crazy storage format from SMlib
			// I am using register variables here to try and speed up the loops.
			register int count=0,jump=0;;
			for (register int i=0; i<polygon_count; i++)
			{
				face_list[count++] = rPolygonVertexCounts[i];
				for (int j=0; j<(int)rPolygonVertexCounts[i]; j++)
				{
					face_list[count++] = rPolygonVertexIndices[j+jump];
				}
				jump += rPolygonVertexCounts[i];
			}

			// create hoops shell entity 
			//HC_KEY hkShell;
			m_hkShell = HD::KInsert_Shell (total_polygon_points, shell_points, face_list_length, face_list);

			// Add normals to the shell entity
			//if (m_bGenerateNormals)
			//	HC_MSet_Vertex_Normals (m_hkShell, 0, total_polygon_points, normals);

			// Set the hoops key as an attribute on the face. Remove any existing attribute.

			if (NULL != m_pIwObj && NULL != m_pContext)
			{
				IwAttribute *pAttribute = m_pIwObj->FindAttribute (AttributeID_HKEY);
				if (pAttribute != NULL)
					m_pIwObj->RemoveAttribute (pAttribute, FALSE);
				IwLongAttribute *pLongAttribute = new (*m_pContext) IwLongAttribute (
					AttributeID_HKEY, m_hkShell, IW_AB_COPY);
				m_pIwObj->AddAttribute (pLongAttribute);
			}

			// clean up malloc'd memory
			//if (shell_points)
			//	free(shell_points);
			//if (normals)
			//	free(normals);
			//if (face_list)
			//	free(face_list);
		}

		return IW_SUCCESS;
	}


}