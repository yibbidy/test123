#pragma once
using namespace VEDM::Windows;

namespace PESMLIB
{
   public ref class BrepRegionDecoder : public SelectionDecoder
   {
   public:
      BrepRegionDecoder(void);
      ~BrepRegionDecoder(void);
      System::Object^  Decode (Selection^ , int index);
   };
}
