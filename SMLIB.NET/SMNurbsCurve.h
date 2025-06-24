#pragma once
#include "smobject.h"
#include "smextent1d.h"

using namespace System::Runtime::InteropServices;
//using namespace PEHoops::ThreeDGS;

namespace PESMLIB
{
	__gc public class NurbsCurve :	public SMObject
	{
	public:
		NurbsCurve();
		NurbsCurve (Context*, XML::XmlElement *pXMLElem);
		virtual ~NurbsCurve();

		virtual void Draw ();
		virtual void Undraw ();

      // Object Overridables
		bool Equals (System::Object __gc *obj);

      // IComparable interface
		int CompareTo (System::Object __gc *);

	  void ReverseParameterization ();
      Extent1d __gc * GetNaturalInterval ();
      void Tessellate(Extent1d __gc *extInterval, double dChordHeightTolerance, double dAngleToleranceDeg,
         ULONG lMinimumNumberOfSegments, System::Collections::ArrayList __gc *parameters,
         System::Collections::ArrayList __gc * points);

	public private:
		virtual void AttachIwObj (Context *pContext, IwObject *);

	protected private:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	//private:
	//	HC::KEY m_HoopsKey;
	};
}