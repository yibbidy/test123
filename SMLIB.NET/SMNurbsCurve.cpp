#include "StdAfx.h"
#include "smnurbscurve.h"
#include "smvector3d.h"

namespace PESMLIB
{
	NurbsCurve::NurbsCurve() : SMObject ()
	{
		//m_HoopsKey = (HC::KEY)(GCHandle::op_Explicit(GCHandle::Alloc(this)).ToInt32() >> 1);
	}

	NurbsCurve::NurbsCurve (Context *oContext, XML::XmlElement *pElem) : SMObject ()
	{
		try
		{
			//m_HoopsKey = (HC::KEY)(GCHandle::op_Explicit(GCHandle::Alloc(this)).ToInt32() >> 1);
			m_pContext = oContext;
			if (HasIwContext())
			{
				m_pIwObj = (IwObject *) new (GetIwContext()) IwBSplineCurve;
				m_pXMLElem = pElem;

				// Now populate the Brep via the DOM if there is anything in the DOM.
				// Otherwise it is just the node where we are supposed to store the 
				// state later.

				if (m_pXMLElem != NULL)
				{
					GetFromDOM ();
					CreateId ();
				}
			}
			else
			{
				m_pIwObj = NULL;
				m_pXMLElem = NULL;
			}
		}
		catch (...)
		{
			// TODO
		}
	}

	NurbsCurve::~NurbsCurve()
	{
		//if (0 != m_HoopsKey)
		//{ 
		//	GCHandle::op_Explicit((IntPtr)(m_HoopsKey << 1)).Free();
		//	m_HoopsKey = (HC::KEY)0;
		//}
	}

