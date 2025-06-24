#pragma once
#include "smobject.h"
#include "smextent1d.h"

using namespace System::Runtime::InteropServices;
//using namespace PEHoops::ThreeDGS;
#include "SMNurbsCurve.h"

namespace PESMLIB
{
	public ref class CompositeCurve :	public SMObject
	{
	public:
		CompositeCurve();
//		CompositeCurve (Context*, XML::XmlElement *pXMLElem);
		virtual ~CompositeCurve();

	  void ReverseParameterization ();
      Extent1d^  GetNaturalInterval ();
      void Tessellate(Extent1d^ extInterval, double dChordHeightTolerance, double dAngleToleranceDeg,
         ULONG lMinimumNumberOfSegments, System::Collections::ArrayList^ parameters,
         System::Collections::ArrayList^  points);
      long GetNumSegments ();
      NurbsCurve^  GetCurveSegment (int iSeg);
      static void BuildCompositesFromCurves (Context^ pContext, XML::XmlElement *pXMLElem, 
         System::Collections::ArrayList^ arrNurbsCurves, bool bMakeHomogeneous,
         double dSamePtTol, double dDistToCreateLine, System::Collections::ArrayList^ arrComposites);

	public private:
		virtual void AttachIwObj (Context *pContext, IwObject *);

	protected private:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	};
}