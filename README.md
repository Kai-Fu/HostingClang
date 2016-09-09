HostingClang
=======

I assume you know at least Clang is a C++ compiler, otherwise you are wasting time here.

If you have experience of developing software with embedded scripting language support like Python and Lua, you probably know what I mean by hosting Clang?

Yes, what I’m doing in this tutorial-like project here is to demonstrate the ability of making C++ an embedded language for the application you are developing. 

Python and Lua are natively supporting being embedded since they are open source and their interpreters are portable to most platforms. 
Recently C# becomes popular for embedded game play language because of Unity3D. You probably know Unity3D is built on the top of Mono as the cross-platform JIT engine of C# language.

Now what about C++? 

With Clang and LLVM infrastructure, not only does it become possible, but brings you the rich assets in C++ code as well the exciting performance advantages over those script languages.

I know there are many people who did the simliar things in various alternative methods. This project is meant to make it KISS(keep it simple and stupid).

Before the born of LLVM, I experienced using Tiny C Compiler(http://bellard.org/tcc/) and hijacking GNU compiler source code to achieve JIT-ing C/C++ source code, but it was not happy experience because of either lacking of x64 support or messup in understanding the code.

I also tried using compiler as the external tools to generate DLL/SO files and dynamically load them. But barely does it look attractive since I cannot participate any processes in the compilation and linking process, which means no reflections of data types and no linking to customized symbols.




