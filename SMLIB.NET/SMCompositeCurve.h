#pragma once
#include "smobject.h"
#include "smextent1d.h"

using namespace System::Runtime::InteropServices;
//using namespace PEHoops::ThreeDGS;
#include "SMNurbsCurve.h"

namespace PESMLIB
{
	__gc public class CompositeCurve :	public SMObject
	{
	public:
		CompositeCurve();
//		CompositeCurve (Context*, XML::XmlElement *pXMLElem);
		virtual ~CompositeCurve();

	  void ReverseParameterization ();
      Extent1d __gc * GetNaturalInterval ();
      void Tessellate(Extent1d __gc *extInterval, double dChordHeightTolerance, double dAngleToleranceDeg,
         ULONG lMinimumNumberOfSegments, System::Collections::ArrayList __gc *parameters,
         System::Collections::ArrayList __gc * points);
      long GetNumSegments ();
      NurbsCurve __gc * GetCurveSegment (int iSeg);
      static void BuildCompositesFromCurves (Context __gc *pContext, XML::XmlElement *pXMLElem, 
         System::Collections::ArrayList __gc *arrNurbsCurves, bool bMakeHomogeneous,
         double dSamePtTol, double dDistToCreateLine, System::Collections::ArrayList __gc *arrComposites);

	public private:
		virtual void AttachIwObj (Context *pContext, IwObject *);

	protected private:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	};
}