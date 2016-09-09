#pragma once

#pragma warning(push)

#pragma warning(disable: 4267 4800 4244 4291 4996 4141 4624 4996 )
#include <llvm/IR/Module.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/SourceMgr.h>

#include <llvm/IRReader/IRReader.h>

#pragma warning(pop)

class ModuleJitter
{
	llvm::LLVMContext mCtx;
	llvm::SMDiagnostic mDiagInfo;
	llvm::Module* mModule;
	std::unique_ptr<llvm::ExecutionEngine> mEE;

	std::unique_ptr<llvm::Module> CompileFromCxxFile(const char* fileName);
	bool CreateEE(std::unique_ptr<llvm::Module> module);
public:
	ModuleJitter(const char* cxxFileName);
	virtual ~ModuleJitter();

	bool IsOk() const;
	void* GetFunctionPtr(const char* funcName);
	llvm::ExecutionEngine* GetEE();
	llvm::Module* GetModule();
};

