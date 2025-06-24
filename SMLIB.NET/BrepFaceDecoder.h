#pragma once
using namespace VEDM::Windows;

namespace PESMLIB
{
   __gc public class BrepFaceDecoder : public SelectionDecoder
   {
   public:
      BrepFaceDecoder(void);
      ~BrepFaceDecoder(void);
      System::Object __gc * Decode (Selection __gc *selection, int index);
   };
}
