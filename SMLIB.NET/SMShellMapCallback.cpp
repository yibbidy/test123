#include "StdAfx.h"
#include "SMObject.h"
#include "smshellmapcallback.h"


namespace PESMLIB
{
	ShellMapCallback::ShellMapCallback(XML::XmlNode* XmlShellsNode)
	{
		m_XmlDoc = XmlShellsNode->OwnerDocument;
		XML::XmlElement* XmlShellElem = m_XmlDoc->CreateElement("SurfShell");
		//		XmlShellElem->SetAttribute("bVisible", System::Convert::ToString(true));
		//		XmlShellElem->SetAttribute("iValidFields", "1");	// PE_SURFPTPROP_POSITION | PE_SURFPTPROP_NORMAL

		XmlShellsNode->AppendChild(XmlShellElem);

		m_XmlShellPtsElem = m_XmlDoc->CreateElement("ShellPoints");
		XmlShellElem->AppendChild(m_XmlShellPtsElem);
		m_XmlShellFacesElem = m_XmlDoc->CreateElement("ShellFaces");
		XmlShellElem->AppendChild(m_XmlShellFacesElem);
	}

	ShellMapCallback::~ShellMapCallback(void)
	{
	}

	IwStatus ShellMapCallback::OutputMesh(IwTArray<ULONG>& rPolygonVertexCounts,
		IwTArray<ULONG> & rPolygonVertexIndices, 
		IwTArray<IwVector3d> & rPolygon3DPoints, 
		IwTArray<IwVector3d> & rSurfaceNormals, 
		IwTArray<IwPoint2d> & rPolygonUVPoints, 
		IwSurface * pSurface, IwFace *pFace)
	{
		// called by the smlib tess object to output meshes
		// using the hoops shell entity
		// The precomputed normals are passed to the shell

		if (m_eOutputType == IW_PO_TRIANGLE_MESH)
		{
			long nCount = rPolygon3DPoints.GetSize ();
			//			m_XmlShellPtsElem->SetAttribute("iRowCount", "0");
			//			m_XmlShellPtsElem->SetAttribute("iColCount", "0");
			m_XmlShellPtsElem->SetAttribute("iExtCount", System::Convert::ToString(nCount));

			for (long iPt = 0; iPt < nCount; iPt++)
			{
				XML::XmlElement* XmlPtElement = m_XmlDoc->CreateElement("SPnt");
				m_XmlShellPtsElem->AppendChild(XmlPtElement);
				System::String^ sCoord = System::String::Concat(
					System::Convert::ToString(rPolygon3DPoints[iPt].x), 
					System::Convert::ToString(" "),
					System::Convert::ToString(rPolygon3DPoints[iPt].y),
					System::Convert::ToString(" "),
					System::Convert::ToString(rPolygon3DPoints[iPt].z));
				XmlPtElement->SetAttribute("Coord", sCoord);
			}

			long nFaces = rPolygonVertexCounts.GetSize();
			long lJump = 0;
			for (long lFace = 0; lFace < nFaces; lFace++)
			{
				System::String* sVertices;
				XML::XmlElement* XmlFaceElement = m_XmlDoc->CreateElement("SFace");
				m_XmlShellFacesElem->AppendChild(XmlFaceElement);
				for (long lVertex = 0; lVertex < (int) rPolygonVertexCounts[lFace]; lVertex++)
				{
					if (!lVertex) sVertices = System::Convert::ToString((int)(rPolygonVertexIndices[lVertex + lJump]));
					else sVertices = System::String::Concat(
						sVertices, " ", System::Convert::ToString((int)(rPolygonVertexIndices[lVertex + lJump])));
				}
				XmlFaceElement->SetAttribute("Ind", sVertices);
				lJump += rPolygonVertexCounts[lFace];
			}
		}
		return IW_SUCCESS;
	}
}
