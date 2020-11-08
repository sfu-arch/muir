#define DEBUG_TYPE "graph"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include <iostream>
#include <numeric>

#include "AliasEdgeWriter.h"
#include "Dandelion/Node.h"
#include "GraphGeneratorPass.h"

using namespace std;
using namespace llvm;
using namespace graphgen;
using namespace dandelion;

using InstructionList = std::list<InstructionNode>;
using ArgumentList    = std::list<ArgumentNode>;
using BasicBlockList  = std::list<SuperNode>;
using NodeList        = std::list<Node>;

extern cl::opt<string> target_fn;
extern cl::opt<string> config_path;

namespace graphgen {

char GraphGeneratorPass::ID = 0;

RegisterPass<GraphGeneratorPass> X("graphgen", "Generating graph pass");
}  // namespace graphgen

// Right now I couldn't move to C++17 because of tapir, so I used a simulated
// implementation of transform_reduce from C++17
// later on by moving to c++, this function is not needed anymore
template <class InputIt, class T, class BinaryOp, class UnaryOp>
T
transform_reduce(InputIt first, InputIt last, T init, BinaryOp binop, UnaryOp unary_op) {
  T generalizedSum = init;
  for (auto iter = first; iter != last; iter++) {
    generalizedSum = binop(unary_op(*iter), generalizedSum);
  }
  return generalizedSum;
}

template <typename Iter, typename Q>
void
push_range(Q& q, Iter begin, Iter end) {
  for (; begin != end; ++begin)
    q.push(*begin);
}

void inline findAllLoops(Loop* L, SetVector<Loop*>& Loops) {
  // Recursively find all subloops.
  for (Loop* SL : L->getSubLoops()) {
    findAllLoops(SL, Loops);
  }
  // Store current loop
  Loops.insert(L);
}

uint32_t
returnNumPred(BasicBlock* BB) {
  uint32_t c = 0;
  for (auto _bb_it : llvm::predecessors(BB)) {
    if (BB != _bb_it)
      c++;
  }
  return c;
}

/**
 * definedInCaller - Return true if the specified value is defined in the
 * function being code extracted, but not in the region being extracted. These
 * values must be passed in as live-ins to the function.
 */
bool
definedInCaller(const SetVector<BasicBlock*>& Blocks, Value* V) {
  if (isa<Argument>(V))
    return true;
  if (Instruction* I = dyn_cast<Instruction>(V)) {
    if (!Blocks.count(I->getParent()))
      return true;
  }
  return false;
}

/** definedInRegion - Return true if the specified value is defined in the
 * extracted region.
 */
bool
definedInRegion(const SetVector<BasicBlock*>& Blocks, Value* V) {
  if (Instruction* I = dyn_cast<Instruction>(V))
    if (Blocks.count(I->getParent()))
      return true;
  return false;
}

template <class T>
Instruction*
findParallelInstruction(Function& F) {
  for (auto& BB : F) {
    for (auto& I : BB) {
      if (isa<T>(I))
        return &I;
    }
  }
  return nullptr;
}

template <class T>
std::vector<T*>
getNodeList(Graph* _graph) {
  std::vector<T*> return_list;
  for (auto& _node : _graph->instructions()) {
    if (auto cast_node = dyn_cast<T>(&*_node))
      return_list.push_back(cast_node);
  }
  return return_list;
}

static SetVector<Loop*>
getLoops(LoopInfo& LI) {
  SetVector<Loop*> Loops;

  // iterate through top level loops. Store all subloops
  // and top level loop in Loops SetVector.
  for (auto& L : LI) {
    findAllLoops(L, Loops);
  }
  return Loops;
}

std::set<Loop*>
getOuterLoops(LoopInfo& LI) {
  std::set<Loop*> outer_loops;
  for (auto& L : getLoops(LI)) {
    if (L->getLoopDepth() == 1)
      outer_loops.insert(L);
  }
  return outer_loops;
}

void
UpdateLiveInConnections(Loop* _loop,
                        LoopNode* _loop_node,
                        std::map<llvm::Value*, Node*> map_value_node) {
  for (auto B : _loop->blocks()) {
    for (auto& I : *B) {
      // Detecting Live-ins
      for (auto OI = I.op_begin(); OI != I.op_end(); OI++) {
        Value* _value = *OI;

        if (definedInCaller(
                SetVector<BasicBlock*>(_loop->blocks().begin(), _loop->blocks().end()),
                _value)) {
          if (map_value_node.find(_value) == map_value_node.end())
            assert(!"Couldn't find the live-in source");
          if (map_value_node.find(&I) == map_value_node.end())
            assert(!"Couldn't find the live-in target");

          auto new_live_in =
              _loop_node->insertLiveInArgument(_value, ArgumentNode::LoopLiveIn);
          auto _src = map_value_node[_value];
          auto _tar = map_value_node[&I];

          new_live_in->setParentNode(_tar);

          // TODO later we need to get ride of these lines
          if (auto call_out = dyn_cast<CallNode>(_tar))
            _tar = call_out->getCallOut();
          if (auto call_in = dyn_cast<CallNode>(_src))
            _src = call_in->getCallIn();

          if (!_src->existDataOutput(new_live_in)) {
            _src->replaceDataOutputNode(_tar, new_live_in);
            _tar->replaceDataInputNode(_src, new_live_in);
            new_live_in->addDataInputPort(_src);
          } else {
            _src->removeNodeDataOutputNode(_tar);
            _tar->replaceDataInputNode(_src, new_live_in);
          }

          new_live_in->addDataOutputPort(_tar);
        }
      }
    }
  }
}

void
UpdateLiveOutConnections(Loop* _loop,
                         LoopNode* _loop_node,
                         std::map<llvm::Value*, Node*> map_value_node) {
  for (auto B : _loop->blocks()) {
    for (auto& I : *B) {
      /**
       * The function needs these steps:
       * 1) Detect Live-outs
       * 2) Insert a new Live-outs into loop header
       * 3) Update the dependencies
       */
      for (auto* U : I.users()) {
        if (!definedInRegion(
                SetVector<BasicBlock*>(_loop->blocks().begin(), _loop->blocks().end()),
                U)) {
          auto new_live_out =
              _loop_node->insertLiveOutArgument(U, ArgumentNode::LoopLiveOut);

          auto _src = map_value_node[&I];
          auto _tar = map_value_node[U];

          new_live_out->setParentNode(_tar);

          // TODO later we need to get ride of these lines
          if (auto call_out = dyn_cast<CallNode>(_tar))
            _tar = call_out->getCallOut();
          if (auto call_in = dyn_cast<CallNode>(_src))
            _src = call_in->getCallIn();

          _src->replaceDataOutputNode(_tar, new_live_out);
          _tar->replaceDataInputNode(_src, new_live_out);
          new_live_out->addDataInputPort(_src);

          new_live_out->addDataOutputPort(_tar);
        }
      }
    }
  }
}

void
UpdateInnerLiveInConnections(Loop* _loop,
                             std::map<llvm::Loop*, LoopNode*> loop_value_node,
                             std::map<llvm::Value*, Node*> map_value_node) {
  for (auto B : _loop->blocks()) {
    for (auto& I : *B) {
      for (auto OI = I.op_begin(); OI != I.op_end(); OI++) {
        Value* _value = *OI;

        if (definedInCaller(
                SetVector<BasicBlock*>(_loop->blocks().begin(), _loop->blocks().end()),
                _value)) {
          auto _loop_node        = loop_value_node[_loop];
          auto _parent_loop_node = loop_value_node[_loop->getParentLoop()];

          // TODO: This part needs to be re-thinked!
          auto new_live_in =
              _loop_node->insertLiveInArgument(_value, ArgumentNode::LoopLiveIn);
          new_live_in->setParentNode(map_value_node[_value]);

          Node* _src = nullptr;
          if (_parent_loop_node->findLiveInNode(_value) == nullptr)
            _src = map_value_node[_value];
          else
            _src = _parent_loop_node->findLiveInNode(_value);
          auto _tar = map_value_node[&I];

          // TODO later we need to get ride of these lines
          if (auto call_out = dyn_cast<CallNode>(_tar))
            _tar = call_out->getCallOut();
          if (auto call_in = dyn_cast<CallNode>(_src))
            _src = call_in->getCallIn();

          if (!_src->existDataOutput(new_live_in)) {
            _src->replaceDataOutputNode(_tar, new_live_in);
            _tar->replaceDataInputNode(_src, new_live_in);
            new_live_in->addDataInputPort(_src);
          } else {
            _src->removeNodeDataOutputNode(_tar);
            _tar->replaceDataInputNode(_src, new_live_in);
          }

          new_live_in->addDataOutputPort(_tar);
        }
      }
    }
  }
}

