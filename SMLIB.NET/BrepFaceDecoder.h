#pragma once
using namespace VEDM::Windows;

namespace PESMLIB
{
   public ref class BrepFaceDecoder : public SelectionDecoder
   {
   public:
      BrepFaceDecoder(void);
      ~BrepFaceDecoder(void);
      System::Object^  Decode (Selection^ selection, int index);
   };
}
