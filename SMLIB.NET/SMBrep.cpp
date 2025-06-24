#include "stdafx.h"
#include "smbrep.h"
#include "SMPolygonOutputCallback.h"
#include <IwMerge.h>
#include "BrepIO.h"
#include "SMShellMapCallback.h"
#include "BrepRegionProxy.h"
#include "BrepFaceProxy.h"
#include "BrepEdgeProxy.h"
#include "SMPolygonOutputCallbackMoldedForms.h"
#include "SMCompositeCurve.h"

#using <System.Windows.Forms.dll>
#include <fstream>
using namespace std;

using namespace System::Windows::Forms;
using namespace System::IO;
using namespace System::Collections;
using namespace System::Text;
using namespace VEDM::Windows;
using namespace System::Diagnostics;

namespace PESMLIB
{
	Brep::Brep(void) : SMObject()
	{

		m_lFaceCounter = 0;
		m_lRegionCounter = 0;
		m_lEdgeCounter = 0;
		m_bCountersDirty = false;
		m_bSuspendUpdating = false;
		m_hkFaces = new System::Collections::ArrayList();
		m_hkEdges = new System::Collections::ArrayList();
	}

	Brep::~Brep()
	{
	}

	Brep::Brep (Context __gc * oContext, XML::XmlElement __gc * pElem) : SMObject()
	{
		m_lFaceCounter = 0;
		m_lRegionCounter = 0;
		m_lEdgeCounter = 0;
		m_pContext = oContext;
		m_bCountersDirty = false;
		m_bSuspendUpdating = false;
		m_dependencies = new Utilities::StringObjDictionary ();
		m_hkFaces = new System::Collections::ArrayList();
		m_hkEdges = new System::Collections::ArrayList();

		if (HasIwContext())
		{
			m_pIwObj = (IwObject *) new (GetIwContext()) IwBrep;
			((IwBrep *) m_pIwObj)->m_bMakeComposites = TRUE;
			m_pXMLElem = pElem;

			// Now populate the Brep via the DOM if there is anything in the DOM.
			// Otherwise it is just the node where we are supposed to store the 
			// state later. Create a new persistent ID if necessary. If there was
			// an ID in the DOM we would have gotten it. Lastly set the ID
			// attribute on the Brep.

			if (m_pXMLElem != NULL)
			{
				GetFromDOM ();
				CreateId ();
				SetIwObjAttribute ();
				AddToDOM ();
			}
		}
		else
		{
			m_pIwObj = NULL;
			m_pXMLElem = NULL;
		}
	}

	void Brep::AddDependency (IPersistentObject __gc *pObj)
	{
		try
		{
			// Create a moniker for the object and add it to the dictionary.

			XmlNewMoniker __gc *pMoniker = __try_cast<XmlNewMoniker *> (XmlNewMoniker::MonikerFromObject (pObj));
			if (NULL != pMoniker)
			{
				// Add the dependency. If the dependency already exists, an ArgumentException is thrown so the 
				// AddToDOM method will be skipped since nothing was changed.

				try
				{
					m_dependencies->Add (pObj->IdSelf, pMoniker);
					AddToDOM ();
				}
				catch (...)
				{
				}
			}
		}
		catch (...)
		{
			Console::WriteLine("Could not add dependency to Brep.");
		}
	}

	void Brep::RemoveDependency (IPersistentObject __gc *pObj)
	{
		try
		{
			if (pObj && m_dependencies->Contains (pObj->IdSelf))
			{
				m_dependencies->Remove (pObj->IdSelf);
				AddToDOM ();
			}
		}
		catch (...)
		{
			Console::WriteLine("Could not remove dependency from Brep.");
		}
	}

	System::Object __gc * Brep::GetDependency (System::String __gc *sObjId)
	{
		try
		{
			if (m_dependencies->Contains(sObjId))
				return __try_cast<XmlNewMoniker *> (m_dependencies->get_Item(sObjId))->ObjectFromMoniker ();
			else
				return NULL;
		}
		catch (...)
		{
			Console::WriteLine("Could not get dependency from Brep.");
		}

		return NULL;
	}

	System::Collections::ArrayList __gc * Brep::GetObjectDependencies ()
	{
		try
		{
			System::Collections::ArrayList __gc *arrDependents = new System::Collections::ArrayList ();
			IDictionaryEnumerator __gc *pEnum = m_dependencies->GetEnumerator ();
			while (pEnum->MoveNext ())
			{
				System::Object __gc *pObj = __try_cast<XmlNewMoniker *> (pEnum->Value)->ObjectFromMoniker ();
				if (pObj != NULL)
					arrDependents->Add (pObj);
			}

			return arrDependents;
		}
		catch (Exception *)
		{
			Console::WriteLine("Could not determine object dependency in BrepRegionProxy");
		}

		return NULL;
	}

	void Brep::AttachIwObj (Context __gc *pContext, IwObject *pIwObj)
	{

		try
		{
			// Check to make sure object type is appropriate

			if (pIwObj && pIwObj->IsKindOf (IwBrep_TYPE))
			{
				// Free any old IwObject. Update the DOM.

				if (m_pIwObj)
					delete m_pIwObj;
				m_pIwObj = pIwObj;
				((IwBrep *) m_pIwObj)->m_bMakeComposites = TRUE;
				SetIwObjAttribute ();
				m_pContext = pContext;
				if (m_pXMLElem)
					AddToDOM ();
			}
		}
		catch (...)
		{
			//TODO throw an exception
		}
	}

	IwRegion * Brep::GetIwRegionFromID (long nID)
	{
		IwRegion *pIwRegion = NULL;

		try
		{
			if (m_pIwObj != NULL)
			{
				if (nID == -1) // infinite region
					pIwRegion = ((IwBrep *) m_pIwObj)->GetInfiniteRegion ();
				else
				{
					IwTArray<IwRegion *> arrRegions;
					((IwBrep *) m_pIwObj)->GetRegions (arrRegions);
					int nRegions = arrRegions.GetSize ();

					for (int iRegion = 0; iRegion < nRegions; iRegion++)
					{
						IwAttribute *pAttribute = arrRegions[iRegion]->FindAttribute (AttributeID_BREPREGIONCOUNTER);
						if (pAttribute && pAttribute->GetNumLongElements () > 0)
						{
							const long *lAttributes = pAttribute->GetLongElementsAddress ();
							if (lAttributes[0] == nID)
							{
								pIwRegion = arrRegions[iRegion];
								break;
							}
						}
					}
				}
			}
		}
		catch (...)
		{
			System::Console::WriteLine ("Could not get IwRegion from specified ID.");
		}
		return pIwRegion;
	}

	void Brep::HighlightFeature (HC::KEY segKey, BrepFeatureType eFeature, long nID)
	{
		HC::KEY  hkey;

		if (eFeature == BrepFeatureType::Brep_Face)
		{
			BrepFaceProxy *pFace = GetFace(nID);
			if (pFace != NULL)
			{
				System::Object __gc *oAttrib = pFace->FindAttribute(PESMLIB::AttributeID_HKEY);
				if (oAttrib != NULL)
				{
					System::Int32 *pkey = dynamic_cast<System::Int32 *>(oAttrib);
					if (NULL != pkey)
					{
						hkey = (HC::KEY)(*pkey);
						HD::iHightMode = 3;
						HD::Add_Face_ID(hkey);
						//HC::Open_Segment_By_Key (segKey);
						//HC::Open_Segment ("highlight");
						//HC::Set_Color(sHighlightColor);
						//HC::Set_Rendering_Options("attribute lock = (face pattern, color = (face, markers))" );
						//HC::Close_Segment ();
						//if (HC::Show_Existence_By_Key (hkey, "self"))
						//	HC::Move_By_Key (hkey, "highlight");
						//HC::Close_Segment ();
					}
				}
			}
		}
		else if (eFeature == BrepFeatureType::Brep_Region)
		{
			if (m_pIwObj)
			{
				//HC::Open_Segment_By_Key (segKey);
				//HC::Open_Segment("highlight");
				//HC::Set_Color(sHighlightColor);
				//HC::Close_Segment();

				// Find all of the faces in the specified region and highlight

				IwRegion *pIwRegion = GetIwRegionFromID (nID);
				if (pIwRegion)
				{
					HD::iHightMode = 2;
					IwTArray<IwShell *> arrShells;
					pIwRegion->GetShells (arrShells);
					int nShells = arrShells.GetSize ();

					for (int iShell = 0; iShell < nShells; iShell++)
					{
						IwTArray<IwFaceuse *> arrFaceuses;
						arrShells[iShell]->GetFaceuses (arrFaceuses);
						int nFaceuses = arrFaceuses.GetSize ();

						for (int iFaceuse = 0; iFaceuse < nFaceuses; iFaceuse++)
						{
							IwFace *pFace = arrFaceuses[iFaceuse]->GetFace ();
							if (pFace)
							{
								IwAttribute *pFaceAttrib = pFace->FindAttribute (AttributeID_HKEY);
								if (pFaceAttrib && pFaceAttrib->GetNumLongElements () > 0)
								{
									const long *lFaceAttribs = pFaceAttrib->GetLongElementsAddress ();
									HD::Add_Face_ID(lFaceAttribs[0]);
									//if (HC::Show_Existence_By_Key ((HC::KEY) lFaceAttribs[0], "self"))
									//	HC::Move_By_Key ((HC::KEY) lFaceAttribs[0], "highlight");
								}
							}
						}
					} // for (int iShell = 0; iShell < nShells; iShell++)
				} // if (pIwRegion)

				//HC::Close_Segment ();

			} // if (m_pIwObj)
		}
		else if (eFeature == BrepFeatureType::Brep_Edge)
		{
			BrepEdgeProxy *pEdge = GetEdge(nID);
			if (pEdge != NULL)
			{
				System::Object *oAttrib = pEdge->FindAttribute(AttributeID_HKEY);
				if (oAttrib != NULL)
				{
					System::Int32 * pkey = dynamic_cast<System::Int32 *>(oAttrib);
					if (NULL != pkey)
					{
						hkey = (HC::KEY)(*pkey);
						HD::iHightMode = 5;
						HD::Add_Face_ID(hkey);
						//HC::KEY keyEdgeSeg = HC::KShow_Owner_By_Key(hkey);
						////HC::Open_Segment_By_Key (segKey);
						////HC::Open_Segment("edges");
						//HC::Open_Segment_By_Key(keyEdgeSeg);
						//HC::Open_Segment ("highlight");
						//HC::Set_Color(sHighlightColor);
						//HC::Close_Segment ();
						//if (HC::Show_Existence_By_Key (hkey, "self"))
						//	HC::Move_By_Key (hkey, "highlight");
						//HC::Close_Segment ();
						////HC::Close_Segment ();
					}
				}
			}
		}
		else if (eFeature == BrepFeatureType::Brep_Vertex)
		{
		}
	}

	void Brep::UnHighlightFeature (HC::KEY segKey, BrepFeatureType eFeature, long nID)
	{
		HC::KEY  hkey;

		if (eFeature == BrepFeatureType::Brep_Face)
		{
			BrepFaceProxy *pFace = GetFace(nID);
			if (pFace != NULL)
			{
				System::Object __gc *oAttrib = pFace->FindAttribute(PESMLIB::AttributeID_HKEY);
				if (oAttrib != NULL)
				{
					System::Int32 *pkey = dynamic_cast<System::Int32 *>(oAttrib);
					if (NULL != pkey)
					{
						hkey = (HC::KEY)(*pkey);
						//HC::Open_Segment_By_Key (segKey);
						//HC::Open_Segment ("highlight");
						//if (HC::Show_Existence_By_Key (hkey, "self"))
						//	HC::Move_By_Key (hkey, "..");
						//HC::Close_Segment ();
						//HC::Close_Segment ();
					}
				}
			}
		}
		else if (eFeature == BrepFeatureType::Brep_Region)
		{
			if (m_pIwObj)
			{
				//HC::Open_Segment_By_Key (segKey);
				//HC::Open_Segment("highlight");

				// Find all of the faces in the specified region an highlight

				IwRegion *pIwRegion = GetIwRegionFromID (nID);
				if (pIwRegion)
				{
					IwTArray<IwShell *> arrShells;
					pIwRegion->GetShells (arrShells);
					int nShells = arrShells.GetSize ();

					for (int iShell = 0; iShell < nShells; iShell++)
					{
						IwTArray<IwFaceuse *> arrFaceuses;
						arrShells[iShell]->GetFaceuses (arrFaceuses);
						int nFaceuses = arrFaceuses.GetSize ();

						for (int iFaceuse = 0; iFaceuse < nFaceuses; iFaceuse++)
						{
							IwFace *pFace = arrFaceuses[iFaceuse]->GetFace ();
							if (pFace)
							{
								IwAttribute *pFaceAttrib = pFace->FindAttribute (AttributeID_HKEY);
								if (pFaceAttrib && pFaceAttrib->GetNumLongElements () > 0)
								{
									const long *lFaceAttribs = pFaceAttrib->GetLongElementsAddress ();
									//if (HC::Show_Existence_By_Key ((HC::KEY) lFaceAttribs[0], "self"))
									//	HC::Move_By_Key ((HC::KEY) lFaceAttribs[0], "..");
								}
							}
						}
					}
				} // if (pIwRegion)

				//HC::Close_Segment();
				//HC::Close_Segment ();
			}
		}
		else if (eFeature == BrepFeatureType::Brep_Edge)
		{
			BrepEdgeProxy *pEdge = GetEdge(nID);
			if (pEdge != NULL)
			{
				System::Object *oAttrib = pEdge->FindAttribute(PESMLIB::AttributeID_HKEY);
				if (oAttrib != NULL)
				{
					System::Int32 * pkey = dynamic_cast<System::Int32 *>(oAttrib);
					if (NULL != pkey)
					{
						hkey = (HC::KEY)(*pkey);

						////HC::Open_Segment_By_Key (segKey);
						////HC::Open_Segment("edges");
						////HC::Open_Segment ("highlight");
						//HC::KEY keyEdgeSeg = HC::KShow_Owner_By_Key(hkey);
						//HC::Open_Segment_By_Key(keyEdgeSeg);
						//if (HC::Show_Existence_By_Key (hkey, "self"))
						//	HC::Move_By_Key (hkey, "..");
						//HC::Close_Segment();
						////HC::Close_Segment ();
						////HC::Close_Segment ();
						////HC::Close_Segment ();
					}
				}
			}
		}
		else if (eFeature == BrepFeatureType::Brep_Vertex)
		{
		}
	}

	// IGeometry Interface.

	void Brep::Highlight (HC::KEY keySeg)
	{
		HD::iHightMode = 0;
		HD::Add_Face_ID(keySeg);
		//HC::KEY key;
		//HC::Open_Segment_By_Key (keySeg);
		//{
		//	HC::Open_Segment("highlight");
		//	{
		//		HC::Set_Color(sHighlightColor);
		//		//HC::Bring_To_Front (".");
		//	}
		//	HC::Close_Segment();

		//	HC::Begin_Contents_Search (".", "geometry");
		//	{
		//		StringBuilder* sType = new StringBuilder ("geometry");
		//		while (HC::Find_Contents (sType, &key))
		//		{
		//			HC::Move_By_Key(key, "highlight");
		//		}
		//	}
		//	HC::End_Contents_Search ();
		//	HC::Open_Segment("edges");
		//	{
		//		HC::Open_Segment("highlight");
		//		{
		//			HC::Set_Color(sHighlightColor);
		//			//HC::Bring_To_Front (".");
		//		}
		//		HC::Close_Segment();
		//		HC::Begin_Contents_Search (".", "geometry");
		//		{
		//			StringBuilder* sType = new StringBuilder ("geometry");
		//			while (HC::Find_Contents (sType, &key))
		//			{
		//				HC::Move_By_Key(key, "highlight");
		//			}
		//		}
		//		HC::End_Contents_Search ();
		//	}
		//	HC::Close_Segment();

		//}
		//HC::Close_Segment ();
	}

	void Brep::UnHighlight (HC::KEY keySeg)
	{
	//	HC::KEY key;
	//	HC::Open_Segment_By_Key (keySeg);
	//	{
	//		HC::Open_Segment("highlight");
	//		{
	//			HC::Begin_Contents_Search (".", "geometry");
	//			{
	//				StringBuilder* sType = new StringBuilder ("geometry");
	//				while (HC::Find_Contents (sType, &key))
	//				{
	//					HC::Move_By_Key(key, "..");
	//				}
	//			}
	//			HC::End_Contents_Search ();
	//		}
	//		HC::Close_Segment();
	//		HC::Open_Segment("edges/highlight");
	//		{
	//			HC::Begin_Contents_Search (".", "geometry");
	//			{
	//				StringBuilder* sType = new StringBuilder ("geometry");
	//				while (HC::Find_Contents (sType, &key))
	//				{
	//					HC::Move_By_Key(key, "..");
	//				}
	//			}
	//			HC::End_Contents_Search ();
	//		}
	//		HC::Close_Segment();
	//		HC::Delete_Segment("highlight");
	//	}
	//	HC::Close_Segment ();
	}

	System::Object * Brep::GetReferencableObject ()
	{
		return this;
	}
	int Brep::getObjectIndex()
	{
		return HD::gObjectCount - 1;
	}

	void Brep::InsertGraphics (bool bDrawDetailed, int handle)
	{
		InsertGraphics (m_hkFaces, m_hkEdges, false);
		if (!bDrawDetailed)
			HD::set_planes_only();
		HD::KInsert_Object();
	}

