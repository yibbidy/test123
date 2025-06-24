#include "StdAfx.h"
#include ".\brepedgeproxy.h"
#include "brepfaceproxy.h"
#using <mscorlib.dll>

namespace PESMLIB
{
	BrepEdgeProxy::BrepEdgeProxy(void)
	{
		m_lEdgeID = -1;
		m_pBrep = 0;
		m_pContext = 0;
	}

	BrepEdgeProxy::BrepEdgeProxy (PESMLIB::Context^ pContext, PESMLIB::Brep^ pBrep, long lEdgeID)
	{
		m_pContext = pContext;
		m_pBrep = pBrep;
		m_lEdgeID = lEdgeID;
	}

	BrepEdgeProxy::~BrepEdgeProxy(void)
	{
	}

	int BrepEdgeProxy::get_EdgeUseCount()
	{
		IwEdge * pEdge = this->GetIwEdge();
		IwTArray<IwEdgeuse*> sEUs;
		pEdge->GetEdgeuses(sEUs);
		return sEUs.GetSize()/2;
	}

	void BrepEdgeProxy::ComputeProperties(double *pdLength)
	{
		try
		{
			IwEdge *pIwEdge = GetIwEdge ();
			if (pIwEdge)
			{
				double rdRadius = 0.0, rdLength, rdArea, rdVolume;
				IwVector3d ptOrigin (0., 0., 0.);
				IwTArray<IwVector3d> arrVecMoments;

				IwStatus status = pIwEdge->ComputePreciseProperties (
					0.001, ptOrigin, rdRadius, rdLength, rdArea, rdVolume, arrVecMoments);

				if (status == IW_SUCCESS)
				{
					*pdLength = rdLength;
				}
				else
				{
					*pdLength = pIwEdge->GetCurve()->ApproximateLength(pIwEdge->GetInterval(), 10);
				}
			}
		}
		catch (System::Exception^ e)
		{
			Console::WriteLine (e->Message);
		}
	}

	double BrepEdgeProxy::GetLength()
	{
		double dLength = 0.0;
		if (m_lEdgeID != -1)
			ComputeProperties (&dLength);
		return dLength;
	}

	Vector3d * BrepEdgeProxy::GetPoint(double percentage)
	{
		IwEdge *pEdge = NULL;
		if (m_lEdgeID != -1)
		{
			IwBrep *pBrep = (IwBrep *)m_pBrep->GetIwObj();
			IwTArray<IwEdge *> arrEdges;
			pBrep->GetEdges(arrEdges);
			ULONG numEdges = arrEdges.GetSize();
			for (ULONG iEdge = 0; iEdge < numEdges; iEdge++)
			{
				// Try to find the Edge counter attribute.

				IwAttribute *pAttribute = 0;
				if (NULL != (pAttribute = ((IwAObject *)arrEdges[iEdge])->FindAttribute(AttributeID_BREPEDGECOUNTER)) &&
					pAttribute->GetNumLongElements() > 0)
				{
					const long *lAttributes = pAttribute->GetLongElementsAddress();
					if (lAttributes[0] == m_lEdgeID)
					{
						pEdge = arrEdges[iEdge];
						break;
					}
				}
			}
			if (NULL != pEdge)
			{
				IwExtent1d interval = pEdge->GetInterval();
				
				double dEvalParam = ((interval.GetMax() - interval.GetMin()) * percentage) + interval.GetMin();
				IwCurve *pCurve = pEdge->GetCurve();
				IwVector3d * aPoint = new IwVector3d();
				pCurve->Evaluate(dEvalParam, 0, TRUE, aPoint);
				// Create Vector3d
				Vector3d *pPoint = new Vector3d(aPoint[0].x, aPoint[0].y, aPoint[0].z);
				// Delete aMidpoint before exiting
				delete aPoint;
				return pPoint;
			}
		}
		return NULL;
	}


	Vector3d * BrepEdgeProxy::GetMidPoint()
	{	
		return GetPoint(.5);
	}