	int NurbsCurve::CompareTo (System::Object __gc *obj)
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
				NurbsCurve *pNurbsCurve = __try_cast<NurbsCurve *> (obj);
				if (NULL != pNurbsCurve)
				{
				   if (NULL != m_pXMLElem)
						   return IdSelf->CompareTo (pNurbsCurve->IdSelf);
				   else if (m_pIwObj == pNurbsCurve->GetIwObj ())
					  return 0;
				   else if (m_pIwObj < pNurbsCurve->GetIwObj ())
					  return -1;
				   else
					  return 1;
				}   
			}
		}
		catch (System::InvalidCastException*)
		{
			Console::WriteLine ("Could not cast object to NurbsCurve.");
			throw new System::ArgumentException ("Object is not a NurbsCurve.");
		}

		return 1;
	}

	bool NurbsCurve::Equals (System::Object __gc * obj)
	{
		try
		{
         // If this is a persistent brep, compare id's, otherwise compare underlying geometry.

			NurbsCurve *pNurbsCurve = __try_cast<NurbsCurve *> (obj);
			if (NULL != pNurbsCurve)
			{
            if (NULL != m_pXMLElem)
            {
				   if (pNurbsCurve->get_IdSelf () == get_IdSelf () && get_IdSelf () != NULL)
					   return true;
            }
            else if (m_pIwObj == pNurbsCurve->GetIwObj ())
               return true;
			}
		}
		catch(System::InvalidCastException*)
		{
			Console::WriteLine("Could not cast object to NurbsCurve");
		}
		return false;
	}

	void NurbsCurve::AttachIwObj (Context *pContext, IwObject *pIwObj)
	{
		try
		{
			// Check to make sure object type is appropriate

			if (pIwObj && pIwObj->IsKindOf (IwBSplineCurve_TYPE))
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
			// TODO
		}
	}

   void NurbsCurve::Tessellate(Extent1d __gc *extInterval, double dChordHeightTolerance, 
      double dAngleToleranceDeg, ULONG lMinimumNumberOfSegments, 
      System::Collections::ArrayList __gc *parameters, System::Collections::ArrayList __gc * points)
   {
      try
      {
         if (parameters != NULL)
            parameters->Clear ();
         if (points != NULL)
            points->Clear ();

         if (m_pIwObj != NULL)
         {
            IwTArray<double> iwparameters;
            IwTArray<IwPoint3d> iwpoints;
            IwExtent1d interval(extInterval->Min, extInterval->Max);

            if (((IwBSplineCurve *) m_pIwObj)->Tessellate (interval, dChordHeightTolerance,
               dAngleToleranceDeg, lMinimumNumberOfSegments, parameters ? &iwparameters : NULL,
               points ? &iwpoints : NULL) == IW_SUCCESS)
            {
               for (int iParam = 0; iParam < (int) iwparameters.GetSize (); iParam++)
                  parameters->Add (__box(iwparameters[iParam]));
               for (int iPt = 0; iPt < (int) iwpoints.GetSize (); iPt++)
                  points->Add (new PESMLIB::Vector3d (iwpoints[iPt].x, iwpoints[iPt].y, iwpoints[iPt].z));
            }
         }
      }
	  catch (Exception *)
      {
         // TODO
      }
      return;
   }

   void NurbsCurve::ReverseParameterization ()
   {
	   try
	   {
		   if (m_pIwObj)
		   {
			   IwExtent1d newExtent;
			   ((IwBSplineCurve *) m_pIwObj)->ReverseParameterization (((IwBSplineCurve *) m_pIwObj)->GetNaturalInterval (), newExtent);
		   }
	   }
	   catch (Exception *)
	   {
	   }
	   return;
   }

   Extent1d __gc * NurbsCurve::GetNaturalInterval ()
   {
      try
      {
         if (m_pIwObj != NULL)
         {
            IwExtent1d iwExtent = ((IwBSplineCurve *) m_pIwObj)->GetNaturalInterval ();
            Extent1d __gc *extent = new Extent1d (iwExtent.GetMin (), iwExtent.GetMax ());
            return extent;
         }
      }
	  catch (Exception *)
      {
      }
      return NULL;
   }

	void NurbsCurve::Draw ()
	{
		try
		{
			if (m_pIwObj != NULL)
			{
				// Flush any current geometry in the segment

				//if (HC::Show_Existence_By_Key (m_HoopsKey, "self"))
				//	HC::Flush_By_Key (m_HoopsKey);

				// Get Hoops Nurbs curve input.

				ULONG ulDimension, ulDegree;
				IwTArray<IwVector3d> arrCtrlPts;
				IwBSplineCurveForm oCurveForm;
				IwTArray<ULONG> arrMultiplicities;
				IwTArray<double> arrKnots, arrWeights;
				IwKnotType oKnotType;

				if (IW_SUCCESS != ((IwBSplineCurve *) m_pIwObj)->GetCanonical (
					ulDimension, ulDegree, arrCtrlPts, oCurveForm, arrMultiplicities, 
					arrKnots, oKnotType, arrWeights))
				{
					//TODO
				}

				//HC::POINT fCntrlPts __gc[] = 0;
				float fWeights __gc[] = 0;
				float fKnots __gc[] = 0;
				int iNumPts = arrCtrlPts.GetSize ();
				//if (iNumPts > 0)
				//{
				//	fCntrlPts = new HC::POINT[iNumPts];
				//	for (long lPt = 0; lPt < iNumPts; lPt++)
				//	{
				//		fCntrlPts[lPt].x = (float) arrCtrlPts[lPt].x;
				//		fCntrlPts[lPt].y = (float) arrCtrlPts[lPt].y;
				//		fCntrlPts[lPt].z = (float) arrCtrlPts[lPt].z;
				//	}

				//	if (iNumPts == arrWeights.GetSize ())
				//	{
				//		fWeights = new float __gc[iNumPts];
				//		for (long lWeight = 0; lWeight < iNumPts; lWeight++)
				//			fWeights[lWeight] = (float) arrWeights[lWeight];
				//	}

				//	int nKnots = ((IwBSplineCurve *) m_pIwObj)->GetNumberNaturalKnots ();
				//	int iNumKnots = arrKnots.GetSize ();
				//	fKnots = new float __gc[nKnots];
				//	for (long lKnot = 0, lCnt = 0; lKnot < iNumKnots; lKnot++)
				//	{
				//		for (ULONG lMult = 0; lMult < arrMultiplicities[lKnot]; lMult++)
				//			fKnots[lCnt++] = (float) arrKnots[lKnot];
				//	}
				//}

				// This method will output triangles using sPolyOutput class

				//if (HC::Show_Existence_By_Key (m_HoopsKey, "self"))
				//{
				//	HC::Open_Segment_By_Key (m_HoopsKey);
				//	{
				//		HC::Insert_NURBS_Curve ((int) ulDegree, iNumPts, fCntrlPts, fWeights, fKnots, 0., 1.);
				//	}
				//	HC::Close_Segment ();
				//}
				//else
				//{
				//	HC::KEY hKey = HC::KOpen_Segment ("");
				//	{
				//		HC::Insert_NURBS_Curve ((int) ulDegree, iNumPts, fCntrlPts, fWeights, fKnots, 0., 1.);
				//	}
				//	HC::Close_Segment ();
				//	HC::Renumber_Key (hKey, m_HoopsKey, "global");
				//}
				/*
				if (fWeights)
				delete [] fWeights;
				if (fCntrlPts)
				delete [] fCntrlPts;
				if (fKnots)
				delete [] fKnots;*/
			}
		}
		catch (...)
		{
			// TODO
		}
	}

	void NurbsCurve::Undraw ()
	{
		return;
	}

	void NurbsCurve::AddToDOM ()
	{
		try
		{
			if (m_pIwObj != NULL && m_pXMLElem != NULL)
			{
				// Add the persistent ID.

				m_pXMLElem->SetAttribute ("idSelf", GetId ());
			}
		}
		catch (...)
		{
			// TODO
		}
	}

	void NurbsCurve::GetFromDOM ()
	{
		try
		{
			if (m_pXMLElem != NULL && m_pIwObj != NULL && m_pContext != NULL)
			{
				// Get the persistent GUID.

				String *sId = m_pXMLElem->GetAttribute ("idSelf");
				if (sId->Length > 0)
					SetId (sId);
			}
		}
		catch (...)
		{
			// TODO
		}
	}
}