	void Brep::InsertGraphics (System::Collections::ArrayList __gc *hkFaces, System::Collections::ArrayList __gc *hkEdges, bool bDrawDetailed)
	{

		try
		{
			if (!m_pIwObj)
				return;

			// Set up tesselation parameters
			// The values used here are pretty coarse, not good for fastship hulls
			// The tolerances should one day make it to the parameter list of the function

			double dChordHeightTol = 0.1; // Maximum chord height of polygon (much faster than 0.01 as before)
			double dAngleTolInDegrees = 45.0; // Maximum angle in U or V that the polygon will span. 
			ULONG lMinCurveDivisions = 0; // Don't use this tolerance 
			double dMax3DEdgeLength = 0.;// Maximum length of an edge 
			double dMin3DEdgeLength = 0.0; // Minimum length of an edge - not used 
			double dMaxAspectRatio =  0.0; // 4:1 aspect ratio 
			double dMinUVRatio = 0.0; // Smallest subdivision of a surface is 

			if (m_tessParam != NULL)
			{
				// Use provided dimension tolerances
				dChordHeightTol = m_tessParam->ChordHeight;
				dAngleTolInDegrees = m_tessParam->MaxAngle;
				dMax3DEdgeLength = m_tessParam->Max3dEdgeLength;
				dMaxAspectRatio = m_tessParam->MaxAspectRatio;
				dMinUVRatio = m_tessParam->MinUvRatio;
			}

			// Create a curve tessellation driver and a surface tessellation
			// driver to control how subdivision is performed. 
			IwCurveTessDriver sCrvTess ( lMinCurveDivisions,
				dChordHeightTol,
				dAngleTolInDegrees );

			IwSurfaceTessDriver sSrfTess ( dChordHeightTol,
				dAngleTolInDegrees,
				dMax3DEdgeLength,
				dMin3DEdgeLength,
				dMinUVRatio,
				dMaxAspectRatio );

			// Declare an instance of IwTess class ; sTess

			IwTess sTess (GetIwContext(), sCrvTess, sSrfTess);

			// Note that the Brep sent into the Tessellator gets modified
			// so you usually will want to make a copy of it is follows:
			IwBrep *pCopy = new (GetIwContext()) IwBrep (*((IwBrep *) m_pIwObj));

			// Now do the tessellation process to produce a shell in UV
			// space of the surfaces. Use the phased tesselation both to conserver
			// memory and to facilitate setting of face attributes

			IwBoolean rbFailedFaces = FALSE;

			if (IW_SUCCESS != sTess.Phase1SetupBrep (pCopy, rbFailedFaces))
			{
				// TODO - throw exception
				return;
			}

			if (rbFailedFaces == TRUE)
			{
				// TODO - not sure what but this library really shouldn't be popping up graphical windows
				//			MessageBox::Show("At least on face failed to tesselate properly.", "Tesselation Error",
				//				MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
			}

			// This method will output triangles using sPolyOutput class that knows about hoops
			PolygonOutputCallback oPolyOutput;

			// Enter output file to write tesselation data to
			oPolyOutput.s_lNumberTriangles = 0;
			oPolyOutput.SetOutputType(IW_PO_TRIANGLE_MESH);	
			oPolyOutput.m_pContext = &GetIwContext ();
			oPolyOutput.SetGenerateNormals (FALSE);

			IwTArray<IwFace *> arrFaces;
			pCopy->GetFaces (arrFaces);
			ULONG ulNumFaces = arrFaces.GetSize ();

			IwTArray<IwFace *> arrRealFaces;
			((IwBrep *) m_pIwObj)->GetFaces (arrRealFaces);

			//HC::Open_Segment_By_Key (keyGeom);
			try
			{
				//HC::Set_Color ("edges=gray");
				//HC::Set_Visibility("vertices=off");
				HD::KInsert_Face(ulNumFaces);

				for (ULONG ulFaceCnt = 0; ulFaceCnt < ulNumFaces; ulFaceCnt++)
				{
					oPolyOutput.m_pIwObj = (IwAObject *) arrFaces[ulFaceCnt];
					if (IW_SUCCESS == sTess.Phase2CreateFacePolygons (arrFaces[ulFaceCnt]))
					{
						sTess.OutputFacePolygons (arrFaces[ulFaceCnt], oPolyOutput);
						if (hkFaces != NULL)
						{
							HC::KEY hkFace = (HC::KEY) oPolyOutput.GetShellKey();
							hkFaces->Add(__box(hkFace));
						}
					}
					else
					{
						Console::WriteLine("Failed to create polygon face " + ulFaceCnt);
						hkFaces->Add(__box((HC::KEY)0));
					}

					sTess.Phase3DeleteFacePolygons (arrFaces[ulFaceCnt]);

					// Now copy the AttributeID_HKEY attributes from the face copies to the real faces.

					IwAttribute *pAttribute = arrFaces[ulFaceCnt]->FindAttribute (AttributeID_HKEY);
					if (NULL != pAttribute)
					{
						IwAttribute *pExistingAttribute = arrRealFaces[ulFaceCnt]->FindAttribute (AttributeID_HKEY);
						if (pExistingAttribute != NULL)
							arrRealFaces[ulFaceCnt]->RemoveAttribute (pExistingAttribute, FALSE);
						IwAttribute *pNewAttribute = pAttribute->MakeCopy (GetIwContext());
						arrRealFaces[ulFaceCnt]->AddAttribute (pNewAttribute);
					}
				} // for (ULONG ulFaceCnt = 0; ulFaceCnt < ulNumFaces; ulFaceCnt++)
			}
			catch (Exception *)
			{
				Console::WriteLine("Error in tesselation of polygons");
			}
			__finally
			{
				//HC::Close_Segment ();
			}

			// Tesselate edge curves

			IwTArray<IwEdge *> arrEdges;
			((IwBrep *) m_pIwObj)->GetEdges(arrEdges);
			ULONG ulNumEdges = arrEdges.GetSize();

			//HC::Open_Segment_By_Key (keyGeom);
			try
			{
				//HC::Set_Color ("lines=blue");
				//HC::Set_Line_Weight(3.0);
				HD::KInsert_Edge(ulNumEdges);
				for (ULONG ulEdgeCnt = 0; ulEdgeCnt < ulNumEdges; ulEdgeCnt++)
				{
					IwStatus valStatus = arrEdges[ulEdgeCnt]->ValidateGeometry (TRUE);
					IwCurve *pCurve;
					pCurve = arrEdges[ulEdgeCnt]->GetCurve();
					IwTArray<IwVector3d> pPoints;
					IwExtent1d interval = arrEdges[ulEdgeCnt]->GetInterval ();

					// The logic here was utilized in the test ui program, by specifying a non-zero
					// dMax3dDistance value some edge tesselation problems are fixed although not all.
					// The Tessellate call actually calls TessellateByBisection but if minIntervals is
					// 1 then the dMax3dDistance param is set to zero and that test is skipped.
					IwStatus tessStatus = pCurve->TessellateByBisection (interval,
						dChordHeightTol, dAngleTolInDegrees, 3., NULL, &pPoints);
//					IwStatus tessStatus = pCurve->Tessellate(arrEdges[ulEdgeCnt]->GetInterval(), 
//						dChordHeightTol, dAngleTolInDegrees, 1, NULL, &pPoints);

					int nPts = pPoints.GetSize();
					//HC_POINT* hpt = new HC_POINT[nPts];
					float hpt __gc[];

					hpt = new float __gc[3*nPts];
					//HC::Open_Segment("vertices");
					//HC::Set_Marker_Symbol("[]");
					for (int i = 0; i < nPts; i++)
					{
						IwVector3d pt = pPoints[i];
						hpt[3*i] = (float)pt.x;
						hpt[3*i+1] = (float)pt.y;
						hpt[3*i+2] = (float)pt.z;
					}
					//HC::Insert_Marker(pPoints[0].x, pPoints[0].y, pPoints[0].z); // Mark the first, and...
					//HC::Insert_Marker(pPoints[nPts-1].x, pPoints[nPts-1].y, pPoints[nPts-1].z); // last point on an edge
					//HC::Close_Segment();

					// Open an unnamed segment to hold the tesselated curve
					int keyEdge;
					//HC::KEY tmpKey = HC::KOpen_Segment("edges");
					try
					{
						if (arrEdges[ulEdgeCnt]->IsLamina())
						{
							//HC::Open_Segment("lamina");
							//HC::Set_Color_By_Index("lines",6);
							keyEdge = HD::KInsert_Polyline (nPts, hpt);
							if (hkEdges != NULL)
								hkEdges->Add(__box(keyEdge));
							//HC::Close_Segment();
						}
						else
						{
							keyEdge = HD::KInsert_Polyline (nPts, hpt);
							if (hkEdges != NULL)
								hkEdges->Add(__box(keyEdge));
						}

						// Set the hoops key as an attribute on the edge. Remove any existing attribute.
						IwAttribute *pAttribute = arrEdges[ulEdgeCnt]->FindAttribute (AttributeID_HKEY);
						if (pAttribute != NULL)
							arrEdges[ulEdgeCnt]->RemoveAttribute (pAttribute, FALSE);
						IwLongAttribute *pLongAttribute = new (GetIwContext()) IwLongAttribute (
							AttributeID_HKEY, keyEdge, IW_AB_COPY);
						arrEdges[ulEdgeCnt]->AddAttribute (pLongAttribute);
					}
					catch (...)
					{
						Console::WriteLine("Error inserting edges");
					}
					__finally
					{
						//HC::Close_Segment();
					}

					delete [] hpt;
				}
			}
			catch (...)
			{
				Console::WriteLine("Error tesselating edges");
			}
			__finally
			{
				//HC::Close_Segment ();
			}

			if (bDrawDetailed)
				HD::KInsert_Object();


			//AddToDOM (); is this needed 
		}
		catch (System::Exception __gc *ex)
		{
			System::Console::WriteLine (ex->get_Message());
		}
		catch (...)
		{
			// TODO
		}
	}

	//void Brep::Transform (PEHoops::MVO::Transformation * oTransformation)
	//{
	//	// TODO
	//}

	void Brep::ComputeBoundingBoxOld(HC::NL_POINT __gc * ptMin, HC::NL_POINT __gc * ptMax)
	{
		IwExtent3d oBox;
		((IwBrep *)m_pIwObj)->CalculateBoundingBox(oBox);
		ptMin->x = (float)oBox.GetMin().x;
		ptMin->y = (float)oBox.GetMin().y;
		ptMin->z = (float)oBox.GetMin().z;
		ptMax->x = (float)oBox.GetMax().x;
		ptMax->y = (float)oBox.GetMax().y;
		ptMax->z = (float)oBox.GetMax().z;
	}

	bool Brep::ComputeBoundingBox (HC::NL_POINT __gc * ptMin, HC::NL_POINT __gc * ptMax)
	{
		if (ptMax->x == -FLT_MAX && ptMin->z == FLT_MAX)
		{
			ComputeBoundingBoxOld(ptMin, ptMax);
			return true;
		}

		try
		{
			// Compute the axis aligned bounding box for the brep. Initially we used
			// the SMLIB brep method CalculateBoundingBox but at least as of v 6.70 we
			// found  there were times where it appeared to give a different answer than expected.
			// The SMLIB method works by walking through the topology tree (list of vertices,
			// edges and faces) growing the box as needed. We suspect that the reason we
			// might be getting unexpected results in some cases is because the brep has
			// some vertices in its list that are not being used but were not removed by
			// a call to removetopologicaledgesandvertices and hence don't show up in the
			// geometric representation. As a workaround we just use the same logic as a
			// compartment where we walk the brep regions and compute the bounding box of the
			// associated faces.

			/* Old code here
			IwExtent3d oBox;
			((IwBrep *) m_pIwObj)->CalculateBoundingBox (oBox);
			ptMin->x = (float) oBox.GetMin().x;
			ptMin->y = (float) oBox.GetMin().y;
			ptMin->z = (float) oBox.GetMin().z;
			ptMax->x = (float) oBox.GetMax().x;
			ptMax->y = (float) oBox.GetMax().y;
			ptMax->z = (float) oBox.GetMax().z;
			*/

			// New code here

			BrepRegionProxy __gc *pInfRegion = GetInfiniteRegion ();
			if (pInfRegion != NULL)
			{
				pInfRegion->ComputeBoundingBox (ptMin, ptMax);
			}
			else
			{
				ptMin->Set (0.,0.,0.);
				ptMax->Set (0.,0.,0.);
				return false;
			}
		}
		catch (...)
		{
			System::Console::WriteLine ();
			return false;
		}
		return true;
	}

	void Brep::SetBrepAttributes ()
	{

		// Walk the list of regions, faces, and edges in the Brep assigning attributes as needed.

		if (m_pIwObj != NULL)
		{
			IwTArray<IwFace *> arrFaces;
			((IwBrep *) m_pIwObj)->GetFaces (arrFaces);
			ULONG numFaces = arrFaces.GetSize ();
			for (ULONG iFace = 0; iFace < numFaces; iFace++)
			{
				// Only assign attribute if none already exists of this type or if the
				// counters are dirty.

				IwAttribute *pExistingAttribute = arrFaces[iFace]->FindAttribute (AttributeID_BREPFACECOUNTER);
				if (NULL != pExistingAttribute && m_bCountersDirty)
				{
					arrFaces[iFace]->RemoveAttribute (pExistingAttribute, TRUE);
					pExistingAttribute = NULL;
				}

				if (NULL == pExistingAttribute)
				{
					IwLongAttribute *pAttribute = new (GetIwContext()) IwLongAttribute (
						AttributeID_BREPFACECOUNTER, m_lFaceCounter, IW_AB_COPY);
					arrFaces[iFace]->AddAttribute (pAttribute);
					if (m_lFaceCounter == LONG_MAX)
						m_lFaceCounter = 0;
					else
						m_lFaceCounter++;
				}
			}

			IwTArray<IwRegion *> arrRegions;
			((IwBrep *) m_pIwObj)->GetRegions (arrRegions);
			ULONG numRegions = arrRegions.GetSize ();
			for (ULONG iRegion = 0; iRegion < numRegions; iRegion++)
			{
				// Only assign attribute if none already exists of this type or if the
				// counters are dirty.

				IwAttribute *pExistingAttribute = arrRegions[iRegion]->FindAttribute (AttributeID_BREPREGIONCOUNTER);
				if (NULL != pExistingAttribute && m_bCountersDirty)
				{
					arrRegions[iRegion]->RemoveAttribute (pExistingAttribute, TRUE);
					pExistingAttribute = NULL;
				}

				if (NULL == pExistingAttribute)
				{
					IwLongAttribute *pAttribute = new (GetIwContext()) IwLongAttribute (
						AttributeID_BREPREGIONCOUNTER, m_lRegionCounter, IW_AB_COPY);
					arrRegions[iRegion]->AddAttribute (pAttribute);
					if (m_lRegionCounter == LONG_MAX)
						m_lRegionCounter = 0;
					else
						m_lRegionCounter++;
				}
			}

			IwTArray<IwEdge *> arrEdges;
			((IwBrep *) m_pIwObj)->GetEdges (arrEdges);
			ULONG numEdges = arrEdges.GetSize ();
			for (ULONG iEdge = 0; iEdge < numEdges; iEdge++)
			{
				// Only assign attribute if none already exists of this type or if the
				// counters are dirty.

				IwAttribute *pExistingAttribute = arrEdges[iEdge]->FindAttribute (AttributeID_BREPEDGECOUNTER);
				if (NULL != pExistingAttribute && m_bCountersDirty)
				{
					arrEdges[iEdge]->RemoveAttribute (pExistingAttribute, TRUE);
					pExistingAttribute = NULL;
				}

				if (NULL == pExistingAttribute)
				{
					IwLongAttribute *pAttribute = new (GetIwContext()) IwLongAttribute (
						AttributeID_BREPEDGECOUNTER, m_lEdgeCounter, IW_AB_COPY);
					arrEdges[iEdge]->AddAttribute (pAttribute);
					if (m_lEdgeCounter == LONG_MAX)
						m_lEdgeCounter = 0;
					else
						m_lEdgeCounter++;
				}
			}

			m_bCountersDirty = false;

		} // if (m_pIwObj != NULL)
	}

	void Brep::ExportBrepToFile (String __gc *sFilename)
	{

		if (m_pIwObj != NULL && m_pXMLElem != NULL)
		{
			// Write Brep as a part to a file

			IwTArray<IwCurve*> arrCurves;
			IwTArray<IwSurface*> arrSurfaces;
			IwTArray<long> arrTrees;
			IwTArray<IwBrep*> arrBreps;

			try
			{
				ostrstream tmpStream;
				BrepIO oBrepIO (&tmpStream);

				arrBreps.Add ((IwBrep *) m_pIwObj);
				IwStatus sStatus = IwBrepData::WritePartToFile (
					(char *) Marshal::StringToHGlobalAnsi (sFilename).ToPointer (),
					arrCurves, arrSurfaces, arrTrees, arrBreps, IW_ASCII);
			}
			catch (...)
			{
				Console::WriteLine ("Could not export Brep to file.");
			}
		}
	}

	int Brep::CompareTo (System::Object __gc *obj)
	{
		try
		{
			if (this == NULL && obj == NULL)
				return 0;
			else if (obj == NULL)
				return 1;
			else if (this == NULL)
				return -1;
			else
			{
				Brep *brep = __try_cast<Brep *> (obj);
				if (NULL != brep)
				{
					if (NULL != m_pXMLElem)
						return IdSelf->CompareTo (brep->IdSelf);
					else if (m_pIwObj == brep->GetIwObj ())
						return 0;
					else if (m_pIwObj < brep->GetIwObj ())
						return -1;
					else
						return 1;
				}

			}
		}
		catch (System::InvalidCastException*)
		{
			Console::WriteLine ("Could not cast object to Brep.");
			throw new System::ArgumentException ("Object is not a Brep.");
		}

		return 1;
	}

	bool Brep::Equals (System::Object __gc * obj)
	{
		try
		{
			// If this is a persistent brep, compare id's, otherwise compare underlying geometry.

			Brep *pBrep = __try_cast<Brep *> (obj);
			if (NULL != pBrep)
			{
				if (NULL != m_pXMLElem)
				{
					if (pBrep->get_IdSelf () == get_IdSelf () && get_IdSelf () != NULL)
						return true;
				}
				else if (m_pIwObj == pBrep->GetIwObj ())
					return true;
			}
		}
		catch(System::InvalidCastException*)
		{
			Console::WriteLine("Could not cast object to Brep");
		}
		return false;
	}

	void Brep::SuspendUpdating ()
	{
		m_bSuspendUpdating = true;
	}

	void Brep::ResumeUpdating ()
	{
		m_bSuspendUpdating = false;
		AddToDOM ();
	}

	void Brep::AddToDOM ()
	{
		try
		{
			// Set face, region, and any other Brep specific attributes, even for non-persistent breps.

			SetBrepAttributes ();

			if (m_pIwObj != NULL && m_pXMLElem != NULL && !m_bSuspendUpdating)
			{
				// Add the persistent ID.
				m_pXMLElem->SetAttribute("idSelf", GetId());

				// Update the topology counters.
				m_pXMLElem->SetAttribute ("lFaceCounter", m_lFaceCounter.ToString ());
				m_pXMLElem->SetAttribute ("lRegionCounter", m_lRegionCounter.ToString ());
				m_pXMLElem->SetAttribute ("lEdgeCounter", m_lEdgeCounter.ToString ());

				// Add the Brep dependencies. Start by deleting any existing dependencies in the DOM.

				Xml::XmlNode __gc *ndDependencies = m_pXMLElem->SelectSingleNode ("Dependencies");
				if (NULL == ndDependencies)
				{
					ndDependencies = m_pXMLElem->OwnerDocument->CreateElement ("Dependencies");
					m_pXMLElem->AppendChild (ndDependencies);
				}
				else
					ndDependencies->RemoveAll ();

				IDictionaryEnumerator __gc *pEnum = m_dependencies->GetEnumerator ();
				while (pEnum->MoveNext ())
				{
					XmlNewMoniker __gc *pMoniker = __try_cast<XmlNewMoniker *> (pEnum->Value);
					if (NULL != pMoniker)
					{
						Xml::XmlElement __gc *ndDependency = m_pXMLElem->OwnerDocument->CreateElement ("Dependency");
						ndDependency->SetAttribute ("sKey", __try_cast<System::String *> (pEnum->Key));
						Xml::XmlNode __gc *ndObjMoniker = m_pXMLElem->OwnerDocument->ImportNode (pMoniker->XmlElement, true);
						ndDependency->AppendChild (ndObjMoniker);
						ndDependencies->AppendChild (ndDependency);
					}
				}

				// Write Brep as a part to a local DB object

				IwTArray<IwCurve*> arrCurves;
				IwTArray<IwSurface*> arrSurfaces;
				IwTArray<long> arrTrees;
				IwTArray<IwBrep*> arrBreps;

				std::ostrstream tmpStream;
				BrepIO oBrepIO (&tmpStream);

				arrBreps.Add ((IwBrep *) m_pIwObj);
				IwStatus sStatus = IwBrepData::WritePartToDB (
					arrCurves, arrSurfaces, arrTrees, arrBreps, oBrepIO);


				// Get the string from the stream.

				String __gc * sBrepIO;
				Process __gc * cp = Process::GetCurrentProcess();

                if ((cp->PrivateMemorySize64 > 250000000 && oBrepIO.GetStringBufLength() > 19000) || oBrepIO.GetStringBufLength() > 999999)
				{
				    String __gc * fpath = String::Concat(Path::GetTempPath(), new String("NCEDN_"),
					__box(cp->Id)->ToString(),
					String::Concat(new String("_"), __box(DateTime::Now.Ticks)->ToString()));
				    sBrepIO = String::Concat(new String("TEMPFILE-"), fpath);
				    File::WriteAllText(fpath, oBrepIO.GetStringBuf());
				}
				else
				    sBrepIO = oBrepIO.GetStringBuf();



				// Delete any existing CDATA nodes.
				XML::XmlNodeList __gc *pList = m_pXMLElem->ChildNodes;
				XML::XmlNode __gc *pNode;

				IEnumerator* iEnum = pList->GetEnumerator();
				while (iEnum->MoveNext())
				{
					pNode = dynamic_cast<XML::XmlNode*>(iEnum->Current);
					if (XML::XmlNodeType::CDATA == pNode->NodeType)
					{
						m_pXMLElem->RemoveChild(pNode);
						// reset the enumerator
						iEnum = pList->GetEnumerator();
					}
				}

				// Create the new CDATA node.
				XML::XmlCDataSection* pSect = m_pXMLElem->OwnerDocument->CreateCDataSection(sBrepIO);
				m_pXMLElem->AppendChild(pSect);	

			} // if (m_pIwObj != NULL && m_pXMLElem != NULL)
		}
		catch (System::Exception __gc * ex)
		{
			ex->get_Message();
		}
	}

