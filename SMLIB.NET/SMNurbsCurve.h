#pragma once
#include "smobject.h"
#include "smextent1d.h"

using namespace System::Runtime::InteropServices;
//using namespace PEHoops::ThreeDGS;

namespace PESMLIB
{
	public ref class NurbsCurve :	public SMObject
	{
	public:
		NurbsCurve();
		NurbsCurve (Context*, XML::XmlElement *pXMLElem);
		virtual ~NurbsCurve();

		virtual void Draw ();
		virtual void Undraw ();

      // Object Overridables
		bool Equals (System::Object^ obj);

      // IComparable interface
		int CompareTo (System::Object^ );

	  void ReverseParameterization ();
      Extent1d^  GetNaturalInterval ();
      void Tessellate(Extent1d^ extInterval, double dChordHeightTolerance, double dAngleToleranceDeg,
         ULONG lMinimumNumberOfSegments, System::Collections::ArrayList^ parameters,
         System::Collections::ArrayList^  points);

	public private:
		virtual void AttachIwObj (Context *pContext, IwObject *);

	protected private:
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	//private:
	//	HC::KEY m_HoopsKey;
	};
}