	bool BrepEdgeProxy::IsLine()
	{
		IwEdge *pEdge = NULL;
		if (m_lEdgeID != -1)
		{
			IwBrep *pBrep = (IwBrep *)m_pBrep->GetIwObj();
			IwTArray<IwEdge *> arrEdges;
			pBrep->GetEdges(arrEdges);
			ULONG numEdges = arrEdges.GetSize();
			for (ULONG iEdge = 0; iEdge < numEdges; iEdge++)
			{
				// Try to find the Edge counter attribute.

				IwAttribute *pAttribute = 0;
				if (NULL != (pAttribute = ((IwAObject *)arrEdges[iEdge])->FindAttribute(AttributeID_BREPEDGECOUNTER)) &&
					pAttribute->GetNumLongElements() > 0)
				{
					const long *lAttributes = pAttribute->GetLongElementsAddress();
					if (lAttributes[0] == m_lEdgeID)
					{
						pEdge = arrEdges[iEdge];
						break;
					}
				}
			}
			if (NULL != pEdge)
			{
				IwPoint3d unused1;
				IwVector3d unused2;
				IwCurve *pCurve = pEdge->GetCurve();
				return (bool)pCurve->IsLine(20, 1.0e-05, unused1, unused2);
			}
		}

		return false;
	}

	System::Collections::ArrayList^  BrepEdgeProxy::GetFaces()
	{
		System::Collections::ArrayList^  listFaceProxies = new System::Collections::ArrayList();
		IwEdge * pIwEdge = NULL;
		IwBrep * pIwBrep = (IwBrep *) (this->get_Brep()->GetIwObj());
		IwTArray<IwEdge *> arrEdges;
		pIwBrep->GetEdges(arrEdges);
		int nEdges = arrEdges.GetSize();
		for (int i = 0; i < nEdges; i++)
		{
			IwAttribute *pAttribute = arrEdges[i]->FindAttribute (AttributeID_BREPEDGECOUNTER);
			if (pAttribute && pAttribute->GetNumLongElements () > 0)
			{
				const long *lAttributes = pAttribute->GetLongElementsAddress ();
				if (lAttributes[0] == this->EdgeID)
				{
					pIwEdge = arrEdges[i];
					break;
				}
			}
		}
		IwTArray<IwFace *> arrFaces;
		pIwEdge->GetFaces(arrFaces);
		int nFaces = arrFaces.GetSize();
		for (int i = 0; i < nFaces; i++)
		{
			// Try to find the Face counter attribute.
			IwAttribute *pAttribute = 0;
			if (NULL != (pAttribute = ((IwAObject *) arrEdges[i])->FindAttribute (AttributeID_BREPFACECOUNTER)) &&
				pAttribute->GetNumLongElements () > 0)
			{
				const long *lAttributes = pAttribute->GetLongElementsAddress ();
				BrepFaceProxy *faceProxy = new BrepFaceProxy (m_pContext, this->Brep, lAttributes[0]);
				listFaceProxies->Add (faceProxy);
			}
		}
		return listFaceProxies;
	}

	void BrepEdgeProxy::InsertGraphics (bool bDrawDetailed, int handle)
	{
	}

	//void BrepEdgeProxy::RemoveGraphics (HC::KEY keyGeom)
	//{
	//	if (m_pBrep != NULL)
	//		m_pBrep->RemoveFeatureGraphics (keyGeom, Brep::BrepFeatureType::Brep_Edge, (HC::KEY) m_lEdgeID);
	//}

	//void BrepEdgeProxy::Transform (Transformation * oTransformation)
	//{
	//}

	bool BrepEdgeProxy::ComputeBoundingBox (HC::NL_POINT^  ptMin, HC::NL_POINT^  ptMax)
	{
		return false;
	}

	void BrepEdgeProxy::Highlight (HC::KEY keySeg)
	{
		if (m_pBrep != NULL)
			m_pBrep->HighlightFeature (keySeg, Brep::BrepFeatureType::Brep_Edge, m_lEdgeID);
	}