void
UpdateInnerLiveOutConnections(Loop* _loop,
                              std::map<llvm::Loop*, LoopNode*> loop_value_node,
                              std::map<llvm::Value*, Node*> map_value_node) {
  for (auto B : _loop->blocks()) {
    for (auto& I : *B) {
      auto _loop_node = loop_value_node[_loop];
      // auto _parent_loop_node = loop_value_node[_loop->getParentLoop()];

      // The function needs these steps:
      // 1) Detect Live-outs
      // 2) Insert a new Live-outs into loop header
      // 3) Update the dependencies
      for (auto* U : I.users()) {
        if (!definedInRegion(
                SetVector<BasicBlock*>(_loop->blocks().begin(), _loop->blocks().end()),
                U)) {
          auto new_live_out =
              _loop_node->insertLiveOutArgument(U, ArgumentNode::LoopLiveOut);

          auto _src = map_value_node[&I];
          auto _tar = map_value_node[U];

          new_live_out->setParentNode(_tar);

          // auto _tar = _parent_loop_node->findLiveOut(U);
          // Node * _tar = nullptr;
          // if(_parent_loop_node->findLiveOut(U) == nullptr)
          //_tar = map_value_node[U];
          // else
          //_tar = _parent_loop_node->findLiveOut(U);

          // TODO later we need to get ride of these lines
          if (auto call_out = dyn_cast<CallNode>(_tar))
            _tar = call_out->getCallOut();
          if (auto call_in = dyn_cast<CallNode>(_src))
            _src = call_in->getCallIn();

          _src->replaceDataOutputNode(_tar, new_live_out);
          _tar->replaceDataInputNode(_src, new_live_out);
          new_live_out->addDataInputPort(_src);
          new_live_out->addDataOutputPort(_tar);
        }
      }
    }
  }
}

/**
 * TODO: Right now we don't run loop simplify pass, because of cross checking
 * with our old output, but next step is to run simplify pass
 * because of the gauranties that the pass gives us
 */
LoopSummary
GraphGeneratorPass::summarizeLoop(Loop* L, LoopInfo& LI) {
  LoopSummary summary(
      "L_" + std::to_string(L->getStartLoc() ? L->getStartLoc()->getLine() : LID));
  LID++;

  // Check if loop is in a simplify form
  if (!L->isLoopSimplifyForm()) {
    errs() << "[llvm-clouser] Loop not in Simplify Form\n";
  }

  // Start filling loop summary
  // 1)Loop enable signal and backward edge these two are always uniqu,
  summary.header = L->getHeader();
  for (auto _bb_it : llvm::predecessors(L->getHeader())) {
    // Forward edge
    if (!L->contains(_bb_it))
      summary.enable = _bb_it->getTerminator();
    // Backward edge
    else
      summary.loop_back = _bb_it->getTerminator();
    blacklist_control_edge[_bb_it->getTerminator()].push_back(L->getHeader());
  }

  // 2)Looping exit branches
  L->getExitBlocks(summary.exit_blocks);
  for (auto _bb_it : summary.exit_blocks) {
    for (auto _bb_ex_it : llvm::predecessors(_bb_it)) {
      if (L->contains(_bb_ex_it)) {
        summary.loop_finish.insert(_bb_ex_it->getTerminator());
        blacklist_control_edge[_bb_ex_it->getTerminator()].push_back(_bb_it);
      }
    }
  }

  // 3)Get all the sub-loops
  for (auto SL : L->getSubLoops()) {
    findAllLoops(SL, summary.sub_loops);
  }

  // 4)Extracting all the live-ins
  for (auto& bb : L->blocks()) {
    for (auto& ins : *bb) {
      for (auto& op : ins.operands()) {
        bool is_live_in = isa<Argument>(&*op);
        if (auto _inst = dyn_cast<Instruction>(op))
          is_live_in = !L->contains(_inst) ? true : false;

        auto _intrinsic_op = CallSite(op);
        if (_intrinsic_op.getInstruction()) {
          auto _intrinsic_call =
              dyn_cast<Function>(_intrinsic_op.getCalledValue()->stripPointerCasts());

          if (_intrinsic_call) {
            if (_intrinsic_call->isDeclaration()) {
              is_live_in = false;
            }
          }
        }

        // If the value is live-in, we need to check if
        // subloops contain the instruction or not (not the operand).
        if (is_live_in) {
          bool contain = false;
          contain      = transform_reduce(
              L->getSubLoopsVector().begin(),
              L->getSubLoopsVector().end(),
              false,
              [](auto _l, auto _r) { return _l | _r; },
              [&ins, &op, &summary](auto _sub_l) {
                if (_sub_l->contains(&ins)) {
                  summary.live_in_out_loop[op].insert(_sub_l);
                  return true;
                } else
                  return false;
              });

          if (!contain) {
            summary.live_in_out_ins[op].insert(&ins);
          }

          if (L->getParentLoop()) {
            if (!isa<llvm::Argument>(op)) {
              if (L->getParentLoop()->contains(dyn_cast<llvm::Instruction>(op))) {
                summary.live_in_in_ins.insert(op);
              }
            }
          } else
            summary.live_in_in_ins.insert(op);

          // Adding to data edge blacklist
          blacklist_loop_live_in_data_edge[op].push_back(&ins);
        }
      }
    }
  }

  // Extracting loop live-outs
  for (auto B : L->blocks()) {
    for (auto& ins : *B) {
      // Detecting Live-outs
      for (auto user : ins.users()) {
        bool is_live_out = false;
        auto _inst_user  = dyn_cast<Instruction>(user);
        is_live_out      = !L->contains(_inst_user) ? true : false;

        if (is_live_out) {
          // Add connections between loops
          bool contain = false;
          contain      = transform_reduce(
              L->getSubLoopsVector().begin(),
              L->getSubLoopsVector().end(),
              false,
              [](auto _l, auto _r) { return _l | _r; },
              [&ins, &summary](auto _sub_l) {
                if (_sub_l->contains(&ins)) {
                  summary.live_out_in_loop[&ins].insert(_sub_l);
                  return true;
                } else
                  return false;
              });

          if (!contain) {
            summary.live_out_in_ins.insert(&ins);
          }

          // If current loop doesn't have a parent loop or the parent
          // node contains the user then there is a edge from loop to
          // the user instruction
          auto _parent_loop = L->getParentLoop();
          if (_parent_loop) {
            if (_parent_loop->contains(_inst_user))
              summary.live_out_out_ins[&ins].insert(_inst_user);
            else
              summary.live_out_out_loop[&ins].insert(_parent_loop);
          } else
            summary.live_out_out_ins[&ins].insert(_inst_user);

          blacklist_loop_live_out_data_edge[&ins].push_back(dyn_cast<Instruction>(user));
        }
      }
    }
  }

  for (auto B : L->blocks()) {
    for (auto& ins : *B) {
      // Detecting Live-ins
      for (auto user : ins.users()) {
        // Checking for carry loop dependency
        bool is_carry   = false;
        bool fail       = false;
        auto _inst_user = dyn_cast<Instruction>(user);

        for (auto _p_B : L->blocks()) {
          for (auto& _p_ins : *_p_B) {
            if (&_p_ins == &ins) {
              fail = true;
              break;
            }
            if (&_p_ins == user) {
              is_carry = true;
              break;
            }
          }
          if (fail || is_carry)
            break;
        }

        if (is_carry) {
          // Now we have to check if user instruction doesn't belong
          // to the subloops
          bool contain = transform_reduce(
              L->getSubLoopsVector().begin(),
              L->getSubLoopsVector().end(),
              false,
              [](auto _l, auto _r) { return _l | _r; },
              [&_inst_user, &summary, L](auto _sub_l) {
                return (_sub_l->contains(_inst_user));
              });

          if (!contain) {
            DEBUG(ins.dump());
            DEBUG(_inst_user->dump());
            summary.carry_dependencies[&ins].push_back(_inst_user);
            blacklist_carry_dependency_data_edge[&ins].push_back(
                dyn_cast<Instruction>(user));
            break;
          }
        }
      }
    }
  }

  return summary;
}

bool
GraphGeneratorPass::doInitialization(Module& M) {
  for (auto& F : M) {
    if (F.isDeclaration())
      continue;

    if (F.getName() == this->dependency_graph->graph_info.Name) {
      this->LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    }
  }
  // TODO: Add code here if it's needed before pas

  return false;
}

bool
GraphGeneratorPass::doFinalization(Module& M) {
  // TODO: Add code here to do post pass
  return false;
}

/**
 * Set pass dependencies
 */

/**
 * Iterating over target function's basicblocks and
 * then make supernode for each of them and add them to the node list
 */
