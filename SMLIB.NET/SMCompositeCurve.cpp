#include "StdAfx.h"
#include "smcompositecurve.h"
#include "smvector3d.h"

namespace PESMLIB
{
	CompositeCurve::CompositeCurve() : SMObject ()
	{
      m_pIwObj = 0;
	}
/*
	CompositeCurve::CompositeCurve (Context *oContext, XML::XmlElement *pElem) : SMObject ()
	{
		try
		{
			m_pContext = oContext;
			if (HasIwContext())
			{
				m_pIwObj = (IwObject *) new (GetIwContext()) IwCompositeCurve;
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
*/
	CompositeCurve::~CompositeCurve()
	{
	}

	void CompositeCurve::AttachIwObj (Context *pContext, IwObject *pIwObj)
	{
		try
		{
			// Check to make sure object type is appropriate

			if (pIwObj && pIwObj->IsKindOf (IwCompositeCurve_TYPE))
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

   void CompositeCurve::Tessellate(Extent1d^ extInterval, double dChordHeightTolerance, 
      double dAngleToleranceDeg, ULONG lMinimumNumberOfSegments, 
      System::Collections::ArrayList^ parameters, System::Collections::ArrayList^  points)
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

            if (((IwCompositeCurve *) m_pIwObj)->Tessellate (interval, dChordHeightTolerance,
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

   Extent1d^  CompositeCurve::GetNaturalInterval ()
   {
      try
      {
         if (m_pIwObj != NULL)
         {
            IwExtent1d iwExtent = ((IwCompositeCurve *) m_pIwObj)->GetNaturalInterval ();
            Extent1d^ extent = new Extent1d (iwExtent.GetMin (), iwExtent.GetMax ());
            return extent;
         }
      }
	  catch (Exception *)
      {
      }
      return NULL;
   }

	void CompositeCurve::AddToDOM ()
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

	void CompositeCurve::GetFromDOM ()
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

   long CompositeCurve::GetNumSegments ()
   {
      try
      {
         if (m_pIwObj)
         {
            return ((IwCompositeCurve *) m_pIwObj)->GetNumSegments ();
         }
      }
	  catch (Exception *)
      {
      }
      return 0;
   }
      
   void CompositeCurve::ReverseParameterization ()
   {
	   // The SMLIB routine does nothing for a composite curve and returns IW_ERR. We
	   // just walk through each curve in the composite reversing the parameterization.
	   try
	   {
		   if (m_pIwObj)
		   {
			   for (ULONG iSeg = 0; iSeg < ((IwCompositeCurve *) m_pIwObj)->GetNumSegments (); iSeg++)
			   {
				   IwCurve *pCurve = 0;
				   const IwCompositeCurveSegment *pSeg = ((IwCompositeCurve *) m_pIwObj)->GetCurveSegment (iSeg);
				   if (pSeg != NULL && (pCurve = pSeg->m_pParentCurve) != NULL)
				   {
					   IwExtent1d newExtent;
					   pCurve->ReverseParameterization (pCurve->GetNaturalInterval (), newExtent);
				   }
			   }
		   }
	   }
	   catch (Exception *)
	   {
	   }
	   return;
   }

   NurbsCurve^  CompositeCurve::GetCurveSegment (int iSeg)
   {
      try
      {
         if (m_pIwObj)
         {
            const IwCompositeCurveSegment *pSeg = ((IwCompositeCurve *) m_pIwObj)->GetCurveSegment (iSeg);
            if (pSeg)
            {
               IwCurve *pCurve = pSeg->m_pParentCurve;

               // Make a copy of the curve since it will be attached to a new NurbsCurve object.
               // Use the Copy method as opposed to the copy constructor because the former obeys
               // polymorphism so the resultant curve is of the same type as the original whereas
               // the latter appears not to.
               
               IwCurve *pNewIwCurve = 0;
               pCurve->Copy (*pCurve->GetContext (), pNewIwCurve);

               // Flip the direction of the curve if necessary.

               if (!pSeg->m_bSameSense)
               {
                  IwExtent1d newExtent;
                  pNewIwCurve->ReverseParameterization (pNewIwCurve->GetNaturalInterval (), newExtent);
               }
               NurbsCurve^ pNewCurve = new NurbsCurve ();
               pNewCurve->AttachIwObj (m_pContext, pNewIwCurve);
               return pNewCurve;
            }
         }
      }
	  catch (Exception *)
      {
      }
      return NULL;
   }

   void CompositeCurve::BuildCompositesFromCurves (Context *pContext, XML::XmlElement *pXMLElem, 
         System::Collections::ArrayList^ arrNurbsCurves, bool bMakeHomogeneous,
         double dSamePtTol, double dDistToCreateLine, System::Collections::ArrayList^ arrComposites)
   {
      try
      {
         if (NULL == arrNurbsCurves || NULL == arrComposites || NULL == pContext)
            return;

         // According to the SMLIB 6.5 documentation, "At the current time this method is 
         // still in development. You must specify a non-zero dSamePointTolerance. You may
         // specify a non-zero dDistanceToCreateLine if you want to put a small line segment 
         // to fill gaps which are bigger than the dSamePointTolerance. All other tolerances
         // should be 0.0."

         IwTArray<IwBSplineCurve *> arrIwCurves;
         for (int iCurve = 0; iCurve < arrNurbsCurves->Count; iCurve++)
         {
			   NurbsCurve *pCurve = __try_cast<NurbsCurve *> (arrNurbsCurves->get_Item (iCurve));
            if (pCurve != NULL)
               arrIwCurves.Add ((IwBSplineCurve *) pCurve->ExtractIwObj ()); // input curves are consumed.
         }
         IwTArray<IwCompositeCurve *> arrIwCompCurves;
         IwStatus status = IwCompositeCurve::BuildCompositesFromCurves (pContext->GetIwContext (), arrIwCurves,
            bMakeHomogeneous, 0.0, dSamePtTol, 0.0, 0.0, dDistToCreateLine, 0.0, arrIwCompCurves);

         if (status == IW_SUCCESS)
         {
            for (int iCurve = 0; iCurve < (int) arrIwCompCurves.GetSize (); iCurve++)
            {
               CompositeCurve^ newCurve = new CompositeCurve ();
               newCurve->AttachIwObj (pContext, arrIwCompCurves[iCurve]);
               arrComposites->Add (newCurve);
            }
         }
      }
	  catch (Exception *)
      {
      }
      return;
   }
}