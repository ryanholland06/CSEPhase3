#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <vector>
#include <fstream>

using namespace std;

class VirtualMachine {
	int msize;
	int rsize;
	int pc, ir, sr, sp, clock;
	vector<int> mem;
	vector<int> r;
	int base, limit;
	int total_limit;
public:
	VirtualMachine() : msize(256), rsize(4), clock(0) { mem.reserve(msize); r.reserve(rsize); }
	void load(fstream&, int base, int & limit);
	void run(int, fstream &, fstream &);
	friend
		class OS;
}; // VirtualMachine

#endif