void
GraphGeneratorPass::visitBasicBlock(BasicBlock& BB) {
  map_value_node[&BB] = this->dependency_graph->insertSuperNode(BB);
}

void
GraphGeneratorPass::visitInstruction(Instruction& Ins) {
  // Here we have to check see whether we have missed any instruction or not
  // TODO Couldn't figure out about Tapir visitor functions
  if (auto _detach_ins = dyn_cast<llvm::DetachInst>(&Ins))
    map_value_node[&Ins] = this->dependency_graph->insertDetachNode(*_detach_ins);
  else if (auto _reattach_ins = dyn_cast<llvm::ReattachInst>(&Ins))
    map_value_node[&Ins] = this->dependency_graph->insertReattachNode(*_reattach_ins);
  else if (auto _sync_ins = dyn_cast<llvm::SyncInst>(&Ins))
    map_value_node[&Ins] = this->dependency_graph->insertSyncNode(*_sync_ins);
  else {
    Ins.print(errs(), true);
    errs() << "\n";
    assert(!"Instruction is not supported");
  }
}

void
GraphGeneratorPass::visitFAdd(llvm::BinaryOperator& I) {
  map_value_node[&I] = this->dependency_graph->insertFaddNode(I);
}

void
GraphGeneratorPass::visitFSub(llvm::BinaryOperator& I) {
  map_value_node[&I] = this->dependency_graph->insertFsubNode(I);
}

void
GraphGeneratorPass::visitFMul(llvm::BinaryOperator& I) {
  map_value_node[&I] = this->dependency_graph->insertFmulNode(I);
}

void
GraphGeneratorPass::visitFDiv(llvm::BinaryOperator& I) {
  map_value_node[&I] = this->dependency_graph->insertFdiveNode(I);
}

void
GraphGeneratorPass::visitBinaryOperator(llvm::BinaryOperator& I) {
  map_value_node[&I] = this->dependency_graph->insertBinaryOperatorNode(I);
}

void
GraphGeneratorPass::visitICmpInst(llvm::ICmpInst& I) {
  map_value_node[&I] = this->dependency_graph->insertIcmpOperatorNode(I);
}

void
GraphGeneratorPass::visitBranchInst(llvm::BranchInst& I) {
  map_value_node[&I] = this->dependency_graph->insertBranchNode(I);
}

void
GraphGeneratorPass::visitPHINode(llvm::PHINode& I) {
  map_value_node[&I] = this->dependency_graph->insertPhiNode(I);
}

void
GraphGeneratorPass::visitSelectInst(llvm::SelectInst& I) {
  map_value_node[&I] = this->dependency_graph->insertSelectNode(I);
}

void
GraphGeneratorPass::visitFCmp(llvm::FCmpInst& I) {
  map_value_node[&I] = this->dependency_graph->insertFcmpNode(I);
}

void
GraphGeneratorPass::visitSExtInst(llvm::SExtInst& I) {
  map_value_node[&I] = this->dependency_graph->insertSextNode(I);
}

void
GraphGeneratorPass::visitZExtInst(llvm::ZExtInst& I) {
  map_value_node[&I] = this->dependency_graph->insertZextNode(I);
}

void
GraphGeneratorPass::visitTruncInst(llvm::TruncInst& I) {
  map_value_node[&I] = this->dependency_graph->insertTruncNode(I);
}

void
GraphGeneratorPass::visitSIToFPInst(llvm::SIToFPInst& I) {
  map_value_node[&I] = this->dependency_graph->insertSTIoFPNode(I);
}

void
GraphGeneratorPass::visitFPToUIInst(llvm::FPToUIInst& I) {
  map_value_node[&I] = this->dependency_graph->insertFPToUINode(I);
}

void
GraphGeneratorPass::visitAllocaInst(llvm::AllocaInst& I) {
  auto alloca_type = I.getAllocatedType();
  auto DL          = I.getModule()->getDataLayout();
  auto num_byte    = DL.getTypeAllocSize(alloca_type);
  uint32_t size    = 1;

  if (alloca_type->isIntegerTy() || alloca_type->isArrayTy()) {
    auto alloca_node   = this->dependency_graph->insertAllocaNode(I, size, num_byte);
    map_value_node[&I] = alloca_node;
    memory_buffer_map[&I] =
        this->dependency_graph->createBufferMemory(alloca_node, size, num_byte);
  } else if (alloca_type->isPointerTy()) {
    auto alloca_node   = this->dependency_graph->insertAllocaNode(I, size, num_byte);
    map_value_node[&I] = alloca_node;
    memory_buffer_map[&I] =
        this->dependency_graph->createBufferMemory(alloca_node, size, num_byte);
    I.print(errs(), true);
    errs() << "Alloca is pointer\n";
    // assert(!"Don't support for this alloca");
  } else {
    I.print(errs(), true);
    assert(!"Don't support for this alloca");
  }
}

void
GraphGeneratorPass::visitGetElementPtrInst(llvm::GetElementPtrInst& I) {
  auto& gep_pass_ctx = getAnalysis<helpers::GepInformation>();

  if (gep_pass_ctx.GepAddress.count(&I) == 0) {
    I.dump();
    assert(!"No gep information");
  }

  map_value_node[&I] =
      this->dependency_graph->insertGepNode(I, gep_pass_ctx.GepAddress[&I]);
  if (map_value_node.count(&I) == 0) {
    I.dump();
    assert(!"Gep node is not picked up!!");
  }
}

void
GraphGeneratorPass::visitLoadInst(llvm::LoadInst& I) {
  map_value_node[&I] = this->dependency_graph->insertLoadNode(I);
}

void
GraphGeneratorPass::visitBitCastInst(llvm::BitCastInst& I) {
  for (auto ins : I.users()) {
    if (isa<CallInst>(&*ins)) {
      auto called =
          dyn_cast<Function>(CallSite(ins).getCalledValue()->stripPointerCasts());
      if (!called) {
        return;
      }
      if (called->isDeclaration()) {
        return;
      }
    }
  }
  map_value_node[&I] = this->dependency_graph->insertBitcastNode(I);
}

void
GraphGeneratorPass::visitStoreInst(llvm::StoreInst& I) {
  map_value_node[&I] = this->dependency_graph->insertStoreNode(I);
}

void
GraphGeneratorPass::visitReturnInst(llvm::ReturnInst& I) {
  map_value_node[&I] = this->dependency_graph->insertReturnNode(I);
}

void
GraphGeneratorPass::visitCallInst(llvm::CallInst& I) {
  if (auto _call = dyn_cast<CallInst>(&I)) {
    auto called =
        dyn_cast<Function>(CallSite(_call).getCalledValue()->stripPointerCasts());
    if (!called) {
      assert(!"Function pointer is not supported, please consider to remove the function pointers");
    }
    if (called->isDeclaration())
      assert(!"Library function call is not supported");
  }

  map_value_node[&I] = this->dependency_graph->insertCallNode(I);
}

void
GraphGeneratorPass::visitFunction(Function& F) {
  dependency_graph->setFunction(&F);

  // Here we make a graph
  // Filling function argument nodes
  for (auto& f_arg : F.args()) {
    map_value_node[&f_arg] = this->dependency_graph->getSplitCall()->insertLiveInArgument(
        &f_arg, ArgumentNode::LiveIn);
  }

  // Filling function global nodes
  for (auto& g_var : F.getParent()->getGlobalList()) {
    map_value_node[&g_var] = this->dependency_graph->insertFunctionGlobalValue(g_var);
  }
}

/**
 * In this function we iterate over each function argument and connect all
 * of
 * its
 * successors as a data input port.
 * If the input is constant value we find the value and make a ConstNode for
 * that value
 */
