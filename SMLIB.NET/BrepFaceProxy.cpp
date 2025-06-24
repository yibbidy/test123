#include "StdAfx.h"
#include "brepfaceproxy.h"
#include "BrepEdgeProxy.h"
#include "brepregionproxy.h"
#using <mscorlib.dll>

namespace PESMLIB
{
   BrepFaceProxy::BrepFaceProxy(void)
   {
      m_lFaceID = -1;
      m_pBrep = 0;
      m_pContext = 0;
      m_dArea = System::Double::MinValue;
   }

   BrepFaceProxy::BrepFaceProxy (PESMLIB::Context __gc *pContext, PESMLIB::Brep __gc *pBrep, long lFaceID)
   {
      m_pContext = pContext;
      m_pBrep = pBrep;
      m_lFaceID = lFaceID;
      m_dArea = System::Double::MinValue;
   }

   BrepFaceProxy::~BrepFaceProxy(void)
   {
   }
	
   void BrepFaceProxy::ComputeProperties (double __gc& dArea, double __gc& dVolume,
      System::Collections::ArrayList __gc *arrMoments)
   {
      try
      {
         IwFace *pIwFace = GetIwFace ();
         if (pIwFace)
         {
            double rdArea, rdVolume;
            IwVector3d ptOrigin (0., 0., 0.);
            IwTArray<IwVector3d> arrVecMoments;

            IwStatus status = pIwFace->ComputePreciseProperties(IW_OT_UNKNOWN, 0.001,
               ptOrigin, 0.0, 0.0, rdArea, rdVolume, arrVecMoments );

            if (status == IW_SUCCESS && rdArea > 0.0)
            {
               dArea = rdArea;
               dVolume = rdVolume;

               // Fill the output moment vector. From the SMLIB documentation, arrVecMoments[0] 
               // contains Ix, Iy, Iz - Area Static moments, arrVecMoments[1] Contains Ixx, Iyy, 
               // Izz - Area Moments of Inertia, arrVecMoments[2] contains Ixy, Iyz, Izx - Area
               // Products of Inertia, arrVecMoments[3] contains Ixx, Iyy, Izz - Area Second 
               // Moment about coordinate axii, arrVecMoments[4] contains Ix, Iy, Iz - Volume 
               // Static Moments, arrVecMoments[5] contains Ixx, Iyy, Izz - Volume Moments of 
               // Inertia, arrVecMoments[6] contains Iyz, Ixz, Ixy - Volume Products of Inertia,
               // arrVecMoments[7] contains Ixx, Iyy, Izz - Volume Second Moment About Coordinate Axii

               for (unsigned int iMom = 0; iMom < arrVecMoments.GetSize (); iMom++)
               {
                  arrMoments->Add (new Vector3d (arrVecMoments[iMom].x, 
                     arrVecMoments[iMom].y, arrVecMoments[iMom].z));
               }
            }
            else // couldn't compute precise properties to compute approximate properties.
            {
               IwPoint3d rDeltaBarycenter;
               IwVector3d aDeltaMoments[6];

               status = pIwFace->ComputeProperties(IW_OT_UNKNOWN, 0.001, 0.001,
                  ptOrigin, rdArea, rdVolume, rDeltaBarycenter, aDeltaMoments);

               if (status == IW_SUCCESS)
               {
                  dArea = rdArea;
                  dVolume = rdVolume;

                  // Fill the output moment vector. From the SMLIB documentation, aDeltaMoments[0]
                  // contains Ixx, Iyy, Izz, aDeltaMoments[1] contains Ixy, Iyz, Izx. To make this
                  // array the same as far as it can be, put the first moment in their as the [0]
                  // element before appending the output computed moments.
 
                  arrMoments->Add (new Vector3d (rDeltaBarycenter.x * rdArea / dVolume,
                     rDeltaBarycenter.y * rdArea / dVolume, rDeltaBarycenter.z * rdArea / dVolume));
                  for (unsigned int iMom = 0; iMom < 2; iMom++)
                     arrMoments->Add (new Vector3d (aDeltaMoments[iMom].x,
                        aDeltaMoments[iMom].y, aDeltaMoments[iMom].z));
               }
            }
         } // if (pIwFace)
		}
      catch (System::Exception __gc *e)
      {
         Console::WriteLine (e->Message);
      }
   }

 	double BrepFaceProxy::GetMyArea()
	{
		double dArea = 0.0, dVolume = 0.0;
		if (m_lFaceID != -1)
      {
		   if (m_dArea == System::Double::MinValue)
		   {
				ArrayList __gc *arrMoments = new ArrayList ();
				ComputeMyProperties (dArea, dVolume, arrMoments);
				m_dArea = dArea;
		   }
		}
      return m_dArea;
	}

