#pragma once
using namespace VEDM::Windows;

namespace PESMLIB
{
   __gc public class BrepEdgeDecoder : public SelectionDecoder
   {
   public:
      BrepEdgeDecoder(void);
      ~BrepEdgeDecoder(void);
      System::Object __gc * Decode (Selection __gc *selection, int index);
   };
};