void
GraphGeneratorPass::findDataPorts(Function& F) {
  // Check wether we already have iterated over the instructions
  assert(map_value_node.size() > 0 && "Instruction map can not be empty!");

  // auto loop_context = &getAnalysis<loopclouser::LoopClouser>();

  // Connecting function arguments to the spliter
  for (auto _fun_arg_it = this->dependency_graph->funarg_begin();
       _fun_arg_it != this->dependency_graph->funarg_end();
       _fun_arg_it++) {
    auto _fun_arg_node = dyn_cast<ArgumentNode>(_fun_arg_it->get());
    auto _spliter      = this->dependency_graph->getSplitCall();

    _spliter->addDataOutputPort(_fun_arg_node);
    _fun_arg_node->addDataInputPort(_spliter);
  }

  for (auto ins_it = inst_begin(F); ins_it != inst_end(F); ++ins_it) {
    // Check if its debug function
    if (auto _call = dyn_cast<CallInst>(&*ins_it)) {
      auto called =
          dyn_cast<Function>(CallSite(_call).getCalledValue()->stripPointerCasts());
      if (!called) {
        continue;
      }

      // Skip debug function
      if (called->isDeclaration()) {
        continue;
      }
    }

    auto _node_src =
        this->map_value_node.find(&*ins_it);  // it should be Instruction node

    for (uint32_t c = 0; c < ins_it->getNumOperands(); ++c) {
      auto operand = ins_it->getOperand(c);

      // Skip debug functions
      if (auto _call = dyn_cast<CallInst>(operand)) {
        auto called =
            dyn_cast<Function>(CallSite(_call).getCalledValue()->stripPointerCasts());
        if (!called) {
          continue;
        }

        // Skip debug function
        if (called->isDeclaration()) {
          continue;
        }
      }

      // First operand of a call function is a pointer to itself
      if (llvm::isa<llvm::Function>(operand))
        continue;

      if (auto target_bb = dyn_cast<llvm::BasicBlock>(operand)) {
        // If target operand is basicblock it means the instruction
        // is
        // control instruction
        // 1) First find the basicblock node
        // 2) Add bb as a control output
        // 3) Add ins as a control input

        // First we check if the edges is listed as blacklist
        bool find = (find_if(blacklist_control_edge[&*ins_it].begin(),
                             blacklist_control_edge[&*ins_it].end(),
                             [operand](auto _b_edge) { return _b_edge == operand; })
                     != blacklist_control_edge[&*ins_it].end())
                        ? true
                        : false;
        if (find)
          continue;

        auto _node_dest = this->map_value_node.find(operand);  // it should be supernode
        assert(isa<SuperNode>(_node_dest->second)
               && "Destination node should be super node!");

        assert(isa<InstructionNode>(_node_src->second)
               && "Source node should be instruction node!");

        // We don't connect reattach node data dependency
        if (isa<ReattachNode>(_node_src->second))
          continue;

        auto _dst  = _node_dest->second;
        Node* _src = nullptr;
        if (isa<BranchNode>(_node_src->second)) {
          _src = dyn_cast<BranchNode>(_node_src->second);
          if (ins_it->getNumOperands() == 3) {
            // LLVM IR -> CBranch(cmpInput(0), trueDst(1),
            // falseDst(2))
            // XXX There is a bug in llvm IR
            // in CBranch first element in the IR actually is C
            // = 2
            // and second elemnt is C = 1
            auto _inst_branch = dyn_cast<BranchInst>(&*ins_it);
            auto _inst_operand =
                this->map_value_node.find(_inst_branch->getSuccessor(c - 1));
            if (c == 1) {
              // Handeling LLVM Bug
              dyn_cast<BranchNode>(_src)->addTrueBranch(_inst_operand->second);
              //_inst_operand->second);
            } else if (c == 2) {
              dyn_cast<BranchNode>(_src)->addFalseBranch(_inst_operand->second);
              //_inst_operand->second);
            }
          } else {
            // The node is Ubranch
            _src->addControlOutputPort(_dst, c);
          }

          // Get the actual contorl input port id
          uint32_t _id = 0;
          for (auto _pred : llvm::predecessors(target_bb)) {
            if (&*ins_it == _pred->getTerminator())
              break;
            else
              _id++;
          }
          _dst->addControlInputPortIndex(_src, _id);

        } else if (isa<DetachNode>(_node_src->second)) {
          // TODO fix the Detachnode connections
          _src = dyn_cast<DetachNode>(_node_src->second);
          _src->addControlOutputPort(_dst, c);
          _dst->addControlInputPort(_src);

        } else if (isa<ReattachNode>(_node_src->second)) {
          // TODO fix the Reattachnode connections
          _src = dyn_cast<ReattachNode>(_node_src->second);
          _src->addControlOutputPort(_dst, c);
          _dst->addControlInputPort(_src);

        } else if (isa<SyncNode>(_node_src->second)) {
          // TODO fix the Sync node connections
          _src = dyn_cast<SyncNode>(_node_src->second);
          _src->addControlOutputPort(_dst, c);
          _dst->addControlInputPort(_src);

        } else {
          assert(!"Wrong cast of control node!");
        }

      } else {
        // If the operand is constant we have to create a new node
        if (isa<llvm::AllocaInst>(&*ins_it))
          continue;

        Node* _const_node = nullptr;
        if (auto const_value = dyn_cast<llvm::ConstantInt>(operand)) {
          _const_node = this->dependency_graph->insertConstIntNode(*const_value);
          map_value_node[operand] = _const_node;

          /**
           * Constant values should get their enable signal from
           * their incoming BB
           */
          if (auto _phi_ins = dyn_cast<PHINode>(&*ins_it)) {
            DEBUG(operand->dump());
            DEBUG(dbgs() << "Index : " << c << "\n");
            DEBUG(_phi_ins->getIncomingBlock(c)->dump());

            _const_node->addControlInputPort(this->map_value_node[_phi_ins->getParent()]);

            this->map_value_node[_phi_ins->getParent()]->addControlOutputPort(
                _const_node);

            dyn_cast<SuperNode>(this->map_value_node[_phi_ins->getParent()])
                ->addconstIntNode(dyn_cast<ConstIntNode>(_const_node));

          } else {
            _const_node->addControlInputPort(this->map_value_node[ins_it->getParent()]);
            this->map_value_node[ins_it->getParent()]->addControlOutputPort(_const_node);
            dyn_cast<SuperNode>(this->map_value_node[ins_it->getParent()])
                ->addconstIntNode(dyn_cast<ConstIntNode>(_const_node));
          }

        } else if (auto const_value = dyn_cast<llvm::ConstantFP>(operand)) {
          DEBUG(errs() << PURPLE("[DEBUG] "));
          DEBUG(operand->dump());
          _const_node = this->dependency_graph->insertConstFPNode(*const_value);
          map_value_node[operand] = _const_node;

          /**
           * Constant values should get their
           * enable signal from
           * their incoming BB
           */
          if (auto _phi_ins = dyn_cast<PHINode>(&*ins_it)) {
            DEBUG(operand->dump());
            DEBUG(dbgs() << "Index : " << c << "\n");
            DEBUG(_phi_ins->getIncomingBlock(c)->dump());

            _const_node->addControlInputPort(this->map_value_node[_phi_ins->getParent()]);

            this->map_value_node[_phi_ins->getParent()]->addControlOutputPort(
                _const_node);

            dyn_cast<SuperNode>(this->map_value_node[_phi_ins->getParent()])
                ->addconstFPNode(dyn_cast<ConstFPNode>(_const_node));

          } else {
            _const_node->addControlInputPort(this->map_value_node[ins_it->getParent()]);
            this->map_value_node[ins_it->getParent()]->addControlOutputPort(_const_node);
            dyn_cast<SuperNode>(this->map_value_node[ins_it->getParent()])
                ->addconstFPNode(dyn_cast<ConstFPNode>(_const_node));
          }
        } else if (llvm::isa<llvm::UndefValue>(operand)) {
          // TODO define an undef node instead of uisng empty
          // const
          // node!
          _const_node             = this->dependency_graph->insertConstIntNode();
          map_value_node[operand] = _const_node;

          _const_node->addControlInputPort(this->map_value_node[ins_it->getParent()]);
          this->map_value_node[ins_it->getParent()]->addControlOutputPort(_const_node);
          dyn_cast<SuperNode>(this->map_value_node[ins_it->getParent()])
              ->addconstIntNode(dyn_cast<ConstIntNode>(_const_node));
        }

        auto _node_src  = this->map_value_node.find(operand);
        auto _node_dest = this->map_value_node.find(&*ins_it);

        auto _src = _node_src->second;
        auto _dst = _node_dest->second;

        if (_dst == nullptr)
          continue;
        if (_src == nullptr)
          continue;


        // TODO later we need to get ride of these lines
        if (auto call_out = dyn_cast<CallNode>(_node_dest->second))
          _dst = call_out->getCallOut();
        if (auto call_in = dyn_cast<CallNode>(_node_src->second))
          _src = call_in->getCallIn();

        // Live in
        bool find_live_in = false;
        for (auto _data_src : blacklist_loop_live_in_data_edge) {
          for (auto _data_edge : _data_src.getSecond()) {
            if ((_data_src.getFirst() == operand) && (_data_edge == &*ins_it)) {
              find_live_in = true;
            }
          }
        }
        if (find_live_in) {
          for (auto _loop_edge : live_in_loop_ins_edge) {
            auto _loop_node = this->loop_value_node[_loop_edge.getFirst()];

            auto _edge = find_if(
                _loop_edge.getSecond().begin(),
                _loop_edge.getSecond().end(),
                [operand, ins_it](auto _f_edge) {
                  return ((_f_edge.first == operand) && (_f_edge.second == &*ins_it));
                });

            if (_edge != _loop_edge.getSecond().end()) {
              auto _node_src = _loop_node->findLiveInNode(_edge->first);
              auto _node_tar = this->map_value_node[_edge->second];

              // TODO later we need to get ride of these lines
              if (auto call_out = dyn_cast<CallNode>(_node_tar))
                _node_tar = call_out->getCallOut();
              if (auto call_in = dyn_cast<CallNode>(_node_src))
                _node_src = call_in->getCallIn();

              _node_src->addDataOutputPort(_node_tar);
              _node_tar->addDataInputPort(_node_src);
            }
            //}
          }

          continue;
        }

        // Live out
        bool find_live_out = false;
        for (auto _data_src : blacklist_loop_live_out_data_edge) {
          for (auto _data_edge : _data_src.getSecond()) {
            if ((_data_src.getFirst() == operand) && (_data_edge == &*ins_it)) {
              find_live_out = true;
            }
          }
        }
        if (find_live_out) {
          for (auto _loop_edge : live_out_loop_ins_edge) {
            auto _loop_node = this->loop_value_node[_loop_edge.getFirst()];

            auto _edge = find_if(
                _loop_edge.getSecond().begin(),
                _loop_edge.getSecond().end(),
                [operand, ins_it](auto _f_edge) {
                  return ((_f_edge.first == operand) && (_f_edge.second == &*ins_it));
                });

            if (_edge != _loop_edge.getSecond().end()) {
              auto _node_src = _loop_node->findLiveOutNode(_edge->first);
              auto _node_tar = this->map_value_node[_edge->second];

              if (auto call_out = dyn_cast<CallNode>(_node_tar))
                _node_tar = call_out->getCallOut();
              if (auto call_in = dyn_cast<CallNode>(_node_src))
                _node_src = call_in->getCallIn();

              _node_src->addDataOutputPort(_node_tar);
              _node_tar->addDataInputPort(_node_src);
            }
          }

          continue;
        }

        // carry dependencies
        bool find_carry = false;
        for (auto _data_src : blacklist_carry_dependency_data_edge) {
          for (auto _data_edge : _data_src.getSecond()) {
            if ((_data_src.getFirst() == operand) && (_data_edge == &*ins_it))
              find_carry = true;
          }
        }
        if (find_carry) {
          auto _carry = loop_edge_map[std::make_pair(operand, &*ins_it)];

          _src->addDataOutputPort(_carry);
          _carry->addDataInputPort(_src);

          _carry->addDataOutputPort(_dst);
          _dst->addDataInputPort(_carry);

          continue;
        }

        if (_node_src == this->map_value_node.end()) {
          DEBUG(operand->print(errs(), true));
          DEBUG(errs() << "\n");
          DEBUG(ins_it->print(errs(), true));
          DEBUG(operand->dump());
          assert(!"The source instruction couldn't find!\n"
                  "[HINT] Look at the LLVM type it you don't have one to one mapping"
                  "between LLVM and your graph library");
        }

        if (_node_dest == this->map_value_node.end()) {
          DEBUG(operand->print(errs(), true));
          DEBUG(errs() << "\n");
          DEBUG(ins_it->print(errs(), true));
          assert(!"The destination instruction couldn't find!\n"
                  "[HINT] Look at the LLVM type it you don't have one to one mapping"
                  "between LLVM and your graph library");
        }

        // Make sure that _src and _dst pointing
        // to correct node
        if (_const_node)
          _src = _const_node;
        if (auto call_out = dyn_cast<CallNode>(_dst))
          _dst = call_out->getCallOut();
        if (auto call_in = dyn_cast<CallNode>(_src))
          _src = call_in->getCallIn();

        _src->addDataOutputPort(_dst);
        _dst->addDataInputPort(_src);
      }
    }

    // Connecting Load and Store nodes to Memory system
    // TODO: We need to decide how to connect the memory nodes
    // to the memory system
    auto node_inst = this->map_value_node.find(&*ins_it);
    if (node_inst->second == nullptr)
      continue;
    if (auto _ld_node = dyn_cast<LoadNode>(node_inst->second)) {
      // TODO right now we consider all the connections to the cache
      // or
      // regfile
      // We need a pass to trace the pointers
      auto load_inst = dyn_cast<LoadInst>(&*ins_it);
      if (auto gep_inst = dyn_cast<GetElementPtrInst>(load_inst->getPointerOperand())) {
        if (auto alloca = dyn_cast<AllocaInst>(gep_inst->getPointerOperand())) {
          auto scratchpad_mem = this->dependency_graph->returnScratchpadMem(alloca);
          scratchpad_mem->addReadMemoryReqPort(_ld_node);
          scratchpad_mem->addReadMemoryRespPort(_ld_node);
          _ld_node->addReadMemoryReqPort(scratchpad_mem);
          _ld_node->addReadMemoryRespPort(scratchpad_mem);
        } else {
          this->dependency_graph->getMemoryUnit()->addReadMemoryReqPort(_ld_node);
          this->dependency_graph->getMemoryUnit()->addReadMemoryRespPort(_ld_node);
          _ld_node->addReadMemoryReqPort(this->dependency_graph->getMemoryUnit());
          _ld_node->addReadMemoryRespPort(this->dependency_graph->getMemoryUnit());
        }
      } else {
        this->dependency_graph->getMemoryUnit()->addReadMemoryReqPort(_ld_node);
        this->dependency_graph->getMemoryUnit()->addReadMemoryRespPort(_ld_node);
        _ld_node->addReadMemoryReqPort(this->dependency_graph->getMemoryUnit());
        _ld_node->addReadMemoryRespPort(this->dependency_graph->getMemoryUnit());
      }
    }

    // Store node connection
    //
    else if (auto _st_node = dyn_cast<StoreNode>(node_inst->second)) {
      auto store_inst = dyn_cast<StoreInst>(&*ins_it);
      if (auto gep_inst = dyn_cast<GetElementPtrInst>(store_inst->getPointerOperand())) {
        if (auto alloca = dyn_cast<AllocaInst>(gep_inst->getPointerOperand())) {
          auto scratchpad_mem = this->dependency_graph->returnScratchpadMem(alloca);
          scratchpad_mem->addWriteMemoryReqPort(_st_node);
          scratchpad_mem->addWriteMemoryRespPort(_st_node);
          _st_node->addWriteMemoryReqPort(scratchpad_mem);
          _st_node->addWriteMemoryRespPort(scratchpad_mem);
        } else {
          this->dependency_graph->getMemoryUnit()->addWriteMemoryReqPort(_st_node);
          this->dependency_graph->getMemoryUnit()->addWriteMemoryRespPort(_st_node);
          _st_node->addWriteMemoryReqPort(this->dependency_graph->getMemoryUnit());
          _st_node->addWriteMemoryRespPort(this->dependency_graph->getMemoryUnit());
        }
      } else {
        this->dependency_graph->getMemoryUnit()->addWriteMemoryReqPort(_st_node);
        this->dependency_graph->getMemoryUnit()->addWriteMemoryRespPort(_st_node);
        _st_node->addWriteMemoryReqPort(this->dependency_graph->getMemoryUnit());
        _st_node->addWriteMemoryRespPort(this->dependency_graph->getMemoryUnit());
      }
    }

    // Alloca node connection to the stack allocator
    // else if (auto _alloca_node = dyn_cast<AllocaNode>(
    // this->map_value_node.find(&*ins_it)->second)) {

    // auto new_mem = this->dependency_graph->createBufferMemory();
    // memory_buffer_map.insert(std::make_pair(&*ins_it, new_mem));
    //}

    // Connection FPNode operations
    else if (auto _fpdiv_node = dyn_cast<FdiveOperatorNode>(node_inst->second)) {
      _fpdiv_node->setRouteID(this->dependency_graph->getFPUNode()->numReadMemReqPort());
      this->dependency_graph->getFPUNode()->addReadMemoryReqPort(_fpdiv_node);
      this->dependency_graph->getFPUNode()->addReadMemoryRespPort(_fpdiv_node);
      _fpdiv_node->addReadMemoryReqPort(this->dependency_graph->getFPUNode());
      _fpdiv_node->addReadMemoryRespPort(this->dependency_graph->getFPUNode());
    }

    // TODO: We need to have diveider as well and connect the diveder
    // node
    // like above!
  }
}

