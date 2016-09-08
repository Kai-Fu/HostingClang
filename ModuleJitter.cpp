#include "ModuleJitter.h"

#include <windows.h>
#include <sstream>
#include <iostream>

//
// Invoke an executable file with absolute path and wait the spawned process finishes and return its result
//
static bool InvokeExecutable(const char* exeFileName, const char* args, int& result)
{
	std::ostringstream cmdLineStream;
	cmdLineStream << '\"' << exeFileName << '\"' << ' ';
	cmdLineStream << args;
	std::string cmdLineStr = cmdLineStream.str();

	STARTUPINFOA startInfo;
	PROCESS_INFORMATION procInfo;
	memset(&startInfo, 0, sizeof(startInfo));
	startInfo.cb = sizeof(startInfo);
	//startInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	BOOL ret = ::CreateProcessA(nullptr,
		&cmdLineStr[0],
		nullptr, nullptr,
		FALSE,
		0,//STARTF_USESTDHANDLES, // Creation flags
		nullptr,
		nullptr,  // Currently directory
		&startInfo,
		&procInfo
	);
	if (!ret)
		return false;

	::WaitForSingleObject(procInfo.hProcess, INFINITE);
	DWORD procRet = 0;
	::GetExitCodeProcess(procInfo.hProcess, &procRet);
	result = (int)procRet;
	::CloseHandle(procInfo.hProcess);
	::CloseHandle(procInfo.hThread);
	return true;
}

#define CLANG_EXE_NAME "clang.exe"

static std::string GetClangExecutablePath()
{
	std::string ret;
	char *customClangPath;
	size_t envLen = -1;
	errno_t err = _dupenv_s(&customClangPath, &envLen, "CLANG_EXE_PATH");
	if (envLen) {
		ret = std::string(customClangPath) + "/" + CLANG_EXE_NAME;
		free(customClangPath);
	}
	else {
		// This is windows default installation folder
		ret = "C:/Program Files/LLVM/bin/"  CLANG_EXE_NAME;
	}
	struct stat s;
	if (stat(ret.c_str(), &s) == 0) {
		if (s.st_mode & S_IFREG) {
			return std::move(ret);
		}
	}
	return "";
}

static std::string GenerateTempIRFileName()
{
	char tempPath[MAX_PATH];
	char tempFile[MAX_PATH];
	if (!::GetTempPathA(MAX_PATH, tempPath))
		return "";
	else {
		if (!::GetTempFileNameA(tempPath, nullptr, 0, tempFile))
			return "";
		else
			return tempFile;
	}
}


std::unique_ptr<llvm::Module> ModuleJitter::CompileFromCxxFile(const char * fileName)
{
	std::ostringstream argsStream;
	auto irFile = GenerateTempIRFileName();
	if (irFile.empty())
		return false;
	auto clangExe = GetClangExecutablePath();
	if (clangExe.empty())
		return false;

	// The command line to compile C++ source file to IR byte code should like:
	//    clang -S -emit-llvm test.cpp -o test.ll
	// Note here -S is needed since I cannot perform any linking time operation during generating LLVM IR code.
	argsStream << "-S -emit-llvm ";
	argsStream << fileName << " ";
	argsStream << "-o \"";
	argsStream << irFile << "\"";
	std::string argsStr = argsStream.str();

	int clangRet = -1;
	std::unique_ptr<llvm::Module> ret;
	if (InvokeExecutable(clangExe.c_str(), argsStr.c_str(), clangRet)) {
		// A successful process usually returns zero as the result
		if (clangRet == 0) {
			ret = llvm::parseIRFile(irFile.c_str(), mDiagInfo , mCtx);
		}
	}
	::DeleteFileA(irFile.c_str());
	return std::move(ret);
}

bool ModuleJitter::CreateEE(std::unique_ptr<llvm::Module> module)
{
	if (module.get() == nullptr)
		return false;
	mModule = module.get();
	std::unique_ptr<llvm::EngineBuilder> EB = std::make_unique<llvm::EngineBuilder>(std::move(module));
	llvm::SmallVector<std::string, 4> attrs;
	llvm::Triple targetTriple;
	// Append "-elf" to make MCJIT to generate ELF data in memory(Windows defaults to COFF)
	targetTriple.setTriple(llvm::sys::getProcessTriple() + "-elf");
	auto eeTarget = EB->selectTarget(targetTriple, "", "", attrs);
	mModule->setDataLayout(eeTarget->createDataLayout());
	mEE.reset(EB->create(eeTarget));
	return mEE.get() != nullptr;
}


ModuleJitter::ModuleJitter(const char* cxxFileName)
{
	CreateEE(CompileFromCxxFile(cxxFileName));
}


ModuleJitter::~ModuleJitter()
{
}

bool ModuleJitter::IsOk() const
{
	return mEE.get() != nullptr;
}

void* ModuleJitter::GetFunctionPtr(const char * funcName)
{
	return (void*)mEE->getFunctionAddress(funcName);;
}
