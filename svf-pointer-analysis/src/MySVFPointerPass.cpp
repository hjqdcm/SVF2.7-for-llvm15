// MySVFPointerPass.cpp - 使用 SVF WPA 分析指针变量
#include "SVF-LLVM/LLVMModule.h"
#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/BasicTypes.h"
#include "SVF-LLVM/SVFIRBuilder.h"

#include "SVFIR/SVFIR.h"
#include "WPA/Andersen.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace SVF;

namespace {
    class MySVFPointerPass : public ModulePass {
    private:
        SVFIR* svfir;
        Andersen* ander;

    public:
        static char ID;
        MySVFPointerPass() : ModulePass(ID), svfir(nullptr), ander(nullptr) {}

        bool runOnModule(Module &M) override {
            errs() << "\n=== SVF Pointer Analysis Pass ===\n";
            errs() << "Analyzing module: " << M.getName() << "\n";

            // 简单版本：先打印 LLVM IR 中的指针信息
            printSimplePointerInfo(M);

            // TODO: 添加完整的 SVF 分析
            /*
            // Step 1: Build SVF Module
            SVFModule* svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(M);

            // Step 2: Build SVFIR (SVF Intermediate Representation)
            SVFIRBuilder builder(svfModule);
            svfir = builder.build();

            // Step 3: Run Andersen's pointer analysis
            ander = AndersenWaveDiff::createAndersenWaveDiff(svfir);
            ander->analyze();

            // Step 4: Print pointer analysis results
            printPointerAnalysis();

            // Step 5: Clean up
            AndersenWaveDiff::releaseAndersenWaveDiff();
            SVFIR::releaseSVFIR();
            LLVMModuleSet::releaseLLVMModuleSet();
            */

            return false;
        }

        void printSimplePointerInfo(Module &M) {
            errs() << "\n=========== Simple Pointer Analysis ===========\n";

            // 分析全局变量中的指针
            errs() << "\n--- Global Pointer Variables ---\n";
            for (auto &G : M.globals()) {
                if (G.getType()->isPointerTy()) {
                    errs() << "Global Pointer: " << G.getName() << "\n";
                    errs() << "  Type: ";
                    G.getType()->print(errs());
                    errs() << "\n";
                }
            }

            // 分析函数中的指针
            for (auto &F : M) {
                errs() << "\n--- Function: " << F.getName() << " ---\n";

                // 分析指令
                for (auto &BB : F) {
                    for (auto &I : BB) {
                        if (I.getType()->isPointerTy()) {
                            errs() << "Pointer Instruction: ";
                            I.print(errs());
                            errs() << "\n";
                            errs() << "  Type: ";
                            I.getType()->print(errs());
                            errs() << "\n";
                        }

                        // 检查特定的指针操作
                        if (auto *Alloca = dyn_cast<AllocaInst>(&I)) {
                            if (Alloca->getAllocatedType()->isPointerTy()) {
                                errs() << "Allocated Pointer: ";
                                Alloca->print(errs());
                                errs() << "\n";
                                errs() << "  Allocated Type: ";
                                Alloca->getAllocatedType()->print(errs());
                                errs() << "\n";
                            }
                        } else if (auto *Load = dyn_cast<LoadInst>(&I)) {
                            if (Load->getType()->isPointerTy()) {
                                errs() << "Loaded Pointer: ";
                                Load->print(errs());
                                errs() << "\n";
                            }
                        } else if (auto *Store = dyn_cast<StoreInst>(&I)) {
                            if (Store->getValueOperand()->getType()->isPointerTy()) {
                                errs() << "Stored Pointer: ";
                                Store->print(errs());
                                errs() << "\n";
                            }
                        }
                    }
                }
            }
        }

        void printPointerAnalysis() {
            errs() << "\n=========== Pointer Variables Analysis ===========\n";

            // Iterate through all nodes in SVFIR
            for (auto it = svfir->begin(); it != svfir->end(); ++it) {
                SVFVar* node = it->second;

                // Get LLVM value
                const Value* val = LLVMModuleSet::getLLVMModuleSet()->getLLVMValue(node->getValue());
                if (val && val->getType()->isPointerTy()) {
                    errs() << "Pointer Node ID: " << node->getId() << "\n";
                    errs() << "  LLVM Value: ";
                    val->print(errs());
                    errs() << "\n";

                    // Print type information
                    errs() << "  Type: ";
                    val->getType()->print(errs());
                    errs() << "\n";

                    // Additional info
                    if (const Instruction* inst = dyn_cast<Instruction>(val)) {
                        errs() << "  Function: " << inst->getFunction()->getName() << "\n";
                    } else if (const GlobalVariable* gv = dyn_cast<GlobalVariable>(val)) {
                        errs() << "  Global Variable: " << gv->getName() << "\n";
                    }

                    // Print points-to information
                    printPointsToInfo(node);

                    errs() << "---------------------------------\n";
                }
            }

            // Statistics
            errs() << "\n=========== Statistics ===========\n";
            errs() << "Total Nodes: " << svfir->getPAGNodeNum() << "\n";
            errs() << "Value Nodes: " << svfir->getValueNodeNum() << "\n";
            errs() << "Object Nodes: " << svfir->getObjectNodeNum() << "\n";
        }

        void printPointsToInfo(SVFVar* node) {
            // Get points-to set for this node
            const PointsTo& pts = ander->getPts(node->getId());

            if (!pts.empty()) {
                errs() << "  Points-to: {";
                bool first = true;
                for (NodeID objId : pts) {
                    if (!first) errs() << ", ";
                    errs() << objId;
                    first = false;

                    // Try to get the LLVM value for the object
                    SVFVar* objNode = svfir->getGNode(objId);
                    if (objNode) {
                        const Value* objVal = LLVMModuleSet::getLLVMModuleSet()->getLLVMValue(objNode->getValue());
                        if (objVal) {
                            errs() << "(";
                            objVal->print(errs());
                            errs() << ")";
                        }
                    }
                }
                errs() << "}\n";
            } else {
                errs() << "  Points-to: {}\n";
            }
        }
    };
}

char MySVFPointerPass::ID = 0;
static RegisterPass<MySVFPointerPass> X(
    "svfpointeranalysis",
    "SVF Pointer Analysis Pass",
    false,
    false
);