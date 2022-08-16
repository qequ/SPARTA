#include "../include/MonotonicFixpointIterator.h"
#include "../include/ConstantAbstractDomain.h"
#include "../include/DisjointUnionAbstractDomain.h"
#include "../include/PatriciaTreeMapAbstractEnvironment.h"
#include "../include/PatriciaTreeSet.h"

namespace typeChecking {
using namespace sparta;

enum class TypesOptions { NUMBER, POINTER };

struct Mnemonic {
  Mnemonic() = default;
  virtual ~Mnemonic() = default;
};

struct Assignment : public Mnemonic {
  Assignment(std::string variable, TypesOptions value)
      : variable(variable), value(value) {}

  std::string variable;
  TypesOptions value;
};

struct Add : public Mnemonic {
  Add(std::string src, std::string dest) : src(src), dest(dest) {}
  std::string src;
  std::string dest;
};

class BasicBlock;

struct Edge final {
  Edge(BasicBlock* source, BasicBlock* target)
      : source(source), target(target) {}

  BasicBlock* source;
  BasicBlock* target;
};

class BasicBlock final {
 public:
  BasicBlock() = default;

  void add(std::unique_ptr<Mnemonic> mnemonic) {
    m_mnemonics.push_back(std::move(mnemonic));
  }

  void add_successor(BasicBlock* successor) {
    m_edges.push_back(std::make_unique<Edge>(this, successor));
    auto* edge = m_edges.back().get();
    m_successors.push_back(edge);
    successor->m_predecessors.push_back(edge);
  }

  const std::vector<std::unique_ptr<Mnemonic>>& mnemonics() const {
    return m_mnemonics;
  }

 private:
  std::vector<std::unique_ptr<Mnemonic>> m_mnemonics;
  std::vector<std::unique_ptr<Edge>> m_edges;
  std::vector<Edge*> m_predecessors;
  std::vector<Edge*> m_successors;

  friend class ProgramInterface;
};

class Program final {
 public:
  Program() = default;

  BasicBlock* create_block() {
    m_basic_blocks.push_back(std::make_unique<BasicBlock>());
    return m_basic_blocks.back().get();
  }

  void set_entry(BasicBlock* entry) { m_entry = entry; }

  void set_exit(BasicBlock* exit) { m_exit = exit; }

 private:
  std::vector<std::unique_ptr<BasicBlock>> m_basic_blocks;
  BasicBlock* m_entry = nullptr;
  BasicBlock* m_exit = nullptr;

  friend class ProgramInterface;
};

class ProgramInterface {
 public:
  using Graph = Program;
  using NodeId = BasicBlock*;
  using EdgeId = Edge*;

  static NodeId entry(const Graph& graph) { return graph.m_entry; }
  static NodeId exit(const Graph& graph) { return graph.m_exit; }
  static std::vector<EdgeId> predecessors(const Graph&, const NodeId& node) {
    return node->m_predecessors;
  }
  static std::vector<EdgeId> successors(const Graph&, const NodeId& node) {
    return node->m_successors;
  }
  static NodeId source(const Graph&, const EdgeId& e) { return e->source; }
  static NodeId target(const Graph&, const EdgeId& e) { return e->target; }
};

class PointerClass {
 private:
  int indirections;

 public:
  PointerClass(int indirections = 0) : indirections(indirections){};

  bool operator==(const PointerClass otherPointer) const {
    return indirections == otherPointer.indirections;
  };
};

using PointerDomain = ConstantAbstractDomain<PointerClass>;
using NumberDomain = ConstantAbstractDomain<uint64_t>;
using PointerNumberDomain =
    DisjointUnionAbstractDomain<PointerDomain, NumberDomain>;

using AbstractEnvironment =
    PatriciaTreeMapAbstractEnvironment<std::string, PointerNumberDomain>;

template <template <typename GraphInterface, typename Domain, typename NodeHash>
          class FixpointIteratorBase>
class FixpointEngine final : public FixpointIteratorBase<
                                 ProgramInterface,
                                 AbstractEnvironment,
                                 std::hash<typename ProgramInterface::NodeId>> {
 private:
  using Base =
      FixpointIteratorBase<ProgramInterface,
                           AbstractEnvironment,
                           std::hash<typename ProgramInterface::NodeId>>;
  using NodeId = typename Base::NodeId;
  using EdgeId = typename Base::EdgeId;

 public:
  explicit FixpointEngine(const Program& program) : Base(program) {}

  void analyze_node(const NodeId& bb,
                    AbstractEnvironment* current_state) const override {
    for (const auto& mnemonic : bb->mnemonics()) {
      analyze_mnemonic(mnemonic.get(), current_state);
    }
  }

  void analyze_mnemonic(Mnemonic* mnemonic,
                        AbstractEnvironment* current_state) const {
    if (auto* assign = dynamic_cast<Assignment*>(mnemonic)) {

      switch (assign->value) {
      case TypesOptions::NUMBER:
        current_state->set(assign->variable, NumberDomain(0));
        break;
      case TypesOptions::POINTER:
        current_state->set(assign->variable, PointerDomain(PointerClass(1)));
        break;
      default:
        throw std::runtime_error("unreachable");
        break;
      }
      // current_state->set(assign->variable, assign->value);
    } else if (auto* add = dynamic_cast<Add*>(mnemonic)) {
      current_state->set(add->dest, current_state->get(add->src));
    } else {
      throw std::runtime_error("unreachable");
    }
  }

  AbstractEnvironment analyze_edge(
      const EdgeId&, const AbstractEnvironment& state) const override {
    return state;
  }
};

} // namespace typeChecking

int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}