	Vector3d * BrepFaceProxy::GetMyCentroid()
	{
		double dArea = 0.0, dVolume = 0.0;
		if (m_lFaceID != -1)
      {
         ArrayList __gc *arrMoments = new ArrayList ();
         ComputeMyProperties (dArea, dVolume, arrMoments);
         if (arrMoments->Count > 0)
         {
            Vector3d __gc *vecMoment = dynamic_cast<Vector3d __gc *> (arrMoments->get_Item (0));
            return new Vector3d (vecMoment->X / dArea, vecMoment->Y / dArea, vecMoment->Z / dArea);
         }
      }
		return NULL;
	}

	Vector3d * BrepFaceProxy::GetNormal()
	{
		Vector3d __gc *v = new Vector3d(0, 0, 0);
		IwFace * pIwFace = GetIwFace();
		if (pIwFace)
		{
			IwVector3d rSurfaceNormal;
			IwExtent2d uvDomain = pIwFace->GetSurface()->GetNaturalUVDomain();
			pIwFace->GetSurface()->EvaluateNormal(IwPoint2d(uvDomain.GetUInterval().GetMid(), uvDomain.GetVInterval().GetMid()), true, true, rSurfaceNormal);
			v->X = rSurfaceNormal.x;
			v->Y = rSurfaceNormal.y;
			v->Z = rSurfaceNormal.z;
		}

		return v;
	}
	
   void BrepFaceProxy::ComputeMyProperties (double __gc& dArea, double __gc& dVolume,
      System::Collections::ArrayList __gc *arrMoments)
   {
      try
      {
         IwFace *pIwFace = GetIwFace ();
         if (pIwFace)
         {
            double rdArea, rdVolume;
            IwVector3d ptOrigin (0., 0., 0.);
            IwTArray<IwVector3d> arrVecMoments;

            {
               IwPoint3d rDeltaBarycenter;
               IwVector3d aDeltaMoments[6];

               IwStatus status = pIwFace->ComputeProperties(IW_OT_UNKNOWN, 0.01, 0.01,
                  ptOrigin, rdArea, rdVolume, rDeltaBarycenter, aDeltaMoments);

               if (status == IW_SUCCESS)
               {
                  dArea = rdArea;
                  dVolume = rdVolume;

                  // Fill the output moment vector. From the SMLIB documentation, aDeltaMoments[0]
                  // contains Ixx, Iyy, Izz, aDeltaMoments[1] contains Ixy, Iyz, Izx. To make this
                  // array the same as far as it can be, put the first moment in their as the [0]
                  // element before appending the output computed moments.
 
                  if (dVolume < 0.0)
				  arrMoments->Add (new Vector3d (-rDeltaBarycenter.x,
                     -rDeltaBarycenter.y, -rDeltaBarycenter.z));
				  else
				  arrMoments->Add (new Vector3d (rDeltaBarycenter.x,
                     rDeltaBarycenter.y, rDeltaBarycenter.z));
                  for (unsigned int iMom = 0; iMom < 2; iMom++)
                     arrMoments->Add (new Vector3d (aDeltaMoments[iMom].x,
                        aDeltaMoments[iMom].y, aDeltaMoments[iMom].z));
               }
            }
         } // if (pIwFace)
		}
      catch (System::Exception __gc *e)
      {
         Console::WriteLine (e->Message);
      }
   }
   
   double BrepFaceProxy::GetArea()
	{
		double dArea = 0.0, dVolume = 0.0;
		if (m_lFaceID != -1)
      {
			ArrayList __gc *arrMoments = new ArrayList ();
			ComputeProperties (dArea, dVolume, arrMoments);
		}
      return dArea;
	}

	double BrepFaceProxy::GetVolume()
	{
		double dArea = 0.0, dVolume = 0.0;
		if (m_lFaceID != -1)
      {
         ArrayList __gc *arrMoments = new ArrayList ();
         ComputeProperties (dArea, dVolume, arrMoments);
      }
      return dVolume;
	}

	Vector3d * BrepFaceProxy::GetCentroid()
	{
		double dArea = 0.0, dVolume = 0.0;
		if (m_lFaceID != -1)
      {
         ArrayList __gc *arrMoments = new ArrayList ();
         ComputeProperties (dArea, dVolume, arrMoments);
         if (arrMoments->Count > 0)
         {
            Vector3d __gc *vecMoment = dynamic_cast<Vector3d __gc *> (arrMoments->get_Item (0));
            return new Vector3d (vecMoment->X / dArea, vecMoment->Y / dArea, vecMoment->Z / dArea);
         }
      }
		return NULL;
	}


	void BrepFaceProxy::InsertGraphics (bool bDrawDetailed, int handle)
   {
   }

 //  void BrepFaceProxy::RemoveGraphics (HC::KEY keyGeom)
 //  {
 //     if (m_pBrep != NULL)
 //        m_pBrep->RemoveFeatureGraphics (keyGeom, Brep::BrepFeatureType::Brep_Face, (HC::KEY) m_lFaceID);
 //  }