	void Brep::GetFromDOM ()
	{
		try
		{
			if (m_pXMLElem != NULL && m_pIwObj != NULL && HasIwContext())
			{
				// Get the persistent GUID.

				String __gc * sId = m_pXMLElem->GetAttribute("idSelf");
				if (sId->Length > 0)
					SetId (sId);

				String __gc *sFaceCounter = m_pXMLElem->GetAttribute ("lFaceCounter");
				if (sFaceCounter->Length > 0)
					m_lFaceCounter = (long) Convert::ToInt32 (sFaceCounter);

				String __gc *sRegionCounter = m_pXMLElem->GetAttribute ("lRegionCounter");
				if (sRegionCounter->Length > 0)
					m_lRegionCounter = (long) Convert::ToInt32 (sRegionCounter);

				// Get the dependencies

				m_dependencies->Clear ();
				Xml::XmlNode __gc *ndDependencies = m_pXMLElem->SelectSingleNode ("Dependencies");
				if (NULL != ndDependencies)
				{
					Xml::XmlNodeList __gc *ndChildren = ndDependencies->ChildNodes;
					for (int iItem = 0; iItem < ndChildren->Count; iItem++)
					{
						Xml::XmlElement __gc *elDependency = __try_cast<Xml::XmlElement *> (ndChildren->ItemOf[iItem]);
						Xml::XmlNodeList __gc *ndMonikers = elDependency->GetElementsByTagName ("Moniker");
						Xml::XmlNode __gc *ndMoniker = ndMonikers->ItemOf[0];
						if (NULL != ndMoniker)
						{
							XmlNewMoniker __gc *pMoniker = new XmlNewMoniker (ndMoniker);
							System::String __gc *sIdObj = elDependency->GetAttribute ("sKey");
							if (NULL != pMoniker && NULL != sIdObj && sIdObj->Length > 0)
								m_dependencies->Add (sIdObj, pMoniker);
						}
					}
				}

				XML::XmlNodeList __gc * pList = m_pXMLElem->ChildNodes;
				XML::XmlNode __gc * pNode;

				IEnumerator __gc * iEnum = pList->GetEnumerator();
				while (iEnum->MoveNext())
				{
					pNode = dynamic_cast<XML::XmlNode __gc *>(iEnum->Current);

					if (XML::XmlNodeType::CDATA == pNode->NodeType)
					{
						String* sBrep = pNode->Value;

						// Read Brep from a local DB object

						IwTArray<IwCurve*> arrCurves;
						IwTArray<IwSurface*> arrSurfaces;
						IwTArray<long> arrTrees;
						IwTArray<IwBrep*> arrBreps;
						System::IntPtr iPtr = Marshal::StringToHGlobalAnsi (sBrep);
						void *pVoid = iPtr.ToPointer ();
						char *pChar = (char *) pVoid;
						std::istrstream tmpStream (pChar);
						//                  std::istrstream tmpStream (pChar, sBrep->Length);
						//						istrstream tmpStream ((char *) Marshal::StringToHGlobalAnsi (sBrep).ToPointer ());
						BrepIO oBrepIO (&tmpStream);

						IwStatus sStatus = IwBrepData::ReadPartFromDB (GetIwContext (),
							arrCurves, arrSurfaces, arrTrees, arrBreps, oBrepIO);

						if (sStatus != IW_SUCCESS)
							System::Windows::Forms::MessageBox::Show("Failed to read brep properly.");

						if (arrBreps.GetSize () > 0)
							m_pIwObj = arrBreps[0];

						break;
					}
				}
			}
		}
		catch (System::Exception __gc * ex)
		{
			ex->get_Message();
		}
	}

	void Brep::Copy (Brep __gc * srcBrep)
	{
		try
		{
			if (srcBrep == NULL || !HasIwContext()) // should be allowed to copy non-persistent breps || m_pXMLElem == NULL)
			{
				return;
			}

			if (m_pIwObj)
			{
				delete m_pIwObj;
				m_pIwObj = 0;
			}

			m_pIwObj = new (GetIwContext()) IwBrep (*((IwBrep *) (srcBrep->m_pIwObj)));

			// Give the underlying copied surfaces new GUIDS
			IwBrep *pBrep = (IwBrep *)(m_pIwObj);
			IwTArray<IwFace *> arrFaces;
			pBrep->GetFaces(arrFaces);
			if (arrFaces.GetSize() > 1)
			{
				for (unsigned int iFace = 0; iFace < arrFaces.GetSize(); iFace++)
				{
					IwFace * pFace = arrFaces[iFace];
					IwSurface *pSurface = pFace->GetSurface();
					IwAttribute* pAttribute = pSurface->FindAttribute(AttributeID_IDSELF);
					if (pAttribute != NULL)
						pSurface->RemoveAttribute(pAttribute);
					pSurface->AddAttribute(SMObject::CreateStringAttribute(this->GetIwContext(),
						AttributeID_IDSELF, Guid::NewGuid().ToString("B")));
				}
			}

			if (m_pIwObj == NULL)
			{
				// todo: throw exception
			}
			else
			{
				SetIwObjAttribute ();
				AddToDOM ();
			}
		}
		catch (System::Exception __gc *e)
		{
			Console::WriteLine (e->Message);
		}
	}

