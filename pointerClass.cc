#include "../include/ConstantAbstractDomain.h"
#include "../include/DisjointUnionAbstractDomain.h"
#include <iostream>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

using namespace sparta;


class PointerClass {
 private:
  int indirections;
 public:
  PointerClass(int indirections=0) : indirections(indirections){};

  bool operator==(const PointerClass otherPointer) const {
    return indirections == otherPointer.indirections;
  };
};

using PointerDomain = ConstantAbstractDomain<PointerClass>;
using NumberDomain = ConstantAbstractDomain<uint64_t>;
using PointerNumberDomain =
    DisjointUnionAbstractDomain<PointerDomain, NumberDomain>;


int main(int argc, char const* argv[]) {
  auto p1 = PointerClass(1);
  auto p2 = PointerClass(0);
  PointerNumberDomain num = NumberDomain(2);
  PointerNumberDomain num2 = NumberDomain(2);

  PointerNumberDomain p = PointerDomain(p1);
  PointerNumberDomain pd2 = PointerDomain(p2);


  auto dom = p.maybe_get<NumberDomain>();

  if (dom == boost::none) {
    std::cout << "ww";
  }

  return 0;
}
