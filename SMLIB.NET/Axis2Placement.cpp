#include "StdAfx.h"
#include ".\axis2placement.h"
#using <mscorlib.dll>

namespace PESMLIB
{
	Axis2Placement::Axis2Placement(void)
	{
		m_pIwAxis2Placement = new IwAxis2Placement();
	}

	Axis2Placement::~Axis2Placement(void)
	{
		if (m_pIwAxis2Placement)
		{
			delete m_pIwAxis2Placement;
			m_pIwAxis2Placement = NULL;
		}
	}

	void Axis2Placement::GetCannonical(Vector3d *pvecOrigin, Vector3d *pvecXAxis, Vector3d *pvecYAxis)
	{
		IwVector3d origin;
		IwVector3d xaxis;
		IwVector3d yaxis;
		m_pIwAxis2Placement->GetCanonical(origin, xaxis, yaxis);
		pvecOrigin->SetCanonical(origin.x, origin.y, origin.z);
		pvecXAxis->SetCanonical(xaxis.x, xaxis.y, xaxis.z);
		pvecYAxis->SetCanonical(yaxis.x, yaxis.y, yaxis.z);
	}

	void Axis2Placement::SetCannonical(Vector3d *pvecOrigin, Vector3d *pvecXAxis, Vector3d *pvecYAxis)
	{
		m_pIwAxis2Placement->SetCanonical(*(pvecOrigin->GetIwObj()), *(pvecXAxis->GetIwObj()), *(pvecYAxis->GetIwObj()));
	}

	Vector3d* Axis2Placement::GetOrigin(void)
	{
		IwVector3d vecOrigin = m_pIwAxis2Placement->GetOrigin();
		return new Vector3d(vecOrigin.x, vecOrigin.y, vecOrigin.z);
	}

	Vector3d* Axis2Placement::GetXAxis(void)
	{
		IwVector3d vecXAxis = m_pIwAxis2Placement->GetXAxis();
		return new Vector3d(vecXAxis.x, vecXAxis.y, vecXAxis.z);
	}

	Vector3d* Axis2Placement::GetYAxis(void)
	{
		IwVector3d vecYAxis = m_pIwAxis2Placement->GetYAxis();
		return new Vector3d(vecYAxis.x, vecYAxis.y, vecYAxis.z);
	}

	Vector3d* Axis2Placement::GetZAxis(void)
	{
		IwVector3d vecZAxis = m_pIwAxis2Placement->GetZAxis();
		return new Vector3d(vecZAxis.x, vecZAxis.y, vecZAxis.z);
	}

	Axis2Placement* Axis2Placement::Invert(void)
	{
		IwAxis2Placement a2p;
		m_pIwAxis2Placement->Invert(a2p);
		Vector3d *pvecOrigin = new Vector3d(a2p.GetOrigin().x, a2p.GetOrigin().y, a2p.GetOrigin().z);
		Vector3d *pvecXAxis = new Vector3d(a2p.GetXAxis().x, a2p.GetXAxis().y, a2p.GetXAxis().z);
		Vector3d *pvecYAxis = new Vector3d(a2p.GetYAxis().x, a2p.GetYAxis().y, a2p.GetYAxis().z);
		Axis2Placement *pResult = new Axis2Placement();
		pResult->SetCannonical(pvecOrigin, pvecXAxis, pvecYAxis);
		return pResult;
	}

	Vector3d* Axis2Placement::MirrorPoint(Vector3d *pPointToMirror)
	{
		IwVector3d result;
		m_pIwAxis2Placement->MirrorPoint(*(pPointToMirror->GetIwObj()), result);
		return new Vector3d(result.x, result.y, result.z);
	}

	void Axis2Placement::RotateAboutAxis(double dAngRadians, Vector3d *pAxis)
	{
		m_pIwAxis2Placement->RotateAboutAxis(dAngRadians, *(pAxis->GetIwObj()));
	}

	void Axis2Placement::RotateAboutAxisAtPoint(double dAngRadians, Vector3d *pCenter, Vector3d *pAxis)
	{
		m_pIwAxis2Placement->RotateAboutAxisAtPoint(dAngRadians, *(pCenter->GetIwObj()), *(pAxis->GetIwObj()));
	}

	Axis2Placement* Axis2Placement::TransformAxis2Placement(Axis2Placement *pInput)
	{
		IwAxis2Placement result;
		m_pIwAxis2Placement->TransformAxis2Placement(*(pInput->GetIwObj()), result);
		Vector3d *pvecOrigin = new Vector3d(result.GetOrigin().x, result.GetOrigin().y, result.GetOrigin().z);
		Vector3d *pvecXAxis = new Vector3d(result.GetXAxis().x, result.GetXAxis().y, result.GetXAxis().z);
		Vector3d *pvecYAxis = new Vector3d(result.GetYAxis().x, result.GetYAxis().y, result.GetYAxis().z);
		Axis2Placement *pResult = new Axis2Placement();
		pResult->SetCannonical(pvecOrigin, pvecXAxis, pvecYAxis);
		return pResult;
	}

	Vector3d* Axis2Placement::TransformPoint(Vector3d *pvecInput)
	{
		IwVector3d vecResult;
		m_pIwAxis2Placement->TransformPoint(*(pvecInput->GetIwObj()), vecResult);
		return new Vector3d(vecResult.x, vecResult.y, vecResult.z);
	}

	Vector3d* Axis2Placement::TransformVector(Vector3d *pvecInput)
	{
		IwVector3d vecResult;
		m_pIwAxis2Placement->TransformVector(*(pvecInput->GetIwObj()), vecResult);
		return new Vector3d(vecResult.x, vecResult.y, vecResult.z);
	}

	void Axis2Placement::Translate(Vector3d *pvecTranslation)
	{
		m_pIwAxis2Placement->Translate(*(pvecTranslation->GetIwObj()));
	}

	IwAxis2Placement* Axis2Placement::ExtractObj ()
	{
		IwAxis2Placement *pIwObj = m_pIwAxis2Placement;
		m_pIwAxis2Placement = NULL;

		return pIwObj;
	}

	const IwAxis2Placement* Axis2Placement::GetIwObj ()
	{
		return m_pIwAxis2Placement;
	}

	void Axis2Placement::AddToDOM ()
	{
		if (m_pIwAxis2Placement != NULL && m_pXMLElem != NULL)
		{
			// Add the persistent ID.
			m_pXMLElem->SetAttribute("idSelf", GetId());
		}
	}

	void Axis2Placement::GetFromDOM ()
	{
		if (m_pXMLElem != NULL && m_pIwAxis2Placement != NULL)
		{
			// Get the persistent GUID.
			String* sId = m_pXMLElem->GetAttribute("idSelf");
			if (sId->Length > 0)
				SetId (sId);
		}
	}
}