   //void BrepFaceProxy::Transform (Transformation * oTransformation)
   //{
   //}

	bool BrepFaceProxy::ComputeBoundingBox (HC::NL_POINT __gc * ptMin, HC::NL_POINT __gc * ptMax)
   {
      try
      {
         IwExtent3d bbox;
         bbox.Init ();
         IwFace *pFace = GetIwFace ();
         if (pFace)
         {
            // And now, a story.......
            // To calculate the bounding box for a face we would expect to use the 
            // CalculateBoundingBox method on the IwFace directly. However, we found
            // that this just calls the CalculateBoundingBox method on the surface
            // associated with the face providing the u-v domain of the face as an
            // input argument. However, we found that the u-v domain of the face 
            // encompasses the entire surface even though geometrically the face is
            // a bounded subset of the surface. Then we decided we could use the
            // CalculateUVDomainFromTrimCurves method of the face to get a more correct
            // u-v domain and pass this directly into the CalculateBoundingBox call on the
            // surface. However, we found in the documentation that this input u-v domain
            // is not currently used by the method. Then we decided to take the approach
            // that the brep uses to compute a bounding box and walk the list of vertices,
            // edges, and optionally faces. We would not walk faces given what was learned
            // above. However, we found that the edges seemed to cause the bounding box
            // to be too big. We are not sure why but either the GetInterval method on the
            // edges returns too large of an interval or the CalculateBoundingBox call on
            // the associated curve is returning too large a value. Therefore, we just walk
            // through the vertices to get the bounding box. This is not guaranteed to return
            // a large enough box since areas of the faces and edges between vertices can go
            // outside the vertices but it is a reasonable first order estimate and the best
            // we currently have.

//            pFace->CalculateBoundingBox (bbox);

//            IwExtent2d domain;
//            pFace->CalculateUVDomainFromTrimCurves (domain);
//            pFace->GetSurface ()->CalculateBoundingBox (domain, &bbox);

            IwTArray<IwVertex*> sVertices;
            pFace->GetVertices(sVertices);
            for (ULONG i=0; i<sVertices.GetSize(); i++) 
            {
               IwVertex *pV = sVertices[i];
               IwExtent3d sVBBox(pV->GetPoint());
               bbox.Union(sVBBox, bbox);
            }
/*
            // Now load the Edge Tree
            IwTArray<IwEdge*> sEdges;
            pFace->GetEdges(sEdges);
            for (ULONG j=0; j<sEdges.GetSize(); j++) 
            {
               IwEdge *pE = sEdges[j];
               IwExtent3d sEBBox;
               SER(pE->GetCurve()->CalculateBoundingBox(pE->GetInterval(),&sEBBox));
               bbox.Union(sEBBox, bbox);
            }
*/
            ptMin->x = (float) bbox.GetMin ().x;
            ptMin->y = (float) bbox.GetMin ().y;
            ptMin->z = (float) bbox.GetMin ().z;
            ptMax->x = (float) bbox.GetMax ().x;
            ptMax->y = (float) bbox.GetMax ().y;
            ptMax->z = (float) bbox.GetMax ().z;

            return true;
         }
      }
      catch (System::Exception __gc *ex)
      {
         Console::WriteLine (ex->Message);
      }
      return false;
   }

   void BrepFaceProxy::Highlight (HC::KEY keySeg)
   {
      if (m_pBrep != NULL)
         m_pBrep->HighlightFeature (keySeg, Brep::BrepFeatureType::Brep_Face, m_lFaceID);
   }

   void BrepFaceProxy::UnHighlight (HC::KEY keySeg)
   {
      if (m_pBrep)
         m_pBrep->UnHighlightFeature (keySeg, Brep::BrepFeatureType::Brep_Face, m_lFaceID);
   }

   System::Object __gc * BrepFaceProxy::GetReferencableObject ()
   {
      return m_pBrep;
   }

