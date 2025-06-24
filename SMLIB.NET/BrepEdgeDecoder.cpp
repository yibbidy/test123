#include "StdAfx.h"
#include "brepedgedecoder.h"
#include "smbrep.h"
#include "brepedgeproxy.h"
#using <mscorlib.dll>

namespace PESMLIB
{
	BrepEdgeDecoder::BrepEdgeDecoder(void)
	{
	}

	BrepEdgeDecoder::~BrepEdgeDecoder(void)
	{
	}

	System::Object __gc * BrepEdgeDecoder::Decode (Selection __gc *selection, int index)
	{
		BrepEdgeProxy __gc *proxyObj = NULL;

		try
		{
			if (_selectionMap != System::Type::Missing)
			{
				System::Object __gc *obj = _selectionMap->GetGeometry (selection->GeomSeg);
				if (obj != System::Type::Missing)
				{
				Brep __gc *pBrep = dynamic_cast<Brep __gc *> (obj);
				if (NULL != pBrep)
					proxyObj = pBrep->GetEdge (selection->Geom);
				}
			}
		}
		catch (...)
		{
			Console::WriteLine("Could not decode brep edge selection.");
		}

		return proxyObj;
	}	
}