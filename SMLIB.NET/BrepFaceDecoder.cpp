#include "StdAfx.h"
#include "brepfacedecoder.h"
#include "smbrep.h"
#include "brepfaceproxy.h"
#using <mscorlib.dll>

namespace PESMLIB
{
   BrepFaceDecoder::BrepFaceDecoder(void)
   {
   }

   BrepFaceDecoder::~BrepFaceDecoder(void)
   {
   }

   System::Object __gc * BrepFaceDecoder::Decode (Selection __gc *selection, int index)
   {
      BrepFaceProxy __gc *proxyObj = NULL;

      try
      {
         if (_selectionMap != System::Type::Missing)
         {
            System::Object __gc *obj = _selectionMap->GetGeometry (selection->GeomSeg);
            if (obj != System::Type::Missing)
            {
               Brep __gc *pBrep = dynamic_cast<Brep __gc *> (obj);
               if (NULL != pBrep)
                  proxyObj = pBrep->GetFace (selection->Geom);
            }
         }
      }
	  catch (Exception *)
      {
         Console::WriteLine("Could not decode brep face selection.");
      }

      return proxyObj;
   }	
}