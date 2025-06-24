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

   System::Object __gc * BrepRegionDecoder::Decode (Selection __gc *selection, int index)
   {
      BrepRegionProxy __gc *proxyObj = NULL;

      if (_selectionMap != System::Type::Missing)
      {
         System::Object *obj = _selectionMap->GetGeometry (selection->GeomSeg);
         if (obj != System::Type::Missing)
         {
            Brep __gc *pBrep = dynamic_cast<Brep __gc *> (obj);
            if (NULL != pBrep)
            {
               System::Collections::ArrayList __gc *arrRegions = 
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