/**
 * This function has two tasks:
 * 1) Iterate over the basicblock's insturcitons and make a list of
 * instructions
 * 2) Make control dependnce edges
 */
void
GraphGeneratorPass::fillBasicBlockDependencies(Function& F) {
  // Find the entry basic block and connect it to the splitnode
  for (auto& BB : F) {
    // Find the entry basic block and connect it to the split node
    if (&BB == &F.getEntryBlock()) {
      auto _en_bb = dyn_cast<SuperNode>(this->map_value_node[&BB]);
      this->dependency_graph->getSplitCall()->addControlOutputPort(_en_bb);
      _en_bb->addControlInputPort(this->dependency_graph->getSplitCall());
    }
    // Here we check wether the basicblock is reached from multiple
    // places.
    // If it has been reached from multiple places it means it CAN have
    // a
    // value which is feeded from multiple places. But still we are not
    // sure
    // and we have to make sure that if there is PHI node whithin the
    // basicblock.
    if (auto _bb = dyn_cast<SuperNode>(this->map_value_node[&BB])) {
      for (auto& I : BB) {
        // Check if I is not a call to debug function
        if (auto _call = dyn_cast<CallInst>(&I)) {
          auto called =
              dyn_cast<Function>(CallSite(_call).getCalledValue()->stripPointerCasts());
          if (!called) {
            continue;
          }

          // Skip debug function
          if (called->isDeclaration())
            continue;
        }

        // Iterate over the basicblock's instructions
        auto instruction_node = this->map_value_node[&I];

        if (instruction_node == nullptr)
          continue;
        if (auto _ins = dyn_cast<InstructionNode>(this->map_value_node[&I])) {
          _bb->addInstruction(_ins);

          _ins->setParentNode(_bb);

          // TODO: Why we are skipping reattach node?
          // Because you don't need to make reattach node's
          // connecitons
          // they already exist.
          if (llvm::isa<ReattachNode>(_ins))
            continue;

          // Detect Phi instrucctions
          if (auto _phi_ins = dyn_cast<PhiSelectNode>(_ins)) {
            //_bb->setNodeType(SuperNode::Mask);
            _bb->addPhiInstruction(_phi_ins);
            _phi_ins->setParentNode(_bb);

            // Since we know that the BB has a PHI node,
            // we change the type to Mask
            _bb->setNodeType(SuperNode::Mask);
          }

          if (isa<llvm::CallInst>(I)) {
            auto _call_node = dyn_cast<CallNode>(this->map_value_node[&I]);

            _bb->addControlOutputPort(_call_node->getCallOut());
            _bb->addControlOutputPort(_call_node->getCallIn());
            _call_node->getCallOut()->addControlInputPort(_bb);
            _call_node->getCallIn()->addControlInputPort(_bb);
          } else {
            _bb->addControlOutputPort(_ins);
            _ins->addControlInputPort(_bb);
          }

        } else
          assert(!"The instruction is not visited!");
      }

    } else
      assert(!"The basicblock is not visited!");
  }
}

