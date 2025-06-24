#include "stdafx.h"
#include "smnurbssurface.h"
using namespace System::Text;

namespace PESMLIB
{

	NurbsSurface::NurbsSurface(void) : SMObject()
	{
	}

	NurbsSurface::NurbsSurface(Context* oContext, XML::XmlElement* pElem) : SMObject()
	{
		m_pContext = oContext;
		if (HasIwContext())
		{
			m_pIwObj = (IwObject *) new (GetIwContext()) IwBSplineSurface;
			m_pXMLElem = pElem;

			// Now populate the Brep via the DOM if there is anything in the DOM.
			// Otherwise it is just the node where we are supposed to store the 
			// state later.

			if (m_pXMLElem != NULL)
			{
				GetFromDOM ();
				CreateId ();
				SetIwObjAttribute ();
			}
		}
		else
		{
			m_pIwObj = NULL;
			m_pXMLElem = NULL;
		}
	}

	int NurbsSurface::CompareTo (System::Object^ obj)
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
				NurbsSurface *pNurbsSurface = __try_cast<NurbsSurface *> (obj);
				if (NULL != pNurbsSurface)
				{
				   if (NULL != m_pXMLElem)
					  return IdSelf->CompareTo (pNurbsSurface->IdSelf);
				   else if (m_pIwObj == pNurbsSurface->GetIwObj ())
					  return 0;
				   else if (m_pIwObj < pNurbsSurface->GetIwObj ())
					  return -1;
				   else
					  return 1;
				}
			}
		}
		catch (System::InvalidCastException*)
		{
			Console::WriteLine ("Could not cast object to NurbsSurface.");
			throw new System::ArgumentException ("Object is not a NurbsSurface.");
		}

		return 1;
	}

	bool NurbsSurface::Equals (System::Object^  obj)
	{
		try
		{
         // If this is a persistent brep, compare id's, otherwise compare underlying geometry.

			NurbsSurface *pNurbsSurface = __try_cast<NurbsSurface *> (obj);
			if (NULL != pNurbsSurface)
			{
            if (NULL != m_pXMLElem)
            {
				   if (pNurbsSurface->get_IdSelf () == get_IdSelf () && get_IdSelf () != NULL)
					   return true;
            }
            else if (m_pIwObj == pNurbsSurface->GetIwObj ())
               return true;
			}
		}
		catch(System::InvalidCastException*)
		{
			Console::WriteLine("Could not cast object to NurbsSurface");
		}
		return false;
	}

	void NurbsSurface::AttachIwObj (Context *pContext, IwObject *pIwObj)
	{
		try
		{
			// Check to make sure object type is appropriate

			if (pIwObj && pIwObj->IsKindOf (IwBSplineSurface_TYPE))
			{
				// Free any old IwObject. Update the DOM.

				if (m_pIwObj)
					delete m_pIwObj;
				m_pIwObj = pIwObj;
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

	void NurbsSurface::Copy (NurbsSurface^  srcNurbs)
   {
      try
      {
         if (srcNurbs == NULL || !HasIwContext())
         {
            // todo: throw exception
         }

         if (m_pIwObj)
         {
            delete m_pIwObj;
            m_pIwObj = 0;
         }

         // Copy the IwBsplineSurface.

         IwBSplineSurface *pSrcNurbs = (IwBSplineSurface *) srcNurbs->GetIwObj ();
         m_pIwObj = new (GetIwContext ()) IwBSplineSurface (*pSrcNurbs);

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
	  catch (Exception *)
      {
         Console::WriteLine("Could not copy surface.");
      }
   }

   void NurbsSurface::SetCanonical (ULONG lUDegree, ULONG lVDegree,
                const IwTArray<IwPoint3d> & crControlPointsList,
                IwBSplineSurfaceForm eBSplineSurfaceForm,
                const IwTArray<ULONG> & crUMultiplicities,
                const IwTArray<ULONG> & crVMultiplicities,
                const IwTArray<double> & crUKnots,
                const IwTArray<double> & crVKnots, IwKnotType eKnotType,
                const IwTArray<double> * cpOptWeights,
                const IwExtent2d * cpOptUVDomain)
   {
		if (HasIwContext())
		{
			if (m_pIwObj != NULL)
			{
				delete m_pIwObj;
				m_pIwObj = 0;
			}
            
         IwBSplineSurface *pTmp;
         IwStatus status = IwBSplineSurface::CreateCanonical(GetIwContext(), lUDegree, lVDegree,
            crControlPointsList, eBSplineSurfaceForm, crUMultiplicities, crVMultiplicities,
            crUKnots, crVKnots, eKnotType, cpOptWeights, cpOptUVDomain, pTmp);

         m_pIwObj = pTmp;
			if (m_pIwObj != NULL)
				AddToDOM ();
		}
   }

 //  void NurbsSurface::Transform (PEHoops::MVO::Transformation^  oTransformation)
	//{
 //     try
 //     {
 //        IwBSplineSurface *pSurface = (IwBSplineSurface *) GetIwObj ();
 //        if (pSurface != NULL && oTransformation != NULL)
 //        {
 //           float matrix __gc[] = oTransformation->Matrix;
 //           IwAxis2Placement crRotateNMove;
 //           crRotateNMove.SetCanonical (
 //              IwPoint3d ((double) matrix[12], (double) matrix[13], (double) matrix[14]), 
 //              IwVector3d ((double) matrix[0], (double) matrix[4], (double) matrix[8]), 
 //              IwVector3d ((double) matrix[1], (double) matrix[5], (double) matrix[9]));
 //           pSurface->Transform (crRotateNMove);
 //        }
 //     }
 //     catch (System::Exception^ ex)
 //     {
 //        System::Console::WriteLine (ex->Message);
 //     }
	//}

	bool NurbsSurface::ComputeBoundingBox (HC::NL_POINT^  ptMin, HC::NL_POINT^  ptMax)
	{
      // TODO
		bool bRet = false;
		return bRet;
	}

	void NurbsSurface::Highlight (HC::KEY keySeg)
	{
	//	HC::KEY key;
	//	HC::Open_Segment_By_Key (keySeg);
	//	{
	//		HC::Open_Segment("highlight");
	//		{
	//			HC::Set_Color(sHighlightColor);
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
	//	HC::Close_Segment ();
	}

	void NurbsSurface::UnHighlight (HC::KEY keySeg)
	{
	//	HC::KEY key;
	//	HC::Open_Segment_By_Key (keySeg);
	//	{
	//		HC::Open_Segment("highlight");
	//		{
	//			HC::Begin_Contents_Search (".", "geometry");
	//			{
	//				StringBuilder* sType = new StringBuilder ("geometry");
	//				while (HC::Find_Contents (sType,&key))
	//				{
	//					HC::Move_By_Key(key, "..");
	//				}
	//			}
	//			HC::End_Contents_Search ();
	//		}
	//		HC::Close_Segment();
	//	}
	//	HC::Close_Segment ();
	}

	System::Object * NurbsSurface::GetReferencableObject ()
	{
		return this;
	}

	void NurbsSurface::InsertGraphics (bool bDrawDetailed, int handle)
	{
	//	try
	//	{
	//		if (m_pIwObj != NULL)
	//		{

	//			// Get Hoops Nurbs curve input.

	//			ULONG ulUDegree, ulVDegree;
	//			IwTArray<IwVector3d> arrCtrlPts;
	//			IwBSplineSurfaceForm oSurfaceForm;
	//			IwTArray<ULONG> arrUMultiplicities, arrVMultiplicities;
	//			IwTArray<double> arrUKnots, arrVKnots, arrWeights;
	//			IwKnotType oKnotType;
	//			int iUNumPts = 0;
	//			int iVNumPts = 0;

	//			if (IW_SUCCESS != ((IwBSplineSurface *) m_pIwObj)->GetCanonical (
	//				ulUDegree, ulVDegree, arrCtrlPts, oSurfaceForm, arrUMultiplicities, 
	//				arrVMultiplicities, arrUKnots, arrVKnots, oKnotType, arrWeights))
	//			{
	//				//TODO
	//			}

	//			HC::POINT fCntrlPts __gc[] = 0;
	//			float fWeights __gc[] = 0;
	//			float fUKnots __gc[] = 0, fVKnots __gc[] = 0;
	//			int iNumPts = arrCtrlPts.GetSize ();
	//			if (iNumPts > 0)
	//			{
	//				fCntrlPts = new HC::POINT __gc[iNumPts];
	//				for (long lPt = 0; lPt < iNumPts; lPt++)
	//				{
	//					fCntrlPts[lPt].x = (float) arrCtrlPts[lPt].x;
	//					fCntrlPts[lPt].y = (float) arrCtrlPts[lPt].y;
	//					fCntrlPts[lPt].z = (float) arrCtrlPts[lPt].z;
	//				}

	//				if (iNumPts == arrWeights.GetSize ())
	//				{
	//					fWeights = new float __gc[iNumPts];
	//					for (long lWeight = 0; lWeight < iNumPts; lWeight++)
	//						fWeights[lWeight] = (float) arrWeights[lWeight];
	//				}

	//				int nUKnots = ((IwBSplineSurface *) m_pIwObj)->GetNumberNaturalKnots (IW_SP_U);
	//				int iUNumKnots = arrUKnots.GetSize ();
	//				fUKnots = new float __gc[nUKnots];
	//				for (long lUKnot = 0, lUCnt = 0; lUKnot < iUNumKnots; lUKnot++)
	//				{
	//					for (ULONG lMult = 0; lMult < arrUMultiplicities[lUKnot]; lMult++)
	//						fUKnots[lUCnt++] = (float) arrUKnots[lUKnot];
	//				}

	//				int nVKnots = ((IwBSplineSurface *) m_pIwObj)->GetNumberNaturalKnots (IW_SP_V);
	//				int iVNumKnots = arrVKnots.GetSize ();
	//				fVKnots = new float __gc[nVKnots];
	//				for (long lVKnot = 0, lVCnt = 0; lVKnot < iVNumKnots; lVKnot++)
	//				{
	//					for (ULONG lMult = 0; lMult < arrVMultiplicities[lVKnot]; lMult++)
	//						fVKnots[lVCnt++] = (float) arrVKnots[lVKnot];
	//				}
	//				iUNumPts = nUKnots - ulUDegree - 1;
	//				iVNumPts = nVKnots - ulVDegree - 1;
	//			}

	//			// This method will output triangles using sPolyOutput class
	//			HC::Open_Segment_By_Key (keyGeom);
	//			{
	//				// Set Hoops attributes

	//				HC::Set_Marker_Size(0.3);
	//				HC::Set_Marker_Symbol("@");
	//				HC::Set_Color_By_Index("markers", 4);
	//				HC::Set_Color_By_Index("edges",	9);
	//				HC::Set_Color_By_Index("faces",	3);
	//				HC::Set_Color_By_Index("lines",	6);
	//				HC::Set_Rendering_Options("nurbs surface=(budget=800)");
	//				HC::Set_Selectability("geometry=v");
	//				//HC::Set_Visibility("edges=on");
	//				//HC::Set_Line_Pattern("-	-");
	//				HC::Insert_NURBS_Surface ((int) ulUDegree, (int) ulVDegree, 
	//					iUNumPts, iVNumPts, fCntrlPts, fWeights, fUKnots, fVKnots);
	//			}
	//			HC::Close_Segment ();
	//		}
	//	}
	//	catch (...)
	//	{
	//		// TODO
	//	}
	}

	//void NurbsSurface::RemoveGraphics (KEY keyGeom)
 //  {
 //     // Flush the contents of the segment. It is the callers responsibility to create
 //     // or delete the containing segment.

 //     HC::Open_Segment_By_Key (keyGeom);
 //     HC::Flush_Contents ("...", "everything");
 //     HC::Close_Segment ();
	//}

	void NurbsSurface::AddToDOM ()
	{
		if (m_pIwObj != NULL && m_pXMLElem != NULL)
		{
			// Add the persistent ID.
			m_pXMLElem->SetAttribute("idSelf", GetId());
		}
	}

	void NurbsSurface::GetFromDOM ()
	{
		if (m_pXMLElem != NULL && m_pIwObj != NULL && HasIwContext())
		{
			// Get the persistent GUID.

			String *sId = m_pXMLElem->GetAttribute("idSelf");
			if (sId->Length > 0) SetId (sId);

			// Now populate the Surface via the DOM;

			ULONG luDegree, lvDegree, luPoles, lvPoles;
			IwTArray<IwVector3d> arrCP;
			IwBSplineSurfaceForm eForm;
			IwTArray<ULONG> arrUMulti, arrVMulti;
			IwTArray<double> arrUKnots, arrVKnots;
			IwKnotType eKnotType;
			IwTArray<double> arrOptWeights;
			IwExtent2d oOptUVDomain;

			luDegree = Convert::ToUInt32(m_pXMLElem->GetAttribute("iOrderU")) - 1;
			lvDegree = Convert::ToUInt32(m_pXMLElem->GetAttribute("iOrderV")) - 1;
			luPoles = Convert::ToUInt32(m_pXMLElem->GetAttribute("nPolesU")) - 1;
			lvPoles = Convert::ToUInt32(m_pXMLElem->GetAttribute("nPolesV")) - 1;

			// Extract poles (3d points and weights). Check for rational surface.
			// If non-rational don't search for weights and set optional weights
			// argument to NULL;

			IwTArray<double> *pOptWeights = 0;
			String* sTrueFalse = m_pXMLElem->GetAttribute("bRationalU");
			bool bRational = (sTrueFalse == "True") ? true : false;

			if (bRational)
				pOptWeights = &arrOptWeights;

			XML::XmlElement* pElPole = dynamic_cast<XML::XmlElement*>(m_pXMLElem->SelectSingleNode("Poles/Pole"));

			while (pElPole != NULL)
			{
				IwVector3d oPoint;
				double dWeight;

				oPoint.x = Convert::ToDouble(pElPole->GetAttribute("dX"));
				oPoint.y = Convert::ToDouble(pElPole->GetAttribute("dY"));
				oPoint.z = Convert::ToDouble(pElPole->GetAttribute("dZ"));
				if (bRational)
				{
					try
					{
						dWeight = Convert::ToDouble(pElPole->GetAttribute ("dH"));
					}
					catch (...)
					{
						dWeight = 1.;
					}
					arrOptWeights.Add (dWeight);
				}
				arrCP.Add (oPoint);
				pElPole = dynamic_cast<XML::XmlElement*>(pElPole->NextSibling);
			}

			// Set enumerated types

			eForm = IW_SF_UNSPECIFIED;
			eKnotType = IW_KT_UNSPECIFIED;

			// Get knot information.

			XML::XmlElement* pElKnot = dynamic_cast<XML::XmlElement*>(m_pXMLElem->SelectSingleNode("Knots[@eDirection='u']/Knot"));

			while (pElKnot != NULL)
			{
				ULONG ulMulti;
				double dValue;

				try
				{
					ulMulti = Convert::ToUInt32(pElKnot->GetAttribute("iMulti"));
				}
				catch (...)
				{
					ulMulti = 1;
				}

				dValue = Convert::ToDouble(pElKnot->GetAttribute("dValue"));

				arrUMulti.Add(ulMulti);
				arrUKnots.Add(dValue);
				pElKnot = dynamic_cast<XML::XmlElement*>(pElKnot->NextSibling);
			}

			pElKnot = dynamic_cast<XML::XmlElement*>(m_pXMLElem->SelectSingleNode("Knots[@eDirection='v']/Knot"));

			while (pElKnot != NULL)
			{
				ULONG ulMulti;
				double dValue;

				try
				{
					ulMulti = Convert::ToUInt32(pElKnot->GetAttribute("iMulti"));
				}
				catch (...)
				{
					ulMulti = 1;
				}

				dValue = Convert::ToDouble(pElKnot->GetAttribute ("dValue"));

				arrVMulti.Add(ulMulti);
				arrVKnots.Add(dValue);
				pElKnot = dynamic_cast<XML::XmlElement*>(pElKnot->NextSibling);
			}

			// Now create the SMLib surface

			if (IW_SUCCESS != (((IwBSplineSurface *) m_pIwObj)->SetCanonical ( 
				luDegree, lvDegree, arrCP, eForm, arrUMulti, arrVMulti, 
				arrUKnots, arrVKnots, eKnotType, pOptWeights)))
			{
				// TODO: throw error
			}
		}
	}

	int NurbsSurface::get_UOrder()
	{
		IwBSplineSurface *bspline = (IwBSplineSurface *) m_pIwObj;
		return bspline->GetDegree(IW_SP_U) + 1;
	}

	int NurbsSurface::get_VOrder()
	{
		IwBSplineSurface *bspline = (IwBSplineSurface *) m_pIwObj;
		return bspline->GetDegree(IW_SP_V) + 1;
	}

	int NurbsSurface::get_RowCount()
	{
		IwBSplineSurface *bspline = (IwBSplineSurface *) m_pIwObj;
		return bspline->GetNumberControlPoints(IW_SP_V);
	}

	int NurbsSurface::get_ColCount()
	{
		IwBSplineSurface *bspline = (IwBSplineSurface *) m_pIwObj;
		return bspline->GetNumberControlPoints(IW_SP_U);
	}


}