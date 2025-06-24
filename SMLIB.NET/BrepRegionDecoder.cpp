#include "StdAfx.h"
#include "brepregiondecoder.h"
#include "smbrep.h"
#include "brepregionproxy.h"
#using <mscorlib.dll>

namespace PESMLIB
{
   BrepRegionDecoder::BrepRegionDecoder(void)
   {
   }

   BrepRegionDecoder::~BrepRegionDecoder(void)
   {
   }

   System::Object^  BrepRegionDecoder::Decode (Selection^ selection, int index)
   {
      BrepRegionProxy^ proxyObj = NULL;

      if (_selectionMap != System::Type::Missing)
      {
         System::Object *obj = _selectionMap->GetGeometry (selection->GeomSeg);
         if (obj != System::Type::Missing)
         {
            Brep^ pBrep = dynamic_cast<Brep^ > (obj);
            if (NULL != pBrep)
            {
               System::Collections::ArrayList^ arrRegions = 
                  pBrep->GetRegionsFromFace (selection->Geom);
               if (arrRegions->Count > 0)
			   {
				   if (index == 1 && arrRegions->Count == 2)
					   proxyObj = dynamic_cast<BrepRegionProxy *> (arrRegions->Item [1]);
				   else
					   proxyObj = dynamic_cast<BrepRegionProxy *> (arrRegions->Item [0]);
			   }
            }
         }
      }

      return proxyObj;
   }	
}