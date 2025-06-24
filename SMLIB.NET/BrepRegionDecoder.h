#pragma once
using namespace VEDM::Windows;

namespace PESMLIB
{
   __gc public class BrepRegionDecoder : public SelectionDecoder
   {
   public:
      BrepRegionDecoder(void);
      ~BrepRegionDecoder(void);
      System::Object __gc * Decode (Selection __gc *, int index);
   };
}