	void BrepEdgeProxy::UnHighlight (HC::KEY keySeg)
	{
		if (m_pBrep)
			m_pBrep->UnHighlightFeature (keySeg, Brep::BrepFeatureType::Brep_Edge, m_lEdgeID);
	}

	System::Object^  BrepEdgeProxy::GetReferencableObject ()
	{
		return m_pBrep;
	}

	int BrepEdgeProxy::CompareTo(System::Object^ obj)
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
				BrepEdgeProxy *proxy = __try_cast<BrepEdgeProxy *> (obj);
				if (proxy)
				{
					if (m_pBrep)
					{
						int iBrepCompare = m_pBrep->CompareTo (proxy->Brep);

						if (iBrepCompare == 0)
						{
							if (EdgeID < proxy->EdgeID)
								return -1;
							else if (EdgeID > proxy->EdgeID)
								return 1;
							else
								return 0;
							//               return EdgeID.CompareTo (proxy->EdgeID);
						}
						else
							return iBrepCompare;
					}
				}
			}
		}
		catch(System::InvalidCastException*)
		{
			Console::WriteLine("Could not cast object to BrepEdgeProxy");
			throw new ArgumentException ("Object is not a BrepEdgeProxy");
		}

		return 1;
	}

	bool BrepEdgeProxy::Equals (System::Object^  obj)
	{
		try
		{
			BrepEdgeProxy *pBrepEdgeProxy = __try_cast<BrepEdgeProxy *> (obj);
			if (NULL != m_pBrep && NULL != pBrepEdgeProxy && m_pBrep->Equals (pBrepEdgeProxy->Brep) &&
				m_lEdgeID == pBrepEdgeProxy->EdgeID)
			{
				return true;
			}
		}
		catch(System::InvalidCastException*)
		{
			Console::WriteLine("Could not cast object to BrepEdgeProxy");
		}
		return false;
	}

	System::Object^  BrepEdgeProxy::FindAttribute (AttributeID ulAttributeID)
	{
		try
		{
			// First retrieve the Brep region from the owning brep.

			if (NULL != m_pBrep)
			{
				IwEdge *pIwEdge = GetIwEdge ();
				if (NULL != pIwEdge)
				{
					IwAttribute *pAttribute = pIwEdge->FindAttribute ((ULONG) ulAttributeID);
					if (NULL != pAttribute)
					{
						if (pAttribute->GetNumLongElements () > 0)
						{
							System::Int32^ newLong = new System::Int32();
							const long *plElements = pAttribute->GetLongElementsAddress ();
							*newLong = plElements[0];
							return __box(*newLong);
						}
						else if (pAttribute->GetNumCharacterElements () > 0)
						{
							const char *pcElements = pAttribute->GetCharacterElementsAddress ();
							System::String^ newString = new System::String (pcElements);
							return newString;
						}
						else if (pAttribute->GetNumDoubleElements () > 0)
						{
							const double *pdElements = pAttribute->GetDoubleElementsAddress ();
							System::Double^ newDouble = new System::Double ();
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
			Console::WriteLine("Could not set attribute on BrepFaceProxy");
		}

		return NULL;
	}

	IwEdge * BrepEdgeProxy::GetIwEdge ()
	{
		IwEdge *pIwEdge = NULL;

		try
		{
			// First retrieve the Brep Edge from the owning brep.

			if (NULL != m_pBrep)
			{
				IwBrep *pIwBrep = (IwBrep *) (m_pBrep->GetIwObj());
				IwTArray<IwEdge *> arrEdges;
				pIwBrep->GetEdges(arrEdges);
				int nEdges = arrEdges.GetSize();
				for (int i = 0; i < nEdges; i++)
				{
					IwAttribute *pAttribute = arrEdges[i]->FindAttribute (AttributeID_BREPEDGECOUNTER);
					if (pAttribute && pAttribute->GetNumLongElements () > 0)
					{
						const long *lAttributes = pAttribute->GetLongElementsAddress ();
						if (lAttributes[0] == this->EdgeID)
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
	bool BrepEdgeProxy::IsDependentOn (IPersistentObject^ pObj)
	{
		try
		{
			// First retrieve the Brep region from the owning brep.

			if (NULL != m_pBrep)
			{
				IwEdge *pIwEdge = GetIwEdge ();
				if (NULL != pIwEdge)
				{
					IwAttribute *pAttribute = pIwEdge->FindAttribute (AttributeID_IDOBJ);
					if (NULL != pAttribute)
					{
						if (pAttribute->GetNumCharacterElements () > 0)
						{
							const char *pcElements = pAttribute->GetCharacterElementsAddress ();
							System::String^ newString = new System::String (pcElements);
							if (newString->IndexOf (pObj->IdSelf) >= 0)
								return true;
						}
					}
				}
			}
		}
		catch (Exception *)
		{
			Console::WriteLine("Could not determine object dependency in BrepEdgeProxy");
		}

		return false;
	}

	System::Collections::ArrayList^  BrepEdgeProxy::GetObjectDependencies ()
	{
		try
		{
			System::Collections::ArrayList^ arrDependents = new System::Collections::ArrayList ();

			// First retrieve the Brep region from the owning brep.

			if (NULL != m_pBrep)
			{
				IwEdge *pIwEdge = GetIwEdge ();
				if (NULL != pIwEdge)
				{
					IwAttribute *pAttribute = pIwEdge->FindAttribute (AttributeID_IDOBJ);
					if (NULL != pAttribute)
					{
						if (pAttribute->GetNumCharacterElements () > 0)
						{
							const char *pcElements = pAttribute->GetCharacterElementsAddress ();
							System::String^ sAttribute = new System::String (pcElements);

							// Parse the string of ObjIDs and obtain the objects from the brep

							Char chars[] = {','};
							sAttribute->Trim (); // remove whitespace
							sAttribute->Trim (chars); // remove excess begin and end commas.
							String *sTokens[] = sAttribute->Split (chars);
							for (int iTok = 0; iTok < sTokens->Count; iTok++)
							{
								// Don't allow null objects to be added.
								System::Object^ dependentObj = m_pBrep->GetDependency (sTokens[iTok]);
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
			Console::WriteLine("Could not determine object dependency in BrepEdgeProxy");
		}

		return NULL;
	}

	void BrepEdgeProxy::RemoveObjectDependency (IPersistentObject^ pIPersistentObject)
	{
		// TO DO: implement this method
	}

	void BrepEdgeProxy::AddObjectDependency (IPersistentObject^ pIPersistentObject)
	{
		try
		{
			// First retrieve the Brep Edge from the owning brep.

			if (NULL != m_pBrep && NULL != m_pContext)
			{
				IwEdge *pIwEdge = GetIwEdge ();
				if (NULL != pIwEdge)
				{
					System::String^ sNewAttribute = System::String::Copy (pIPersistentObject->IdSelf);

					IwAttribute *pExistingAttribute = pIwEdge->FindAttribute (AttributeID_IDOBJ);
					if (NULL != pExistingAttribute)
					{
						// If existing attribute found, get the string and look for this ID. 
						// Don't add the ID again if it already exists.

						System::String^ sAttribute = new System::String (
							pExistingAttribute->GetCharacterElementsAddress ());
						int iObj = sAttribute->IndexOf (pIPersistentObject->IdSelf);
						if (iObj < 0) // object not found so add it
						{
							sNewAttribute = System::String::Format ("{0},{1}", sAttribute, pIPersistentObject->IdSelf);
							pIwEdge->RemoveAttribute (pExistingAttribute, TRUE);
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
						pIwEdge->AddAttribute (pAttribute);
						m_pBrep->AddDependency (pIPersistentObject);
						m_pBrep->AddToDOM (); // remember to update the persistent image if we add an attribute
					}
				}
			}
		}
		catch (Exception *)
		{
			Console::WriteLine("Could not set object attribute on BrepEdgeProxy");
		}
	}
}