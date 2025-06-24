#pragma once
using namespace VEDM::Windows;

namespace PESMLIB
{
   public ref class BrepEdgeDecoder : public SelectionDecoder
   {
   public:
      BrepEdgeDecoder(void);
      ~BrepEdgeDecoder(void);
      System::Object^  Decode (Selection^ selection, int index);
   };
};