   int BrepFaceProxy::CompareTo(System::Object __gc *obj)
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
            BrepFaceProxy *proxy = __try_cast<BrepFaceProxy *> (obj);
            if (proxy)
            {
               if (m_pBrep)
               {
                  int iBrepCompare = m_pBrep->CompareTo (proxy->Brep);

                  if (iBrepCompare == 0)
                  {
                     if (FaceID < proxy->FaceID)
                        return -1;
                     else if (FaceID > proxy->FaceID)
                        return 1;
                     else
                        return 0;
                     //               return FaceID.CompareTo (proxy->FaceID);
                  }
                  else
                     return iBrepCompare;
               }
            }
         }
      }
      catch(System::InvalidCastException*)
      {
         Console::WriteLine("Could not cast object to BrepFaceProxy");
         throw new ArgumentException ("Object is not a BrepFaceProxy");
      }

	  return 1;
   }

   bool BrepFaceProxy::Equals (System::Object __gc * obj)
   {
      try
      {
         BrepFaceProxy *pBrepFaceProxy = __try_cast<BrepFaceProxy *> (obj);
         if (NULL != m_pBrep && NULL != pBrepFaceProxy && m_pBrep->Equals (pBrepFaceProxy->Brep) &&
            m_lFaceID == pBrepFaceProxy->FaceID)
         {
            return true;
         }
      }
      catch(System::InvalidCastException*)
      {
         Console::WriteLine("Could not cast object to BrepFaceProxy");
      }
      return false;
   }

   System::Collections::ArrayList __gc * BrepFaceProxy::GetEdges()
   {

	   System::Collections::ArrayList __gc * listEdgeProxies = new System::Collections::ArrayList();
	   IwFace * pIwFace = GetIwFace ();

      if (NULL != pIwFace)
      {
         IwTArray<IwEdge *> arrEdges;
         pIwFace->GetEdges(arrEdges);
         int nEdges = arrEdges.GetSize();
         for (int i = 0; i < nEdges; i++)
         {
            // Try to find the Edge counter attribute.
            IwAttribute *pAttribute = 0;
            if (NULL != (pAttribute = ((IwAObject *) arrEdges[i])->FindAttribute (AttributeID_BREPEDGECOUNTER)) &&
               pAttribute->GetNumLongElements () > 0)
            {
               const long *lAttributes = pAttribute->GetLongElementsAddress ();
               BrepEdgeProxy *edgeProxy = new BrepEdgeProxy ();
               edgeProxy->Brep = this->Brep;
               edgeProxy->EdgeID = lAttributes[0];
               listEdgeProxies->Add (edgeProxy);
            }
         }
      }
	   return listEdgeProxies;
   }

   IwFace * BrepFaceProxy::GetIwFace ()
   {
	   IwFace *pIwFace = NULL;

	   try
	   {
		   if (NULL != m_pBrep)
            pIwFace = m_pBrep->GetIwFaceFromProxy (this);
	   }
	   catch (Exception *)
	   {
		   Console::WriteLine("Could not get IwFace associated with BrepFaceProxy");
	   }

	   return pIwFace;
   }

   bool BrepFaceProxy::IsDependentOn (IPersistentObject __gc *pObj)
   {
      try
      {
         // First retrieve the Brep region from the owning brep.

         if (NULL != m_pBrep)
         {
            IwFace *pIwFace = GetIwFace ();
            if (NULL != pIwFace)
            {
               IwAttribute *pAttribute = pIwFace->FindAttribute (AttributeID_IDOBJ);
               if (NULL != pAttribute)
               {
                  if (pAttribute->GetNumCharacterElements () > 0)
                  {
                     const char *pcElements = pAttribute->GetCharacterElementsAddress ();
                     System::String __gc *newString = new System::String (pcElements);
                     if (newString->IndexOf (pObj->IdSelf) >= 0)
                        return true;
                  }
               }
            }
         }
      }
      catch (Exception *)
      {
         Console::WriteLine("Could not determine object dependency in BrepFaceProxy");
      }

      return false;
   }

   System::Collections::ArrayList __gc * BrepFaceProxy::GetObjectDependencies ()
   {
      try
      {
         System::Collections::ArrayList __gc *arrDependents = new System::Collections::ArrayList ();

         // First retrieve the Brep region from the owning brep.

         if (NULL != m_pBrep)
         {
            IwFace *pIwFace = GetIwFace ();
            if (NULL != pIwFace)
            {
               IwAttribute *pAttribute = pIwFace->FindAttribute (AttributeID_IDOBJ);
               if (NULL != pAttribute)
               {
                  if (pAttribute->GetNumCharacterElements () > 0)
                  {
                     const char *pcElements = pAttribute->GetCharacterElementsAddress ();
                     System::String __gc *sAttribute = new System::String (pcElements);
                     
                     // Parse the string of ObjIDs and obtain the objects from the brep
                    
                     Char chars[] = {','};
                     sAttribute->Trim (); // remove whitespace
                     sAttribute->Trim (chars); // remove excess begin and end commas.
                     String *sTokens[] = sAttribute->Split (chars);
                     for (int iTok = 0; iTok < sTokens->Count; iTok++)
                     {
						 // Don't allow null objects to be added
						 System::Object __gc *dependentObj = m_pBrep->GetDependency (sTokens[iTok]);
						 if (NULL != dependentObj)
							arrDependents->Add (dependentObj);
                     }
                  }
               }
            }
         }

         return arrDependents;
      }
      catch (Exception *)
      {
         Console::WriteLine("Could not determine object dependency in BrepFaceProxy");
      }

      return NULL;
   }

   void BrepFaceProxy::RemoveObjectDependency (IPersistentObject __gc *pIPersistentObject)
   {
      try
      {
         // First retrieve the Brep face from the owning brep.

         if (NULL != m_pBrep)
         {
            IwFace *pIwFace = GetIwFace ();
            if (NULL != pIwFace)
            {
               // Now try to find an existing attribute with the input object ID.

               IwAttribute *pExistingAttribute = pIwFace->FindAttribute (AttributeID_IDOBJ);
               if (NULL != pExistingAttribute)
               {
                  // If existing attribute type found, get the string and look for this ID and
                  // if found remove it.

                  System::String __gc *sAttribute = new System::String (
                     pExistingAttribute->GetCharacterElementsAddress ());
                  int iObj = sAttribute->IndexOf (pIPersistentObject->IdSelf);
                  if (iObj >= 0) // object found so modify string to remove it
                  {
                     sAttribute->Remove (iObj, pIPersistentObject->IdSelf->Length);
                     pIwFace->RemoveAttribute (pExistingAttribute,TRUE);

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
                        pIwFace->AddAttribute (pAttribute);
                     }

                     // Is it safe to remove the dependency. What if the persistent object is dependent
                     // multiple times for different reasons like a compartment and its faces are now.

                     m_pBrep->RemoveDependency (pIPersistentObject); // this calls AddToDOM so we don't need to
                  }
               }
            }
         }
      }
      catch (Exception *)
      {
         Console::WriteLine("Could not remove object dependency from BrepFaceProxy");
      }
   }

   void BrepFaceProxy::AddObjectDependency (IPersistentObject __gc *pIPersistentObject)
   {
      try
      {
         // First retrieve the Brep face from the owning brep.

         if (NULL != m_pBrep && NULL != m_pContext)
         {
            IwFace *pIwFace = GetIwFace ();
            if (NULL != pIwFace)
            {
               System::String __gc *sNewAttribute = System::String::Copy (pIPersistentObject->IdSelf);

               IwAttribute *pExistingAttribute = pIwFace->FindAttribute (AttributeID_IDOBJ);
               if (NULL != pExistingAttribute)
               {
                  // If existing attribute found, get the string and look for this ID. 
                  // Don't add the ID again if it already exists.

                  System::String __gc *sAttribute = new System::String (
                     pExistingAttribute->GetCharacterElementsAddress ());
                  int iObj = sAttribute->IndexOf (pIPersistentObject->IdSelf);
                  if (iObj < 0) // object not found so add it
                  {
                     sNewAttribute = System::String::Format ("{0},{1}", sAttribute, pIPersistentObject->IdSelf);
                     pIwFace->RemoveAttribute (pExistingAttribute, TRUE);
                  }
                  else // object found so don't do anything
                     sNewAttribute = System::String::Empty;
               }

               IwTArray<long> arrLongEl;
               IwTArray<double> arrDoubleEl;
               IwTArray<char> arrCharEl;

               int lSize = sNewAttribute->Length;
               if (lSize > 0)
               {
                  for (int iString = 0; iString < lSize; iString++)
                     arrCharEl.Add(Convert::ToByte(sNewAttribute->Chars[iString]));

                  IwGenericAttribute *pAttribute = new (m_pContext->GetIwContext()) IwGenericAttribute (
                     AttributeID_IDOBJ, IW_AB_COPY, arrLongEl, arrDoubleEl, arrCharEl);
                  pIwFace->AddAttribute (pAttribute);
                  m_pBrep->AddDependency (pIPersistentObject); 
                  // The AddDependency call above calls AddToDOM if it is successful. However,
                  // if the dependency already exists it does not call AddToDOM because it 
                  // figures the DOM is already up to date so we call it here.
                  m_pBrep->AddToDOM ();
               }
            }
         }
      }
      catch (Exception *)
      {
         Console::WriteLine("Could not set object attribute on BrepFaceProxy");
      }
   }

   void BrepFaceProxy::SetAttribute (AttributeID ulAttributeID, System::Object __gc *oAttrib, 
      AttributeBehavior behavior)
   {
      try
      {
         if (NULL != m_pBrep)
         {
            // First retrieve the Brep face from the owning brep.

            IwFace *pIwFace = GetIwFace ();

            if (NULL != pIwFace)
            {
               // Remove any existing attribute of this type.

               IwAttribute *pExistingAttribute = pIwFace->FindAttribute ((ULONG) ulAttributeID);
               if (NULL != pExistingAttribute)
                  pIwFace->RemoveAttribute (pExistingAttribute, TRUE);

               System::String *sType = oAttrib->GetType ()->FullName;
               if (sType->Equals (S"System.Int32"))
               {
                  System::Int32 *plValue = dynamic_cast<System::Int32 *> (oAttrib);
                  if (NULL != plValue)
                  {
                     IwLongAttribute *pAttribute = new (m_pContext->GetIwContext()) IwLongAttribute (
                        (ULONG) ulAttributeID, *plValue, (IwAttributeBehaviorType) behavior);
                     pIwFace->AddAttribute (pAttribute);
                     m_pBrep->AddToDOM (); // remember to update the persistent image if we add an attribute
                  }
               }
               else if (sType->Equals (S"System.String"))
               {
                  System::String *sValue = dynamic_cast<System::String *> (oAttrib);
                  if (NULL != sValue)
                  {
                     IwTArray<long> arrLongEl;
                     IwTArray<double> arrDoubleEl;
                     IwTArray<char> arrCharEl;

                     int lSize = sValue->Length;
                     if (lSize > 0)
                     {
                        for (int iString = 0; iString < lSize; iString++)
                           arrCharEl.Add(Convert::ToByte(sValue->Chars[iString]));

                        IwGenericAttribute *pAttribute = new (m_pContext->GetIwContext()) IwGenericAttribute (
                           (ULONG) ulAttributeID, (IwAttributeBehaviorType) behavior, arrLongEl, arrDoubleEl, arrCharEl);
                        pIwFace->AddAttribute (pAttribute);
                        m_pBrep->AddToDOM (); // remember to update the persistent image if we add an attribute
                     }
                  }
               }
               else if (sType->Equals (S"System.Double"))
               {
                  System::Double *pdValue = dynamic_cast<System::Double *> (oAttrib);
                  if (NULL != pdValue)
                  {
                     IwTArray<long> arrLongEl;
                     IwTArray<double> arrDoubleEl;
                     IwTArray<char> arrCharEl;

                     arrDoubleEl.Add (*pdValue);

                     IwGenericAttribute *pAttribute = new (m_pContext->GetIwContext()) IwGenericAttribute (
                        (ULONG) ulAttributeID, (IwAttributeBehaviorType) behavior, arrLongEl, arrDoubleEl, arrCharEl);
                     pIwFace->AddAttribute (pAttribute);
                     m_pBrep->AddToDOM (); // remember to update the persistent image if we add an attribute
                  }
               }
            } // if (NULL != pIwFace)
         }
      }
      catch (Exception *)
      {
         Console::WriteLine("Could not set attribute on BrepFaceProxy");
      }
   }

   System::Object __gc * BrepFaceProxy::FindAttribute (AttributeID ulAttributeID)
   {
      try
      {
         // First retrieve the Brep face from the owning brep.

         if (NULL != m_pBrep)
         {
            IwFace *pIwFace = GetIwFace ();
            if (NULL != pIwFace)
            {
               IwAttribute *pAttribute = pIwFace->FindAttribute ((ULONG) ulAttributeID);
               if (NULL != pAttribute)
               {
                  if (pAttribute->GetNumLongElements () > 0)
                  {
                     System::Int32 __gc *newLong = new System::Int32();
                     const long *plElements = pAttribute->GetLongElementsAddress ();
                     *newLong = plElements[0];
                     return __box(*newLong);
                  }
                  else if (pAttribute->GetNumCharacterElements () > 0)
                  {
                     const char *pcElements = pAttribute->GetCharacterElementsAddress ();
                     System::String __gc *newString = new System::String (pcElements);
                     return newString;
                  }
                  else if (pAttribute->GetNumDoubleElements () > 0)
                  {
                     const double *pdElements = pAttribute->GetDoubleElementsAddress ();
                     System::Double __gc *newDouble = new System::Double ();
                     *newDouble = pdElements[0];
                     return __box (*newDouble);
                  }
                  else
                     return NULL;
               }
            }
         }
      }
      catch (Exception *)
      {
         Console::WriteLine("Could not find attribute in BrepFaceProxy");
      }

      return NULL;
   }

   void BrepFaceProxy::RemoveAttribute (AttributeID ulAttributeID)
   {
      try
      {
         // First retrieve the Brep face from the owning brep.

         if (NULL != m_pBrep)
         {
            IwFace *pIwFace = GetIwFace ();
            if (NULL != pIwFace)
            {
               IwAttribute *pAttribute = pIwFace->FindAttribute ((ULONG) ulAttributeID);
               if (NULL != pAttribute)
                  pIwFace->RemoveAttribute (pAttribute, TRUE);
            }
         }
      }
      catch (Exception *)
      {
         Console::WriteLine("Could not remove attribute from BrepFaceProxy");
      }

      return;
   }
