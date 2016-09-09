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

//
// Actually Clang does have the same name mangling as MSVC, so used the exported class member functions just as it is.
//
class TestClass
{
public:
	__declspec(dllexport) TestClass() { std::cout << "Printout from TestClass::constuctor" << std::endl; }
	__declspec(dllexport) ~TestClass() { std::cout << "Printout from TestClass::desstuctor" << std::endl; }

	__declspec(dllexport) void Foo(float val) { std::cout << "Printout from TestClass::Foo : " << val << std::endl; }
};


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

		//
		// Here shows how to get the reflection from hosted C++ code by listing all structure types and dump their debug info.
		//
		auto pModule = module.GetModule();
		auto structList = pModule->getIdentifiedStructTypes();
		for (auto& st : structList) {
			std::cout << "=========================================================" << std::endl;
			std::cout << "Struct name : " << st->getStructName().data() << std::endl;
			auto elemList = st->elements();
			for (auto& elem : elemList) {
				elem->dump();
			}
		}


	}
	llvm::llvm_shutdown();
	
    return 0;
}

