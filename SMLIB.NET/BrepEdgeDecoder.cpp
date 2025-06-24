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

	System::Object^  BrepEdgeDecoder::Decode (Selection^ selection, int index)
	{
		BrepEdgeProxy^ proxyObj = NULL;

		try
		{
			if (_selectionMap != System::Type::Missing)
			{
				System::Object^ obj = _selectionMap->GetGeometry (selection->GeomSeg);
				if (obj != System::Type::Missing)
				{
				Brep^ pBrep = dynamic_cast<Brep^ > (obj);
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