/*
               // The below logic is a big no, no because the newly created surface
               // is taking ownership of the IwSurface but the brep really owns that.
               // We need a surface proxy object that mimics the face proxy logic. It
               // should not destroy the IwSurface object in its destructor and should
               // be tied to the surface through a surfaceId attribute.

   System::Object __gc * BrepFaceProxy::GetSurface ()
   {
      try
      {
         IwFace *pIwFace = GetIwFace ();
         if (pIwFace != NULL)
         {
            IwSurface *pSurface = pIwFace->GetSurface ();
            if (NULL != pSurface)
            {
               if (pSurface->IsKindOf (IwPlane_TYPE))
               {
                  Plane __gc *newPlane = new Plane (m_pContext, NULL);
                  newPlane->AttachIwObj (m_pContext, pSurface);
                  return newPlane;
               }
               else if (pSurface->IsKindOf (IwBSplineSurface_TYPE))
               {
                  NurbsSurface __gc *newSurface = new NurbsSurface (m_pContext, NULL);
                  newSurface->AttachIwObj (m_pContext, pSurface);
                  return newSurface;
               }
            }
         }
      }
      catch (...)
      {
         Console::WriteLine ("Could not get surface from face");
      }
      return NULL;
   }
*/
   System::Object __gc * BrepFaceProxy::GetSurfaceCopy ()
   {
      try
      {
         IwFace *pIwFace = GetIwFace ();
         if (pIwFace != NULL)
         {
            IwSurface *pSurface = pIwFace->GetSurface ();
            if (NULL != pSurface)
            {
               if (pSurface->IsKindOf (IwPlane_TYPE))
               {
                  Plane __gc *newPlane = new Plane (m_pContext, NULL);
                  IwPlane *pPlane = NULL;
                  pSurface->Copy (m_pContext->GetIwContext (), (IwSurface *&) pPlane);
                  newPlane->AttachIwObj (m_pContext, pPlane);
                  return newPlane;
               }
               else if (pSurface->IsKindOf (IwBSplineSurface_TYPE))
               {
                  NurbsSurface __gc *newSurface = new NurbsSurface (m_pContext, NULL);
                  IwBSplineSurface *pNurbsSurface = NULL;
                  pSurface->Copy (m_pContext->GetIwContext (), (IwSurface *&) pNurbsSurface);
                  newSurface->AttachIwObj (m_pContext, pNurbsSurface);
                  return newSurface;
               }
            }
         }
      }
	  catch (Exception *)
      {
         Console::WriteLine ("Could not get surface from face");
      }
      return NULL;
   }

   bool BrepFaceProxy::HasSameSurface (BrepFaceProxy __gc *srcFace)
   {
      try
      {
         IwFace *pIwFace = GetIwFace ();
         IwFace *pIwSrcFace = srcFace->GetIwFace ();

         if (pIwFace != NULL && pIwSrcFace != NULL)
         {
            IwSurface *pSurface = pIwFace->GetSurface ();
            IwSurface *pSrcSurface = pIwSrcFace->GetSurface ();

            if (NULL != pSurface && NULL != pSrcSurface && pSurface == pSrcSurface)
               return true;
         }
      }
	  catch (Exception *)
      {
         Console::WriteLine ("Could not get surface from face");
      }
      return false;
   }

   System::Collections::ArrayList __gc * BrepFaceProxy::GetRegions ()
   {
      try
      {
         System::Collections::ArrayList __gc *arrRegions = new System::Collections::ArrayList ();
         IwFaceuse *pFaceuse[2];
         IwFace *pFace = GetIwFace ();
         if (pFace)
         {
            pFace->GetFaceuses (pFaceuse[0], pFaceuse[1]);
            for (int iUse = 0; iUse < 2; iUse++)
            {
               if (pFaceuse[iUse])
               {
                  IwShell *pShell = pFaceuse[iUse]->GetShell ();
                  if (pShell)
                  {
                     IwRegion *pRegion = pShell->GetRegion ();
                     if (pRegion && pRegion->GetBrep ()->GetInfiniteRegion () != pRegion)
                     {
                        IwAttribute *pRegionAttribute = pRegion->FindAttribute (AttributeID_BREPREGIONCOUNTER);
                        if (pRegionAttribute && pRegionAttribute->GetNumLongElements () > 0)
                        {
                           const long *lRegionIDs = pRegionAttribute->GetLongElementsAddress ();
                           BrepRegionProxy __gc *regionProxy = new BrepRegionProxy (m_pContext, m_pBrep, lRegionIDs[0]);
                           arrRegions->Add (regionProxy);
                        }
                     }
                  }
               }
            }
         }

         return arrRegions;
      }
	  catch (Exception *)
      {
         Console::WriteLine ("Could not get regions from BrepFaceProxy");
      }
      return NULL;
   }
	void BrepFaceProxy::AssignPropertyAttribute (String __gc *sName)
	{
		IwFace * pIwFace=GetIwFace ();
		if (pIwFace)
		{
			IwAttribute *pExistingAttribute = pIwFace->FindAttribute (AttributeID_PROPERTY);
			if (NULL != pExistingAttribute)
			{
				pIwFace->RemoveAttribute (pExistingAttribute, TRUE);
				pExistingAttribute = NULL;
			}

			if (NULL == pExistingAttribute)
			{
				IwTArray<char> iwName;
				int iLength=sName->get_Length();
				for (int i=0;i<iLength;i++)
				{
					iwName.Add((char)sName->get_Chars(i));
				}
				IwGenericAttribute *pAttribute = new (get_Brep()->GetContext()->GetIwContext ()) IwGenericAttribute(
					AttributeID_PROPERTY, IW_AB_COPY,NULL,NULL,iwName);

				pIwFace->AddAttribute (pAttribute);
			}
		}
	}

	Vector3d __gc * BrepFaceProxy::DropPoint(Vector3d __gc *ptToDrop, Vector3d __gc *vecNormal)
	{
		IwFace * pIwFace = this->GetIwFace();
		IwSurface * pIwSurface = pIwFace->GetSurface();
		IwSolutionArray sSolutions;
		IwExtent2d uvDomain;
		pIwFace->CalculateUVDomainFromTrimCurves(uvDomain);
		IwVector3d vecToDrop = *(ptToDrop->GetIwObj());
		pIwSurface->GlobalPointSolve(uvDomain, IW_SO_MINIMIZE, vecToDrop, 1.0e-3, NULL, IW_SR_SINGLE, sSolutions);
		if (sSolutions.GetSize() > 0)
		{
			IwSolution & rSolution = sSolutions[0];
			IwPoint3d sFndPnt;
			IwVector3d sNormal;
			// Note that curve is first parameter in the solution
			IwPoint2d sUVFound(rSolution.m_vStart[0],rSolution.m_vStart[1]);
			int status = pIwSurface->EvaluatePoint(sUVFound,sFndPnt);
			if (status != IW_SUCCESS)
				throw new Exception("Error occurred while dropping point to surface.");
			status = pIwSurface->EvaluateNormal(sUVFound, false, false, sNormal);
			if (status != IW_SUCCESS)
				throw new Exception("Error occurred while evaluating surface normal of dropped point");
			vecNormal->SetCanonical(sNormal.x, sNormal.y, sNormal.z);
			return new Vector3d(sFndPnt.x, sFndPnt.y, sFndPnt.z);
		}
		else
			// If no solution is found, return copy of original point.
			return new Vector3d(ptToDrop->X, ptToDrop->Y, ptToDrop->Z);
	}

	Vector3d __gc * BrepFaceProxy::DropPointAlongLine(Vector3d __gc *ptToDrop, Vector3d __gc *vecDropDirection, Vector3d __gc *vecNormal)
	{
		IwFace * pIwFace = this->GetIwFace();
		IwSurface * pIwSurface = pIwFace->GetSurface();
		IwSolutionArray sSolutions;
		IwExtent2d uvDomain;
		pIwFace->CalculateUVDomainFromTrimCurves(uvDomain);
		IwVector3d vecToDrop = *(ptToDrop->GetIwObj());
		IwVector3d vecDirection = *(vecDropDirection->GetIwObj());

		pIwSurface->GlobalLineIntersect(uvDomain, vecToDrop, vecDirection, NULL, 0, 1.0e-1, sSolutions);
		if (sSolutions.GetSize() > 0)
		{
			IwSolution & rSolution = sSolutions[0];
			IwPoint3d sFndPnt;
			IwVector3d sNormal;
			// Note that curve is first parameter in the solution
			IwPoint2d sUVFound(rSolution.m_vStart[1],rSolution.m_vStart[2]);
			int status = pIwSurface->EvaluatePoint(sUVFound,sFndPnt);
			if (status != IW_SUCCESS)
				throw new Exception("Error occurred while dropping point to surface along line.");
			status = pIwSurface->EvaluateNormal(sUVFound, 0, 0, sNormal);
			if (status != IW_SUCCESS)
				throw new Exception("Error occurred while evaluating surface normal of dropped point");
			vecNormal->SetCanonical(sNormal.x, sNormal.y, sNormal.z);
			return new Vector3d(sFndPnt.x, sFndPnt.y, sFndPnt.z);
		}
		else
			// If no solution is found, return copy of original point.
			//return new Vector3d(ptToDrop->X, ptToDrop->Y, ptToDrop->Z);
			return new Vector3d(0, 0, 0);
	}
	double BrepFaceProxy::getVertexMin()
	{
			double dMin = 1000000.;
			IwFace * pIwFace=GetIwFace ();
			if (pIwFace)
			{
				IwTArray<IwVertex *> arrVertices;
				pIwFace->GetVertices (arrVertices);
				for (ULONG iVertex = 0; iVertex < arrVertices.GetSize (); iVertex++)
				{
					IwPoint3d sPoint =  arrVertices[iVertex]->GetPoint();
					dMin = min (dMin, sPoint.z);
				}
			}
			return dMin;
	}

	double BrepFaceProxy::getVertexMax()
	{
			double dMax = -1000000.;
			IwFace * pIwFace=GetIwFace ();
			if (pIwFace)
			{
				IwTArray<IwVertex *> arrVertices;
				pIwFace->GetVertices (arrVertices);
				for (ULONG iVertex = 0; iVertex < arrVertices.GetSize (); iVertex++)
				{
					IwPoint3d sPoint =  arrVertices[iVertex]->GetPoint();
					dMax = max (dMax, sPoint.z);
				}
			}
			return dMax;
	}}
