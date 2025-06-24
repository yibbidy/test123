#include "StdAfx.h"
#include "brepregionproxy.h"
#include "smbrep.h"
#include "brepfaceproxy.h"
#using <mscorlib.dll>
using namespace PESMLIB;

namespace PESMLIB
{
   BrepRegionProxy::BrepRegionProxy(void)
   {
      m_lRegionID = -1; // this signifies the infinite region
      m_pBrep = 0;
      m_pContext = 0;
      m_dVolume = System::Double::MinValue;
      m_dArea = System::Double::MinValue;
   }

   BrepRegionProxy::BrepRegionProxy (PESMLIB::Context __gc *pContext, PESMLIB::Brep __gc *pBrep, long lRegionID)
   {
      m_pContext = pContext;
      m_pBrep = pBrep;
      m_lRegionID = lRegionID;
      m_dVolume = System::Double::MinValue;
      m_dArea = System::Double::MinValue;
   }

   BrepRegionProxy::~BrepRegionProxy(void)
   {
   }
      
   bool BrepRegionProxy::Equals (System::Object __gc * obj)
   {
      try
      {
         BrepRegionProxy *pBrepRegionProxy = __try_cast<BrepRegionProxy *> (obj);
         if (NULL != m_pBrep && NULL != pBrepRegionProxy && m_pBrep->Equals (pBrepRegionProxy->Brep) &&
            m_lRegionID == pBrepRegionProxy->RegionID)
         {
            return true;
         }
      }
      catch(System::InvalidCastException*)
      {
         Console::WriteLine("Could not cast object to BrepRegionProxy");
      }
      return false;
   }

