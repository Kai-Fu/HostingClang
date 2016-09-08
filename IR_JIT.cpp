#include "ModuleJitter.h"
#include <iostream>
// 
// These data type and function declarations must have the same data layout with those in C++ file being hosted.
// 
struct float3
{
	float x;
	float y;
	float z;
};
typedef float3 MyTestFuncFN(const float3& a, const float3& b);

//
// Export a function or variable from host side can make its symbol visible to MCJIT linker,
// which makes it accessibly in hosted C++ code.
//
extern "C" {
	__declspec(dllexport) void testExtFunc(float3& ref)
	{
		ref.x += 2.3f;
		std::cout << "Printout from exported function of hosting side : " << ref.y << std::endl;
	}
}

static void _On_LLVM_Fatal_Error(void* pData, const std::string& reason, bool gen_crash_diag)
{
	std::cout << reason << std::endl;
}

int main()
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	LLVMLinkInMCJIT();
	llvm::install_fatal_error_handler(_On_LLVM_Fatal_Error);

	{
		ModuleJitter module("test.cpp");
		if (!module.IsOk())
			return -1;

		MyTestFuncFN* pFN = nullptr;
		try {
			pFN = (MyTestFuncFN*)module.GetFunctionPtr("MyTestFunc");
		}
		catch (...) {
			return -1;
		}
		float3 a = { 1.23456f, 1.1f, 2.2f };
		float3 b = { 2.3f, 4.1f, 6.2f };
		auto c = pFN(a, b);

		printf("Printout from hosting C++ : %f\n", c.y);

	}
	llvm::llvm_shutdown();
	
    return 0;
}

