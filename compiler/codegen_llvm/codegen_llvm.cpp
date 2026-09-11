#include "../std.h"
#include "codegen_llvm.h"
#include "../ex.h"

Codegen_llvm::Codegen_llvm(std::ostream& out, bool debug) :Codegen(out, debug) {
}

void Codegen_llvm::enter(const std::string& l, int frameSize) {
	throw Ex("LLVM backend not implemented");
}

void Codegen_llvm::code(TNode* code) {
	throw Ex("LLVM backend not implemented");
}

void Codegen_llvm::leave(TNode* cleanup, int pop_sz) {
	throw Ex("LLVM backend not implemented");
}

void Codegen_llvm::label(const std::string& l) {
	throw Ex("LLVM backend not implemented");
}

void Codegen_llvm::i_data(int i, const std::string& l) {
	throw Ex("LLVM backend not implemented");
}

void Codegen_llvm::s_data(const std::string& s, const std::string& l) {
	throw Ex("LLVM backend not implemented");
}

void Codegen_llvm::p_data(const std::string& p, const std::string& l) {
	throw Ex("LLVM backend not implemented");
}

void Codegen_llvm::align_data(int n) {
	throw Ex("LLVM backend not implemented");
}

void Codegen_llvm::flush() {
}
