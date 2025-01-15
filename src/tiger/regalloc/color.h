#ifndef TIGER_COMPILER_COLOR_H
#define TIGER_COMPILER_COLOR_H

#include "tiger/codegen/assem.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/util/graph.h"
#include <list>
#include <stack>
#include <unordered_map>
#include <utility>

namespace col {
struct Result {
  Result() : coloring(nullptr), spills(nullptr) {}
  Result(temp::Map *coloring, live::INodeListPtr spills)
      : coloring(coloring), spills(spills) {}
  /// KH-note: if spills is empty, then the reg alloc is done.
  /// otherwise coloring should be ignored, and it should do actual spill, rewriting the code.
  temp::Map *coloring;
  live::INodeListPtr spills;
};

/// KH-note: this class does the route of:
/// build -> simplify -> coalesce -> freeze -> potential-spill -> select
/// (ofcourse how to use this class is up to you)
class Color {
  /* TODO: Put your lab6 code here */
  public:
    Color(live::LiveGraph graph, tab::Table<temp::Temp, live::INode> * tempMap): 
      interf_graph(graph.interf_graph),
      interf_moves(graph.moves),
      temp_node_map(tempMap),
      precolorList(new live::INodeList()),
      coloring(temp::Map::Empty()),
      spillNodes(new live::INodeList())
      {}
    Result DoColoring();

  private:
    int Kcolors;

    /// KH-note: used to store the built live graph
    live::IGraphPtr interf_graph;
    live::MoveList * interf_moves;
    tab::Table<temp::Temp, live::INode> * temp_node_map;

    /// KH-note: used to identify pre-def machine regs
    live::INodeListPtr precolorList;
    /// KH-note: store selection and optimistic spilling results.
    temp::Map * coloring;
    live::INodeListPtr spillNodes;
    /// KH-note: store coalescing results.
    std::unordered_map<live::INodePtr, live::INodePtr> alias_table;

    std::stack<live::INodePtr> selectStack;

    void Build();
    /// KH-note: if it can do a step with less priority, then it must do that.
    /// Possible steps will be pushed into corresponding worklist.
    /// All of these 4 functions only do one step per call.
    void Simplify();
    std::list<live::INodePtr> simplifyWorklist;
    void Coalesce();
    std::list<std::pair<live::INodePtr, live::INodePtr>> coalesceWorklist;
    void Freeze();
    std::list<live::INodePtr> freezeWorklist;
    void Spill();
    std::list<live::INodePtr> spillWorklist;
    void Select();
    
    void SimpSpillHelper(live::INodePtr node);
    void FreezeHelper(live::INodePtr node);

    /// KH-note: extra structure that record node's move info (cope with than movelist)
    struct MoveNode {
      std::list<live::INodePtr> pred;
      std::list<live::INodePtr> succ;
    };
    std::unordered_map<live::INodePtr, MoveNode> move_node_map;
    bool is_move_related(live::INodePtr node) {
      auto & mn = this->move_node_map[node];
      return !(mn.pred.empty() && mn.succ.empty());
    }
    void append_move(live::INodePtr src, live::INodePtr dst) {
      if (this->interf_moves->Contain(src, dst)) {
        return;
      }
      this->move_node_map[src].succ.push_back(dst);
      this->move_node_map[dst].pred.push_back(src);
      this->interf_moves->Append(src, dst);
    }
    void remove_move(live::INodePtr src, live::INodePtr dst) {
      this->move_node_map[src].succ.remove(dst);
      this->move_node_map[dst].pred.remove(src);
      this->interf_moves->Delete(src, dst);
    }
    void remove_move_node(live::INodePtr node) {
      if (this->is_move_related(node)) {
        for (auto dst : this->move_node_map[node].succ) {
          this->move_node_map[dst].pred.remove(node);
          this->interf_moves->Delete(node, dst);
        }
        for (auto src : this->move_node_map[node].pred) {
          this->move_node_map[src].succ.remove(node);
          this->interf_moves->Delete(src, node);
        }
      }
    }

    /// KH-note: this function include:
    /// - add target's neighboor to worklist:
    ///   if degree less than K, move_related to Freeze, non_related to Simplify
    /// - add all moves that include target's neighboor to Coalesce worklist
    void basic_side_effect(live::INodePtr target) {
      for (auto node : target->Pred()->GetList()) {
        if (node->Degree() >= this->Kcolors) {
          continue;
        }
        if (this->is_move_related(node)) {
          this->freezeWorklist.push_back(node);
        }
        else {
          this->simplifyWorklist.push_back(node);
        }
      }
      for (auto move : this->interf_moves->GetList()) {
        if (target->GoesTo(move.first) || target->GoesTo(move.second)) {
          this->coalesceWorklist.push_back(move);
        }
      }
    }
};
} // namespace col

#endif // TIGER_COMPILER_COLOR_H