	void Brep::Mirror (Brep __gc *pBrep, Plane __gc * oMirrorPlane)
	{

		if (pBrep == NULL || !HasIwContext() || m_pXMLElem == NULL)
		{
			// todo: throw exception
		}

		if (pBrep && HasIwContext())
		{
			if (m_pIwObj)
			{
				const IwPlane *pPlane = (IwPlane *) oMirrorPlane->GetIwObj ();
				if (pPlane)
				{
					IwAxis2Placement oMirrorAxis = pPlane->GetPosition ();
					Copy(pBrep);
					if (m_pIwObj != NULL)
					{
						if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->Mirror (oMirrorAxis)))
						{
							// todo: throw exception
						}
					}
				}
			}
		}
		AddToDOM ();
	}

	void Brep::ReplaceSurfaceOfFaces (System::Collections::ArrayList __gc *arrFaceProxies, System::Object __gc *surface)
	{
		try
		{
			if (NULL != arrFaceProxies && NULL != surface && NULL != m_pIwObj)
			{
				SMObject __gc *smObj = dynamic_cast<SMObject *> (surface);
				if (NULL != smObj)
				{
					((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
					IwSurface *pSurface = (IwSurface *) smObj->ExtractIwObj ();

					IwCFace *pCFace = 0;
					IwFace *pIwFirstFace = 0;

					for (int iFace = 0; iFace < arrFaceProxies->Count; iFace++)
					{
						IwFace *pIwFace = this->GetIwFaceFromProxy (dynamic_cast<BrepFaceProxy *> (arrFaceProxies->get_Item (iFace)));
						if (pIwFace)
						{
							IwStatus status = ((IwBrep *) m_pIwObj)->ReplaceSurface (pIwFace, pSurface, FALSE, NULL, TRUE);
							if (status != IW_SUCCESS)
							{
								// TO DO throw exception
							}
							if (iFace == 0)
								pIwFirstFace = pIwFace;
							else if (iFace == 1)
								pCFace = new((IwBrep *) m_pIwObj) IwCFace (pIwFirstFace, pIwFace);
							else 
								pCFace->AddFace (pIwFace);
						}
					}

					// Added this to force the brep to recompute based on new surfaces.

					//IwBrepCache *pCache = (IwBrepCache*) IwCacheMgr::GetOrCreateObjectCache(IW_OC_BREP, m_pIwObj); 
					//if (pCache)
					//   pCache->BuildTrees ();

					AddToDOM ();
					((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
				}
			}
		}
		catch (...)
		{
			Console::WriteLine ("Could not replace surface of faces in brep.");
		}
	}

	void Brep::ReplaceSurface (BrepFaceProxy __gc *face, NurbsSurface __gc *surface, bool bCreateTrimCurves)
	{
		try
		{
			if (NULL != face && NULL != surface && NULL != m_pIwObj)
			{
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;

				IwFace *pIwFace = NULL;
				IwTArray<IwFace *> arrFaces;
				((IwBrep *) m_pIwObj)->GetFaces(arrFaces);
				int nFaces = arrFaces.GetSize();
				for (int i = 0; i < nFaces; i++)
				{
					IwAttribute *pAttribute = arrFaces[i]->FindAttribute (AttributeID_BREPFACECOUNTER);
					if (pAttribute && pAttribute->GetNumLongElements () > 0)
					{
						const long *lAttributes = pAttribute->GetLongElementsAddress ();
						if (lAttributes[0] == face->FaceID)
						{
							pIwFace = arrFaces[i];
							break;
						}
					}
				}
				if (pIwFace)
				{
					IwBSplineSurface *pSurface = (IwBSplineSurface *) surface->ExtractIwObj ();
					IwStatus status = ((IwBrep *) m_pIwObj)->ReplaceSurface (pIwFace, pSurface, FALSE, NULL, bCreateTrimCurves ? TRUE : FALSE);
					if (status != IW_SUCCESS)
					{
						// TO DO throw exception
					}
				}
				AddToDOM ();
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
			}
		}
		catch (...)
		{
			Console::WriteLine ("Could not replace surface of brep.");
		}
	}

	void Brep::CreateBrepFromRegion (BrepRegionProxy __gc *regionProxy, Brep __gc *destBrep)
	{
		IwBrep *destIwBrep = 0;
		IwRegion *regionToCopy = 0;
		IwTArray<IwFace *> arrFacesToCopy;

		if (m_pIwObj && destBrep && (destIwBrep = (IwBrep *) destBrep->GetIwObj ()))
		{
			// Find the region with the input id as an attribute.

			regionToCopy = GetIwRegionFromID (regionProxy->RegionID);

			if (regionToCopy)
			{
				// Extract the region's faces as faces to copy.

				IwTArray<IwShell *> arrShells;
				regionToCopy->GetShells (arrShells);
				int nShells = arrShells.GetSize ();

				for (int iShell = 0; iShell < nShells; iShell++)
				{
					IwTArray<IwFaceuse *> arrFaceUses;
					arrShells[iShell]->GetFaceuses (arrFaceUses);
					int nFaceUses = arrFaceUses.GetSize ();

					for (int iFaceUse = 0; iFaceUse < nFaceUses; iFaceUse++)
					{
						arrFacesToCopy.AddUnique (arrFaceUses[iFaceUse]->GetFace ());
					}
				}

				// Copy the faces into the destination brep.

				destIwBrep->m_bEditingEnabled = TRUE;
				IwStatus status = ((IwBrep *) m_pIwObj)->CopyFaces (arrFacesToCopy, destIwBrep);
				if (status != IW_SUCCESS)
				{
					System::Windows::Forms::MessageBox::Show("Failed to copy original Brep faces to destination Brep.");
				}
				destIwBrep->m_bEditingEnabled = FALSE;
			}
		}
	}

	void Brep::CreateFromNurbsSurface (NurbsSurface __gc * oSurface)
	{

		IwBSplineSurface *pSrf = 0;
		IwFace *pFace = 0;

		if (m_pIwObj && (pSrf = (IwBSplineSurface *) oSurface->GetIwObj ()) != NULL)
		{
			((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
			((IwBrep *) m_pIwObj)->m_bMakeComposites = TRUE;
			IwBSplineSurface *pNewSrf = new (GetIwContext ()) IwBSplineSurface (*pSrf);
			if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
				pNewSrf, pNewSrf->GetNaturalUVDomain (), pFace)))
			{
				// todo: throw exception
			}
			pNewSrf = 0; // It was consumed by brep
			if (pFace != NULL)
			{
				if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->FixAllFaceDiscontinuties ()))
				{
					// todo: throw exception
					System::Windows::Forms::MessageBox::Show("Failed to create brep from surface.");
				}
				AddToDOM ();
			}
			// If original surface had discontinuities, the result of the above operation will have
			// broken the surface into sub-surfaces. Loop through all faces (if more than one) and set the 
			// IdSelf attribute on all sub-surfaces.
			IwBrep *pBrep = ((IwBrep *) m_pIwObj);
			IwTArray<IwFace *> arrFaces;
			pBrep->GetFaces(arrFaces);
			IwAttribute *pAttributeIdSelf = NULL;
			if (arrFaces.GetSize() > 1)
			{
				for (unsigned int iFace = 0; iFace < arrFaces.GetSize(); iFace++)
				{
					IwFace * pFace = arrFaces[iFace];
					IwSurface *pSurface = pFace->GetSurface();
					pSurface->AddAttribute(SMObject::CreateStringAttribute(this->GetIwContext(),
						AttributeID_IDSELF, Guid::NewGuid().ToString("B")));
				}
			}
			((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
		}
	}

	bool Brep::JoinBreps (Brep* vecBreps __gc[])
	{

		try
		{
			return MergeBreps (vecBreps[0], vecBreps, BooleanMerge_Union, true, true);
		}
		catch (...)
		{
			return false;
		}
	}

	// Property equivalent to IsManifold() function
	bool Brep::get_Manifold()
	{
		return IsManifold();
	}

	bool Brep::IsManifold()
	{

		bool bManifold = false;

		try
		{
			if (m_pIwObj != NULL)
			{
				IwBoolean bIwManifold = ((IwBrep *) m_pIwObj)->IsManifoldSolid ();
				if (bIwManifold)
					bManifold = true;
			}
		}
		catch (...)
		{
			// TODO
		}

		return bManifold;
	}

	void Brep::MakeManifold ()
	{
		try
		{
			if (m_pIwObj != NULL)
			{
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;

				// Try to sew the faces together and make the Brep manifold 
				// (or cellular) but don't fail if you can't.

				IwStatus sStatus = IW_SUCCESS;
				sStatus = ((IwBrep *) m_pIwObj)->MakeManifold(NULL,TRUE);

				if (sStatus == IW_SUCCESS)
				{
					// Only orient if it is truly a manifold solid. The orientation call seems 
					// to cause problems later on if it is not a manifold solid.
					IwBoolean rbMaybeNotClosedSolid;
					if (((IwBrep *) m_pIwObj)->IsManifoldSolid ())
						((IwBrep *) m_pIwObj)->OrientTrimmedSurfaces (TRUE, rbMaybeNotClosedSolid, FALSE);
				}

				// Set the counters dirty flag since topology counters may no longer be unique.

				m_bCountersDirty = true;
				AddToDOM ();
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
			}
		}
		catch (...)
		{
			// TODO
		}
	}


	void Brep::Scale(double dSx, double dSy, double dSz)
	{
		IwVector3d* pVecScale = new IwVector3d(dSx, dSy, dSz);
		try
		{
			if (m_pIwObj != NULL)
			{
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
				IwStatus sStatus = IW_SUCCESS;

				// If dSx, dSy, and dSz are not all the same, then it is necessary to
				// first convert all underlying geometry to NURBs

				if (dSx != dSy || dSx != dSz || dSy != dSz)
				{
					sStatus = ((IwBrep *) m_pIwObj)->TurnToNURBS();
					if (sStatus != IW_SUCCESS)
					{
						String __gc* sInfo = "Failed to convert brep geometry to NURBS.";
						System::Windows::Forms::MessageBox::Show(sInfo);
					}
				}

				// Transform Brep by scaling only
				IwAxis2Placement* pAxis = new IwAxis2Placement();
				sStatus =((IwBrep *) m_pIwObj)->Transform(*pAxis, pVecScale);
				if (sStatus != IW_SUCCESS)
				{
					String __gc* sInfo = "Scaling of brep failed.";
					System::Windows::Forms::MessageBox::Show(sInfo);
				}

				// Update the result in the DOM
				AddToDOM ();
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
			}
		}
		catch (...)
		{
			// TODO
		}
	}

	void Brep::Translate(double dDx, double dDy, double dDz)
	{
		IwVector3d* pVecScale = new IwVector3d(1.0, 1.0, 1.0);
		IwVector3d* pVecTranslate = new IwVector3d(dDx, dDy, dDz);
		try
		{
			if (m_pIwObj != NULL)
			{
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
				IwStatus sStatus = IW_SUCCESS;

				// Transform Brep by translation only
				IwAxis2Placement* pAxis = new IwAxis2Placement();
				pAxis->Translate(*pVecTranslate);
				sStatus =((IwBrep *) m_pIwObj)->Transform(*pAxis, pVecScale);
				if (sStatus != IW_SUCCESS)
				{
					String __gc* sInfo = "Translation of brep failed.";
					System::Windows::Forms::MessageBox::Show(sInfo);
				}

				// Update the result in the DOM
				AddToDOM ();
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
			}
		}
		catch (...)
		{
			// TODO
		}
	}

	void Brep::Rotate(double dAngx, double dAngy, double dAngz)
	{
		IwVector3d* pVecScale = new IwVector3d(1.0, 1.0, 1.0);
		IwVector3d* pVecXaxis = new IwVector3d(1.0, 0.0, 0.0);
		IwVector3d* pVecYaxis = new IwVector3d(0.0, 1.0, 0.0);
		IwVector3d* pVecZaxis = new IwVector3d(0.0, 0.0, 1.0);
		try
		{
			if (m_pIwObj != NULL)
			{
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
				IwStatus sStatus = IW_SUCCESS;

				// Transform Brep by rotating about primary axes
				IwAxis2Placement* pAxis = new IwAxis2Placement();
				pAxis->RotateAboutAxis(dAngx, *pVecXaxis);
				pAxis->RotateAboutAxis(dAngy, *pVecYaxis);
				pAxis->RotateAboutAxis(dAngz, *pVecZaxis);
				sStatus =((IwBrep *) m_pIwObj)->Transform(*pAxis, pVecScale);
				if (sStatus != IW_SUCCESS)
				{
					String __gc* sInfo = "Rotation of brep failed.";
					System::Windows::Forms::MessageBox::Show(sInfo);
				}

				// Update the result in the DOM
				AddToDOM ();
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
			}
		}
		catch (...)
		{
			// TODO
		}
	}

	void Brep::RotateAboutPoint(double dAng, double dOrigX, double dOrigY, double dOrigZ,
		double dAxisX, double dAxisY, double dAxisZ)
	{
		IwVector3d* pVecScale = new IwVector3d(1.0, 1.0, 1.0);
		IwVector3d* pPtOrigin = new IwVector3d(dOrigX, dOrigY, dOrigZ);
		IwVector3d* pVecAxis = new IwVector3d(dAxisX, dAxisY, dAxisZ);
		try
		{
			if (m_pIwObj != NULL)
			{
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
				IwStatus sStatus = IW_SUCCESS;

				// Transform Brep by rotating about an axis thru a point
				IwAxis2Placement* pAxis = new IwAxis2Placement();
				pAxis->RotateAboutAxisAtPoint(dAng, *pPtOrigin, *pVecAxis);
				sStatus =((IwBrep *) m_pIwObj)->Transform(*pAxis, pVecScale);
				if (sStatus != IW_SUCCESS)
				{
					String __gc* sInfo = "Rotation of brep failed.";
					System::Windows::Forms::MessageBox::Show(sInfo);
				}

				// Update the result in the DOM
				AddToDOM ();
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
			}
		}
		catch (...)
		{
			// TODO
		}
	}

	void Brep::Simplify()
	{
		try
		{
			if (m_pIwObj != NULL)
			{
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;

				// Try to simplify brep by removing extraneous topological features; this
				// should have the effect of improving the tesselation and rendering of the brep.

				IwStatus sStatus = IW_SUCCESS;
				sStatus = ((IwBrep *) m_pIwObj)->RemoveTopologicalEdgesAndVertices();


				if (sStatus != IW_SUCCESS)
				{
					String* sInfo = "Simplification of brep failed.";
					System::Windows::Forms::MessageBox::Show(sInfo);
				}

				AddToDOM ();
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
			}
		}
		catch (...)
		{
			// TODO
		}
	}

	void Brep::SewFaces ()
	{
		try
		{
			if (m_pIwObj != NULL)
			{
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;

				// Try to sew the faces together and make the Brep manifold 
				// (or cellular) but don't fail if you can't.

				ULONG lNumSewn, lNumLam;
				double dMaxVGap, dMaxEGap;
				IwStatus sStatus = IW_SUCCESS;
				double dTol = this->Tolerance;

				sStatus = ((IwBrep *) m_pIwObj)->SewFaces(
					dTol, lNumSewn, lNumLam, dMaxVGap, dMaxEGap);

				if (sStatus != IW_SUCCESS)
				{
					StringBuilder* sInfo = new StringBuilder ("");
					sInfo->AppendFormat ("Could not sew faces.\nMax edge gap is {0}\nMax vertex gap is {1}\n",
						dMaxEGap.ToString (), dMaxVGap.ToString ());
					sInfo->AppendFormat ("Num Sewn = {0}\nNum Lamina = {1}", 
						lNumSewn.ToString (), lNumLam.ToString ());
					System::Windows::Forms::MessageBox::Show(sInfo->ToString ());
				}

				// Update the DOM. However we must first delete all face counter and
				// region counter attributes as they were inherited from the brep faces 
				// and regions that were merged in and hence may not be unique in this 
				// brep so we will assign new face and region counters;
				/*
				IwTArray<IwFace *> arrFaces;
				((IwBrep *) m_pIwObj)->GetFaces (arrFaces);
				ULONG numFaces = arrFaces.GetSize ();
				for (ULONG iFace = 0; iFace < numFaces; iFace++)
				{
				IwAttribute *pAttribute = 0;

				while ((pAttribute = arrFaces[iFace]->FindAttribute (AttributeID_BREPFACECOUNTER)))
				arrFaces[iFace]->RemoveAttribute (pAttribute);
				}
				*/
				IwTArray<IwRegion *> arrRegions;
				((IwBrep *) m_pIwObj)->GetRegions (arrRegions);
				ULONG numRegions = arrRegions.GetSize ();
				for (ULONG iRegion = 0; iRegion < numRegions; iRegion++)
				{
					IwAttribute *pAttribute = 0;

					while ((pAttribute = arrRegions[iRegion]->FindAttribute (AttributeID_BREPREGIONCOUNTER)))
						arrRegions[iRegion]->RemoveAttribute (pAttribute, TRUE);
				}

				AddToDOM ();
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
			}
		}
		catch (System::Exception *ex)
		{
			System::Console::WriteLine (ex->Message);
		}
		catch (...)
		{
			// TODO
		}
	}

	void Brep::CreateBrepFromRegionsSlow (System::Collections::ArrayList __gc *listRegions, bool bRemoveFromOriginal)
	{
		try
		{
			// This routine copies all of the regions in the region list into this brep. It is assumed that the
			// input regions in listRegions are all from the same brep. Any that are not from the same brep as
			// the first region in the list will be ignored.

			IwRegion *regionToCopy = 0;
			IwTArray<IwFace *> arrFacesToCopy;
			IwBrep * pThisBrep = (IwBrep *) GetIwObj();
			IwBrep * pSourceBrep = 0;

			if (m_pIwObj != NULL)
			{
				// Find the region with the input id as an attribute.

				IwTArray<IwRegion *> arrRegionsToCopy;
				int nRegions = 0;

				for (int i = 0; i < listRegions->Count; i++)
				{
					BrepRegionProxy * regionProxy = dynamic_cast<BrepRegionProxy *> (listRegions->get_Item(i));

					// Get the brep containing the input regions. Skip any regions that are from a different brep.

					if (i == 0)
						pSourceBrep = (IwBrep *) regionProxy->Brep->GetIwObj();
					else if (pSourceBrep != (IwBrep *) regionProxy->Brep->GetIwObj())
						continue;

					// Add the IwRegion to the array.

					IwRegion *pIwRegion = regionProxy->Brep->GetIwRegionFromID (regionProxy->RegionID);
					if (pIwRegion)
						arrRegionsToCopy.Add (pIwRegion);
				}

				if (arrRegionsToCopy.GetSize() > 0)
				{
					// Extract the region's faces as faces to copy.
					for (unsigned int i = 0; i < arrRegionsToCopy.GetSize(); i++)
					{
						IwTArray<IwShell *> arrShells;
						arrRegionsToCopy[i]->GetShells (arrShells);
						int nShells = arrShells.GetSize ();

						for (int iShell = 0; iShell < nShells; iShell++)
						{
							IwTArray<IwFaceuse *> arrFaceUses;
							arrShells[iShell]->GetFaceuses (arrFaceUses);
							int nFaceUses = arrFaceUses.GetSize ();

							for (int iFaceUse = 0; iFaceUse < nFaceUses; iFaceUse++)
							{
								arrFacesToCopy.AddUnique (arrFaceUses[iFaceUse]->GetFace ());
							}
						}
					}

					// New approach: copy whole brep, then delete faces NOT in region. This turns out to be pretty
					// slow as breps get more complex and we get lots of compartments in subdivision app so use
					// a dictionary with face id as the key to expedite things.

					IwBrep *pCopyBrep = new (GetIwContext()) IwBrep (*(pSourceBrep));
					pCopyBrep->m_bEditingEnabled = TRUE;
					IwTArray<IwFace *>arrCopyFaces;

					pCopyBrep->GetFaces(arrCopyFaces);
					std::map<long, IwFace *> mapCopyFaces;
					for (unsigned int iFace = 0; iFace < arrCopyFaces.GetSize (); iFace++)
					{
						IwAttribute *pCopyAttrib = arrCopyFaces[iFace]->FindAttribute( AttributeID_BREPFACECOUNTER);
						long lCopyId = pCopyAttrib->GetLongElementsAddress()[0];
						mapCopyFaces.insert (map<long, IwFace*> ::value_type(lCopyId, arrCopyFaces[iFace]));
					}
					for (unsigned int iKeepFace = 0; iKeepFace < arrFacesToCopy.GetSize (); iKeepFace++)
					{
						IwAttribute *pKeepAttrib = arrFacesToCopy[iKeepFace]->FindAttribute( AttributeID_BREPFACECOUNTER);
						long lKeepId = pKeepAttrib->GetLongElementsAddress()[0];
						IwFace *pFaceToKeep = mapCopyFaces[lKeepId]; // find the face with that ID
						ULONG nFoundIndex = 0;
						if (NULL != pFaceToKeep && arrCopyFaces.FindElement (pFaceToKeep, nFoundIndex))
							arrCopyFaces.RemoveAt (nFoundIndex);
					}
					if (arrCopyFaces.GetSize ())
						pCopyBrep->RemoveFaces (arrCopyFaces);

					//				pCopyBrep->SewAndOrient ();
					//pCopyBrep->MakeManifold(NULL, TRUE);
					IwBoolean bMaybeNotClosedSolid;
					if (pCopyBrep->IsManifoldSolid ())
						pCopyBrep->OrientTrimmedSurfaces (FALSE, bMaybeNotClosedSolid, TRUE);
					pCopyBrep->RemoveTopologicalEdgesAndVertices();
					pCopyBrep->m_bEditingEnabled = FALSE;
					this->m_pIwObj->~IwObject();
					this->m_pIwObj = pCopyBrep;
				}
			}
		}
		catch (System::Exception *ex)
		{
			System::Console::WriteLine (ex->Message);
		}
		catch (...)
		{
			// TODO
		}
	}

	void Brep::CreateBrepFromFace (BrepFaceProxy __gc *face)
	{
		IwTArray<IwFace *> arrFacesToCopy;
		IwBrep * pThisBrep = (IwBrep *) GetIwObj();
		IwBrep * pSourceBrep = (IwBrep *) face->Brep->GetIwObj();;

		if (m_pIwObj != NULL)
		{
			arrFacesToCopy.AddUnique (face->GetIwFace());

			// Copy the faces into the destination brep
			pThisBrep->m_bEditingEnabled = TRUE;
			pThisBrep->m_bMakeComposites = TRUE;
			pThisBrep->SetTolerance(pSourceBrep->GetTolerance());
			IwStatus status = pSourceBrep->CopyFaces (arrFacesToCopy, pThisBrep );
			if (status != IW_SUCCESS)
			{
				System::Windows::Forms::MessageBox::Show("Failed to copy original Brep faces to destination Brep.");
			}

			pThisBrep->m_bEditingEnabled = FALSE;
		}
	}

	void Brep::CreateBrepFromRegions (System::Collections::ArrayList __gc *listRegions, bool bRemoveFromOriginal)
	{
		// This routine copies all of the regions in the region list into this brep. It is assumed that the
		// input regions in listRegions are all from the same brep. Any that are not from the same brep as
		// the first region in the list will be ignored.

		IwRegion *regionToCopy = 0;
		IwTArray<IwFace *> arrFacesToCopy;
		IwBrep * pThisBrep = (IwBrep *) GetIwObj();
		IwBrep * pSourceBrep = 0;

		if (m_pIwObj != NULL)
		{
			// Find the region with the input id as an attribute.

			IwTArray<IwRegion *> arrRegionsToCopy;
			int nRegions = 0;

			for (int i = 0; i < listRegions->Count; i++)
			{
				BrepRegionProxy * regionProxy = dynamic_cast<BrepRegionProxy *> (listRegions->get_Item(i));

				// Get the brep containing the input regions. Skip any regions that are from a different brep.

				if (i == 0)
					pSourceBrep = (IwBrep *) regionProxy->Brep->GetIwObj();
				else if (pSourceBrep != (IwBrep *) regionProxy->Brep->GetIwObj())
					continue;

				// Add the IwRegion to the array.

				IwRegion *pIwRegion = regionProxy->Brep->GetIwRegionFromID (regionProxy->RegionID);
				if (pIwRegion)
					arrRegionsToCopy.Add (pIwRegion);
			}

			if (arrRegionsToCopy.GetSize() > 0)
			{
				// Extract the region's faces as faces to copy.
				for (unsigned int i = 0; i < arrRegionsToCopy.GetSize(); i++)
				{
					IwTArray<IwShell *> arrShells;
					arrRegionsToCopy[i]->GetShells (arrShells);
					int nShells = arrShells.GetSize ();

					for (int iShell = 0; iShell < nShells; iShell++)
					{
						IwTArray<IwFaceuse *> arrFaceUses;
						arrShells[iShell]->GetFaceuses (arrFaceUses);
						int nFaceUses = arrFaceUses.GetSize ();

						for (int iFaceUse = 0; iFaceUse < nFaceUses; iFaceUse++)
						{
							arrFacesToCopy.AddUnique (arrFaceUses[iFaceUse]->GetFace ());
						}
					}
				}

				// Copy the faces into the destination brep
				pThisBrep->m_bEditingEnabled = TRUE;
				pThisBrep->m_bMakeComposites = TRUE;
				pThisBrep->SetTolerance(pSourceBrep->GetTolerance());
//				pSourceBrep->SetTolerance (0.0);
//				pThisBrep->SetTolerance(0.0);
				IwStatus status = pSourceBrep->CopyFaces (arrFacesToCopy, pThisBrep );
				if (status != IW_SUCCESS)
				{
					System::Windows::Forms::MessageBox::Show("Failed to copy original Brep faces to destination Brep.");
				}

				this->SewFaces();
//				pThisBrep->RemoveTopologicalEdgesAndVertices ();
				pThisBrep->m_bEditingEnabled = FALSE;
/*
				// New approach: copy whole brep, then delete faces NOT in region. This turns out to be pretty
				// slow as breps get more complex and we get lots of compartments in subdivision app so use
				// a dictionary with face id as the key to expedite things.

				IwBrep *pCopyBrep = new (GetIwContext()) IwBrep (*(pSourceBrep));
				pCopyBrep->m_bEditingEnabled = TRUE;
				IwTArray<IwFace *>arrCopyFaces;

				pCopyBrep->GetFaces(arrCopyFaces);
				std::map<long, IwFace *> mapCopyFaces;
				for (unsigned int iFace = 0; iFace < arrCopyFaces.GetSize (); iFace++)
				{
					IwAttribute *pCopyAttrib = arrCopyFaces[iFace]->FindAttribute( AttributeID_BREPFACECOUNTER);
					long lCopyId = pCopyAttrib->GetLongElementsAddress()[0];
					mapCopyFaces.insert (map<long, IwFace*> ::value_type(lCopyId, arrCopyFaces[iFace]));
				}
				for (unsigned int iKeepFace = 0; iKeepFace < arrFacesToCopy.GetSize (); iKeepFace++)
				{
					IwAttribute *pKeepAttrib = arrFacesToCopy[iKeepFace]->FindAttribute( AttributeID_BREPFACECOUNTER);
					long lKeepId = pKeepAttrib->GetLongElementsAddress()[0];
					IwFace *pFaceToKeep = mapCopyFaces[lKeepId]; // find the face with that ID
					ULONG nFoundIndex = 0;
					if (NULL != pFaceToKeep && arrCopyFaces.FindElement (pFaceToKeep, nFoundIndex))
						arrCopyFaces.RemoveAt (nFoundIndex);
				}
				if (arrCopyFaces.GetSize ())
					pCopyBrep->RemoveFaces (arrCopyFaces);

//				pCopyBrep->SewAndOrient ();
				//pCopyBrep->MakeManifold(NULL, TRUE);
				IwBoolean bMaybeNotClosedSolid;
				if (pCopyBrep->IsManifoldSolid ())
					pCopyBrep->OrientTrimmedSurfaces (FALSE, bMaybeNotClosedSolid, TRUE);
				pCopyBrep->RemoveTopologicalEdgesAndVertices();
				pCopyBrep->m_bEditingEnabled = FALSE;
				this->m_pIwObj->~IwObject();
				this->m_pIwObj = pCopyBrep;*/
			}
		}
	}

	bool  Brep::MergeBreps (Brep* brepResult, Brep* vecBreps __gc[], BooleanMergeType oMergeType, bool bSewFaces, bool bMakeManifold)
	{
		bool bRet = true;

		try
		{
			// Convert from our enumeration to SMLib

			IwBooleanOperationType oBOpType;

			switch (oMergeType)
			{
			case BooleanMerge_Union:
			default:
				oBOpType = IW_BO_UNION;
				break;
			case BooleanMerge_Intersection:
				oBOpType = IW_BO_INTERSECTION;
				break;
			case BooleanMerge_Difference:
				oBOpType = IW_BO_DIFFERENCE;
				break;
			case BooleanMerge_Merge:
				oBOpType = IW_BO_MERGE;
				break;
			case BooleanMerge_PartialMerge:
				oBOpType = IW_BO_PARTIAL_MERGE;
				break;
			}

			double dAngleTol = 20.0*IW_PI/180.0;
			double dApproxTol = 0.;

			int iSize = vecBreps->GetUpperBound (0) - vecBreps->GetLowerBound (0) + 1;
			IwBrep *pResult = NULL;
			IwBrep *pBrep1 = new (brepResult->GetIwContext())IwBrep (*((IwBrep *) vecBreps[0]->GetIwObj ()));
			pBrep1->m_bEditingEnabled = TRUE;
			pBrep1->m_bMakeComposites = TRUE;

			for (int iBrep = 1; iBrep < iSize; iBrep++)
			{
				IwStatus status = IW_SUCCESS;
				try
				{
					IwBrep *pBrep2 = new (brepResult->GetIwContext()) IwBrep (*((IwBrep *) vecBreps[iBrep]->GetIwObj ()));
					pBrep2->m_bEditingEnabled = TRUE;
					pBrep2->m_bMakeComposites = TRUE;

					// This seems to be necessary to avoid tolerance issues in the merge operation
					pBrep1->SewAndOrient ();
					pBrep1->SetTolerance (0.0);
					pBrep2->SewAndOrient ();
					pBrep2->SetTolerance (0.0);

					// Get the tolerance for the merge object (after resetting brep tol)
					dApproxTol = max (pBrep1->GetTolerance (), pBrep2->GetTolerance ());

					IwMerge oMerge (brepResult->GetIwContext(), pBrep1, pBrep2, dApproxTol, dAngleTol);
					pResult = NULL; // pResult is allocated by the boolean operation

					// According to SMLIB documentation, only use ManifoldBoolean if both breps are manifold solids
					if (pBrep1->IsManifoldSolid () && pBrep2->IsManifoldSolid ())
						status = oMerge.ManifoldBoolean (oBOpType, pResult);
					else
						status = oMerge.NonManifoldBoolean (oBOpType, pResult);
				}
				catch(Exception *e)
				{
					Console::WriteLine(e->Message);
					status = 0; // not IW_SUCCESS
				}
				if (status != IW_SUCCESS)
				{
					System::Windows::Forms::MessageBox::Show("BRep merge operation failed.");
					bRet = false;
					pResult = pBrep1; // in case this was the last brep and the merge failed we still have something to return
				}
				else // if successful set brep1 to previous result.
				{
					pBrep1 = pResult;
				}
			}

			if (pResult != NULL)
			{
				// Try to sew the faces together and make the Brep manifold 
				// (or cellular) but don't fail if you can't.

				ULONG lNumSewn, lNumLam;
				double dMaxVGap, dMaxEGap;
				IwStatus sStatus = IW_SUCCESS;

				if (bSewFaces)
				{
					if (IW_SUCCESS != pResult->SewFaces(
						brepResult->Tolerance, lNumSewn, lNumLam, dMaxVGap, dMaxEGap))
					{
						bRet = false;
					}
				}

				if (bMakeManifold)
				{
					if (IW_SUCCESS != pResult->MakeManifold(NULL,TRUE))
						bRet = false;
				}

				// Set the counters dirty flag since topology counters may no longer be unique.

				brepResult->m_bCountersDirty = true;

				// Attach IwObj and update the DOM

				pResult->m_bEditingEnabled = FALSE;
				brepResult->AttachIwObj (brepResult->m_pContext, pResult);
				//				brepResult->m_pIwObj = pResult;
				//				brepResult->AddToDOM ();
			}
		}
		catch (...)
		{
			return false;
		}
		return bRet;
	}



	bool  Brep::NonManifoldMergeBreps (Brep* brepResult, Brep* vecBreps __gc[], BooleanMergeType oMergeType, bool bSewFaces, bool bMakeManifold)
	{
		bool bRet = true;

		try
		{
			// Convert from our enumeration to SMLib

			IwBooleanOperationType oBOpType;

			switch (oMergeType)
			{
			case BooleanMerge_Union:
			default:
				oBOpType = IW_BO_UNION;
				break;
			case BooleanMerge_Intersection:
				oBOpType = IW_BO_INTERSECTION;
				break;
			case BooleanMerge_Difference:
				oBOpType = IW_BO_DIFFERENCE;
				break;
			case BooleanMerge_Merge:
				oBOpType = IW_BO_MERGE;
				break;
			case BooleanMerge_PartialMerge:
				oBOpType = IW_BO_PARTIAL_MERGE;
				break;
			}

			double dAngleTol = 20.0*IW_PI/180.0;
			double dApproxTol = brepResult->Tolerance;


			int iSize = vecBreps->GetUpperBound (0) - vecBreps->GetLowerBound (0) + 1;
			IwBrep *pResult = new (brepResult->GetIwContext())IwBrep (*((IwBrep *) vecBreps[0]->GetIwObj ()));
			pResult->m_bEditingEnabled = TRUE;
			pResult->m_bMakeComposites = TRUE;

			for (int iBrep = 1; iBrep < iSize; iBrep++)
			{
				IwStatus status = IW_SUCCESS;
				try
				{
					IwBrep *pBrep = new (brepResult->GetIwContext()) IwBrep (*((IwBrep *) vecBreps[iBrep]->GetIwObj ()));
					pBrep->m_bEditingEnabled = TRUE;
					pBrep->m_bMakeComposites = TRUE;

					IwMerge oMerge (brepResult->GetIwContext(), pResult, pBrep, dApproxTol, dAngleTol);
					status = oMerge.NonManifoldBoolean (oBOpType, pResult);
				}
				catch(Exception *e)
				{
					Console::WriteLine(e->Message);
					status = 0;
				}
				if (status != IW_SUCCESS)
				{
					System::Windows::Forms::MessageBox::Show("BRep merge operation failed.");
					bRet = false;
				}
			}

			if (pResult != NULL)
			{
				// Try to sew the faces together and make the Brep manifold 
				// (or cellular) but don't fail if you can't.

				ULONG lNumSewn, lNumLam;
				double dMaxVGap, dMaxEGap;
				IwStatus sStatus = IW_SUCCESS;

				if (bSewFaces)
				{
					if (IW_SUCCESS != pResult->SewFaces(
						brepResult->Tolerance, lNumSewn, lNumLam, dMaxVGap, dMaxEGap))
					{
						bRet = false;
					}
				}

				if (bMakeManifold)
				{
					if (IW_SUCCESS != pResult->MakeManifold(NULL,TRUE))
						bRet = false;
				}

				// Set the counters dirty flag since topology counters may no longer be unique.

				brepResult->m_bCountersDirty = true;

				// Attach IwObj and update the DOM

				pResult->m_bEditingEnabled = FALSE;
				brepResult->AttachIwObj (brepResult->m_pContext, pResult);
				//				brepResult->m_pIwObj = pResult;
				//				brepResult->AddToDOM ();
			}
		}
		catch (...)
		{
			return false;
		}
		return bRet;
	}


	void Brep::CreateBoundedPlane (Plane __gc *newPlane, Vector3d __gc *ptMin, Vector3d __gc *ptMax)
	{
		// Create a face from a plane with bounds. newPlane gets consumed.

		if (m_pIwObj == NULL || m_pXMLElem == NULL)
		{
			// todo: throw exception
		}

		((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
		((IwBrep *) m_pIwObj)->m_bMakeComposites = TRUE;

		double dAngleTol = 20.0*IW_PI/180.0;
		double dApproxTol = this->Tolerance;

		IwFace *pNewFace = 0;
		IwExtent3d oBox;
		oBox.SetMinMax (*(ptMin->GetIwObj ()), *(ptMax->GetIwObj ()));
		double dPad = oBox.GetSize ().Length () / 10.;
		oBox.ExpandAbsolute (dPad);
		String __gc *sId = newPlane->GetIwObjAttribute();
		IwPlane *pPlane = (IwPlane *) newPlane->ExtractIwObj ();
		if (pPlane)
		{
			// Walk through all of the corners of the expanded extents box projecting them
			// to the plane and extract the min and max projections to use as the plane domain

			IwPoint2d ptMax (System::Double::MinValue, System::Double::MinValue), ptMin (System::Double::MaxValue, System::Double::MaxValue);
			IwTArray<IwPoint3d> arrPoints;
			arrPoints.Add (oBox.GetMin ());
			arrPoints.Add (oBox.GetMax ());

			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					for (int k = 0; k < 2; k++)
					{
						IwPoint3d ptToProject;
						IwPoint2d ptProjected;
						ptToProject.Set (arrPoints[i].x, arrPoints[j].y, arrPoints[k].z);
						if (IW_SUCCESS == pPlane->ProjectPointToUVDomain (ptToProject, ptProjected))
						{
							ptMin.x = min (ptMin.x, ptProjected.x);
							ptMin.y = min (ptMin.y, ptProjected.y);
							ptMax.x = max (ptMax.x, ptProjected.x);
							ptMax.y = max (ptMax.y, ptProjected.y);
						}
					}
				}
			}

			// Check for valid min/max points

			if (ptMin.GetMaxDimension () == System::Double::MaxValue || ptMax.GetMaxDimension () == System::Double::MinValue)
			{
				//TODO: throw and exception
				return;
			}

			IwExtent2d oDomain;
			oDomain.AddPoint2d (ptMin);
			oDomain.AddPoint2d (ptMax);
			if (IW_SUCCESS != (pPlane->TrimWithDomain (oDomain)))
			{
				// todo: throw exception
				System::Windows::Forms::MessageBox::Show("Error trimming plane to domain defined by bounding box.");
			}

			if (IW_SUCCESS != ((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
				pPlane, pPlane->GetNaturalUVDomain (), pNewFace))
			{
				// todo: throw exception
				System::Windows::Forms::MessageBox::Show("Error creating brep face from bounded planar surface.");
			}
		}
		if (m_pIwObj != NULL)
		{
			// Add to DOM. Set the counters dirty flag since topology counters 
			// may no longer be unique.

			m_bCountersDirty = true;

			AddToDOM ();
			((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
		}
	}


	void Brep::CreateConePatch (Vector3d __gc * ptOrigin, Vector3d __gc * vecAxis, 
		double dBottomRadius, double dTopRadius, double dHeight, bool bCapped)
	{

		if (m_pIwObj == NULL || m_pXMLElem == NULL)
		{
			// todo: throw exception
		}

		((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
		((IwBrep *) m_pIwObj)->m_bMakeComposites = TRUE;

		IwVector3d vecZaxis = *(vecAxis->GetIwObj());
		vecZaxis.Unitize();
		IwVector3d vecXRefAxis;
		if (fabs(vecZaxis.x) < .5)
			vecXRefAxis.Set(1.0, 0.0, 0.0);
		else
			vecXRefAxis.Set(0.0, 1.0, 0.0);

		IwAxis2Placement * axes = new IwAxis2Placement();
		axes->SetSTEPCanonical(*(ptOrigin->GetIwObj()), vecZaxis, vecXRefAxis);

		IwFace *pNewFace = NULL;
		IwBSplineSurface *pNewSrf = NULL;
		IwAttribute *pAttribute = NULL;


		if (IW_SUCCESS != IwBSplineSurface::CreateConePatch(m_pContext->GetIwContext(), *axes, dBottomRadius, dTopRadius, 
			0.0, 360.0, dHeight, IW_CO_QUADRATIC, pNewSrf))
		{
			// Throw exception
			throw new Exception("Error creating cone patch.");
		}
		// Create and add an IdSelf attribute to this surface
		pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
		pNewSrf->AddAttribute(pAttribute);

		if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
			pNewSrf, pNewSrf->GetNaturalUVDomain (), pNewFace)))
		{
			// Throw exception
			throw new Exception ("Error creating face from surface (cylinder)");
		}

		// Create capped ends if required.
		if (bCapped)
		{
			double dMaxDim = fabs(dBottomRadius);
			if (fabs(dTopRadius) > dMaxDim)
				dMaxDim = fabs(dTopRadius);
			if (fabs(dHeight) > dMaxDim)
				dMaxDim = fabs(dHeight);
			dMaxDim *= 1.1; // Pad the cap size by 10%

			// Create a plane on the bottom of the cone/cylinder
			IwVector3d ptBottom = *(ptOrigin->GetIwObj());
			IwVector3d vecTop = *(vecAxis->GetIwObj());
			vecTop.Unitize();
			IwExtent2d uvExtent;
			IwPoint2d ptMin;
			IwPoint2d ptMax;
			ptMin.Set(-dMaxDim, -dMaxDim);
			ptMax.Set(dMaxDim, dMaxDim);
			uvExtent.SetMinMax(ptMin, ptMax);

			IwPlane * bottomPlate = new (GetIwContext())IwPlane(ptBottom, vecTop);
			bottomPlate->TrimWithDomain(uvExtent);
			// Create and add an IdSelf attribute to this surface
			pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
			bottomPlate->AddAttribute(pAttribute);

			// Create temporary brep
			IwBrep * brepBottom = new (GetIwContext()) IwBrep();
			if (IW_SUCCESS != brepBottom->CreateFaceFromSurface (
				bottomPlate, bottomPlate->GetNaturalUVDomain (), pNewFace))
			{
				// Throw exception
				throw new Exception ("Error creating cap on bottom of cone/cylinder.");
			}
			else
				brepBottom->m_bEditingEnabled = TRUE;

			// Create a plane on the top of the cone/cylinder
			IwVector3d ptTop;
			ptTop.Set(ptBottom.x + vecTop.x * dHeight,
				ptBottom.y + vecTop.y * dHeight,
				ptBottom.z + vecTop.z * dHeight);
			IwPlane * topPlate = new (GetIwContext())IwPlane(ptTop, vecTop);
			topPlate->TrimWithDomain(uvExtent);
			// Create and add an IdSelf attribute to this surface
			pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
			topPlate->AddAttribute(pAttribute);

			// Create temporary brep
			IwBrep * brepTop = new (GetIwContext()) IwBrep();
			if (IW_SUCCESS != brepTop->CreateFaceFromSurface (
				topPlate, topPlate->GetNaturalUVDomain (), pNewFace))
			{
				// Throw exception
				throw new Exception ("Error creating cap on top of cone/cylinder.");
			}
			else
				brepTop->m_bEditingEnabled = TRUE;

			// Merge breps
			((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
			double dAngleTol = 20.0*IW_PI/180.0;
			double dApproxTol = this->Tolerance;
			IwBrep * pResult = 0;
			IwMerge oMerge (GetIwContext(), (IwBrep *) m_pIwObj, brepBottom, dApproxTol, dAngleTol);
			if (IW_SUCCESS == oMerge.ManifoldBoolean (IW_BO_MERGE, pResult))
			{
				brepBottom = 0;
				m_pIwObj = pResult;
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
				IwMerge oMerge2 (GetIwContext(), (IwBrep *) m_pIwObj, brepTop, dApproxTol, dAngleTol);
				if (IW_SUCCESS == oMerge2.ManifoldBoolean (IW_BO_MERGE, pResult))
				{
					brepTop = 0;
					m_pIwObj = pResult;
					if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->MakeManifold(NULL,TRUE)))
					{
						// Throw exception
						//throw new Exception ("Error making capped cone/cylinder manifold.");
					}
				}
			}
			((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
		}

		if (m_pIwObj != NULL)
		{
			// Add to DOM. Set the counters dirty flag since topology counters 
			// may no longer be unique.

			m_bCountersDirty = true;
			AddToDOM ();
			((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
		}
	}

	void Brep::CreateSphere (Vector3d __gc * ptOrigin, Vector3d __gc * vecAxis, double dRadius)
	{

		if (m_pIwObj == NULL || m_pXMLElem == NULL)
		{
			// todo: throw exception
		}

		((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
		((IwBrep *) m_pIwObj)->m_bMakeComposites = TRUE;

		IwVector3d vecZaxis = *(vecAxis->GetIwObj());
		IwVector3d vecXRefAxis;
		if (fabs(vecZaxis.x) < .5)
			vecXRefAxis.Set(1.0, 0.0, 0.0);
		else
			vecXRefAxis.Set(0.0, 1.0, 0.0);

		IwAxis2Placement * axes = new IwAxis2Placement();
		axes->SetSTEPCanonical(*(ptOrigin->GetIwObj()), vecZaxis, vecXRefAxis);

		IwFace *pNewFace = NULL;
		IwSphere *pNewSrf = NULL;
		IwAttribute *pAttribute = NULL;

		if (IW_SUCCESS != IwSphere::CreateCanonical(m_pContext->GetIwContext(), *axes, dRadius, pNewSrf))
		{
			// Throw exception
			throw new Exception("Error creating canonical sphere.");
		}
		// Create and add an IdSelf attribute to this surface
		pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
		pNewSrf->AddAttribute(pAttribute);

		if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
			pNewSrf, pNewSrf->GetNaturalUVDomain (), pNewFace)))
		{
			// Throw exception
			throw new Exception ("Error creating face from surface (cylinder)");
		}

		if (m_pIwObj != NULL)
		{
			// Add to DOM. Set the counters dirty flag since topology counters 
			// may no longer be unique.

			m_bCountersDirty = true;

			AddToDOM ();
			((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
		}
	}

	void Brep::CreateBilinearPatch (Vector3d __gc * ptU0V0, Vector3d __gc * ptU1V0, Vector3d __gc * ptU0V1, Vector3d __gc * ptU1V1)
	{

		if (m_pIwObj == NULL || m_pXMLElem == NULL)
		{
			// todo: throw exception
		}

		((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
		((IwBrep *) m_pIwObj)->m_bMakeComposites = TRUE;

		IwFace *pNewFace = NULL;
		IwBSplineSurface *pNewSrf = NULL;
		IwAttribute *pAttribute = NULL;

		// Create box from six bi-linear surfaces

		IwVector3d *pt00 = new IwVector3d();
		IwVector3d *pt01 = new IwVector3d();
		IwVector3d *pt10 = new IwVector3d();
		IwVector3d *pt11 = new IwVector3d();

		// Set Face
		pt00->Set(ptU0V0->X, ptU0V0->Y, ptU0V0->Z);
		pt10->Set(ptU1V0->X, ptU1V0->Y, ptU1V0->Z);
		pt01->Set(ptU0V1->X, ptU0V1->Y, ptU0V1->Z);
		pt11->Set(ptU1V1->X, ptU1V1->Y, ptU1V1->Z);

		if (IW_SUCCESS != IwBSplineSurface::CreateBilinearSurface(
			m_pContext->GetIwContext(), *pt00, *pt10, *pt01, *pt11, pNewSrf))
		{
			// Throw exception
			throw new Exception("Error creating cylinder through box.");
		}
		// Create and add an IdSelf attribute to this surface
		pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
		pNewSrf->AddAttribute(pAttribute);

		if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
			pNewSrf, pNewSrf->GetNaturalUVDomain (), pNewFace)))
		{
			// Throw exception
			throw new Exception ("Error creating face from bi-linear surface");
		}

		if (m_pIwObj != NULL)
		{
			// Add to DOM. Set the counters dirty flag since topology counters 
			// may no longer be unique.

			m_bCountersDirty = true;

			AddToDOM ();
			((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
		}
	}

	void Brep::CreateBox (Vector3d __gc * ptMin, Vector3d __gc * ptMax)
	{

		if (m_pIwObj == NULL || m_pXMLElem == NULL)
		{
			// todo: throw exception
		}

		((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
		((IwBrep *) m_pIwObj)->m_bMakeComposites = TRUE;

		IwFace *pNewFace = NULL;
		IwBSplineSurface *pNewSrf = NULL;
		IwAttribute *pAttribute = NULL;

		// Create box from six bi-linear surfaces

		IwVector3d *pt00 = new IwVector3d();
		IwVector3d *pt01 = new IwVector3d();
		IwVector3d *pt10 = new IwVector3d();
		IwVector3d *pt11 = new IwVector3d();

		// First face
		pt00->Set(ptMin->X, ptMin->Y, ptMin->Z);
		pt10->Set(ptMax->X, ptMin->Y, ptMin->Z);
		pt01->Set(ptMin->X, ptMax->Y, ptMin->Z);
		pt11->Set(ptMax->X, ptMax->Y, ptMin->Z);

		if (IW_SUCCESS != IwBSplineSurface::CreateBilinearSurface(
			m_pContext->GetIwContext(), *pt00, *pt10, *pt01, *pt11, pNewSrf))
		{
			// Throw exception
			throw new Exception("Error creating cylinder through box.");
		}
		// Create and add an IdSelf attribute to this surface
		pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
		pNewSrf->AddAttribute(pAttribute);

		if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
			pNewSrf, pNewSrf->GetNaturalUVDomain (), pNewFace)))
		{
			// Throw exception
			throw new Exception ("Error creating face from bi-linear surface");
		}

		// Second face
		pt00->Set(ptMin->X, ptMin->Y, ptMax->Z);
		pt10->Set(ptMax->X, ptMin->Y, ptMax->Z);
		pt01->Set(ptMin->X, ptMax->Y, ptMax->Z);
		pt11->Set(ptMax->X, ptMax->Y, ptMax->Z);

		if (IW_SUCCESS != IwBSplineSurface::CreateBilinearSurface(
			m_pContext->GetIwContext(), *pt00, *pt10, *pt01, *pt11, pNewSrf))
		{
			// Throw exception
			throw new Exception("Error creating cylinder through box.");
		}
		// Create and add an IdSelf attribute to this surface
		pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
		pNewSrf->AddAttribute(pAttribute);

		if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
			pNewSrf, pNewSrf->GetNaturalUVDomain (), pNewFace)))
		{
			// Throw exception
			throw new Exception ("Error creating face from bi-linear surface");
		}


		// Third face
		pt00->Set(ptMin->X, ptMin->Y, ptMin->Z);
		pt10->Set(ptMin->X, ptMax->Y, ptMin->Z);
		pt01->Set(ptMin->X, ptMin->Y, ptMax->Z);
		pt11->Set(ptMin->X, ptMax->Y, ptMax->Z);

		if (IW_SUCCESS != IwBSplineSurface::CreateBilinearSurface(
			m_pContext->GetIwContext(), *pt00, *pt10, *pt01, *pt11, pNewSrf))
		{
			// Throw exception
			throw new Exception("Error creating cylinder through box.");
		}
		// Create and add an IdSelf attribute to this surface
		pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
		pNewSrf->AddAttribute(pAttribute);

		if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
			pNewSrf, pNewSrf->GetNaturalUVDomain (), pNewFace)))
		{
			// Throw exception
			throw new Exception ("Error creating face from bi-linear surface");
		}

		// Fourth face
		pt00->Set(ptMax->X, ptMin->Y, ptMin->Z);
		pt10->Set(ptMax->X, ptMax->Y, ptMin->Z);
		pt01->Set(ptMax->X, ptMin->Y, ptMax->Z);
		pt11->Set(ptMax->X, ptMax->Y, ptMax->Z);

		if (IW_SUCCESS != IwBSplineSurface::CreateBilinearSurface(
			m_pContext->GetIwContext(), *pt00, *pt10, *pt01, *pt11, pNewSrf))
		{
			// Throw exception
			throw new Exception("Error creating cylinder through box.");
		}
		// Create and add an IdSelf attribute to this surface
		pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
		pNewSrf->AddAttribute(pAttribute);

		if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
			pNewSrf, pNewSrf->GetNaturalUVDomain (), pNewFace)))
		{
			// Throw exception
			throw new Exception ("Error creating face from bi-linear surface");
		}
		// Fifth face
		pt00->Set(ptMin->X, ptMin->Y, ptMin->Z);
		pt10->Set(ptMin->X, ptMin->Y, ptMax->Z);
		pt01->Set(ptMax->X, ptMin->Y, ptMin->Z);
		pt11->Set(ptMax->X, ptMin->Y, ptMax->Z);

		if (IW_SUCCESS != IwBSplineSurface::CreateBilinearSurface(
			m_pContext->GetIwContext(), *pt00, *pt10, *pt01, *pt11, pNewSrf))
		{
			// Throw exception
			throw new Exception("Error creating cylinder through box.");
		}
		// Create and add an IdSelf attribute to this surface
		pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
		pNewSrf->AddAttribute(pAttribute);

		if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
			pNewSrf, pNewSrf->GetNaturalUVDomain (), pNewFace)))
		{
			// Throw exception
			throw new Exception ("Error creating face from bi-linear surface");
		}

		// Sixth face
		pt00->Set(ptMin->X, ptMax->Y, ptMin->Z);
		pt10->Set(ptMin->X, ptMax->Y, ptMax->Z);
		pt01->Set(ptMax->X, ptMax->Y, ptMin->Z);
		pt11->Set(ptMax->X, ptMax->Y, ptMax->Z);

		if (IW_SUCCESS != IwBSplineSurface::CreateBilinearSurface(
			m_pContext->GetIwContext(), *pt00, *pt10, *pt01, *pt11, pNewSrf))
		{
			// Throw exception
			throw new Exception("Error creating cylinder through box.");
		}
		// Create and add an IdSelf attribute to this surface
		pAttribute = SMObject::CreateStringAttribute(this->GetIwContext(), AttributeID_IDSELF, Guid::NewGuid().ToString("B"));
		pNewSrf->AddAttribute(pAttribute);

		if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->CreateFaceFromSurface (
			pNewSrf, pNewSrf->GetNaturalUVDomain (), pNewFace)))
		{
			// Throw exception
			throw new Exception ("Error creating face from bi-linear surface");
		}

		// Sew faces of the box.

		ULONG lNumSewn, lNumLam;
		double dMaxVGap, dMaxEGap;
		if (IW_SUCCESS != ((IwBrep *) m_pIwObj)->SewFaces(
			1.0e-3, lNumSewn, lNumLam, dMaxVGap, dMaxEGap))
		{
			// Throw exception
			throw new Exception ("Error sewing faces of box.");
		}

		if (m_pIwObj != NULL)
		{
			// Add to DOM. Set the counters dirty flag since topology counters 
			// may no longer be unique.

			m_bCountersDirty = true;
			AddToDOM ();
			((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
		}
	}

	void Brep::BoundPlane (Plane __gc *plane)
	{
		try
		{
			if (m_pIwObj == NULL || m_pXMLElem == NULL)
			{
				// todo: throw exception
			}

			IwExtent3d oBox;
			((IwBrep *) m_pIwObj)->CalculateBoundingBox (oBox);
			double dPad = oBox.GetSize ().Length () / 10.;
			oBox.ExpandAbsolute (dPad);
			IwPlane *pPlane = (IwPlane *) plane->GetIwObj ();
			if (pPlane)
			{
				// Walk through all of the corners of the expanded extents box projecting them
				// to the plane and extract the min and max projections to use as the plane domain

				IwPoint2d ptMax (System::Double::MinValue, System::Double::MinValue), ptMin (System::Double::MaxValue, System::Double::MaxValue);
				IwTArray<IwPoint3d> arrPoints;
				arrPoints.Add (oBox.GetMin ());
				arrPoints.Add (oBox.GetMax ());

				for (int i = 0; i < 2; i++)
				{
					for (int j = 0; j < 2; j++)
					{
						for (int k = 0; k < 2; k++)
						{
							IwPoint3d ptToProject;
							IwPoint2d ptProjected;
							ptToProject.Set (arrPoints[i].x, arrPoints[j].y, arrPoints[k].z);
							if (IW_SUCCESS == pPlane->ProjectPointToUVDomain (ptToProject, ptProjected))
							{
								ptMin.x = min (ptMin.x, ptProjected.x);
								ptMin.y = min (ptMin.y, ptProjected.y);
								ptMax.x = max (ptMax.x, ptProjected.x);
								ptMax.y = max (ptMax.y, ptProjected.y);
							}
						}
					}
				}

				// Check for valid min/max points

				if (ptMin.GetMaxDimension () == System::Double::MaxValue || ptMax.GetMaxDimension () == System::Double::MaxValue)
				{
					//TODO: throw and exception
					return;
				}

				IwExtent2d oDomain;
				oDomain.AddPoint2d (ptMin);
				oDomain.AddPoint2d (ptMax);
				if (IW_SUCCESS != (pPlane->TrimWithDomain (oDomain)))
				{
					// todo: throw exception
				}
			}
		}
		catch (...)
		{
			Console::WriteLine ("Could not bound plane.");
		}
	}

	System::Collections::ArrayList __gc * Brep::InsertInternalPlane (Plane __gc * oPlane)
	{
		// This routine returns a collection of face proxies representing the newly created faces
		// associated with the input plane. It does not include any subdivided faces created from
		// faces already existing in the Brep.

		System::Collections::ArrayList __gc *arrFaceProxies = new System::Collections::ArrayList ();

		try
		{
			if (m_pIwObj == NULL || m_pXMLElem == NULL)
			{
				// todo: throw exception
			}

			((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
			((IwBrep *) m_pIwObj)->m_bMakeComposites = TRUE;

			double dAngleTol = 20.0*IW_PI/180.0;
			double dApproxTol = 1.0e-3;

			IwBrep *pResult = 0;
			IwFace *pNewFace = 0;
			String __gc * sGuid = 0;

			IwExtent3d oBox;
			((IwBrep *) m_pIwObj)->CalculateBoundingBox (oBox);
			double dPad = oBox.GetSize ().Length () / 10.;
			oBox.ExpandAbsolute (dPad);
			IwPlane *pPlane = (IwPlane *) oPlane->ExtractIwObj ();
			if (pPlane)
			{
				// Walk through all of the corners of the expanded extents box projecting them
				// to the plane and extract the min and max projections to use as the plane domain

				IwPoint2d ptMax (System::Double::MinValue, System::Double::MinValue), ptMin (System::Double::MaxValue, System::Double::MaxValue);
				IwTArray<IwPoint3d> arrPoints;
				arrPoints.Add (oBox.GetMin ());
				arrPoints.Add (oBox.GetMax ());

				for (int i = 0; i < 2; i++)
				{
					for (int j = 0; j < 2; j++)
					{
						for (int k = 0; k < 2; k++)
						{
							IwPoint3d ptToProject;
							IwPoint2d ptProjected;
							ptToProject.Set (arrPoints[i].x, arrPoints[j].y, arrPoints[k].z);
							if (IW_SUCCESS == pPlane->ProjectPointToUVDomain (ptToProject, ptProjected))
							{
								ptMin.x = min (ptMin.x, ptProjected.x);
								ptMin.y = min (ptMin.y, ptProjected.y);
								ptMax.x = max (ptMax.x, ptProjected.x);
								ptMax.y = max (ptMax.y, ptProjected.y);
							}
						}
					}
				}

				// Check for valid min/max points

				if (ptMin.GetMaxDimension () == System::Double::MaxValue || ptMax.GetMaxDimension () == System::Double::MaxValue)
				{
					//TODO: throw and exception
					return arrFaceProxies;
				}

				IwExtent2d oDomain;
				oDomain.AddPoint2d (ptMin);
				oDomain.AddPoint2d (ptMax);
				if (IW_SUCCESS != (pPlane->TrimWithDomain (oDomain)))
				{
					// todo: throw exception
				}

				IwBrep *pPlaneBrep = new (GetIwContext()) IwBrep;
				pPlaneBrep->m_bMakeComposites = TRUE;
				pPlaneBrep->m_bEditingEnabled = TRUE;
				pPlaneBrep->SetTolerance (this->Tolerance);
				if (IW_SUCCESS != (pPlaneBrep->CreateFaceFromSurface (
					pPlane, pPlane->GetNaturalUVDomain (), pNewFace)))
				{
					// todo: throw exception
				}

				// Set an attribute on the new face so we can identify it after merging
				// and return the new faces. First create a GUID to identify the new face

				sGuid = Guid::NewGuid().ToString("B");
				if (pNewFace)
				{
					IwTArray<long> arrLongEl;
					IwTArray<double> arrDoubleEl;
					IwTArray<char> arrCharEl;

					int lSize = sGuid->Length;
					if (lSize > 0)
					{
						for (int iString = 0; iString < lSize; iString++)
							arrCharEl.Add(Convert::ToByte(sGuid->Chars[iString]));

						IwGenericAttribute *pAttribute = new (m_pContext->GetIwContext()) IwGenericAttribute (
							AttributeID_IDOBJ, IW_AB_COPY, arrLongEl, arrDoubleEl, arrCharEl);
						pNewFace->AddAttribute (pAttribute);
					}
				}

				IwMerge oMerge (GetIwContext(), (IwBrep *) m_pIwObj, pPlaneBrep,
					dApproxTol, dAngleTol);
				if (IW_SUCCESS != (oMerge.ManifoldBoolean (IW_BO_DIFFERENCE, pResult)))
				{
					// todo: throw exception
				}
				m_pIwObj = pResult;
			}

			if (m_pIwObj != NULL)
			{
				// Sew the faces together and make the Brep manifold (or cellular)

				ULONG lNumSewn, lNumLam;
				double dMaxVGap, dMaxEGap;
				if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->SewFaces(
					0.001, lNumSewn, lNumLam, dMaxVGap, dMaxEGap)))
				{
					// todo: throw exception
				}

				if (IW_SUCCESS != (((IwBrep *) m_pIwObj)->MakeManifold(NULL,TRUE)))
				{
					// todo: throw exception
				}

				// Find the newly created faces and return them.

				IwTArray<IwFace *> arrFaces;
				IwTArray<IwFace *> arrNewPlaneFaces;
				((IwBrep *) m_pIwObj)->GetFaces (arrFaces);
				for (unsigned int iFace = 0; iFace < arrFaces.GetSize (); iFace++)
				{
					// Search for AttributeID_IDOBJ attributes. Any new faces from the plane should
					// have only this attribute (unless CreateFaceFromSurface called earlier puts the
					// surface's attributes on the face. Just to be sure however look for the GUID
					// above out of a possibly larger attribute string.

					IwAttribute *pExistingAttribute = arrFaces[iFace]->FindAttribute (AttributeID_IDOBJ);
					if (NULL != pExistingAttribute)
					{
						if (pExistingAttribute->GetNumCharacterElements () > 0)
						{
							const char *pcElements = pExistingAttribute->GetCharacterElementsAddress ();
							System::String __gc *sAttribute = new System::String (pcElements);
							int iObj = sAttribute->IndexOf (sGuid);
							if (iObj >= 0)
							{
								// New face found so add it to the list of new faces to return 
								// and remove the attribute

								arrNewPlaneFaces.Add (arrFaces[iFace]);

								sAttribute->Remove (iObj, sGuid->Length);
								arrFaces[iFace]->RemoveAttribute (pExistingAttribute, TRUE);

								IwTArray<long> arrLongEl;
								IwTArray<double> arrDoubleEl;
								IwTArray<char> arrCharEl;

								int lSize = sAttribute->Length;
								if (lSize > 0)
								{
									for (int iString = 0; iString < lSize; iString++)
										arrCharEl.Add(Convert::ToByte(sAttribute->Chars[iString]));

									IwGenericAttribute *pAttribute = new (m_pContext->GetIwContext()) IwGenericAttribute (
										AttributeID_IDOBJ, IW_AB_COPY, arrLongEl, arrDoubleEl, arrCharEl);
									arrFaces[iFace]->AddAttribute (pAttribute);
								}
							}
						}
					}
				}

				// Set the counters dirty flag since topology counters may no longer be unique.

				m_bCountersDirty = true;

				// Add to DOM

				AddToDOM ();
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;

				// Now populate the array of new face proxies to return

				for (unsigned int iFace = 0; iFace < arrNewPlaneFaces.GetSize (); iFace++)
				{
					// Try to find the face counter attribute.

					IwAttribute *pAttribute = 0;
					if (NULL != (pAttribute = ((IwAObject *) arrNewPlaneFaces[iFace])->FindAttribute (
						AttributeID_BREPFACECOUNTER)) &&	pAttribute->GetNumLongElements () > 0)
					{
						const long *lAttributes = pAttribute->GetLongElementsAddress ();
						BrepFaceProxy *faceProxy = new BrepFaceProxy (m_pContext, this, lAttributes[0]);
						arrFaceProxies->Add (faceProxy);
					}
				}
			} // if (m_pIwObj != NULL)
		}
		catch (...)
		{
			Console::WriteLine ("Could not insert internal plane.");
		}

		return arrFaceProxies;
	}

	String* Brep::GetInfo(void)
	{

		StringBuilder* sInfo = new StringBuilder ("");

		if (m_pIwObj != NULL)
		{
			IwTArray<IwFace*> sFaces;
			IwTArray<IwEdge*> sEdges;
			IwTArray<IwVertex*> sVertices;
			((IwBrep *) m_pIwObj)->GetFaces(sFaces);
			((IwBrep *) m_pIwObj)->GetEdges(sEdges);
			((IwBrep *) m_pIwObj)->GetVertices(sVertices);
			ULONG lWires = 0;
			ULONG lLaminas = 0;
			ULONG lManifold = 0;
			for (ULONG j=0; j<sEdges.GetSize(); j++) 
			{
				IwEdge *pE = sEdges[j];
				if (pE->IsWire()) lWires++;
				if (pE->IsLamina()) lLaminas++;
				if (pE->IsManifold()) lManifold++;
			}

			IwTArray<IwRegion*> sRegions;
			IwTArray<IwShell*> sShells;
			((IwBrep *) m_pIwObj)->GetRegions(sRegions);

			ULONG lNumShells=0;
			for (ULONG i=0; i<sRegions.GetSize(); i++) 
			{
				IwRegion *pR = sRegions[i];
				pR->GetShells(sShells);
				lNumShells += sShells.GetSize();
			}

			//sInfo->AppendFormat ("    # Regions = {0}\n    # Shells = {1}\n",
			//   sRegions.GetSize().ToString(), lNumShells.ToString());

			for (ULONG i = 0; i<sRegions.GetSize(); i++)
			{
				IwRegion *pR = sRegions[i];
				pR->GetShells(sShells);
				//sInfo->AppendFormat ("\n       Region #{0}\n", i.ToString ());
				// Add Region index
				sInfo->AppendFormat("{0}\t", i.ToString("000"));
				/*
				for (ULONG k=0; k<sShells.GetSize(); k++)
				{
				IwShell *pS = sShells[k];
				sInfo->AppendFormat("       Shell[{0}][{1}] contains - {2} elements\n",
				i.ToString(), k.ToString(), pS->GetSize().ToString());
				}
				*/
				// Add name - should be more descriptive
				sInfo->AppendFormat("Region {0}\t",i.ToString());


				double dRelativeAccuracy = 1.e-2, dEstimatedArea = 0.;
				double dArea = 0., dVolume = 0.;
				IwVector3d oOrigin (0., 0., 0.);
				IwTArray<IwVector3d> arrMoments;
				arrMoments.SetSize (7);

				IwStatus sStatus = pR->ComputePreciseProperties (dRelativeAccuracy, oOrigin, 
					dEstimatedArea, dArea, dVolume, arrMoments);
				if (sStatus != IW_SUCCESS)
				{
					// TODO
				}
				// Add infinite region indicator
				if (dVolume < 0.0)
					sInfo->Append("Yes\t");
				else
					sInfo->Append("No\t");
				// Add Volume
				sInfo->AppendFormat("{0}\t",dVolume.ToString("###,##0.000"));
				// Add Centroid of volume
				IwVector3d vecCentroid (0., 0., 0.);
				if (dVolume != 0.)
				{
					vecCentroid.x = arrMoments[4].x / dVolume;
					vecCentroid.y = arrMoments[4].y / dVolume;
					vecCentroid.z = arrMoments[4].z / dVolume;
				}
				sInfo->AppendFormat("{0}\t",vecCentroid.x.ToString("###,##0.000"));
				sInfo->AppendFormat("{0}\t",vecCentroid.y.ToString("###,##0.000"));
				sInfo->AppendFormat("{0}\t",vecCentroid.z.ToString("###,##0.000"));
				// Add Area
				sInfo->AppendFormat("{0}\t",dArea.ToString("###,##0.000"));

				/*
				sInfo->AppendFormat("\n       Area = {0}\n", dArea.ToString("###,##0.000"));
				sInfo->AppendFormat("       Volume = {0}\n", dVolume.ToString("###,##0.000"));
				sInfo->Append("       Centroid:\n");
				sInfo->AppendFormat("           X = {0}\n", vecCentroid.x.ToString ("###,##0.000"));
				sInfo->AppendFormat("           Y = {0}\n", vecCentroid.y.ToString ("###,##0.000"));
				sInfo->AppendFormat("           Z = {0}\n", vecCentroid.z.ToString ("###,##0.000"));
				*/
			}
			/*
			sInfo->AppendFormat("    # Faces = {0}\n    # Edges = {1}\n",
			sFaces.GetSize().ToString(), sEdges.GetSize().ToString());

			sInfo->AppendFormat("        Wire Edges = {0}\n"
			"        Lamina Edges = {1}\n"
			"        Manifold Edges = {2}\n",
			lWires.ToString(), lLaminas.ToString(), lManifold.ToString());

			sInfo->AppendFormat ("    # Vertices = {0}\n", sVertices.GetSize().ToString());
			*/
		}
		return sInfo->ToString ();
	}

	//void Brep::RemoveGraphics (HC::KEY segKey)
	//{
	//	RemoveGraphics(segKey, m_hkFaces, m_hkEdges);
	//}

	//void Brep::RemoveGraphics (KEY segKey, System::Collections::ArrayList __gc *hkFaces, System::Collections::ArrayList __gc *hkEdges)
	//{
	//	// Flush the contents of the segment. It is the callers responsibility to create
	//	// or delete the containing segment.

	//	HC::Open_Segment_By_Key (segKey);
	//	HC::Flush_Contents ("...", "everything");
	//	HC::Close_Segment ();
	//	hkFaces->Clear();
	//	hkEdges->Clear();
	//}

	//void Brep::RemoveFeatureGraphics (HC::KEY segKey, BrepFeatureType eFeature, long faceID)
	//{
	//	if (eFeature == BrepFeatureType::Brep_Face)
	//	{
	//		BrepFaceProxy __gc *pFace = GetFace (faceID);
	//		if (NULL != pFace)
	//		{
	//			System::Object __gc *oAttrib = pFace->FindAttribute( AttributeID_HKEY);
	//			if (oAttrib != NULL)
	//			{
	//				System::Int32 *pkey = dynamic_cast<System::Int32 *>(oAttrib);
	//				if (NULL != pkey)
	//				{
	//					HC::KEY faceKey = (HC::KEY)(*pkey);
	//					HC::Open_Segment_By_Key (segKey);
	//					{
	//						if (HC::Show_Existence_By_Key (faceKey, "self"))
	//							HC::Delete_By_Key (faceKey);
	//						else
	//						{
	//							HC::Open_Segment ("highlight");
	//							if (HC::Show_Existence_By_Key (faceKey, "self"))
	//								HC::Delete_By_Key (faceKey);
	//							HC::Close_Segment ();
	//						}
	//					}
	//					HC::Close_Segment ();
	//				}
	//			}
	//		}
	//	}
	//}

	void Brep::DeleteFaces (System::Collections::ArrayList __gc *arrFaceIDs)
	{
		try
		{
			if (m_pIwObj != NULL)
			{
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;
				IwTArray<IwFace *> arrFaces;
				IwTArray<IwFace *> arrFacesToDelete;
				((IwBrep *) m_pIwObj)->GetFaces (arrFaces);
				ULONG ulNumFaces = arrFaces.GetSize ();
				int iSize = arrFaceIDs->Count;

				for (ULONG ulFaceCnt = 0; ulFaceCnt < ulNumFaces; ulFaceCnt++)
				{
					IwAttribute *pAttribute = 0;
					ULONG nElements = 0;

					if (NULL != (pAttribute = ((IwAObject *) arrFaces[ulFaceCnt])->FindAttribute (AttributeID_BREPFACECOUNTER)) &&
						(nElements = pAttribute->GetNumLongElements ()) > 0)
					{
						const long *lAttributes = pAttribute->GetLongElementsAddress ();
						for (int iFaceID = 0; iFaceID < iSize; iFaceID++)
						{
							if (lAttributes[0] == System::Convert::ToInt32 (arrFaceIDs->get_Item (iFaceID)->ToString ()))
							{
								arrFacesToDelete.Add (arrFaces[ulFaceCnt]);
								break;
							}
						}
					}
				}

				((IwBrep *) m_pIwObj)->RemoveFaces (arrFacesToDelete);

				// Add to DOM

				AddToDOM ();
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;

				// Fire invalidated event

				BrepEventArgs __gc *eventArgs = new BrepEventArgs ();
				this->BrepChanged (this, eventArgs);
			}
		}
		catch (...)
		{
			System::Console::WriteLine ("Exception caught in DeleteFaces method of Brep.");
		}
	}

	void Brep::Dump()
	{
		try
		{
			if (m_pIwObj != NULL)
			{
				((IwBrep *) m_pIwObj)->Dump();
			}
		}
		catch (...)
		{
			// TODO
		}

	}

	void Brep::CreatePlanarSections(Plane* vecPlanes __gc[], System::Collections::ArrayList& vecCurves, bool bConnectDisjointCurves)
	{
		// Create planar sections computes planar curves cut through this brep by intersecting
		// with the input plane collection. For each plane, the intersection can be a single NURBS
		// curve, a composite curve, or an ArrayList of curves/composites if the caller does not
		// choose to connect disjoint segments. The caller must interpret the returned ArrayList
		// members to determine which of these 3 types each member corresponds to.
		try
		{
			if (m_pIwObj != NULL && m_pContext != NULL)
			{
				// Loop thru each of the planes creating sections.

				long nPlanes = vecPlanes->Count;
				double dAngleTol = 20.0*IW_PI/180.0;
				double dApproxTol = 1.0e-3;

				for (long iPlane = 0; iPlane < nPlanes; iPlane++)
				{
					IwPlane *pIwPlane = (IwPlane *) vecPlanes[iPlane]->GetIwObj ();
					if (pIwPlane != NULL)
					{
						IwAxis2Placement oPosition = pIwPlane->GetPosition ();
						IwTArray<IwCurve *> arrSections;
						if (IW_SUCCESS != ((IwBrep *) m_pIwObj)->CreatePlanarSectionCurves (
							GetIwContext (), oPosition.GetOrigin (),
							oPosition.GetZAxis (), &dApproxTol, &dAngleTol,
							&arrSections, NULL, NULL))
						{
							// TODO
						}

						// Create a composite curve for the curve segments if 
						// there is more than 1 curve segment. Also, since it is possible
						// the input plane coincides with edges of adjoining faces, we must
						// screen out duplicate curves which arise from the CreatePlanarSectionCurves
						// call above.

						long nSections = arrSections.GetSize ();
						if (nSections == 1)
						{
							IwCurve *pCurve = arrSections[0];
							if (pCurve->IsKindOf (IwBSplineCurve_TYPE))
							{
								NurbsCurve *pSection = new NurbsCurve ();
								if (pSection != NULL)
								{
									pSection->AttachIwObj (m_pContext, pCurve);
									vecCurves.Add (pSection);
								}
							}
						}
						else // create composite curve with unique curves
						{
							System::Collections::ArrayList __gc *arrNurbsCurves = new ArrayList ();
							double dMaxDistance = Tolerance;
							double dMaxDistFound;

							for (unsigned int iSection = 0; iSection < arrSections.GetSize (); iSection++)
							{
								// Walk through the rest of the curves and if any are the same
								// as this curve geometrically remove it from the list and decrement
								// the later section counter.

								IwCurve *pCurve = arrSections[iSection];
								for (unsigned int iLaterSection = iSection + 1; iLaterSection < arrSections.GetSize (); iLaterSection++)
								{
									IwCurve *pOtherCurve = arrSections[iLaterSection];
									IwStatus status = pCurve->CurveMaxDistanceBetween (pCurve->GetNaturalInterval (),
										*pOtherCurve, pOtherCurve->GetNaturalInterval ().GetMin (),
										pOtherCurve->GetNaturalInterval ().GetMax (), 0, &dMaxDistance, dMaxDistFound);
									if (status == IW_SUCCESS && dMaxDistFound <= dMaxDistance) // duplicate found
									{
										arrSections.RemoveAt (iLaterSection, 1);
										iLaterSection--;
									}
								}

								if (pCurve->IsKindOf (IwBSplineCurve_TYPE))
								{
									NurbsCurve *pSection = new NurbsCurve ();
									if (pSection != NULL)
									{
										pSection->AttachIwObj (m_pContext, pCurve);
										arrNurbsCurves->Add (pSection);
									}
								}
							}
							bool bMakeHomogeneous = false;
							double dSamePtTol = this->VertexTolerance;
							double dDistToCreateLine = 0.0; // do not connect disjoint curve segments
							if (bConnectDisjointCurves)
								dDistToCreateLine = 1000.0; // intended to connect disjoint curve segments
							System::Collections::ArrayList __gc *vecStationCurves = new System::Collections::ArrayList ();
							CompositeCurve::BuildCompositesFromCurves (m_pContext, NULL,
								arrNurbsCurves, bMakeHomogeneous, dSamePtTol, dDistToCreateLine,
								vecStationCurves);

							// It is possible that multiple composite curves are returned (if disjoint).
							// If so add the entire arraylist of curves.

							if (vecStationCurves->Count > 1)
								vecCurves.Add (vecStationCurves);
							else if (vecStationCurves->Count == 1)
								vecCurves.Add (vecStationCurves->get_Item(0));
						}
					}
				}
			}
		}
		catch (...)
		{
			// TODO
		}
	}

	void Brep::GetSurfaceShell(double dChordHeightTol, double dAngleTolInDegrees, 
		double dMax3DEdgeLength, double dMaxAspectRatio, double dMinUVRatio, XML::XmlDocument* xmlDoc, 
		XML::XmlElement* xmlShellsElem)
	{

		IwBrep *pCopy = NULL;
		const ULONG lMinCurveDivisions = 0; // Don't use this tolerance 
		const double dMin3DEdgeLength = 0.0; // Minimum length of an edge - not used 

		try
		{
			if (m_pIwObj != NULL && m_pContext != NULL)
			{
				// Note that the Brep sent into the Tessellator gets modified
				// so you usually will want to make a copy of it is follows:

				pCopy = new (GetIwContext()) IwBrep (*((IwBrep *) m_pIwObj));
				if (pCopy != NULL)
				{
					IwRegion *pInfRegion = pCopy->GetInfiniteRegion ();
					// get all the faces and delete those without at least one use in the infinite region
					IwTArray<IwFace*> faces, removablefaces;
					IwFaceuse *faceuse1, *faceuse2;
					pCopy->GetFaces(faces);
					long lFaceCount = faces.GetSize();

					for (long lFace=0; lFace < lFaceCount; lFace++)
					{
						faces[lFace]->GetFaceuses(faceuse1, faceuse2);
						if (faceuse1->GetShell()->GetRegion() != pInfRegion && faceuse2->GetShell()->GetRegion() != pInfRegion)
						{
							removablefaces.Add(faces[lFace]);
						}
					}
					pCopy->m_bEditingEnabled = TRUE;
					IwStatus stRemove = pCopy->RemoveFaces(removablefaces);
					pCopy->RemoveTopologicalEdgesAndVertices();
					pCopy->m_bEditingEnabled = FALSE;

					// The logic of this function was stolen from one of the SMLib examples.
					// The tolerances should one day make it to the parameter list of the function

					// Create a curve tessellation driver and a surface tessellation
					// driver to control how subdivision is performed. 
					IwCurveTessDriver sCrvTess ( lMinCurveDivisions,
						dChordHeightTol, dAngleTolInDegrees );

					IwSurfaceTessDriver sSrfTess ( dChordHeightTol,
						dAngleTolInDegrees,
						dMax3DEdgeLength,
						dMin3DEdgeLength,
						dMinUVRatio,
						dMaxAspectRatio );

					// Declare an instance of IwTess class ; sTess

					IwBoolean bFacesFailed = FALSE;
					IwTess sTess (GetIwContext(), sCrvTess, sSrfTess);

					// Get the infinite region and its associated faces.
					IwTArray<IwFace *> arrFaces;
					if (pInfRegion != NULL)
					{
						// Perform a phase 1 tesselation of the Brep.
						//						unsigned int dwTicks = GetTickCount();
						if (IW_SUCCESS != sTess.Phase1SetupBrep (pCopy, bFacesFailed))
						{
							// TODO: throw error
							return;
						}
						if (bFacesFailed == TRUE)
						{
							// TODO: Throw error
							System::Windows::Forms::MessageBox::Show("At least on face failed to tesselate properly.");
						}
						//						unsigned int dwTime = GetTickCount()-dwTicks;

						IwTArray<IwShell *> arrShells;

						pInfRegion->GetShells (arrShells);
						long lShellSize = arrShells.GetSize ();
						for (long lShell = 0; lShell < lShellSize; lShell++)
						{
							IwTArray<IwFaceuse *> arrFaceuses;

							arrShells[lShell]->GetFaceuses (arrFaceuses);
							long lFaces = arrFaceuses.GetSize ();

							// Map to shells

							for (long lFace = 0; lFace < lFaces; lFace++)
							{
								ShellMapCallback shellCallback(xmlShellsElem);
								shellCallback.SetOutputType(IW_PO_TRIANGLE_MESH);	
								IwFace *pFace = arrFaceuses[lFace]->GetFace ();
								arrFaces.Add (pFace);
								if (sTess.Phase2CreateFacePolygons (pFace) == IW_SUCCESS)
								{
									sTess.OutputFacePolygons (pFace, shellCallback);
									System::String* str = xmlDoc->InnerXml;
								}
								else
								{
									System::Windows::Forms::MessageBox::Show("At least on face failed to tesselate properly.");
								}
								sTess.Phase3DeleteFacePolygons (pFace);
							}
						}
					}
				}
				//delete pCopy; 
				//pCopy = NULL;
			}
		}
		catch (...)
		{
			//			if (pCopy != NULL)
			//			{
			//				delete pCopy;
			//           pCopy = NULL;
			//			}
			return;
		}
	}
	XML::XmlDocumentFragment __gc * Brep::GetSurfaceShell(double dChordHeightTol, double dAngleTolInDegrees, 
		double dMax3DEdgeLength, double dMaxAspectRatio, double dMinUVRatio)
	{

		XML::XmlDocument* xmlDoc = new XML::XmlDocument();
		xmlDoc->LoadXml("<SurfaceShells/>");
		GetSurfaceShell(dChordHeightTol, dAngleTolInDegrees, dMax3DEdgeLength, dMaxAspectRatio, dMinUVRatio, xmlDoc,
			xmlDoc->DocumentElement);

		XML::XmlDocumentFragment* xmlFrag = xmlDoc->CreateDocumentFragment();
		XML::XmlNode* cloneNode = xmlDoc->DocumentElement->CloneNode(true);
		xmlFrag->AppendChild(cloneNode);
		return xmlFrag;

	}

	void Brep::ShowRegionData ()
	{
		System::Text::StringBuilder *message = new System::Text::StringBuilder ();

		if (m_pIwObj)
		{
			IwTArray<IwRegion *> arrRegions;
			((IwBrep *) m_pIwObj)->GetRegions (arrRegions);
			int nRegions = arrRegions.GetSize ();

			for (int iRegion = 0; iRegion < nRegions; iRegion++)
			{
				IwAttribute *pAttribute = arrRegions[iRegion]->FindAttribute (AttributeID_BREPREGIONCOUNTER);
				if (pAttribute && pAttribute->GetNumLongElements () > 0)
				{
					const long *lAttributes = pAttribute->GetLongElementsAddress ();
					message->AppendFormat ("\n\nRegion {0}\n", lAttributes[0].ToString ());

					IwTArray<IwShell *> arrShells;
					arrRegions[iRegion]->GetShells (arrShells);
					int nShells = arrShells.GetSize ();

					for (int iShell = 0; iShell < nShells; iShell++)
					{
						IwTArray<IwFaceuse *> arrFaceuses;
						arrShells[iShell]->GetFaceuses (arrFaceuses);
						int nFaceuses = arrFaceuses.GetSize ();

						for (int iFaceuse = 0; iFaceuse < nFaceuses; iFaceuse++)
						{
							IwFace *pFace = arrFaceuses[iFaceuse]->GetFace ();
							if (pFace)
							{
								IwAttribute *pFaceAttrib = pFace->FindAttribute (AttributeID_BREPFACECOUNTER);
								if (pFaceAttrib && pFaceAttrib->GetNumLongElements ())
								{
									const long *lFaceAttribs = pFaceAttrib->GetLongElementsAddress ();
									message->AppendFormat ("\tFace {0}\n", lFaceAttribs[0].ToString ());
								}
							}
						}
					}
				}
			}
			System::Windows::Forms::MessageBox::Show (message->ToString ());
		}
	}

	System::Collections::ArrayList __gc * Brep::GetRegionsFromFace (HC::KEY hkFace)
	{
		System::Collections::ArrayList __gc *arrRegionProxies = new System::Collections::ArrayList ();

		if (m_pIwObj != NULL)
		{
			// Cache the infinite region for later screening

			IwRegion *pInfRegion = ((IwBrep *) m_pIwObj)->GetInfiniteRegion ();

			// First find the face with the specified ID

			IwTArray<IwFace *> arrFaces;
			((IwBrep *) m_pIwObj)->GetFaces (arrFaces);
			int nFaces = arrFaces.GetSize ();

			for (int iFace = 0; iFace < nFaces; iFace++)
			{
				IwAttribute *pAttribute = arrFaces[iFace]->FindAttribute (AttributeID_HKEY);
				if (pAttribute && pAttribute->GetNumLongElements () > 0)
				{
					const long *lAttributes = pAttribute->GetLongElementsAddress ();
					if (lAttributes[0] == hkFace)
					{
						IwFaceuse *faceuses[2];
						arrFaces[iFace]->GetFaceuses (faceuses[0], faceuses[1]);
						for (int iFaceuse = 0; iFaceuse < 2; iFaceuse++)
						{
							if (faceuses[iFaceuse])
							{
								IwShell *shell = faceuses[iFaceuse]->GetShell ();
								if (shell)
								{
									IwRegion *region = shell->GetRegion ();
									if (region && region != pInfRegion)
									{
										IwAttribute *pRegionAttribute = region->FindAttribute (AttributeID_BREPREGIONCOUNTER);
										if (pRegionAttribute && pRegionAttribute->GetNumLongElements () > 0)
										{
											const long *lRegionIDs = pRegionAttribute->GetLongElementsAddress ();
											BrepRegionProxy __gc *regionProxy = 
												new BrepRegionProxy (m_pContext, this, lRegionIDs[0]);
											arrRegionProxies->Add (regionProxy);
										}
									}
								}
							}
						}
						break;
					}
				}
			}
		}

		return arrRegionProxies;
	}

	BrepRegionProxy __gc * Brep::GetInfiniteRegion ()
	{
		try
		{
			if (m_pIwObj != NULL)
			{
				// A region id of -1 signifies the infinite region.
				BrepRegionProxy *regionProxy = new BrepRegionProxy (m_pContext, this, -1);
				return regionProxy;
			}
		}
		catch (...)
		{
			System::Console::WriteLine ("Could not get infinite region for this brep.");
		}

		return NULL;
	}

	System::Collections::ArrayList __gc * Brep::GetRegions ()
	{
		// It is assumed that the regions returned here do not include the infinite region. There is
		// a method GetInfiniteRegion if that region is desired.

		System::Collections::ArrayList __gc *arrRegionProxies = new System::Collections::ArrayList ();

		try
		{
			if (m_pIwObj != NULL)
			{
				IwTArray<IwRegion *> arrRegions;
				((IwBrep *) m_pIwObj)->GetRegions (arrRegions);
				int numRegions = arrRegions.GetSize ();
				IwRegion *pInfiniteRegion = ((IwBrep *) m_pIwObj)->GetInfiniteRegion ();

				for (int iRegion = 0; iRegion < numRegions; iRegion++)
				{
					// Try to find the region counter attribute.

					IwAttribute *pAttribute = 0;
					if (NULL != (pAttribute = ((IwAObject *) arrRegions[iRegion])->FindAttribute (AttributeID_BREPREGIONCOUNTER)) &&
						pAttribute->GetNumLongElements () > 0)
					{
						if (arrRegions[iRegion] == pInfiniteRegion)
							continue;
						const long *lAttributes = pAttribute->GetLongElementsAddress ();
						BrepRegionProxy *regionProxy = new BrepRegionProxy (m_pContext, this, lAttributes[0]);
						arrRegionProxies->Add (regionProxy);
					}
				}
			}
		}
		catch (...)
		{
			System::Console::WriteLine ("Could not get regions for this brep.");
		}

		return arrRegionProxies;
	}

	BrepRegionProxy __gc * Brep::GetRegion(long iRegionId)
	{
		try
		{
			IwRegion *pIwRegion = GetIwRegionFromID (iRegionId);
			if (pIwRegion)
			{
				BrepRegionProxy __gc *regionProxy = new BrepRegionProxy (
					m_pContext, this, iRegionId);
				return regionProxy;
			}
		}
		catch (...)
		{
			System::Console::WriteLine ("Could not get specified region for this brep.");
		}
		return NULL;
	}

	IwFace * Brep::GetIwFaceFromProxy (BrepFaceProxy __gc *faceProxy)
	{
		IwFace *pIwFace = NULL;

		try
		{
			// First retrieve the Brep Face from the owning brep.

			if (NULL != m_pIwObj)
			{
				IwTArray<IwFace *> arrFaces;
				((IwBrep *) m_pIwObj)->GetFaces(arrFaces);
				int nFaces = arrFaces.GetSize();
				for (int i = 0; i < nFaces; i++)
				{
					IwAttribute *pAttribute = arrFaces[i]->FindAttribute (AttributeID_BREPFACECOUNTER);
					if (pAttribute && pAttribute->GetNumLongElements () > 0)
					{
						const long *lAttributes = pAttribute->GetLongElementsAddress ();
						if (lAttributes[0] == faceProxy->FaceID)
						{
							pIwFace = arrFaces[i];
							break;
						}
					}
				}
			}
		}
		catch (...)
		{
			Console::WriteLine("Could not get IwFace associated with BrepFaceProxy");
		}

		return pIwFace;
	}

	void Brep::LocalOperation (ArrayList __gc *arrFaces, ArrayList __gc *arrSurfaces)
	{
		try
		{
			if (NULL != m_pIwObj && NULL != arrFaces && NULL != arrSurfaces && 
				arrFaces->Count == arrSurfaces->Count)
			{
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = TRUE;

				IwTArray<IwFace *> iwarrFaces;
				IwTArray<IwSurface *> iwarrSurfaces;
				for (int iFace = 0; iFace < arrFaces->Count; iFace++)
				{
					BrepFaceProxy *faceProxy = __try_cast<BrepFaceProxy *> (arrFaces->get_Item (iFace));
					SMObject *smObj = __try_cast<SMObject *> (arrSurfaces->get_Item (iFace));
					if (NULL != faceProxy && NULL != smObj)
					{
						IwFace *pIwFace = this->GetIwFaceFromProxy (faceProxy);
						IwSurface *pIwSurface = (IwSurface *) smObj->ExtractIwObj ();
						//                  IwSurface *pIwSurface = (IwSurface *) smObj->GetIwObj ();

						if (NULL != pIwFace && NULL != pIwSurface)
						{
							iwarrFaces.Add (pIwFace);
							iwarrSurfaces.Add (pIwSurface);
						}
					}
				}

				IwStatus status = ((IwBrep *) m_pIwObj)->LocalOperation (iwarrFaces, iwarrSurfaces);
				((IwBrep *) m_pIwObj)->m_bEditingEnabled = FALSE;
			}
		}
		catch (...)
		{
			Console::WriteLine ("Could not perform LocalOperation on brep.");
		}
	}

	System::Collections::ArrayList __gc * Brep::GetFacesOfSurface (System::Object __gc *surface)
	{
		System::Collections::ArrayList __gc *arrFaceProxies = new System::Collections::ArrayList ();

		try
		{
			if (NULL != m_pIwObj && NULL != surface)
			{
				IwTArray<IwFace *> arrFacesOfSurface;
				IwSurface *pSurface = 0;
				SMObject __gc *smObj = dynamic_cast<SMObject *> (surface);
				if (NULL != smObj)
				{
					pSurface = (IwSurface *) smObj->GetIwObj ();
					IwBrep::GetFacesOfSurface (pSurface, arrFacesOfSurface);
					/*
					//            IwFace *pIwFace = GetIwFaceFromProxy (faceProxy);
					//				if (pIwFace != NULL && (pSurface = pIwFace->GetSurface ()) != NULL)
					if (pSurface != NULL)
					{
					IwTArray<IwFace *> arrFaces;
					((IwBrep *) m_pIwObj)->GetFaces (arrFaces);
					IwTArray<IwFace *> arrFacesOfSurface;
					for (unsigned int iFace = 0; iFace < arrFaces.GetSize (); iFace++)
					{
					IwFace *pIwFace = arrFaces[iFace];
					if (pIwFace->GetSurface () == pSurface)
					arrFacesOfSurface.Add (pIwFace);
					}
					}
					//               ((IwBrep *) m_pIwObj)->GetFacesOfSurface (pIwFace->GetSurface (), arrFaces);
					*/
					for (unsigned int iFace = 0; iFace < arrFacesOfSurface.GetSize (); iFace++)
					{
						// Try to find the face counter attribute.

						IwAttribute *pAttribute = 0;
						if (NULL != (pAttribute = ((IwAObject *) arrFacesOfSurface[iFace])->FindAttribute (AttributeID_BREPFACECOUNTER)) &&
							pAttribute->GetNumLongElements () > 0)
						{
							const long *lAttributes = pAttribute->GetLongElementsAddress ();
							BrepFaceProxy *newFaceProxy = new BrepFaceProxy (m_pContext, this, lAttributes[0]);
							arrFaceProxies->Add (newFaceProxy);
						}
					}
				}
			}
		}
		catch (...)
		{
			System::Console::WriteLine ("Could not get faces of surface for this brep.");
		}
		return arrFaceProxies;
	}

	System::Collections::ArrayList __gc * Brep::GetFaces ()
	{
		System::Collections::ArrayList __gc *arrFaceProxies = new System::Collections::ArrayList ();

		if (m_pIwObj != NULL)
		{
			IwTArray<IwFace *> arrFaces;
			((IwBrep *) m_pIwObj)->GetFaces (arrFaces);
			ULONG numFaces = arrFaces.GetSize ();
			for (ULONG iFace = 0; iFace < numFaces; iFace++)
			{
				// Try to find the face counter attribute.

				IwAttribute *pAttribute = 0;
				if (NULL != (pAttribute = ((IwAObject *) arrFaces[iFace])->FindAttribute (AttributeID_BREPFACECOUNTER)) &&
					pAttribute->GetNumLongElements () > 0)
				{
					const long *lAttributes = pAttribute->GetLongElementsAddress ();
					BrepFaceProxy *faceProxy = new BrepFaceProxy (m_pContext, this, lAttributes[0]);
					arrFaceProxies->Add (faceProxy);
				}
			}
		}

		return arrFaceProxies;
	}

	BrepFaceProxy __gc * Brep::GetFace(long iFaceId)
	{
		System::Collections::ArrayList __gc * arrFaceProxies = this->GetFaces();
		for (int i = 0; i < arrFaceProxies->Count; i++)
		{
			BrepFaceProxy *face = __try_cast<BrepFaceProxy *> (arrFaceProxies->get_Item(i));
			if (face->FaceID == iFaceId)
				return face;
		}
		return NULL;
	}

	BrepFaceProxy __gc * Brep::GetFace (HC::KEY hkFace)
	{
		// Find the face with the specified hkFace
		if (NULL != m_pIwObj)
		{
			IwTArray<IwFace *> arrFaces;
			((IwBrep *) m_pIwObj)->GetFaces (arrFaces);
			int nFaces = arrFaces.GetSize ();

			for (int iFace = 0; iFace < nFaces; iFace++)
			{
				IwAttribute *pAttribute = arrFaces[iFace]->FindAttribute (AttributeID_HKEY);

				if (pAttribute && pAttribute->GetNumLongElements () > 0)
				{
					const long *lAttributes = pAttribute->GetLongElementsAddress ();
					if (lAttributes[0] == hkFace)
					{
						if (NULL != (pAttribute = ((IwAObject *) arrFaces[iFace])->FindAttribute (AttributeID_BREPFACECOUNTER)) &&
							pAttribute->GetNumLongElements () > 0)
						{
							const long *lAttributes = pAttribute->GetLongElementsAddress ();
							BrepFaceProxy *faceProxy = new BrepFaceProxy (m_pContext, this, lAttributes[0]);
							return faceProxy;
						}

						return NULL;
					}
				}
			}
		}
		return NULL;
	}

	System::Collections::ArrayList __gc * Brep::GetEdges ()
	{
		System::Collections::ArrayList __gc *arrEdgeProxies = new System::Collections::ArrayList ();

		if (m_pIwObj != NULL)
		{
			IwTArray<IwEdge *> arrEdges;
			((IwBrep *) m_pIwObj)->GetEdges (arrEdges);
			ULONG numEdges = arrEdges.GetSize ();
			for (ULONG iEdge = 0; iEdge < numEdges; iEdge++)
			{
				// Try to find the Edge counter attribute.

				IwAttribute *pAttribute = 0;
				if (NULL != (pAttribute = ((IwAObject *) arrEdges[iEdge])->FindAttribute (AttributeID_BREPEDGECOUNTER)) &&
					pAttribute->GetNumLongElements () > 0)
				{
					const long *lAttributes = pAttribute->GetLongElementsAddress ();
					BrepEdgeProxy *edgeProxy = new BrepEdgeProxy ();
					edgeProxy->Brep = this;
					edgeProxy->EdgeID = lAttributes[0];
					arrEdgeProxies->Add (edgeProxy);
				}
			}
		}

		return arrEdgeProxies;
	}

	BrepEdgeProxy __gc * Brep::GetEdge(long iEdgeId)
	{
		System::Collections::ArrayList __gc * arrEdgeProxies = this->GetEdges();
		for (int i = 0; i < arrEdgeProxies->Count; i++)
		{
			BrepEdgeProxy *edge = __try_cast<BrepEdgeProxy *> (arrEdgeProxies->get_Item(i));
			if (edge->EdgeID == iEdgeId)
				return edge;
		}
		return NULL;
	}

	BrepEdgeProxy __gc * Brep::GetEdge (HC::KEY hkEdge)
	{
		// Find the edge with the specified hkEdge
		if (NULL != m_pIwObj)
		{
			IwTArray<IwEdge *> arrEdges;
			((IwBrep *) m_pIwObj)->GetEdges (arrEdges);
			int nEdges = arrEdges.GetSize ();

			for (int iEdge = 0; iEdge < nEdges; iEdge++)
			{
				IwAttribute *pAttribute = arrEdges[iEdge]->FindAttribute (AttributeID_HKEY);

				if (pAttribute && pAttribute->GetNumLongElements () > 0)
				{
					const long *lAttributes = pAttribute->GetLongElementsAddress ();
					if (lAttributes[0] == hkEdge)
					{
						if (NULL != (pAttribute = ((IwAObject *) arrEdges[iEdge])->FindAttribute (AttributeID_BREPEDGECOUNTER)) &&
							pAttribute->GetNumLongElements () > 0)
						{
							const long *lAttributes = pAttribute->GetLongElementsAddress ();
							BrepEdgeProxy *edgeProxy = new BrepEdgeProxy (m_pContext, this, lAttributes[0]);
							return edgeProxy;
						}
					}
				}
			}
		}
		return NULL;
	}

	BrepRegionProxy __gc * Brep::RegionContainingPoint(Vector3d *pTest)
	{
		IwBrep *pBrep = (IwBrep *)this->GetIwObj();
		IwPoint3d* ptTest = (IwVector3d *) pTest->GetIwObj();
		IwPointClass pcTest;
		try
		{
			if (IW_SUCCESS == pBrep->Point3DClassify(*ptTest, IW_EFF_ZERO, FALSE, pcTest))
			{
				if (pcTest.GetPointClass() == IW_PC_REGION)
				{
					IwRegion *pRegion = (IwRegion *)pcTest.GetObject();
					if (pRegion == pBrep->GetInfiniteRegion())
						return NULL;
					IwAttribute *pRegionAttribute = pRegion->FindAttribute (AttributeID_BREPREGIONCOUNTER);
					if (pRegionAttribute && pRegionAttribute->GetNumLongElements () > 0)
					{
						const long *lRegionIDs = pRegionAttribute->GetLongElementsAddress ();
						return new BrepRegionProxy (m_pContext, this, lRegionIDs[0]);
					}
					return NULL;
				}
			}
		} 
		catch (System::Exception *e)
		{
			System::Console::WriteLine(e->Message);
		}
		return NULL;
	}
	//void Brep::Tesselation (HC::KEY keyGeom,bool bDrawDetailed,double dMaxAspectRatio,double dMax3DEdgeLength,
	//	XML::XmlDocument __gc * pXMLDoc,XML::XmlElement __gc * pXMLRootElm)
	//{

	//	try
	//	{
	//		if (!m_pIwObj)
	//			return;

	//		// Set up tesselation parameters
	//		// The values used here are pretty coarse, not good for fastship hulls
	//		// The tolerances should one day make it to the parameter list of the function

	//		double dChordHeightTol = 0.01; // Maximum chord height of polygon 
	//		double dAngleTolInDegrees = 45.0; // Maximum angle in U or V that the polygon will span. 
	//		ULONG lMinCurveDivisions = 0; // Don't use this tolerance 
	//		//double dMax3DEdgeLength = 0.;//4.0; // Maximum length of an edge 
	//		double dMin3DEdgeLength = 0.0; // Minimum length of an edge - not used 
	//		//double dMaxAspectRatio =  0.0; // 4:1 aspect ratio 
	//		double dMinUVRatio = 0.00001; // Smallest subdivision of a surface is 

	//		if (m_tessParam != NULL)
	//		{
	//			// Use provided dimension tolerances
	//			dChordHeightTol = m_tessParam->ChordHeight;
	//			dAngleTolInDegrees = m_tessParam->MaxAngle;
	//			dMax3DEdgeLength = m_tessParam->Max3dEdgeLength;
	//			dMaxAspectRatio = m_tessParam->MaxAspectRatio;
	//			dMinUVRatio = m_tessParam->MinUvRatio;
	//		}

	//		// Create a curve tessellation driver and a surface tessellation
	//		// driver to control how subdivision is performed. 
	//		IwCurveTessDriver sCrvTess ( lMinCurveDivisions,
	//			dChordHeightTol,
	//			dAngleTolInDegrees );

	//		IwSurfaceTessDriver sSrfTess ( dChordHeightTol,
	//			dAngleTolInDegrees,
	//			dMax3DEdgeLength,
	//			dMin3DEdgeLength,
	//			dMinUVRatio,
	//			dMaxAspectRatio );

	//		// Declare an instance of IwTess class ; sTess

	//		IwTess sTess (GetIwContext(), sCrvTess, sSrfTess);

	//		// Note that the Brep sent into the Tessellator gets modified
	//		// so you usually will want to make a copy of it is follows:
	//		IwBrep *pCopy = new (GetIwContext()) IwBrep (*((IwBrep *) m_pIwObj));

	//		// Now do the tessellation process to produce a shell in UV
	//		// space of the surfaces. Use the phased tesselation both to conserver
	//		// memory and to facilitate setting of face attributes

	//		IwBoolean rbFailedFaces = FALSE;

	//		if (IW_SUCCESS != sTess.Phase1SetupBrep (pCopy, rbFailedFaces))
	//		{
	//			// TODO - throw exception
	//			return;
	//		}

	//		if (rbFailedFaces == TRUE)
	//		{
	//			// TODO - not sure what but this library really shouldn't be popping up graphical windows
	//			//			MessageBox::Show("At least on face failed to tesselate properly.", "Tesselation Error",
	//			//				MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
	//		}

	//		// This method will output triangles using sPolyOutput class that knows about hoops
	//		PolygonOutputCallbackMoldedForms oPolyOutput;

	//		// Enter output file to write tesselation data to
	//		oPolyOutput.s_lNumberPolygons = 0;
	//		//oPolyOutput.SetOutputType(IW_PO_TRIANGLE_MESH);	
	//		oPolyOutput.SetOutputType(IW_PO_QUADRALATERALS);	
	//		oPolyOutput.m_pContext = &GetIwContext ();
	//		oPolyOutput.SetGenerateNormals (FALSE);

	//		IwTArray<IwFace *> arrFaces;
	//		pCopy->GetFaces (arrFaces);
	//		ULONG ulNumFaces = arrFaces.GetSize ();

	//		IwTArray<IwFace *> arrRealFaces;
	//		((IwBrep *) m_pIwObj)->GetFaces (arrRealFaces);

	//		//Attribute each face
	//		for (ULONG ulFaceCnt = 0; ulFaceCnt < ulNumFaces; ulFaceCnt++)
	//		{
	//			oPolyOutput.m_pIwObj = (IwAObject *) arrFaces[ulFaceCnt];
	//			IwAttribute *pAttribute = arrFaces[ulFaceCnt]->FindAttribute (AttributeID_PROPERTY);
	//			if (NULL != pAttribute)
	//			{
	//				//oPolyOutput.m_propertyID=((IwLongAttribute*)pAttribute)->GetValue();
	//			}

	//			if (IW_SUCCESS == sTess.Phase2CreateFacePolygons (arrFaces[ulFaceCnt]))
	//				sTess.OutputFacePolygons (arrFaces[ulFaceCnt], oPolyOutput);
	//			sTess.Phase3DeleteFacePolygons (arrFaces[ulFaceCnt]);

	//			// Now copy the AttributeID_HKEY attributes from the face copies to the real faces.

	//			pAttribute = arrFaces[ulFaceCnt]->FindAttribute (AttributeID_HKEY);
	//			if (NULL != pAttribute)
	//			{
	//				IwAttribute *pExistingAttribute = arrRealFaces[ulFaceCnt]->FindAttribute (AttributeID_HKEY);
	//				if (pExistingAttribute != NULL)
	//					arrRealFaces[ulFaceCnt]->RemoveAttribute (pExistingAttribute, FALSE);
	//				IwAttribute *pNewAttribute = pAttribute->MakeCopy (GetIwContext());
	//				arrRealFaces[ulFaceCnt]->AddAttribute (pNewAttribute);
	//			}
	//		}

	//		//Insert Graphics
	//		HC::Open_Segment_By_Key (keyGeom);
	//		{
	//			HC::Flush_Contents ("...", "geometry");

	//			HC::Set_Visibility ("edges=on");
	//			//HC::UnSet_One_Visibility ("faces");
	//			//HC::UnSet_One_Visibility ("edges");

	//			// create hoops shell entity 
	//			int *face_list=new int[oPolyOutput.m_hoopsShellFaceLength];
	//			int total_polygon_points=oPolyOutput.m_nodeID;
	//			HC_POINT	*shell_points=new HC_POINT[total_polygon_points];

	//			//sort node based on index
	//			std::map<long,SMPoint*> mapNode;
	//			std::multimap<double,SMPoint*>::iterator pos;
	//			for (pos  = oPolyOutput.m_mMapX.begin(); pos != oPolyOutput.m_mMapX.end();pos++)
	//			{
	//				SMPoint* pNode = pos->second;
	//				mapNode[pNode->m_Id]=pNode;
	//			}
	//			int index=0;
	//			std::map<long,SMPoint*> ::iterator itMap;
	//			for (itMap  = mapNode.begin(); itMap != mapNode.end();itMap++)
	//			{
	//				SMPoint* pNode = itMap->second;
	//				shell_points[index].x=(float)pNode->m_x;
	//				shell_points[index].y=(float)pNode->m_y;
	//				shell_points[index].z=(float)pNode->m_z;
	//				index++;
	//			}
	//			index=0;
	//			std::set<SMElem*>::iterator it;
	//			for (it  = oPolyOutput.m_setElem.begin(); it != oPolyOutput.m_setElem.end();it++)
	//			{
	//				SMElem* pElm = *it;
	//				if (pElm->m_Nd[3]==-1)
	//				{
	//					face_list[index++]=3;
	//					face_list[index++]=pElm->m_Nd[0];
	//					face_list[index++]=pElm->m_Nd[1];
	//					face_list[index++]=pElm->m_Nd[2];
	//				}
	//				else
	//				{
	//					face_list[index++]=4;
	//					face_list[index++]=pElm->m_Nd[0];
	//					face_list[index++]=pElm->m_Nd[1];
	//					face_list[index++]=pElm->m_Nd[2];
	//					face_list[index++]=pElm->m_Nd[3];
	//				}
	//			}
	//			HC_KEY hkShell;
	//			hkShell = HC_KInsert_Shell (total_polygon_points,
	//				shell_points, oPolyOutput.m_hoopsShellFaceLength, face_list);
	//			delete []face_list;
	//			delete []shell_points;
	//			//add2dom
	//			{
	//				XML::XmlElement __gc *pXMLElmClass=pXMLDoc->CreateElement("Class");
	//				pXMLRootElm->AppendChild(pXMLElmClass);
	//				pXMLElmClass->SetAttribute ("sName", "FeModel");
	//				pXMLElmClass->SetAttribute ("sInterfaces", "IPHGeometry");
	//				pXMLElmClass->SetAttribute ("sNamespace", "VEDM.Apps.CSafe");
	//				XML::XmlElement __gc *pXMLElmInstances=pXMLDoc->CreateElement("Instances");
	//				pXMLElmClass->AppendChild(pXMLElmInstances);
	//				XML::XmlElement __gc *pXMLElmFeModel=pXMLDoc->CreateElement("FeModel");
	//				pXMLElmInstances->AppendChild(pXMLElmFeModel);
	//				pXMLElmFeModel->SetAttribute ("idSelf", Guid::NewGuid().ToString("B"));
	//				//Nodes
	//				XML::XmlElement __gc *pXMLElmNodes=pXMLDoc->CreateElement("Nodes");
	//				pXMLElmFeModel->AppendChild(pXMLElmNodes);
	//				for (itMap  = mapNode.begin(); itMap != mapNode.end();itMap++)
	//				{
	//					SMPoint* pNode = itMap->second;
	//					XML::XmlElement __gc *pXMLElmNode=pXMLDoc->CreateElement("Node");
	//					pXMLElmNode->SetAttribute ("iId", pNode->m_Id.ToString());
	//					pXMLElmNode->SetAttribute ("dX", pNode->m_x.ToString());
	//					pXMLElmNode->SetAttribute ("dY", pNode->m_y.ToString());
	//					pXMLElmNode->SetAttribute ("dZ", pNode->m_z.ToString());
	//					pXMLElmNodes->AppendChild(pXMLElmNode);
	//				}
	//				//Element
	//				XML::XmlElement __gc *pXMLElmElements=pXMLDoc->CreateElement("Elements");
	//				pXMLElmFeModel->AppendChild(pXMLElmElements);
	//				long elmId=0;
	//				char buf[128];
	//				for (it  = oPolyOutput.m_setElem.begin(); it != oPolyOutput.m_setElem.end();it++)
	//				{
	//					SMElem* pElm = *it;
	//					elmId++;
	//					if (pElm->m_Nd[3]==-1)
	//					{
	//						sprintf(buf,"%d %d %d",pElm->m_Nd[0],pElm->m_Nd[1],pElm->m_Nd[2]);
	//						XML::XmlElement __gc *pXMLElmTri=pXMLDoc->CreateElement("Tri");
	//						pXMLElmTri->SetAttribute ("iId", elmId.ToString());
	//						pXMLElmTri->SetAttribute ("sNodeIds", buf);
	//						pXMLElmTri->SetAttribute ("iIdProp", "abc");
	//						pXMLElmElements->AppendChild(pXMLElmTri);
	//					}
	//					else
	//					{
	//						sprintf(buf,"%d %d %d %d",pElm->m_Nd[0],pElm->m_Nd[1],pElm->m_Nd[2],pElm->m_Nd[3]);
	//						XML::XmlElement __gc *pXMLElmQuad=pXMLDoc->CreateElement("Quad");
	//						pXMLElmQuad->SetAttribute ("iId", elmId.ToString());
	//						pXMLElmQuad->SetAttribute ("sNodeIds", buf);
	//						pXMLElmQuad->SetAttribute ("iIdProp", "abc");
	//						pXMLElmElements->AppendChild(pXMLElmQuad);
	//					}
	//				}		
	//			}
	//		}
	//		HC::Close_Segment ();


	//		/*
	//		// Initializes the variables to pass to the MessageBox::Show method.
	//		String* message = S"Would you like to keep the tesselation?";
	//		String* caption = S"Mesh Control";
	//		MessageBoxButtons buttons = MessageBoxButtons::YesNo;
	//		System::Windows::Forms::DialogResult result;

	//		// Displays the MessageBox.
	//		result = MessageBox::Show(message, caption, buttons,
	//		MessageBoxIcon::Question, MessageBoxDefaultButton::Button1, 
	//		MessageBoxOptions::RightAlign);

	//		if (result == DialogResult::Yes)
	//		{
	//		}
	//		*/
	//	}
	//	catch (...)
	//	{
	//		// TODO
	//	}
	//}
	void Brep::AssignPropertyAttribute (String __gc * sName)
	{
		IwBrep* pIwBrep=(IwBrep *) m_pIwObj;
		IwAttribute *pExistingAttribute = pIwBrep->FindAttribute (AttributeID_PROPERTY);
		if (NULL != pExistingAttribute)
		{
			pIwBrep->RemoveAttribute (pExistingAttribute, TRUE);
			pExistingAttribute = NULL;
		}

		if (NULL == pExistingAttribute)
		{
			//IwLongAttribute *pAttribute = new (GetIwContext()) IwLongAttribute(
			//	AttributeID_PROPERTY, propertyID, IW_AB_COPY);
			IwTArray<char> iwName;
			int iLength=sName->get_Length();
			for (int i=0;i<iLength;i++)
			{
				iwName.Add((char)sName->get_Chars(i));
			}
			IwGenericAttribute *pAttribute = new (GetIwContext()) IwGenericAttribute(
				AttributeID_PROPERTY, IW_AB_COPY,NULL,NULL,iwName);

			pIwBrep->AddAttribute (pAttribute);
		}
	}

	IwEdge * Brep::GetIwEdgeFromProxy (BrepEdgeProxy __gc *edgeProxy)
	{
		IwEdge *pIwEdge = NULL;

		try
		{
			// First retrieve the Brep Edge from the owning brep.

			if (NULL != m_pIwObj)
			{
				IwTArray<IwEdge *> arrEdges;
				((IwBrep *) m_pIwObj)->GetEdges(arrEdges);
				int nEdges = arrEdges.GetSize();
				for (int i = 0; i < nEdges; i++)
				{
					IwAttribute *pAttribute = arrEdges[i]->FindAttribute (AttributeID_BREPEDGECOUNTER);
					if (pAttribute && pAttribute->GetNumLongElements () > 0)
					{
						const long *lAttributes = pAttribute->GetLongElementsAddress ();
						if (lAttributes[0] == edgeProxy->EdgeID)
						{
							pIwEdge = arrEdges[i];
							break;
						}
					}
				}
			}
		}
		catch (...)
		{
			Console::WriteLine("Could not get IwEdge associated with BrepEdgeProxy");
		}

		return pIwEdge;
	}

	void Brep::MergeEdges(BrepEdgeProxy __gc* edge1, BrepEdgeProxy __gc* edge2)
	{
		IwVertex *pDeleteVertex = NULL;
		IwTArray<IwEdge *> pEdges;
		IwTArray<IwFace *> pFaces1;
		IwTArray<IwFace *> pFaces2;
		IwTArray<IwFace *> pCommonFaces;

		IwStatus status;

		IwBrep *pBrep = (IwBrep *)(this->GetIwObj());
		pBrep->m_bEditingEnabled = TRUE;

		IwEdge *pEdge1 = this->GetIwEdgeFromProxy(edge1);
		IwEdge *pEdge2 = this->GetIwEdgeFromProxy(edge2);

		// Determine whether both edges share a common curve
		//IwCurve *pCurve1 = pEdge1->GetCurve();
		//IwCurve *pCurve2 = pEdge2->GetCurve();
		// Return unless edges share bound same faces
		pEdge1->GetFaces(pFaces1);
		pEdge2->GetFaces(pFaces2);
		pFaces1.FindCommonElements(pFaces2, pCommonFaces);

		if (pFaces1.GetSize() == pCommonFaces.GetSize()) //(pCurve1 == pCurve2)
		{
			IwVertex *pVStartE1 = pEdge1->GetVertex();
			IwVertex *pVEndE1 = pEdge1->GetOtherVertex(pVStartE1);
			IwVertex *pVStartE2 = pEdge2->GetVertex();
			IwVertex *pVEndE2 = pEdge2->GetOtherVertex(pVStartE2);
			// Find common vertex, if any; otherwise return
			if (pVEndE1 == pVStartE2)		
			{
				pDeleteVertex = pVEndE1;
				pDeleteVertex->GetEdges(pEdges);
				if (pEdges.GetSize() == 2)
				{
					status = pBrep->DeleteTopologicalVertex(pDeleteVertex, pEdge1);
					m_bCountersDirty = true;
					AddToDOM ();
				}
			}
			else if (pVStartE1 == pVEndE2)
			{
				pDeleteVertex = pVEndE2;
				pDeleteVertex->GetEdges(pEdges);
				if (pEdges.GetSize() == 2)
				{
					status = pBrep->DeleteTopologicalVertex(pDeleteVertex, pEdge2);
					m_bCountersDirty = true;
					AddToDOM ();
				}
			}
			pBrep->m_bEditingEnabled = FALSE;
			if (status != IW_SUCCESS)
				throw new System::Exception("Unable to delete common topological vertex.");
		}
	}

	void Brep::RemoveEdge(BrepEdgeProxy __gc* edge)
	{
		System::Collections::ArrayList __gc* delEdges = new System::Collections::ArrayList();
		delEdges->Add(edge);
		RemoveEdges(delEdges);
	}

	void Brep::RemoveEdges(System::Collections::ArrayList __gc* delEdges)
	{
		// Create a list of underlying edges
		IwTArray<IwEdge *> arrEdges;
		for(int iEdge = 0; iEdge < delEdges->Count; iEdge++)
		{
			BrepEdgeProxy __gc *edge = static_cast<BrepEdgeProxy __gc *>(delEdges->get_Item(iEdge));
			IwEdge *pEdge = this->GetIwEdgeFromProxy(edge);
			arrEdges.Add(pEdge);
		}
		IwBrep *pBrep = (IwBrep *) (this->GetIwObj());
		IwTArray<IwFace*> pFaces;
		IwFace *pFace1 = NULL;
		IwFace *pFace2 = NULL;
		IwSurface *pSurf1 = NULL;
		IwSurface *pSurf2 = NULL;
		IwAttribute *pId1 = NULL;
		IwAttribute *pId2 = NULL;
		for(unsigned int iEdge = 0; iEdge < arrEdges.GetSize(); iEdge++)
		{
			String *sId1 = "1";
			String *sId2 = "2";
			IwEdge *pEdge = arrEdges[iEdge];
			pEdge->GetFaces(pFaces);
			unsigned int nFaces = pFaces.GetSize();
			// Only consider removing edges that join two faces
			if (nFaces <= 2)
			{
				// Check that the underlying geometry is the same for both faces
				pFace1 = pFaces[0];
				pSurf1 = pFace1->GetSurface();
				pId1 = pSurf1->FindAttribute( AttributeID_IDSELF);
				if (pId1 != NULL)
					sId1 = new String(pId1->GetCharacterElementsAddress());
				if (nFaces == 2)
				{
					pFace2 = pFaces[1];
					pSurf2 = pFace2->GetSurface();
					pId2 = pSurf2->FindAttribute( AttributeID_IDSELF);
					if (pId2 != NULL)
						sId2 = new String(pId2->GetCharacterElementsAddress());
				}
				// Compare surfaces using either their IdSelf values or their underlying geometic entities.
				if (nFaces == 1 || sId1->Equals(sId2) || pSurf1 == pSurf2)
				{
					pBrep->m_bEditingEnabled = TRUE;
					long status = pBrep->DeleteEdge(pEdge);
					if (status == IW_SUCCESS)
					{
						m_bCountersDirty = true;
						AddToDOM ();
					}
					else
					{
						System::Windows::Forms::MessageBox::Show("IwBrep::DeleteEdge failed.");
					}
					pBrep->m_bEditingEnabled = FALSE;
					delEdges->Clear();
				}
			}
		}
	}
}
