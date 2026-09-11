#include "../codegen.h"

struct TNode;

class Codegen_llvm : public Codegen {
public:
	Codegen_llvm(std::ostream& out, bool debug);

	virtual void enter(const std::string& l, int frameSize);
	virtual void code(TNode* code);
	virtual void leave(TNode* cleanup, int pop_sz);
	virtual void label(const std::string& l);
	virtual void i_data(int i, const std::string& l);
	virtual void s_data(const std::string& s, const std::string& l);
	virtual void p_data(const std::string& p, const std::string& l);
	virtual void align_data(int n);
	virtual void flush();

private:
	void emitValue(TNode* t);
	void emitMem(TNode* t);
	void emitCall(TNode* t);
	void emitArith(TNode* t);
	void emitRelop(TNode* t);
	void emitJump(TNode* t);
};
