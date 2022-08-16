#include "../include/ConstantAbstractDomain.h"

#include "../include/DisjointUnionAbstractDomain.h"

#include <string>

using namespace sparta;

using IntDomain = ConstantAbstractDomain<int>;
using StringDomain = ConstantAbstractDomain<std::string>;
using IntStringDomain = DisjointUnionAbstractDomain<IntDomain, StringDomain>;

int main(int argc, char const* argv[]) {
  IntStringDomain zero = IntDomain(0);
  IntStringDomain str = StringDomain("");
  zero.join(str).is_top();
  return 0;
}
