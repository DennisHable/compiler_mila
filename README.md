### Pascal-like Compiler (Mila)

A feature-rich compiler for a Pascal-like programming language (Mila) that parses source code, performs semantic analysis, and generates optimized LLVM IR. 

### Project Overview

* **Language Compatibility:** Pascal-like syntax (begin, end, for, downto, do, variable declarations).
* **Advanced Features:** Array structures, nested scopes, complex expression evaluation, and control flow management.
* **Architecture:** Modular Abstract Syntax Tree (AST) with decoupled LLVM Code Generation.
* **Implementation:** Modern C++ using std::unique_ptr for memory management and LLVM C++ API.

### Advanced Capabilities (What it can do)

### 1. Type System & Memory Management

* **Static Typing:** Implements a type system (VarType) supporting INTEGER, STRING, and VOID types.
* **Array Operations:** Full support for array read operations (ArrayIndexNode) and array element assignments (ArrayAssignNode).

### 2. Complex Expression & Condition Evaluation

* **Binary & Unary Operations:** Evaluates standard arithmetic binary operations alongside unary expressions (UnaryExpr).
* **Compound Conditions:** Parses and processes multi-layered conditional expressions (MultCondNode) combining multiple logical statements.
* **Control Flow Branching:** Robust conditional branching (If node) supporting both then and else code paths.

### 3. Loop & Flow Control

* **Loop Traversal:** Native handling of iteration loops (for and downto behaviors).
* **Control Breakpoints:** Explicit loop breaking and execution short-circuiting using BreakNode and ExitNode.

### Project Structure

* CMakeLists.txt - CMake configuration file
* main.cpp - Compiler entry point
* Lexer.hpp / Lexer.cpp - Lexical analyzer (Lexer)
* Parser.hpp / Parser.cpp - Syntax analyzer (Parser) and language grammar logic
* Ast_nodes.cpp - Definition of AST node classes (Expressions, Commands, Functions)
* Ast_gen.cpp - LLVM IR generation logic for individual AST nodes
* fce.c - Glue code providing C-runtime bindings for input/output (write, writeln, read)
* samples/ - Directory with sample .mila source files
* mila - Wrapper script for compiling source code into a binary

### Dependencies

* **Core:** LLVM including headers (Target version: LLVM 19 recommended).
* **Build Tools:** git, cmake, clang, and zlib1g-dev.

### Installation

**Ubuntu / Debian:** 

```bash
sudo apt install llvm llvm-dev clang git cmake zlib1g-dev
```

**macOS (via Homebrew):** 

```bash
brew install git cmake llvm gnu-getopt
```

### Building the Project

The compiler uses a standard CMake toolchain. To build in Debug mode, run: 

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### Running and Compiling Programs

Do not run the raw binary inside the build directory directly. Instead, use the provided mila wrapper script in the root directory. 

### 1. Create a Source File

Create a test file with your Pascal-like code: 

```bash
touch test.mila
```

### 2. Compile to Binary

Compile the .mila source file into an executable binary using the wrapper: 

```bash
./mila test.mila -o test.out
```

### 3. Run via Stdin (Alternative)

You can pipe the source code directly into the compiler binary to see the raw LLVM IR output on stdout: 

```bash
cat test.mila | ./build/mila
```

### Testing

Verification tests are implemented via ctest. 

You can run them from inside the build directory:

```bash
cd build
ctest
```

### Literature & Resources

* [LLVM Official Tutorial](https://llvm.org/docs/tutorial/) - Essential guide for AST to LLVM IR transformation.
* [Creating Your Own Language](https://mukulrathi.co.uk/create-your-own-programming-language/llvm-ir-cpp-api-tutorial/) - A great conceptual guide to the LLVM IR C++ API.
