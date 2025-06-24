#include "stdafx.h"
#include "smplane.h"
using namespace System::Text;

namespace PESMLIB
{
	Plane::Plane(Context^  oContext, XML::XmlElement^  pElem) : SMObject()
	{
		m_pContext = oContext;
		if (HasIwContext())
		{
			Vector3d* oOrigin = new Vector3d;
			Vector3d* oNormal = new Vector3d;
			oOrigin->SetCanonical (0., 0., 0.);
			oNormal->SetCanonical (1., 0., 0.);

			m_pIwObj = new (GetIwContext()) IwPlane (*(IwVector3d *) oOrigin->GetIwObj (), *(IwVector3d *) oNormal->GetIwObj ());
			m_pXMLElem = pElem;
			// Added these calls so that planes used in breps would have GUIDs
			CreateId ();
			SetIwObjAttribute ();

			// Now populate the Plane via the DOM if there is anything in the DOM.
			// Otherwise it is just the node where we are supposed to store the 
			// state later.

			if (m_pXMLElem != NULL)
			{
				GetFromDOM ();
				//CreateId ();
				//SetIwObjAttribute ();
			}
		}
		else
		{
			m_pIwObj = NULL;
			m_pXMLElem = NULL;
		}
	}

	Plane::Plane() : SMObject()
	{
	}

