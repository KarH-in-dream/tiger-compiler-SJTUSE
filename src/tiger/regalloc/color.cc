#include "tiger/regalloc/color.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/output/logger.h"
#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

extern frame::RegManager *reg_manager;

namespace col {
/* TODO: Put your lab6 code here */
Result Color::DoColoring() {
  this->Build();
  while (true) {
    if (!this->simplifyWorklist.empty()) {
      this->Simplify();
    }
    else if (!this->coalesceWorklist.empty()) {
      this->Coalesce();
    }
    else if (!this->freezeWorklist.empty()) {
      this->Freeze();
    }
    else if (!this->spillWorklist.empty()) {
      this->Spill();
    }
    else {
      break;
    }
  }
  this->Select();
  return Result(this->coloring, this->spillNodes);
}

void Color::Build() {
  this->Kcolors = 0;
  for (auto reg : reg_manager->Registers()->GetList()) {
    auto reg_node = this->temp_node_map->Look(reg);
    this->precolorList->Append(reg_node);
    this->coloring->Enter(reg, reg_manager->temp_map_->Look(reg));
    this->Kcolors += 1;
  }

  for (auto [src, dst] : this->interf_moves->GetList()) {
    /// KH-note: map[key] will automatically call default constructor
    this->move_node_map[src].succ.push_back(dst);
    this->move_node_map[dst].pred.push_back(src);
  }

  /// KH-note: Build 4 worklists.
  /// Worklists hold "possible" instruction targets.
  /// They should get checked when taking instructions.
  for (auto node : this->interf_graph->Nodes()->GetList()) {
    if (node->Degree() >= this->Kcolors) {
      this->spillWorklist.push_back(node);
    }
    else if (this->is_move_related(node)) {
      this->freezeWorklist.push_back(node);
    }
    else {
      this->simplifyWorklist.push_back(node);
    }
  }
  for (auto move : this->interf_moves->GetList()) {
    this->coalesceWorklist.push_back(move);
  }
}

void Color::SimpSpillHelper(live::INodePtr node) {
  /// KH-note: do simplify, detach it with other nodes.
  /// DeleteNode() won't delete the Node object in memory, so the deleted node
  ///   still remembers its neighboor when it's detached, but its neighboors can't access it.
  for (auto pred : node->Pred()->GetList()) {
    pred->Pred()->DeleteNode(node);
    pred->Succ()->DeleteNode(node);
  }
  this->interf_graph->Nodes()->DeleteNode(node);
  this->selectStack.push(node);

  /// KH-note: handle side-effects:
  /// - its neighboor can be simplified
  /// - its neighboor can try to coalesce with another node
  /// - its neighboor can be freezed
  /// - its neighboor won't be spilled (but it won't pass the check, so this can be ignored)
  this->basic_side_effect(node);
}

void Color::FreezeHelper(live::INodePtr node) {
  /// KH-note: it does nothing except remove all moves related to its target
  /// side-effects:
  /// - target can be simplified (ofcourse)
  /// - other nodes related to removed moves may be simplified or freezed
  std::list<std::tuple<live::INodePtr, live::INodePtr, live::INodePtr>> targets;
  for (auto & move : this->interf_moves->GetList()) {
    if (node == move.first) {
      targets.push_back({move.first, move.second, move.second});
    }
    else if (node == move.second) {
      targets.push_back({move.first, move.second, move.first});
    }
  }
  for (auto [src, dst, another] : targets) {
    this->remove_move(src, dst);
    if (another && another->Degree() < this->Kcolors) {
      if (this->is_move_related(another)) {
        this->freezeWorklist.push_back(another);
      }
      else {
        this->simplifyWorklist.push_back(another);
      }
    }
  }
  this->simplifyWorklist.push_back(node);
}

void Color::Simplify() {
  /// KH-note: check one elem
  live::INodePtr target = this->simplifyWorklist.front();
  this->simplifyWorklist.pop_front();
  if (this->is_move_related(target)) {
    return;
  }
  if (!this->interf_graph->Nodes()->Contain(target)) {
    return;
  }
  if (target->Degree() >= this->Kcolors) {
    return;
  }
  if (this->precolorList->Contain(target)) {
    return;
  }

  this->SimpSpillHelper(target);
}

void Color::Coalesce() {
  auto [target_src, target_dst] = this->coalesceWorklist.front();
  this->coalesceWorklist.pop_front();
  
  if (!(this->interf_graph->Nodes()->Contain(target_src) 
      && this->interf_graph->Nodes()->Contain(target_dst))) {
    return;
  }
  auto & src_moves = this->move_node_map[target_src].succ;
  if (std::find(src_moves.begin(), src_moves.end(), target_src) == src_moves.end()) {
    /// KH-note: check if the move is valid
    return;
  }
  if (target_src->GoesTo(target_dst)) {
    /// KH-note: src and dst must not interfere
    return;
  }
  if (this->precolorList->Contain(target_src)) {
    return;
  }

  /// KH-note: check if it fits any coalesce law
  bool can_coalesce = true;
  /// KH-note: George law - for any t<->src, t<->dst
  for (auto t : target_src->Pred()->GetList()) {
    if (!t->GoesTo(target_dst)) {
      can_coalesce = false;
      break;
    }
  }
  /// KH-note: Briggs law - coalesced node has at most K-1 edges
  if (target_dst->Pred()->Union(target_src->Pred())->GetList().size() < this->Kcolors) {
    can_coalesce = true;
  }
  if (!can_coalesce) {
    return;
  }

  /// KH-note: do coalesce, detach src and merge to dst.
  /// merge interfere relations first
  for (auto adj_src : target_src->Pred()->GetList()) {
    adj_src->Pred()->DeleteNode(target_src);
    adj_src->Succ()->DeleteNode(target_src);
    if (!adj_src->GoesTo(target_dst)) {
      this->interf_graph->AddEdge(adj_src, target_dst);
      this->interf_graph->AddEdge(target_dst, adj_src);
    }
  }
  /// KH-note: then merge move relations
  for (auto dst : this->move_node_map[target_src].succ) {
    this->remove_move(target_src, dst);
    if (dst != target_dst) {
      this->append_move(target_dst, dst);
    }
  }
  for (auto src : this->move_node_map[target_src].pred) {
    this->remove_move(src, target_src);
    if (src != target_dst) {
      this->append_move(src, target_dst);
    }
  }
  /// KH-note: finally, set alias and remove src from RIG
  this->alias_table[target_src] = target_dst;
  this->interf_graph->Nodes()->DeleteNode(target_src);

  /// KH-note: handle side-effects
  /// - src's old neighboors' degree may decrease
  /// - dst's neighboors' degree won't increase, dst won't "become" to need a spill after coalesce
  /// - moves from and to src are redirected to dst, dst may have one more coalesce with src's srcs
  /// - dst itself may be simplified or freezed
  this->basic_side_effect(target_src);
  for (auto move : this->interf_moves->GetList()) {
    if (target_dst == move.first || target_dst == move.second) {
      this->coalesceWorklist.push_back(move);
    }
  }
  if (target_dst->Degree() < this->Kcolors) {
    if (this->is_move_related(target_dst)) {
      this->freezeWorklist.push_back(target_dst);
    }
    else {
      this->simplifyWorklist.push_back(target_dst);
    }
  }
}

void Color::Freeze() {
  live::INodePtr target = this->freezeWorklist.front();
  this->freezeWorklist.pop_front();
  if (!this->is_move_related(target)) {
    return;
  }
  if (!this->interf_graph->Nodes()->Contain(target)) {
    return;
  }
  if (target->Degree() >= this->Kcolors) {
    return;
  }
  
  this->FreezeHelper(target);
}

void Color::Spill() {
  /// KH-note: I'm busy for ddl, so no priority calculation here.
  live::INodePtr target = this->spillWorklist.front();
  this->spillWorklist.pop_front();
  if (!this->interf_graph->Nodes()->Contain(target)) {
    return;
  }
  if (target->Degree() < this->Kcolors) {
    return;
  }
  if (this->precolorList->Contain(target)) {
    return;
  }

  /// KH-note: remove a node from graph and remove all moves related to it
  /// actually spill := forced-simplify + freeze
  /// this has nothing to do with spillNodes, bcuz we can only know whether a spill
  ///   happens during Select(). Spilling is optimistic.
  this->FreezeHelper(target);
  this->SimpSpillHelper(target);
}

void Color::Select() {
  if (this->interf_graph->Nodes()->GetList().size() != this->precolorList->GetList().size()) {
    TigerLog("!!! Color Error - Select() before fully simplified !!!\n");
    for (auto nd : this->interf_graph->Nodes()->GetList()) {
      if (this->precolorList->Contain(nd)) {
        continue;
      }
      std::cerr << "NOTFOUND " << *(temp::Map::Name()->Look(nd->NodeInfo())) << std::endl;
    }
    return;
  }

  /// KH-note: use color id 0 as invalid
  std::vector<std::string *> color_strs = { nullptr };
  std::unordered_map<live::INodePtr, int> node_colors; 
  for (auto reg : this->precolorList->GetList()) {
    auto col = this->coloring->Look(reg->NodeInfo());
    if (col == nullptr) {
      TigerLog("!!! Color Error - not well pre-colored !!!\n");
      TigerLog("Related: " + *(temp::Map::Name()->Look(reg->NodeInfo())) + "\n");
      return;
    }
    node_colors[reg] = color_strs.size();
    color_strs.push_back(col);
  }

  while (!this->selectStack.empty()) {
    live::INodePtr top = this->selectStack.top();
    this->selectStack.pop();

    std::vector<bool> available_colors(this->Kcolors + 1, true);
    available_colors[0] = false;

    for (auto neigh : top->Pred()->GetList()) {
      auto col = node_colors[neigh];
      available_colors[col] = false;
    }

    int top_col = 0;
    for (auto i = 1; i <= this->Kcolors; i ++) {
      if (available_colors[i]) {
        top_col = i;
        break;
      }
    }

    if (!top_col) {
      /// KH-note: no available color, need actual spill
      this->spillNodes->Append(top);
    }
    else {
      node_colors[top] = top_col;
      this->coloring->Enter(top->NodeInfo(), color_strs[top_col]);
    }
  }
}

} // namespace col
