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

   System::Object^  BrepFaceDecoder::Decode (Selection^ selection, int index)
   {
      BrepFaceProxy^ proxyObj = NULL;

      try
      {
         if (_selectionMap != System::Type::Missing)
         {
            System::Object^ obj = _selectionMap->GetGeometry (selection->GeomSeg);
            if (obj != System::Type::Missing)
            {
               Brep^ pBrep = dynamic_cast<Brep^ > (obj);
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