	int Plane::CompareTo (System::Object^ obj)
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
				Plane *pPlane = __try_cast<Plane *> (obj);
				if (NULL != pPlane)
				{
				   if (NULL != m_pXMLElem)
						return IdSelf->CompareTo (pPlane->IdSelf);
				   else if (m_pIwObj == pPlane->GetIwObj ())
					  return 0;
				   else if (m_pIwObj < pPlane->GetIwObj ())
					  return -1;
				   else
					  return 1;
				}
			}
		}
		catch (System::InvalidCastException*)
		{
			Console::WriteLine ("Could not cast object to Plane.");
			throw new System::ArgumentException ("Object is not a Plane.");
		}

		return 1;
	}

	bool Plane::Equals (System::Object^  obj)
	{
		try
		{
         // If this is a persistent brep, compare id's, otherwise compare underlying geometry.

			Plane *pPlane = __try_cast<Plane *> (obj);
			if (NULL != pPlane)
			{
            if (NULL != m_pXMLElem)
            {
				   if (pPlane->get_IdSelf () == get_IdSelf () && get_IdSelf () != NULL)
					   return true;
            }
            else if (m_pIwObj == pPlane->GetIwObj ())
               return true;
			}
		}
		catch(System::InvalidCastException*)
		{
			Console::WriteLine("Could not cast object to Plane");
		}
		return false;
	}

	void Plane::AttachIwObj (Context *pContext, IwObject *pIwObj)
	{
		try
		{
			// Check to make sure object type is appropriate

			if (pIwObj && pIwObj->IsKindOf (IwPlane_TYPE))
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

	void Plane::GetCanonical (Axis2Placement^ axis2p)
	{
		try
		{
			if (NULL != m_pIwObj && NULL != axis2p)
			{
				IwPlane *pPlane = (IwPlane *) m_pIwObj;
				IwAxis2Placement iwAxis2p;
				IwStatus status = pPlane->GetCanonical (iwAxis2p);
				if (status == IW_SUCCESS)
				{
					IwVector3d origin;
					IwVector3d xaxis;
					IwVector3d yaxis;
					iwAxis2p.GetCanonical (origin, xaxis, yaxis);
					axis2p->SetCannonical (
						new Vector3d (origin.x, origin.y, origin.z),
						new Vector3d (xaxis.x, xaxis.y, xaxis.z),
						new Vector3d (yaxis.x, yaxis.y, yaxis.z));
				}
			}
		}
		catch (System::Exception *e)
		{
			Console::WriteLine (e->Message);
		}
	}

   void Plane::GetCanonical (Vector3d^  oOrigin, Vector3d^  oNormal)
   {
      try
      {
         if (NULL != m_pIwObj && NULL != oOrigin && NULL != oNormal)
         {
            IwPlane *pPlane = (IwPlane *) m_pIwObj;
            IwAxis2Placement axis2placement;
            IwStatus status = pPlane->GetCanonical (axis2placement);
            if (status == IW_SUCCESS)
            {
               IwVector3d zaxis = axis2placement.GetZAxis ();
               IwPoint3d origin = axis2placement.GetOrigin ();
               oOrigin->X = origin.x;
               oOrigin->Y = origin.y;
               oOrigin->Z = origin.z;
               oNormal->X = zaxis.x;
               oNormal->Y = zaxis.y;
               oNormal->Z = zaxis.z;
            }
         }
      }
      catch (System::Exception *e)
      {
         Console::WriteLine (e->Message);
      }
   }

	void Plane::SetCanonical (Vector3d* oOrigin, Vector3d* oNormal)
	{
		if (HasIwContext())
		{
			if (m_pIwObj != NULL)
			{
				delete m_pIwObj;
				m_pIwObj = 0;
			}
			IwVector3d *pOrigin = (IwVector3d *) oOrigin->GetIwObj ();
			IwVector3d *pNormal = (IwVector3d *) oNormal->GetIwObj ();
			if (pOrigin && pNormal)
				m_pIwObj = new (GetIwContext()) IwPlane (*pOrigin, *pNormal);
			if (m_pIwObj != NULL)
			{
				((IwPlane *) m_pIwObj)->MakeNurb ();
				SetIwObjAttribute();
				AddToDOM ();
			}
		}
	}
	void Plane::SetCanonical (Axis2Placement^ axis2p)
	{
		if (HasIwContext())
		{
			if (m_pIwObj != NULL)
			{
				delete m_pIwObj;
				m_pIwObj = 0;
			}

			const IwAxis2Placement *pIwa2p = axis2p->GetIwObj ();
			IwPoint3d iwOrigin = pIwa2p->GetOrigin ();
			IwVector3d iwNormal = pIwa2p->GetZAxis ();

			m_pIwObj = new (GetIwContext ()) IwPlane (iwOrigin, iwNormal);
			if (m_pIwObj != NULL)
			{
				((IwPlane *) m_pIwObj)->MakeNurb ();
				SetIwObjAttribute();
				AddToDOM ();

			}
		}
	}

	void Plane::Copy (Plane^  srcPlane)
   {
      try
      {
         if (srcPlane == NULL || !HasIwContext())
         {
            // todo: throw exception
         }

         if (m_pIwObj)
         {
            delete m_pIwObj;
            m_pIwObj = 0;
         }

         // Copy the IwPlane. For some reason just using the IwPlane copy constructor is not working
         // so we are forced to create a plane using the constructor that takes an origin and normal
         // vector obtained from calling GetCanonical on the source plane.

         IwPlane *pSrcPlane = (IwPlane *) (srcPlane->GetIwObj ());
         IwAxis2Placement oAxis2Placement;
         pSrcPlane->GetCanonical (oAxis2Placement);
         m_pIwObj = new (GetIwContext()) IwPlane (oAxis2Placement.GetOrigin (), oAxis2Placement.GetZAxis ());
         ((IwPlane *) m_pIwObj)->MakeNurb ();
         

         // Copy the IwPlane. 
/*
         IwPlane *pSrcPlane = (IwPlane *) (srcPlane->GetIwObj ());
         IwSurface *pIwSurface = 0;
         pSrcPlane->Copy (GetIwContext (), pIwSurface);
         m_pIwObj = pIwSurface;
*/
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
         Console::WriteLine("Could not copy plane.");
      }
   }

   NurbsSurface^  Plane::GetNurbsSurface ()
   {
      NurbsSurface^ nurbsSurface = NULL;
      try
      {
         IwPlane *pPlane = (IwPlane *) m_pIwObj;
         if (pPlane)
         {
            nurbsSurface = new NurbsSurface (m_pContext, NULL);

            ULONG lUDeg = 1, lVDeg = 1;
            IwPoint3d sData[4];
            IwTArray<IwPoint3d> sCtrlPts(4,sData,4);
            ULONG alUMData[2];
            IwTArray<ULONG> sUKnotMult(2,alUMData,2);
            ULONG alVMData[2];
            IwTArray<ULONG> sVKnotMult(2,alVMData,2);
            double adUData[2];
            IwTArray<double> sUKnots(2,adUData,2);
            double adVData[2];
            IwTArray<double> sVKnots(2,adVData,2);
            IwPoint3d sPnt;
            pPlane->EvaluatePointFast(pPlane->GetNaturalUVDomain ().Evaluate(0,0),sPnt);
            sCtrlPts[0] = sPnt;
            pPlane->EvaluatePointFast(pPlane->GetNaturalUVDomain ().Evaluate(0,1),sPnt);
            sCtrlPts[1] = sPnt;
            pPlane->EvaluatePointFast(pPlane->GetNaturalUVDomain ().Evaluate(1,0),sPnt);
            sCtrlPts[2] = sPnt;
            pPlane->EvaluatePointFast(pPlane->GetNaturalUVDomain ().Evaluate(1,1),sPnt);
            sCtrlPts[3] = sPnt;
            sUKnots[0] = pPlane->GetNaturalUVDomain ().GetMin().x;
            sUKnots[1] = pPlane->GetNaturalUVDomain ().GetMax().x;
            sVKnots[0] = pPlane->GetNaturalUVDomain ().GetMin().y;
            sVKnots[1] = pPlane->GetNaturalUVDomain ().GetMax().y;
            sUKnotMult[0] = 2;    sUKnotMult[1] = 2;
            sVKnotMult[0] = 2;    sVKnotMult[1] = 2;

            nurbsSurface->SetCanonical (lUDeg, lVDeg,
               sCtrlPts, IW_SF_PLANE_SURF, sUKnotMult, sVKnotMult,
               sUKnots, sVKnots, IW_KT_UNSPECIFIED, NULL, NULL);
         }
      }
	  catch (Exception *)
      {
         Console::WriteLine ("Could not get Nurbs surface from plane.");
      }
      return nurbsSurface;
   }

   // IGeometry interface member implementation

 //  void Plane::Transform (PEHoops::MVO::Transformation^  oTransformation)
	//{
 //     try
 //     {
 //        IwPlane *pSurface = (IwPlane *) GetIwObj ();
 //        if (pSurface != NULL && oTransformation != NULL)
 //        {
 //           float matrix __gc[] = oTransformation->Matrix;
 //           IwAxis2Placement crRotateNMove;
 //           crRotateNMove.SetCanonical (
 //              IwPoint3d ((double) matrix[12], (double) matrix[13], (double) matrix[14]), 
 //              IwVector3d ((double) matrix[0], (double) matrix[4], (double) matrix[8]), 
 //              IwVector3d ((double) matrix[1], (double) matrix[5], (double) matrix[9]));
 //           pSurface->Transform (crRotateNMove, NULL);
 //        }
 //     }
 //     catch (System::Exception^ ex)
 //     {
 //        System::Console::WriteLine (ex->Message);
 //     }
	//}

	bool Plane::ComputeBoundingBox (HC::NL_POINT^  ptMin, HC::NL_POINT^  ptMax)
	{
      // TODO
		bool bRet = false;
		return bRet;
	}

	void Plane::Highlight (HC::KEY keySeg)
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

	void Plane::UnHighlight (HC::KEY keySeg)
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

	System::Object^  Plane::GetReferencableObject ()
	{
		return this;
	}

	void Plane::InsertGraphics (bool bDrawDetailed, int handle)
	{
		//try
		//{
		//	if (m_pIwObj != NULL)
		//	{
  //          IwPlane *pPlane = (IwPlane *) m_pIwObj;
  //          if (pPlane)
  //          {
		//		   HC::POINT arrhPts __gc[] = new HC::POINT __gc[4];
  //             IwPoint3d sPnt;

  //             for (int i = 0; i < 2; i++)
  //             {
  //                for (int j = 0; j < 2; j++)
  //                {
  //                   pPlane->EvaluatePointFast(pPlane->GetNaturalUVDomain ().Evaluate(i,(i+j)%2),sPnt);
  //                   arrhPts[2*i + j].x = (float) sPnt.x;
  //                   arrhPts[2*i + j].y = (float) sPnt.y;
  //                   arrhPts[2*i + j].z = (float) sPnt.z;
  //                }
  //             }

  //             HC::Open_Segment_By_Key (keyGeom);
  //             {
  //                // Set Hoops attributes

  //                HC::Set_Marker_Size(0.3);
  //                HC::Set_Marker_Symbol("@");
  //                HC::Set_Color_By_Index("markers", 4);
  //                HC::Set_Color_By_Index("edges",	9);
  //                HC::Set_Color_By_Index("faces",	3);
  //                HC::Set_Color_By_Index("lines",	6);
  //                HC::Set_Selectability("geometry=v");
  //                //HC::Set_Visibility("edges=on");
  //                //HC::Set_Line_Pattern("-	-");
  //                HC::Insert_Polygon (4, arrhPts);
  //             }
  //             HC::Close_Segment ();
  //          }
		//	}
		//}
		//catch (...)
		//{
		//	// TODO
		//}
	}

	//void Plane::RemoveGraphics (KEY keyGeom)
 //  {
 //     // Flush the contents of the segment. It is the callers responsibility to create
 //     // or delete the containing segment.

 //     HC::Open_Segment_By_Key (keyGeom);
 //     HC::Flush_Contents ("...", "everything");
 //     HC::Close_Segment ();
	//}

	void Plane::AddToDOM ()
	{
		if (m_pIwObj != NULL && m_pXMLElem != NULL)
		{
			// Add the persistent ID.
			m_pXMLElem->SetAttribute("idSelf", GetId());
		}
	}

	void Plane::GetFromDOM ()
	{
		if (m_pXMLElem != NULL && m_pIwObj != NULL)
		{
			// Get the persistent GUID.
			String* sId = m_pXMLElem->GetAttribute("idSelf");
			if (sId->Length > 0)
				SetId (sId);
		}
	}
}