   // IComparable interface
   int BrepRegionProxy::CompareTo(System::Object __gc *obj)
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
            BrepRegionProxy *proxy = __try_cast<BrepRegionProxy *> (obj);
            if (proxy)
            {
               if (m_pBrep)
               {
                  int iBrepCompare = m_pBrep->CompareTo (proxy->Brep);

                  if (iBrepCompare == 0)
                  {
                     if (RegionID < proxy->RegionID)
                        return -1;
                     else if (RegionID > proxy->RegionID)
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
         Console::WriteLine("Could not cast object to BrepRegionProxy");
         throw new ArgumentException ("Object is not a BrepRegionProxy");
      }

	  return 1;
   }

   bool BrepRegionProxy::ComputePreciseProperties (
	   double __gc& dArea, double __gc& dVolume, Vector3d __gc *centroid, ArrayList __gc *arrMoments)
   {
	   try
	   {
		   IwRegion *pIwRegion = GetIwRegion ();
		   if (pIwRegion)
		   {
			   double dRelativeAccuracy = 0.01;
			   double dEstimatedArea = 0.;
			   double rdVolume = 0.;
			   double rdArea = 0.;
			   IwVector3d ptOrigin (0., 0., 0.);
			   IwTArray<IwPoint3d> moments;
			   moments.SetSize (8);

			   IwStatus status = pIwRegion->ComputePreciseProperties (dRelativeAccuracy, ptOrigin, 
				   dEstimatedArea, rdArea, rdVolume, moments);

			   // Set computed properties to passed in array variables. Note that the IwRegion
			   // ComputePreciseProperties method calculates a negative volume and moments for infinite
			   // regions so we reverse the sign of infinite regions.

			   if (status == IW_SUCCESS)
			   {
				   arrMoments->Clear ();
				   dArea = rdArea;
				   centroid->X = moments[4].x / rdVolume;
				   centroid->Y = moments[4].y / rdVolume;
				   centroid->Z = moments[4].z / rdVolume;
				   dVolume = rdVolume;
				   for (unsigned int iMom = 0; iMom < moments.GetSize (); iMom++)
				   {
					   Vector3d __gc *vec = new Vector3d (moments[iMom].x, moments[iMom].y, moments[iMom].z);
					   arrMoments->Add (vec);
				   }

				   if (IsInfiniteRegion ())
				   {
					   dVolume = -dVolume;
					   for (int iMom = 4; iMom < arrMoments->Count; iMom++)
					   {
						   Vector3d __gc *vec = dynamic_cast<Vector3d *> (arrMoments->get_Item (iMom));
						   if (vec)
							   vec->Scale (-1.0);
					   }
				   }
			   }
			   else
				   return false;
		   }
	   }
	   catch (System::Exception __gc *e)
	   {
		   Console::WriteLine (e->Message);
		   return false;
	   }
	   return true;
   }

   bool BrepRegionProxy::ComputeProperties (
	   double __gc& dArea, double __gc& dVolume, Vector3d __gc *centroid, ArrayList __gc *arrMoments)
   {
	   try
	   {
		   IwRegion *pIwRegion = GetIwRegion ();
		   if (pIwRegion)
		   {
			   double dEdgeTessTol = 0.01;
			   double dFaceTessTol = 0.01;
			   double rdVolume = 0.;
			   double rdArea = 0.;
			   IwVector3d ptOrigin (0., 0., 0.), ptCentroid;
			   IwVector3d vecMoments[6];

			   IwStatus status = pIwRegion->ComputeProperties (dEdgeTessTol, dFaceTessTol, ptOrigin, 
				   rdArea, rdVolume, ptCentroid, vecMoments);

			   // Set computed properties to passed in array variables. Note that the IwRegion
			   // ComputeProperties method calculates a negative volume and moments for infinite
			   // regions (using our modified version of SMLib 7.2 and hopefully future releases
			   // of SMLib) so we reverse the sign of infinite regions.

			   if (status == IW_SUCCESS)
			   {
				   arrMoments->Clear ();
				   dArea = rdArea;
				   centroid->X = ptCentroid.x;
				   centroid->Y = ptCentroid.y;
				   centroid->Z = ptCentroid.z;
				   dVolume = rdVolume;
				   for (int iMom = 0; iMom < 2; iMom++) // only first 2 moments currently valid in SMLIB
				   {
					   Vector3d __gc *vec = new Vector3d (vecMoments[iMom].x, vecMoments[iMom].y, vecMoments[iMom].z);
					   arrMoments->Add (vec);
				   }

				   if (IsInfiniteRegion ())
				   {
					   dVolume = -dVolume;
					   for (int iMom = 0; iMom < 2; iMom++)
					   {
						   Vector3d __gc *vec = dynamic_cast<Vector3d *> (arrMoments->get_Item (iMom));
						   if (vec)
							   vec->Scale (-1.0);
					   }
				   }
			   }
			   else
				   return false;
		   }
	   }
	   catch (System::Exception __gc *e)
	   {
		   Console::WriteLine (e->Message);
		   return false;
	   }
	   return true;
   }

   double BrepRegionProxy::get_Volume()
   {
	   if (m_dArea == System::Double::MinValue)
		   SetProperties();
	   return m_dVolume;
   }

   double BrepRegionProxy::get_Area()
   {
	   if (m_dArea == System::Double::MinValue)
		   SetProperties();
	   return m_dArea;
   }

   PESMLIB::Vector3d __gc * BrepRegionProxy::get_Centroid()
   {
	   if (m_dArea == System::Double::MinValue)
		   SetProperties();
	   return m_pvecCentroid;
   }

   void BrepRegionProxy::SetProperties()
   {
	   IwRegion *pIwRegion = GetIwRegion ();
	   if (pIwRegion)
	   {
		   double dEdgeTessTol = 0.01;
		   double dFaceTessTol = 0.01;
		   double dVolume = 0.;
		   double dArea = 0.;
		   IwVector3d ptOrigin (0., 0., 0.), ptCentroid;
		   IwVector3d vecMoments[6];
		   Vector3d __gc *vecCentroid = new Vector3d (0.,0.,0.);
		   System::Collections::ArrayList __gc * arrMoments = new System::Collections::ArrayList();

		   if (ComputeProperties (dArea, dVolume, vecCentroid, arrMoments))
		   {
			   m_dVolume = dVolume;
			   m_dArea = dArea;
			   m_pvecCentroid = new PESMLIB::Vector3d (vecCentroid->X, vecCentroid->Y, vecCentroid->Z);
		   }
		   else if (ComputePreciseProperties (dArea, dVolume, vecCentroid, arrMoments))
		   {
			   m_dVolume = dVolume;
			   m_dArea = dArea;
			   m_pvecCentroid = new PESMLIB::Vector3d (vecCentroid->X, vecCentroid->Y, vecCentroid->Z);
		   }
	   }
   }

   System::Collections::ArrayList __gc * BrepRegionProxy::GetFaces()
   {
	   System::Collections::ArrayList __gc * listFaceProxies = new System::Collections::ArrayList();
      IwRegion *pIwRegion = GetIwRegion ();
      if (NULL != pIwRegion)
      {
         IwTArray<IwFace *> arrFaces;
         IwTArray<IwShell *> arrShells;
         pIwRegion->GetShells(arrShells);
         int nShells = arrShells.GetSize();
         for (int i = 0; i < nShells; i++)
         {
            IwTArray<IwFaceuse *> arrFaceuses;
            arrShells[i]->GetFaceuses(arrFaceuses);
            int nFaceuses = arrFaceuses.GetSize();
            for (int j = 0; j < nFaceuses; j++)
            {
               arrFaces.Add(arrFaceuses[j]->GetFace());
            }
         }
         int nFaces = arrFaces.GetSize();
         for (int i = 0; i < nFaces; i++)
         {
            // Try to find the Edge counter attribute.
            IwAttribute *pAttribute = 0;
            if (NULL != (pAttribute = ((IwAObject *) arrFaces[i])->FindAttribute (AttributeID_BREPFACECOUNTER)) &&
               pAttribute->GetNumLongElements () > 0)
            {
               const long *lAttributes = pAttribute->GetLongElementsAddress ();
               BrepFaceProxy *faceProxy = new BrepFaceProxy (m_pContext, this->Brep, lAttributes[0]);
               listFaceProxies->Add (faceProxy);
            }
         }
      }
	   return listFaceProxies;
   }

   // IGeometry implementation

   void BrepRegionProxy::InsertGraphics (bool bDrawDetailed, int handle)
   {
      // Create a temporary brep defined by just this region so that 

//      if (m_pBrep && m_pContext)
//      {
//         PESMLIB::Brep __gc *destBrep = new PESMLIB::Brep (m_pContext, NULL);
//
//         // In order for the CopyFaces method of the Brep to work when copying into
//         // an empty Brep, the tolerance of the destination Brep must be the same as
//         // the vertex tolerance of the source brep because there is a comparison step
//         // which calls CalculateBoundingBox which issues an error on the empty Brep.
//
//         destBrep->Tolerance = m_pBrep->VertexTolerance;
//         m_pBrep->CreateBrepFromRegion (this, destBrep);
////         destBrep->SewFaces ();
////         destBrep->MakeManifold ();
//         destBrep->InsertGraphics (keyGeom, bDrawDetailed);
//      }
   }

   //void BrepRegionProxy::RemoveGraphics (HC::KEY keyGeom)
   //{
   //   // Flush the contents of the segment. It is the callers responsibility to create
   //   // or delete the containing segment.

   //   HC::Open_Segment_By_Key (keyGeom);
   //   HC::Flush_Contents ("...", "everything");
   //   HC::Close_Segment ();
   //}

   //void BrepRegionProxy::Transform (Transformation * oTransformation)
   //{
   //}
	
   bool BrepRegionProxy::ComputeBoundingBox (HC::NL_POINT __gc * ptMin, HC::NL_POINT __gc * ptMax)
   {
      try
      {
         if (ptMin != NULL && ptMax != NULL)
         {
            ptMin->x = System::Single::MaxValue;
            ptMin->y = System::Single::MaxValue;
            ptMin->z = System::Single::MaxValue;
            ptMax->x = System::Single::MinValue;
            ptMax->y = System::Single::MinValue;
            ptMax->z = System::Single::MinValue;

            System::Collections::ArrayList __gc *arrFaceProxies = GetFaces ();

            for (int iFace = 0; iFace < arrFaceProxies->Count; iFace++)
            {
               BrepFaceProxy *pFaceProxy = dynamic_cast<BrepFaceProxy *> (arrFaceProxies->get_Item (iFace));
               if (pFaceProxy)
               {
				  HC::NL_POINT localptMin, localptMax;
                  pFaceProxy->ComputeBoundingBox (&localptMin, &localptMax);
                  ptMin->x = System::Math::Min (localptMin.x, ptMin->x);
                  ptMin->y = System::Math::Min (localptMin.y, ptMin->y);
                  ptMin->z = System::Math::Min (localptMin.z, ptMin->z);
                  ptMax->x = System::Math::Max (localptMax.x, ptMax->x);
                  ptMax->y = System::Math::Max (localptMax.y, ptMax->y);
                  ptMax->z = System::Math::Max (localptMax.z, ptMax->z);
               }
            }

            return true;
         }
      }
      catch (System::Exception __gc *ex)
      {
         Console::WriteLine (ex->Message);
      }
      return false;
   }

   void BrepRegionProxy::Highlight (HC::KEY keySeg)
   {
      if (m_pBrep != NULL)
         m_pBrep->HighlightFeature (keySeg, Brep::BrepFeatureType::Brep_Region, m_lRegionID);
   }

   void BrepRegionProxy::UnHighlight (HC::KEY keySeg)
   {
      if (m_pBrep != NULL)
         m_pBrep->UnHighlightFeature (keySeg, Brep::BrepFeatureType::Brep_Region, m_lRegionID);
   }

   System::Object __gc * BrepRegionProxy::GetReferencableObject ()
   {
      return m_pBrep;
   }

   IwRegion * BrepRegionProxy::GetIwRegion ()
   {
      IwRegion *pIwRegion = NULL;

      try
      {
         // First retrieve the Brep region from the owning brep.

         if (NULL != m_pBrep)
         {
            IwBrep *pIwBrep = (IwBrep *) (m_pBrep->GetIwObj());
            if (m_lRegionID == -1) // infinite region
            {
               pIwRegion = pIwBrep->GetInfiniteRegion ();
            }
            else
            {
               IwTArray<IwRegion *> arrRegions;
               pIwBrep->GetRegions(arrRegions);
               int nRegions = arrRegions.GetSize();
               for (int i = 0; i < nRegions; i++)
               {
                  IwAttribute *pAttribute = arrRegions[i]->FindAttribute (AttributeID_BREPREGIONCOUNTER);
                  if (pAttribute && pAttribute->GetNumLongElements () > 0)
                  {
                     const long *lAttributes = pAttribute->GetLongElementsAddress ();
                     if (lAttributes[0] == this->RegionID)
                     {
                        pIwRegion = arrRegions[i];
                        break;
                     }
                  }
               }
            }
         }
      }
	  catch (Exception *)
      {
         Console::WriteLine("Could not get IwRegion associated with BrepRegionProxy");
      }

      return pIwRegion;
   }

   bool BrepRegionProxy::IsDependentOn (IPersistentObject __gc *pObj)
   {
      try
      {
         // First retrieve the Brep region from the owning brep.

         if (NULL != m_pBrep)
         {
            IwRegion *pIwRegion = GetIwRegion ();
            if (NULL != pIwRegion)
            {
               IwAttribute *pAttribute = pIwRegion->FindAttribute (AttributeID_IDOBJ);
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
         Console::WriteLine("Could not determine object dependency in BrepRegionProxy");
      }

      return false;
   }

   System::Collections::ArrayList __gc * BrepRegionProxy::GetObjectDependencies ()
   {
      try
      {
         System::Collections::ArrayList __gc *arrDependents = new System::Collections::ArrayList ();

         // First retrieve the Brep region from the owning brep.

         if (NULL != m_pBrep)
         {
            IwRegion *pIwRegion = GetIwRegion ();
            if (NULL != pIwRegion)
            {
               IwAttribute *pAttribute = pIwRegion->FindAttribute (AttributeID_IDOBJ);
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
                     bool bInvalidDependencyFound = false;

                     for (int iTok = 0; iTok < sTokens->Count; iTok++)
                     {
                        System::Object __gc *pObj = m_pBrep->GetDependency (sTokens[iTok]);
                        if (NULL != pObj)
                           arrDependents->Add (pObj);
                        else // invalid dependency so remove it from this region
                        {
                           bInvalidDependencyFound = true;
                           int iObj = sAttribute->IndexOf (sTokens[iTok]);
                           if (iObj >= 0) // object found so modify string to remove it
                           {
                              // Check to see if there is a comma to be removed.

                              if (iObj > 0 && sAttribute->Chars[iObj-1] == chars[0])
                                 sAttribute = sAttribute->Remove (iObj-1, sTokens[iTok]->Length+1);
                              else if (iObj == 0 && sAttribute->Length > sTokens[iTok]->Length && sAttribute->Chars[sTokens[iTok]->Length] == chars[0])
                                 sAttribute = sAttribute->Remove (iObj, sTokens[iTok]->Length+1);
                              else
                                 sAttribute = sAttribute->Remove (iObj, sTokens[iTok]->Length);
                           }
                        }
                     }

                     // If while retrieving object dependencies we encountered one or
                     // more dependencies where we could not retrieve the object from the
                     // owning brep, then remove this dependency since it has been orphaned.

                     if (bInvalidDependencyFound)
                     {
                        pIwRegion->RemoveAttribute (pAttribute, TRUE);

                        IwTArray<long> arrLongEl;
                        IwTArray<double> arrDoubleEl;
                        IwTArray<char> arrCharEl;

                        int lSize = sAttribute->Length;
                        if (lSize > 0)
                        {
                           for (int iString = 0; iString < lSize; iString++)
                              arrCharEl.Add(Convert::ToByte(sAttribute->Chars[iString]));
                           IwGenericAttribute *pNewAttribute = new (m_pContext->GetIwContext()) IwGenericAttribute (
                              AttributeID_IDOBJ, IW_AB_COPY, arrLongEl, arrDoubleEl, arrCharEl);
                           pIwRegion->AddAttribute (pNewAttribute);
                           m_pBrep->AddToDOM (); // remember to update the persistent image if we change an attribute
                        }
                     }
                  }
               }
            }
         }

         return arrDependents;
      }
      catch (Exception *)
      {
         Console::WriteLine("Could not determine object dependency in BrepRegionProxy");
      }

      return NULL;
   }

   void BrepRegionProxy::RemoveObjectDependency (IPersistentObject __gc *pIPersistentObject)
   {
      try
      {
         // First retrieve the Brep region from the owning brep.

         if (NULL != m_pBrep)
         {
            IwRegion *pIwRegion = GetIwRegion ();
            if (NULL != pIwRegion)
            {
               // Now try to find an existing attribute with the input object ID.

               IwAttribute *pExistingAttribute = pIwRegion->FindAttribute (AttributeID_IDOBJ);
               if (NULL != pExistingAttribute)
               {
                  // If existing attribute type found, get the string and look for this ID and
                  // if found remove it.

                  System::String __gc *sAttribute = new System::String (
                     pExistingAttribute->GetCharacterElementsAddress ());
                  int iObj = sAttribute->IndexOf (pIPersistentObject->IdSelf);
                  if (iObj >= 0) // object found so modify string to remove it
                  {
                     // Check to see if there is a comma to be removed.

                     if (iObj > 0 && sAttribute->Chars[iObj-1] == ',')
                        sAttribute = sAttribute->Remove (iObj-1, pIPersistentObject->IdSelf->Length+1);
                     else if (iObj == 0 && sAttribute->Length > pIPersistentObject->IdSelf->Length && sAttribute->Chars[pIPersistentObject->IdSelf->Length] == ',')
                        sAttribute = sAttribute->Remove (iObj, pIPersistentObject->IdSelf->Length+1);
                     else
                        sAttribute = sAttribute->Remove (iObj, pIPersistentObject->IdSelf->Length);

                     pIwRegion->RemoveAttribute (pExistingAttribute, TRUE);

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
                        pIwRegion->AddAttribute (pAttribute);
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
         Console::WriteLine("Could not remove object dependency from BrepRegionProxy");
      }
   }

   void BrepRegionProxy::AddObjectDependency (IPersistentObject __gc *pIPersistentObject)
   {
      try
      {
         // First retrieve the Brep region from the owning brep.

         if (NULL != m_pBrep)
         {
            IwRegion *pIwRegion = GetIwRegion ();
            if (NULL != pIwRegion)
            {
               System::String __gc *sNewAttribute = System::String::Copy (pIPersistentObject->IdSelf);

               IwAttribute *pExistingAttribute = pIwRegion->FindAttribute (AttributeID_IDOBJ);
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
                     pIwRegion->RemoveAttribute (pExistingAttribute, TRUE);
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
                  pIwRegion->AddAttribute (pAttribute);
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
         Console::WriteLine("Could not set object attribute on BrepRegionProxy");
      }
   }

   void BrepRegionProxy::SetAttribute (AttributeID ulAttributeID, System::Object __gc *oAttrib, 
      AttributeBehavior behavior)
   {
      try
      {
         if (NULL != m_pBrep)
         {
            // First retrieve the Brep region from the owning brep.

            IwRegion *pIwRegion = GetIwRegion ();

            if (NULL != pIwRegion)
            {
               // Remove any existing attribute of this type.

               IwAttribute *pExistingAttribute = pIwRegion->FindAttribute ((ULONG) ulAttributeID);
               if (NULL != pExistingAttribute)
                  pIwRegion->RemoveAttribute (pExistingAttribute, TRUE);

               System::String *sType = oAttrib->GetType ()->FullName;
               if (sType->Equals (S"System.Int32"))
               {
                  System::Int32 *plValue = dynamic_cast<System::Int32 *> (oAttrib);
                  if (NULL != plValue)
                  {
                     IwLongAttribute *pAttribute = new (m_pContext->GetIwContext()) IwLongAttribute (
                        (ULONG) ulAttributeID, *plValue, (IwAttributeBehaviorType) behavior);
                     pIwRegion->AddAttribute (pAttribute);
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
                        pIwRegion->AddAttribute (pAttribute);
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
                     pIwRegion->AddAttribute (pAttribute);
                     m_pBrep->AddToDOM (); // remember to update the persistent image if we add an attribute
                  }
               }
            } // if (NULL != pIwRegion)
         }
      }
      catch (Exception *)
      {
         Console::WriteLine("Could not set attribute on BrepRegionProxy");
      }
   }

   System::Object __gc * BrepRegionProxy::FindAttribute (AttributeID ulAttributeID)
   {
      try
      {
         // First retrieve the Brep region from the owning brep.

         if (NULL != m_pBrep)
         {
            IwRegion *pIwRegion = GetIwRegion ();
            if (NULL != pIwRegion)
            {
               IwAttribute *pAttribute = pIwRegion->FindAttribute ((ULONG) ulAttributeID);
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
         Console::WriteLine("Could not set attribute on BrepRegionProxy");
      }

      return NULL;
   }

   bool BrepRegionProxy::RayIntersect(PESMLIB::Vector3d __gc *start, PESMLIB::Vector3d __gc *dir)
   {

	   IwPoint3d iwStart(start->X + (dir->X * .000001),
		   start->Y + (dir->Y * .000001),
		   start->Z + (dir->Z * .000001));
	   IwVector3d iwDir(dir->X, dir->Y, dir->Z);

	   IwRegion* r = GetIwRegion();
	   IwPointClassification intersection;

	   IwBrep* b = (IwBrep *)m_pBrep->GetIwObj();

	   IwStatus status = b->RayFire(iwStart, iwDir, r, intersection);

	   bool retVal = (status == IW_SUCCESS && r == GetIwRegion());

	   return retVal;
   }
}