/**
 * This funciton iterates over function loops and generate loop nodes
 */
void
GraphGeneratorPass::updateLoopDependencies(llvm::LoopInfo& loop_info) {
  uint32_t c = 0;

  for (auto& L : getLoops(loop_info)) {
    // Getting list of loop's exit basicblock
    // Remember for loops can have multiple exit point
    auto _l_head = dyn_cast<SuperNode>(map_value_node[L->getHeader()]);

    SmallVector<BasicBlock*, 8> _exit_blocks;
    L->getExitBlocks(_exit_blocks);

    std::vector<std::pair<BasicBlock*, SuperNode*>> _l_exit_blocks;

    for (auto _l : _exit_blocks) {
      _l_exit_blocks.push_back(
          std::make_pair(_l, dyn_cast<SuperNode>(map_value_node[_l])));
    }

    // Change the type of loop head basic block
    // Since we know that there is a backedge
    //_l_head->setNodeType(SuperNode::SuperNodeType::LoopHead);

    // Try to find forward edge to the loop head basic block
    auto _src_forward_br_inst_it = *std::find_if(
        _l_head->inputControl_begin(),
        _l_head->inputControl_end(),
        [&L](auto const _node_it) {
          return !L->contains(dyn_cast<BranchNode>(_node_it.first)->getInstruction());
        });

    // Try to find the backedge to the loop head basic block
    auto _src_back_br_inst_it = *std::find_if(
        _l_head->inputControl_begin(),
        _l_head->inputControl_end(),
        [&L](auto const _node_it) {
          return L->contains(dyn_cast<BranchNode>(_node_it.first)->getInstruction());
        });

    std::list<SuperNode*> _list_exit;
    std::transform(_l_exit_blocks.begin(),
                   _l_exit_blocks.end(),
                   std::back_inserter(_list_exit),
                   [](auto _l_e) -> SuperNode* { return _l_e.second; });

    auto _new_loop =
        std::make_unique<LoopNode>(NodeInfo(c, "Loop_" + std::to_string(c)),
                                   dyn_cast<SuperNode>(map_value_node[L->getHeader()]),
                                   dyn_cast<SuperNode>(map_value_node[L->getLoopLatch()]),
                                   _list_exit);

    // Insert loop node
    auto _loop_node = this->dependency_graph->insertLoopNode(std::move(_new_loop));

    loop_value_node[&*L] = _loop_node;

    _loop_node->setInputControlLoopSignal(_src_forward_br_inst_it.first);
    _loop_node->setActiveOutputLoopSignal(_l_head);
    _l_head->replaceControlInputNode(_src_forward_br_inst_it.first, _loop_node);
    _src_forward_br_inst_it.first->replaceControlOutputNode(_l_head, _loop_node);

    for (auto _l : _list_exit) {
      _loop_node->setLoopEndEnable(_l);
    }

    // Connect the latch ending branch to loopNode
    if (auto _latch_br = dyn_cast<BranchNode>(_src_back_br_inst_it.first)) {
      _loop_node->addLoopBackEdge(_latch_br);
      _latch_br->addControlOutputPort(_loop_node);
      // Check wether branch instruction is conditional
      if (_latch_br->getInstruction()->getNumOperands() > 0) {
        if (dyn_cast<BranchInst>(_latch_br->getInstruction())->getSuccessor(0)
            == _l_head->getBasicBlock()) {
          _latch_br->output_predicate.push_back(
              std::make_pair(_loop_node, BranchNode::PredicateResult::True));
        } else {
          _latch_br->output_predicate.push_back(
              std::make_pair(_loop_node, BranchNode::PredicateResult::False));
        }
      }
    } else
      assert(!"Unexpected terminator!");

    // Connecting end branch to the loop end input
    for (auto _le : _list_exit) {
      auto _tar_exit_br_inst_it = *std::find_if(
          _le->inputControl_begin(), _le->inputControl_end(), [&L](auto const _node_it) {
            if (_node_it.first == nullptr)
              return false;

            if (dyn_cast<BranchNode>(_node_it.first))
              return L->contains(dyn_cast<BranchNode>(_node_it.first)->getInstruction());
            else
              return false;
          });

      // TODO I don't know why sometimes there is no ending
      // instruciton
      // add here, need to investigate latter
      if (_tar_exit_br_inst_it == *_le->inputControl_end()) {
        assert(!"Don't run loop-simplify pass optimization on LL file,"
                "we don't support this type of loop structure right now!\n");
      }

      DEBUG(dyn_cast<BranchNode>(_tar_exit_br_inst_it.first)->getInstruction()->dump());

      // Make the branch instruction as ending branch of the loop
      dyn_cast<BranchNode>(_tar_exit_br_inst_it.first)->setEndingLoopBranch();

      _loop_node->pushLoopExitLatch(_tar_exit_br_inst_it.first);
      _le->replaceControlInputNode(_tar_exit_br_inst_it.first, _loop_node);
      _tar_exit_br_inst_it.first->replaceControlOutputNode(_le, _loop_node);
    }

    // Increament loop counter
    c++;

  }  // Get loops

  for (auto& L : getLoops(loop_info)) {
    auto _loop_node = loop_value_node[L];

    // Filling loop containers
    for (auto B : L->blocks()) {
      if (!B->empty()) {
        _loop_node->pushSuperNode(dyn_cast<SuperNode>(map_value_node[B]));
        for (auto& I : *B) {
          _loop_node->pushInstructionNode(dyn_cast<InstructionNode>(map_value_node[&I]));
        }
      }
    }
  }

  for (auto& L : getOuterLoops(loop_info)) {
    auto _loop_node = loop_value_node[L];
    _loop_node->setOuterLoop();

    // This function should be called after filling the containers
    // always
    // Here we look for Store nodes and then connect them to their
    // ending branch instruction
    _loop_node->setEndingInstructions();

    for (auto _en_instruction : _loop_node->endings()) {
      // We look for the ending branch instruction of each store node
      _en_instruction->getInstruction();
      auto& _br_ins = map_value_node
          [&_en_instruction->getInstruction()->getParent()->getInstList().back()];

      // For now we connect all the store nodes within a loop
      // to their ending branch instruction
      // this condition can be ease down later on
      _en_instruction->addControlOutputPort(_br_ins);
      _br_ins->addControlInputPort(_en_instruction);
    }
  }

  for (auto& L : getOuterLoops(loop_info)) {
    // At this stage we know that outer loop dominante all other loops
    // therefore each live-in for subLoops is a live-in for the outer
    // loop as well.
    // We first connect all the live-ins to the outer loop
    // and then iteratively go trought the subloops and update the
    // connections.
    auto _loop_node = loop_value_node[&*L];
    UpdateLiveInConnections(L, _loop_node, map_value_node);
    UpdateLiveOutConnections(L, _loop_node, map_value_node);

    std::list<Loop*> _loop_list;

    for (auto _l : L->getSubLoops()) {
      _loop_list.push_back(_l);
    }

    while (!_loop_list.empty()) {
      auto _sub_loop = _loop_list.front();
      _loop_list.pop_front();

      UpdateInnerLiveInConnections(_sub_loop, loop_value_node, map_value_node);
      UpdateInnerLiveOutConnections(_sub_loop, loop_value_node, map_value_node);

      for (auto _tmp_sub : _sub_loop->getSubLoopsVector()) {
        _loop_list.push_back(_tmp_sub);
      }
    }
  }


  // Setting parent loops
  for (auto& L : getLoops(loop_info)) {
    auto _loop_node = loop_value_node[&*L];
    if (auto _parent_loop = L->getParentLoop()) {
      _loop_node->setParentLoop(loop_value_node[_parent_loop]);
    }
  }
}

