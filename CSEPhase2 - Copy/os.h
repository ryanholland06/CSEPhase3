#ifndef OS_H
#define OS_H

#include <fstream>
#include <list>
#include <queue>
#include <string>

#include "VirtualMachine.h"
#include "Assembler.h"

using namespace std;

class OS;

class PCB {
	int pc=0;
	int r[4] = {0};
	int ir=0;
	int sr=0;
	int sp = 256;
	int base= 0;
	int limit=0;

	string prog;
	fstream in;
	fstream out;
	fstream stack;
	fstream objectCode;

	int io_completion;

	int turn_around_time;
	int cpu_time;
	int wait_time; int wait_time_begin;
	int io_time;   int io_time_begin;
public:
	PCB(const string &p, const int &b, const int &l) :
		prog(p), base(b), limit(l), pc(b), sr(2), sp(256/*vm.msize*/),
		cpu_time(0), wait_time(0), io_time(0), wait_time_begin(0) { }
	friend
		class OS;
};

class OS {
	VirtualMachine vm;
	Assembler as;


	list<PCB *> pcb; // jobs
	queue<PCB *> readyQ;
	queue<PCB *> waitQ;
	PCB * running;
	const static int TIME_SLICE = 15;
	const static int CONTEXT_SWITCH_TIME = 5;

	ifstream progs;
	//int idle_time;
	//int sys_time; // total sys time
public:
	OS();
	//~OS();
	void loadStack(fstream&);
	void run();
	void loadState();
	void saveState();
	void contextSwitch();
};

#endif
