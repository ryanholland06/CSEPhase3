#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <fstream>
#include <vector>

using namespace std;

class VirtualMachine {
	int msize;
	int rsize;
	int sr = 2;
	int sp = 256;
	int pc, ir,   clock, programlines, context_switch_timer;
	int limitisize;
	int CPU_TIME;
	vector<int> mem;
	vector<int> r;
	int base, limit;
	friend class OS;
public:
	VirtualMachine() : msize(256), rsize(4), clock(0) { mem.reserve(msize); r.reserve(rsize); }
	void run( fstream&, fstream&, fstream&);
	void loadobjectcode(fstream&);
	int get_clock();
	string prog;
}; // VirtualMachine

#endif