void
GraphGeneratorPass::connectOutToReturn(Function& F) {
  for (auto& BB : F) {
    for (auto& I : BB) {
      if (isa<llvm::ReturnInst>(I)) {
        dependency_graph->setOutputNode(map_value_node[&I]);
      }
    }
  }
}

void
GraphGeneratorPass::connectParalleNodes(Function& F) {
  if (!findParallelInstruction<llvm::SyncInst>(F))
    return;
  auto _sync_node   = this->map_value_node[findParallelInstruction<llvm::SyncInst>(F)];
  auto _detach_node = this->map_value_node[findParallelInstruction<llvm::DetachInst>(F)];
  auto _reattach_node =
      this->map_value_node[findParallelInstruction<llvm::ReattachInst>(F)];

  _sync_node->addControlInputPort(_detach_node);
  _sync_node->addControlInputPort(_reattach_node);

  _detach_node->addControlOutputPort(_sync_node);
  _reattach_node->addControlOutputPort(_sync_node);
}

void
GraphGeneratorPass::connectingCalldependencies(Function& F) {
  auto call_instructions = getNodeList<CallNode>(this->dependency_graph.get());
  for (auto _call_node : call_instructions) {
    auto _ins      = _call_node->getInstruction();
    auto& _end_ins = _ins->getParent()->back();
    auto _end_node = map_value_node[&_end_ins];
    if (isa<ReattachNode>(_end_node) || isa<BranchNode>(_end_node)) {
      _end_node->addControlInputPort(_call_node->getCallIn());
      _call_node->getCallIn()->addControlOutputPort(_end_node);
    }
  }
}

/**
 * There is a limitation in forming the graph at this moment
 * this function makes sure, all the store's routeIDs are
 * assigned after load nodes
 */
void
GraphGeneratorPass::updateRouteIDs(Function& F) {
  auto cache   = this->dependency_graph->getMemoryUnit();
  uint32_t cnt = 0;
  DEBUG(dbgs() << "Enter route id update\n");
  DEBUG(dbgs() << "Cache input: " << cache->numReadMemReqPort() << "\n");
  for (auto load_mem : cache->read_req_range()) {
    dyn_cast<LoadNode>(load_mem.first)->setRouteID(cnt);
    cnt++;
    DEBUG(dbgs() << load_mem.first->getName() << "\n");
  }
  for (auto store_mem : cache->write_req_range()) {
    dyn_cast<StoreNode>(store_mem.first)->setRouteID(cnt);
    cnt++;
  }

  for (auto& mem : this->dependency_graph->scratchpads()) {
    uint32_t cnt = 0;
    for (auto load_mem : mem->read_req_range()) {
      dyn_cast<LoadNode>(load_mem.first)->setRouteID(cnt);
      cnt++;
    }
    for (auto store_mem : mem->write_req_range()) {
      dyn_cast<StoreNode>(store_mem.first)->setRouteID(cnt);
      cnt++;
    }
  }
}

// void GraphGeneratorPass::connectingAliasEdges(Function &F) {
// auto alias_context = &getAnalysis<aew::AliasEdgeWriter>();

void
GraphGeneratorPass::buildLoopNodes(Function& F, llvm::LoopInfo& loop_info) {
  uint32_t c_id = 0;
  for (auto& L : getLoops(loop_info)) {
    auto _new_loop =
        std::make_unique<LoopNode>(NodeInfo(c_id, "Loop_" + std::to_string(c_id)));
    c_id++;

    // Insert loop node
    auto _loop_node = this->dependency_graph->insertLoopNode(std::move(_new_loop));

    if (L->getCanonicalInductionVariable())
      _loop_node->setIndeuctionVariable(
          dyn_cast<InstructionNode>(map_value_node[L->getCanonicalInductionVariable()]));

    for (auto& bb : L->blocks()) {
      _loop_node->pushSuperNode(dyn_cast<SuperNode>(map_value_node[bb]));
    }

    loop_value_node[&*L] = _loop_node;

    auto summary = loop_sum[&*L];

    auto findPredecessorID = [](BasicBlock* _src, BasicBlock* _tar) {
      uint32_t _id = 0;
      for (auto _pred : llvm::predecessors(_tar)) {
        if (_src == _pred)
          return _id;
        _id++;
      }
      return _id;
    };

    // Connect enable
    this->map_value_node[summary.enable]->addControlOutputPort(_loop_node);
    _loop_node->setInputControlLoopSignal(this->map_value_node[summary.enable]);

    _loop_node->setActiveOutputLoopSignal(this->map_value_node[summary.header]);
    map_value_node[summary.header]->addControlInputPortIndex(
        _loop_node, findPredecessorID(summary.enable->getParent(), summary.header));

    // Connecting backedge
    if (summary.loop_back->getNumOperands() > 0) {
      if (dyn_cast<BranchInst>(summary.loop_back)->getSuccessor(0) == summary.header) {
        dyn_cast<BranchNode>(this->map_value_node[summary.loop_back])
            ->addTrueBranch(_loop_node);
      } else {
        dyn_cast<BranchNode>(this->map_value_node[summary.loop_back])
            ->addFalseBranch(_loop_node);
      }
      _loop_node->setInputControlLoopSignal(this->map_value_node[summary.loop_back]);
    } else {
      this->map_value_node[summary.loop_back]->addControlOutputPort(_loop_node);
      _loop_node->setInputControlLoopSignal(this->map_value_node[summary.loop_back]);
    }

    _loop_node->setActiveBackSignal(this->map_value_node[summary.header]);
    this->map_value_node[summary.header]->addControlInputPortIndex(
        _loop_node, findPredecessorID(summary.loop_back->getParent(), summary.header));

    // Connecting exit signals
    for (auto _l_f : summary.loop_finish) {
      if (_l_f->getNumOperands() > 0) {
        auto _f = std::find(summary.exit_blocks.begin(),
                            summary.exit_blocks.end(),
                            dyn_cast<BranchInst>(summary.loop_back)->getSuccessor(0));
        if (_f != summary.exit_blocks.end()) {
          dyn_cast<BranchNode>(this->map_value_node[summary.loop_back])
              ->addTrueBranch(_loop_node);
        } else {
          dyn_cast<BranchNode>(this->map_value_node[summary.loop_back])
              ->addFalseBranch(_loop_node);
        }
        _loop_node->setInputControlLoopSignal(this->map_value_node[summary.loop_back]);
      } else {
        this->map_value_node[_l_f]->addControlOutputPort(_loop_node);
        _loop_node->setInputControlLoopSignal(this->map_value_node[_l_f]);
      }
    }

    // Connecting loop exit signals
    for (auto _exit_b : summary.exit_blocks) {
      this->map_value_node[_exit_b]->addControlInputPort(_loop_node);
      _loop_node->setActiveExitSignal(this->map_value_node[_exit_b]);
    }

    // Connecting live-in values
    // Edge type 1
    for (auto _live_in : summary.live_in_in_ins) {
      auto _node = _loop_node->findLiveInNode(_live_in);
      if (_node == nullptr) {
        _node = _loop_node->insertLiveInArgument(_live_in, ArgumentNode::LoopLiveIn);
        dyn_cast<ArgumentNode>(_node)->setParentNode(map_value_node[_live_in]);
      }

      live_in_ins_loop_edge[_live_in].insert(L);
    }
    // Edge type 2
    for (auto _live_in : summary.live_in_out_loop) {
      auto _node = _loop_node->findLiveInNode(_live_in.getFirst());
      if (_node == nullptr) {
        auto new_loop_live_in = _loop_node->insertLiveInArgument(
            _live_in.getFirst(), ArgumentNode::LoopLiveIn);
        new_loop_live_in->setParentNode(map_value_node[_live_in.getFirst()]);
      }
      for (auto _loop_in : _live_in.getSecond()) {
        auto _tmp = this->loop_value_node[_loop_in]->findLiveInNode(_live_in.getFirst());
        if (_tmp == nullptr) {
          auto new_loop_live_in = this->loop_value_node[_loop_in]->insertLiveInArgument(
              _live_in.getFirst(), ArgumentNode::LoopLiveIn);
          new_loop_live_in->setParentNode(map_value_node[_live_in.getFirst()]);
        }

        live_in_loop_loop_edge[_live_in.getFirst()].insert(std::make_pair(L, _loop_in));
      }
    }

    // Edge type 3
    for (auto _src : summary.live_in_out_ins) {
      for (auto _tar : _src.getSecond()) {
        live_in_loop_ins_edge[L].insert(std::make_pair(_src.getFirst(), _tar));
      }
    }

    // Connecting live-outs values
    // Edge type 1
    for (auto _live_out : summary.live_out_in_ins) {
      auto _new_live_out_node = _loop_node->findLiveOutNode(_live_out);
      if (_new_live_out_node == nullptr) {
        _new_live_out_node =
            _loop_node->insertLiveOutArgument(_live_out, ArgumentNode::LoopLiveOut);
        dyn_cast<ArgumentNode>(_new_live_out_node)
            ->setParentNode(map_value_node[_live_out]);
      }

      live_out_ins_loop_edge[_live_out].insert(L);
    }

    // Edge type 2
    for (auto _live_out : summary.live_out_in_loop) {
      auto _node = _loop_node->findLiveOutNode(_live_out.getFirst());
      if (_node == nullptr) {
        auto _new_live_out_node = _loop_node->insertLiveOutArgument(
            _live_out.getFirst(), ArgumentNode::LoopLiveOut);
        dyn_cast<ArgumentNode>(_new_live_out_node)
            ->setParentNode(map_value_node[_live_out.getFirst()]);
      }
      for (auto _n : _live_out.getSecond()) {
        auto _tmp = this->loop_value_node[_n]->findLiveOutNode(_live_out.getFirst());
        if (_tmp == nullptr) {
          this->loop_value_node[_n]->insertLiveOutArgument(_live_out.getFirst(),
                                                           ArgumentNode::LoopLiveOut);
        }
        loop_loop_edge_lout_map[_live_out.getFirst()].push_back(std::make_pair(L, _n));
      }
    }

    // Edge type 3
    for (auto _live_out_edge : summary.live_out_out_ins) {
      for (auto _inst : _live_out_edge.getSecond()) {
        live_out_loop_ins_edge[L].insert(
            std::make_pair(_live_out_edge.getFirst(), _inst));
      }
    }

    // Connecting carry values
    for (auto _carry_depen : summary.carry_dependencies) {
      auto new_carry_depen =
          _loop_node->insertCarryDepenArgument(_carry_depen.getFirst(),
                                               map_value_node[_carry_depen.getFirst()],
                                               ArgumentNode::CarryDependency);
      for (auto _use : _carry_depen.getSecond()) {
        loop_edge_map[std::make_pair(_carry_depen.getFirst(), _use)] = new_carry_depen;
      }
    }
  }

  // Setting parent loops
  for (auto& L : getLoops(loop_info)) {
    auto _loop_node = loop_value_node[&*L];
    if (auto _parent_loop = L->getParentLoop()) {
      outs() << "FIND PARENT\n";
      _loop_node->setParentLoop(loop_value_node[_parent_loop]);
    }
  }
}

