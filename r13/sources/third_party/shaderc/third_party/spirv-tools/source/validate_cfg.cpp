// Copyright (c) 2015-2016 The Khronos Group Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and/or associated documentation files (the
// "Materials"), to deal in the Materials without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Materials, and to
// permit persons to whom the Materials are furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Materials.
//
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
//
// THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

#include "validate.h"

#include <cassert>

#include <algorithm>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "val/BasicBlock.h"
#include "val/Construct.h"
#include "val/Function.h"
#include "val/ValidationState.h"

using std::find;
using std::function;
using std::get;
using std::ignore;
using std::make_pair;
using std::numeric_limits;
using std::pair;
using std::set;
using std::string;
using std::tie;
using std::transform;
using std::unordered_map;
using std::unordered_set;
using std::vector;

using libspirv::BasicBlock;

namespace libspirv {

namespace {

using bb_ptr = BasicBlock*;
using cbb_ptr = const BasicBlock*;
using bb_iter = vector<BasicBlock*>::const_iterator;

struct block_info {
  cbb_ptr block;  ///< pointer to the block
  bb_iter iter;   ///< Iterator to the current child node being processed
};

/// Returns true if a block with @p id is found in the @p work_list vector
///
/// @param[in] work_list  Set of blocks visited in the the depth first traversal
///                       of the CFG
/// @param[in] id         The ID of the block being checked
///
/// @return true if the edge work_list.back().block->id() => id is a back-edge
bool FindInWorkList(const vector<block_info>& work_list, uint32_t id) {
  for (const auto b : work_list) {
    if (b.block->id() == id) return true;
  }
  return false;
}

/// @brief Depth first traversal starting from the \p entry BasicBlock
///
/// This function performs a depth first traversal from the \p entry
/// BasicBlock and calls the pre/postorder functions when it needs to process
/// the node in pre order, post order. It also calls the backedge function
/// when a back edge is encountered.
///
/// @param[in] entry      The root BasicBlock of a CFG
/// @param[in] successor_func  A function which will return a pointer to the
///                            successor nodes
/// @param[in] preorder   A function that will be called for every block in a
///                       CFG following preorder traversal semantics
/// @param[in] postorder  A function that will be called for every block in a
///                       CFG following postorder traversal semantics
/// @param[in] backedge   A function that will be called when a backedge is
///                       encountered during a traversal
/// NOTE: The @p successor_func and predecessor_func each return a pointer to a
/// collection such that iterators to that collection remain valid for the
/// lifetime
/// of the algorithm
void DepthFirstTraversal(const BasicBlock* entry,
                         get_blocks_func successor_func,
                         function<void(cbb_ptr)> preorder,
                         function<void(cbb_ptr)> postorder,
                         function<void(cbb_ptr, cbb_ptr)> backedge) {
  unordered_set<uint32_t> processed;

  /// NOTE: work_list is the sequence of nodes from the root node to the node
  /// being processed in the traversal
  vector<block_info> work_list;
  work_list.reserve(10);

  work_list.push_back({entry, begin(*successor_func(entry))});
  preorder(entry);
  processed.insert(entry->id());

  while (!work_list.empty()) {
    block_info& top = work_list.back();
    if (top.iter == end(*successor_func(top.block))) {
      postorder(top.block);
      work_list.pop_back();
    } else {
      BasicBlock* child = *top.iter;
      top.iter++;
      if (FindInWorkList(work_list, child->id())) {
        backedge(top.block, child);
      }
      if (processed.count(child->id()) == 0) {
        preorder(child);
        work_list.emplace_back(
            block_info{child, begin(*successor_func(child))});
        processed.insert(child->id());
      }
    }
  }
}

}  // namespace

vector<pair<BasicBlock*, BasicBlock*>> CalculateDominators(
    const vector<cbb_ptr>& postorder, get_blocks_func predecessor_func) {
  struct block_detail {
    size_t dominator;  ///< The index of blocks's dominator in post order array
    size_t postorder_index;  ///< The index of the block in the post order array
  };
  const size_t undefined_dom = postorder.size();

  unordered_map<cbb_ptr, block_detail> idoms;
  for (size_t i = 0; i < postorder.size(); i++) {
    idoms[postorder[i]] = {undefined_dom, i};
  }
  idoms[postorder.back()].dominator = idoms[postorder.back()].postorder_index;

  bool changed = true;
  while (changed) {
    changed = false;
    for (auto b = postorder.rbegin() + 1; b != postorder.rend(); ++b) {
      const vector<BasicBlock*>& predecessors = *predecessor_func(*b);
      // first processed/reachable predecessor
      auto res = find_if(begin(predecessors), end(predecessors),
                         [&idoms, undefined_dom](BasicBlock* pred) {
                           return idoms[pred].dominator != undefined_dom;
                         });
      if (res == end(predecessors)) continue;
      const BasicBlock* idom = *res;
      size_t idom_idx = idoms[idom].postorder_index;

      // all other predecessors
      for (const auto* p : predecessors) {
        if (idom == p) continue;
        if (idoms[p].dominator != undefined_dom) {
          size_t finger1 = idoms[p].postorder_index;
          size_t finger2 = idom_idx;
          while (finger1 != finger2) {
            while (finger1 < finger2) {
              finger1 = idoms[postorder[finger1]].dominator;
            }
            while (finger2 < finger1) {
              finger2 = idoms[postorder[finger2]].dominator;
            }
          }
          idom_idx = finger1;
        }
      }
      if (idoms[*b].dominator != idom_idx) {
        idoms[*b].dominator = idom_idx;
        changed = true;
      }
    }
  }

  vector<pair<bb_ptr, bb_ptr>> out;
  for (auto idom : idoms) {
    // NOTE: performing a const cast for convenient usage with
    // UpdateImmediateDominators
    out.push_back({const_cast<BasicBlock*>(get<0>(idom)),
                   const_cast<BasicBlock*>(postorder[get<1>(idom).dominator])});
  }
  return out;
}

void printDominatorList(const BasicBlock& b) {
  std::cout << b.id() << " is dominated by: ";
  const BasicBlock* bb = &b;
  while (bb->immediate_dominator() != bb) {
    bb = bb->immediate_dominator();
    std::cout << bb->id() << " ";
  }
}

#define CFG_ASSERT(ASSERT_FUNC, TARGET) \
  if (spv_result_t rcode = ASSERT_FUNC(_, TARGET)) return rcode

spv_result_t FirstBlockAssert(ValidationState_t& _, uint32_t target) {
  if (_.current_function().IsFirstBlock(target)) {
    return _.diag(SPV_ERROR_INVALID_CFG)
           << "First block " << _.getIdName(target) << " of funciton "
           << _.getIdName(_.current_function().id()) << " is targeted by block "
           << _.getIdName(_.current_function().current_block()->id());
  }
  return SPV_SUCCESS;
}

spv_result_t MergeBlockAssert(ValidationState_t& _, uint32_t merge_block) {
  if (_.current_function().IsBlockType(merge_block, kBlockTypeMerge)) {
    return _.diag(SPV_ERROR_INVALID_CFG)
           << "Block " << _.getIdName(merge_block)
           << " is already a merge block for another header";
  }
  return SPV_SUCCESS;
}

/// Update the continue construct's exit blocks once the backedge blocks are
/// identified in the CFG.
void UpdateContinueConstructExitBlocks(
    Function& function, const vector<pair<uint32_t, uint32_t>>& back_edges) {
  auto& constructs = function.constructs();
  // TODO(umar): Think of a faster way to do this
  for (auto& edge : back_edges) {
    uint32_t back_edge_block_id;
    uint32_t loop_header_block_id;
    tie(back_edge_block_id, loop_header_block_id) = edge;

    auto is_this_header = [=](Construct& c) {
      return c.type() == ConstructType::kLoop &&
             c.entry_block()->id() == loop_header_block_id;
    };

    for (auto construct : constructs) {
      if (is_this_header(construct)) {
        Construct* continue_construct =
            construct.corresponding_constructs().back();
        assert(continue_construct->type() == ConstructType::kContinue);

        BasicBlock* back_edge_block;
        tie(back_edge_block, ignore) = function.GetBlock(back_edge_block_id);
        continue_construct->set_exit(back_edge_block);
      }
    }
  }
}

/// Constructs an error message for construct validation errors
string ConstructErrorString(const Construct& construct,
                            const string& header_string,
                            const string& exit_string,
                            bool post_dominate = false) {
  string construct_name;
  string header_name;
  string exit_name;
  string dominate_text;
  if (post_dominate) {
    dominate_text = "is not post dominated by";
  } else {
    dominate_text = "does not dominate";
  }

  switch (construct.type()) {
    case ConstructType::kSelection:
      construct_name = "selection";
      header_name = "selection header";
      exit_name = "merge block";
      break;
    case ConstructType::kLoop:
      construct_name = "loop";
      header_name = "loop header";
      exit_name = "merge block";
      break;
    case ConstructType::kContinue:
      construct_name = "continue";
      header_name = "continue target";
      exit_name = "back-edge block";
      break;
    case ConstructType::kCase:
      construct_name = "case";
      header_name = "case block";
      exit_name = "exit block";  // TODO(umar): there has to be a better name
      break;
    default:
      assert(1 == 0 && "Not defined type");
  }
  // TODO(umar): Add header block for continue constructs to error message
  return "The " + construct_name + " construct with the " + header_name + " " +
         header_string + " " + dominate_text + " the " + exit_name + " " +
         exit_string;
}

spv_result_t StructuredControlFlowChecks(
    const ValidationState_t& _, const Function& function,
    const vector<pair<uint32_t, uint32_t>>& back_edges) {
  /// Check all backedges target only loop headers and have exactly one
  /// back-edge branching to it
  set<uint32_t> loop_headers;
  for (auto back_edge : back_edges) {
    uint32_t back_edge_block;
    uint32_t header_block;
    tie(back_edge_block, header_block) = back_edge;
    if (!function.IsBlockType(header_block, kBlockTypeLoop)) {
      return _.diag(SPV_ERROR_INVALID_CFG)
             << "Back-edges (" << _.getIdName(back_edge_block) << " -> "
             << _.getIdName(header_block)
             << ") can only be formed between a block and a loop header.";
    }
    bool success;
    tie(ignore, success) = loop_headers.insert(header_block);
    if (!success) {
      // TODO(umar): List the back-edge blocks that are branching to loop
      // header
      return _.diag(SPV_ERROR_INVALID_CFG)
             << "Loop header " << _.getIdName(header_block)
             << " targeted by multiple back-edges";
    }
  }

  // Check construct rules
  for (const Construct& construct : function.constructs()) {
    auto header = construct.entry_block();
    auto merge = construct.exit_block();

    // if the merge block is reachable then it's dominated by the header
    if (merge->reachable() &&
        find(merge->dom_begin(), merge->dom_end(), header) ==
            merge->dom_end()) {
      return _.diag(SPV_ERROR_INVALID_CFG)
             << ConstructErrorString(construct, _.getIdName(header->id()),
                                     _.getIdName(merge->id()));
    }
    if (construct.type() == ConstructType::kContinue) {
      if (find(header->pdom_begin(), header->pdom_end(), merge) ==
          merge->pdom_end()) {
        return _.diag(SPV_ERROR_INVALID_CFG)
               << ConstructErrorString(construct, _.getIdName(header->id()),
                                       _.getIdName(merge->id()), true);
      }
    }
    // TODO(umar):  an OpSwitch block dominates all its defined case
    // constructs
    // TODO(umar):  each case construct has at most one branch to another
    // case construct
    // TODO(umar):  each case construct is branched to by at most one other
    // case construct
    // TODO(umar):  if Target T1 branches to Target T2, or if Target T1
    // branches to the Default and the Default branches to Target T2, then
    // T1 must immediately precede T2 in the list of the OpSwitch Target
    // operands
  }
  return SPV_SUCCESS;
}

spv_result_t PerformCfgChecks(ValidationState_t& _) {
  for (auto& function : _.functions()) {
    // Check all referenced blocks are defined within a function
    if (function.undefined_block_count() != 0) {
      string undef_blocks("{");
      for (auto undefined_block : function.undefined_blocks()) {
        undef_blocks += _.getIdName(undefined_block) + " ";
      }
      return _.diag(SPV_ERROR_INVALID_CFG)
             << "Block(s) " << undef_blocks << "\b}"
             << " are referenced but not defined in function "
             << _.getIdName(function.id());
    }

    // Prepare for dominance calculations.  We want to analyze all the
    // blocks in the function, even in degenerate control flow cases
    // including unreachable blocks.  For this calculation, we create an
    // agumented CFG by adding a pseudo-entry node that is considered the
    // predecessor of each source in the original CFG, and a pseudo-exit
    // node that is considered the successor to each sink in the original
    // CFG.  The augmented CFG is guaranteed to have a single source node
    // (i.e. not having predecessors) and a single exit node (i.e. not
    // having successors).  However, there might be isolated strongly
    // connected components that are not reachable by following successors
    // from the pseudo entry node, and not reachable by following
    // predecessors from the pseudo exit node.

    auto* pseudo_entry = function.pseudo_entry_block();
    auto* pseudo_exit = function.pseudo_exit_block();
    // We need vectors to use as the predecessors (in the augmented CFG)
    // for the source nodes of the original CFG.  It must have a stable
    // address for the duration of the calculation.
    auto* pseudo_entry_vec = function.pseudo_entry_blocks();
    // Similarly, we need a vector to be used as the successors (in the
    // augmented CFG) for sinks in the original CFG.
    auto* pseudo_exit_vec = function.pseudo_exit_blocks();
    // Returns the predecessors of a block in the augmented CFG.
    auto augmented_predecessor_fn = [pseudo_entry, pseudo_entry_vec](
        const BasicBlock* block) {
      auto predecessors = block->predecessors();
      return (block != pseudo_entry && predecessors->empty()) ? pseudo_entry_vec
                                                              : predecessors;
    };
    // Returns the successors of a block in the augmented CFG.
    auto augmented_successor_fn = [pseudo_exit,
                                   pseudo_exit_vec](const BasicBlock* block) {
      auto successors = block->successors();
      return (block != pseudo_exit && successors->empty()) ? pseudo_exit_vec
                                                           : successors;
    };

    // Set each block's immediate dominator and immediate postdominator,
    // and find all back-edges.
    vector<const BasicBlock*> postorder;
    vector<const BasicBlock*> postdom_postorder;
    vector<pair<uint32_t, uint32_t>> back_edges;
    if (!function.ordered_blocks().empty()) {
      /// calculate dominators
      DepthFirstTraversal(pseudo_entry, augmented_successor_fn, [](cbb_ptr) {},
                          [&](cbb_ptr b) { postorder.push_back(b); },
                          [&](cbb_ptr from, cbb_ptr to) {
                            back_edges.emplace_back(from->id(), to->id());
                          });
      auto edges =
          libspirv::CalculateDominators(postorder, augmented_predecessor_fn);
      for (auto edge : edges) {
        edge.first->SetImmediateDominator(edge.second);
      }

      /// calculate post dominators
      DepthFirstTraversal(pseudo_exit, augmented_predecessor_fn, [](cbb_ptr) {},
                          [&](cbb_ptr b) { postdom_postorder.push_back(b); },
                          [&](cbb_ptr, cbb_ptr) {});
      auto postdom_edges = libspirv::CalculateDominators(
          postdom_postorder, augmented_successor_fn);
      for (auto edge : postdom_edges) {
        edge.first->SetImmediatePostDominator(edge.second);
      }
    }
    UpdateContinueConstructExitBlocks(function, back_edges);

    // Check if the order of blocks in the binary appear before the blocks they
    // dominate
    auto& blocks = function.ordered_blocks();
    if (blocks.empty() == false) {
      for (auto block = begin(blocks) + 1; block != end(blocks); ++block) {
        if (auto idom = (*block)->immediate_dominator()) {
          if (idom != pseudo_entry &&
              block == std::find(begin(blocks), block, idom)) {
            return _.diag(SPV_ERROR_INVALID_CFG)
                   << "Block " << _.getIdName((*block)->id())
                   << " appears in the binary before its dominator "
                   << _.getIdName(idom->id());
          }
        }
      }
    }

    /// Structured control flow checks are only required for shader capabilities
    if (_.has_capability(SpvCapabilityShader)) {
      spvCheckReturn(StructuredControlFlowChecks(_, function, back_edges));
    }
  }
  return SPV_SUCCESS;
}

spv_result_t CfgPass(ValidationState_t& _,
                     const spv_parsed_instruction_t* inst) {
  SpvOp opcode = static_cast<SpvOp>(inst->opcode);
  switch (opcode) {
    case SpvOpLabel:
      spvCheckReturn(_.current_function().RegisterBlock(inst->result_id));
      break;
    case SpvOpLoopMerge: {
      uint32_t merge_block = inst->words[inst->operands[0].offset];
      uint32_t continue_block = inst->words[inst->operands[1].offset];
      CFG_ASSERT(MergeBlockAssert, merge_block);

      spvCheckReturn(
          _.current_function().RegisterLoopMerge(merge_block, continue_block));
    } break;
    case SpvOpSelectionMerge: {
      uint32_t merge_block = inst->words[inst->operands[0].offset];
      CFG_ASSERT(MergeBlockAssert, merge_block);

      spvCheckReturn(_.current_function().RegisterSelectionMerge(merge_block));
    } break;
    case SpvOpBranch: {
      uint32_t target = inst->words[inst->operands[0].offset];
      CFG_ASSERT(FirstBlockAssert, target);

      _.current_function().RegisterBlockEnd({target}, opcode);
    } break;
    case SpvOpBranchConditional: {
      uint32_t tlabel = inst->words[inst->operands[1].offset];
      uint32_t flabel = inst->words[inst->operands[2].offset];
      CFG_ASSERT(FirstBlockAssert, tlabel);
      CFG_ASSERT(FirstBlockAssert, flabel);

      _.current_function().RegisterBlockEnd({tlabel, flabel}, opcode);
    } break;

    case SpvOpSwitch: {
      vector<uint32_t> cases;
      for (int i = 1; i < inst->num_operands; i += 2) {
        uint32_t target = inst->words[inst->operands[i].offset];
        CFG_ASSERT(FirstBlockAssert, target);
        cases.push_back(target);
      }
      _.current_function().RegisterBlockEnd({cases}, opcode);
    } break;
    case SpvOpKill:
    case SpvOpReturn:
    case SpvOpReturnValue:
    case SpvOpUnreachable:
      _.current_function().RegisterBlockEnd({}, opcode);
      break;
    default:
      break;
  }
  return SPV_SUCCESS;
}
}  // namespace libspirv