void
GraphGeneratorPass::connectLoopEdge() {
  // Connecting live-ins
  for (auto _edge : live_in_ins_loop_edge) {
    auto _node_src = this->map_value_node.find(_edge.first);
    if (_node_src == this->map_value_node.end()) {
      _edge.first->dump();
      assert(!"WRONG");
    }
    for (auto _tar : _edge.second) {
      auto _loop_dest = this->loop_value_node[_tar];
      auto _node_dest = _loop_dest->findLiveInNode(_edge.first);

      if (_node_dest == nullptr)
        assert(!"There is a bug in loop connections!");

      _node_src->second->addDataOutputPort(_node_dest);
      _node_dest->addDataInputPort(_node_src->second);
    }
  }

  for (auto _edge : live_in_loop_loop_edge) {
    for (auto _loop_edge : _edge.second) {
      auto _node_src =
          this->loop_value_node[_loop_edge.first]->findLiveInNode(_edge.first);
      auto _node_tar =
          this->loop_value_node[_loop_edge.second]->findLiveInNode(_edge.first);

      _node_src->addDataOutputPort(_node_tar);
      _node_tar->addDataInputPort(_node_src);
    }
  }

  // Connecting live-outs
  for (auto _edge : live_out_ins_loop_edge) {
    auto _node_src = this->map_value_node[_edge.first];
    for (auto _tar : _edge.second) {
      auto _loop_dest = this->loop_value_node[_tar];
      auto _node_dest = _loop_dest->findLiveOutNode(_edge.first);

      _node_src->addDataOutputPort(_node_dest);
      _node_dest->addDataInputPort(_node_src);
    }
  }

  for (auto _edge : live_out_loop_loop_edge) {
    for (auto _loop_edge : _edge.second) {
      auto _node_src =
          this->loop_value_node[_loop_edge.first]->findLiveOutNode(_edge.first);
      auto _node_tar =
          this->loop_value_node[_loop_edge.second]->findLiveOutNode(_edge.first);

      _node_src->addDataOutputPort(_node_tar);
      _node_tar->addDataInputPort(_node_src);
    }
  }
}

void
GraphGeneratorPass::connectingStoreToBranch(Function& F) {
  for (auto& BB : F) {
    for (auto& I : BB) {
      if (isa<llvm::StoreInst>(I)) {
        auto _store_node = map_value_node[&I];
        auto _br_inst    = --I.getParent()->getInstList().end();
        _store_node->addControlOutputPort(map_value_node[&*_br_inst]);
        map_value_node[&*_br_inst]->addControlInputPort(_store_node);
      }
    }
  }
}

/**
 * All the initializations for function members
 */
void
GraphGeneratorPass::init(Function& F) {
  // Running analysis on the elements
  buildLoopNodes(F, *LI);
  connectLoopEdge();
  // findControlPorts(F);
  findDataPorts(F);
  fillBasicBlockDependencies(F);
  // updateLoopDependencies(*LI);
  connectOutToReturn(F);
  connectParalleNodes(F);
  connectingCalldependencies(F);
  connectingStoreToBranch(F);
  // connectingAliasEdges(F);

  // Printing the graph
  dependency_graph->optimizationPasses();
  updateRouteIDs(F);
  dependency_graph->printGraph(PrintType::Scala, config_path);

  // Printing muIR graph summary
  if (this->dump_muir) {
    dependency_graph->printMUIR();
  }
}

bool
GraphGeneratorPass::runOnModule(Module& M) {
  for (auto& F : M) {
    if (F.isDeclaration())
      continue;
    if (F.getName() == this->dependency_graph->graph_info.Name) {
      // Iterating over loops, and extracting loops information
      // before running anyother analysis
      auto& LI   = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
      auto loops = getLoops(LI);

      bool hasLoop = (loops.size() > 0);
      do {
        if (loops.size() == loop_sum.size())
          break;

        for (auto& L : loops) {
          if (L->getSubLoops().size() == 0 || loop_sum.count(L) == 0) {
            this->loop_sum.insert(std::make_pair(L, summarizeLoop(L, LI)));
          }
        }

      } while (hasLoop);

      visit(F);


      // Injecting debug infos
      auto& debug_info_pass = getAnalysis<debuginfo::DebugInfo>();
      for (auto& bb : F) {
        for (auto& ins : bb) {
          auto inst_node_find = map_value_node.find(&ins);
          if (inst_node_find != map_value_node.end()) {
            auto inst_node = dyn_cast<InstructionNode>(inst_node_find->second);

            for (auto val : debug_info_pass.node_operands[&ins]) {
              inst_node->debug_parent_node.push_back(val);
            }
          }
        }
      }


      init(F);
    }
  }


  return false;
}
