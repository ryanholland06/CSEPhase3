#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstdlib>
#include "Assembler.h"
#include "VirtualMachine.h"
#include "os.h"
#include <stack>


using namespace std;

OS::OS()
{
	//read all .s
	//assemble each one
	//save .o 
	system("ls *.s > progs");
	string line;

	int base;
	int limit;
	int programcounter = 1;
	// temporary PCB for loading programs into mem
	ifstream assemble("progs");
	list<PCB *> jobs;
	getline(assemble, line);
	while (!assemble.eof())
	{
		string name = line.substr(0, (line.size() - 2));
		string assemblyFile = line;
		string objectFile = name + ".o";


		fstream assembly, objectCode;
		assembly.open(assemblyFile.c_str(), ios::in);
		objectCode.open(objectFile.c_str(), ios::out);
		if (!assembly.is_open() or !objectCode.is_open()) {
			cout << "Couldn't open " << assemblyFile << " and/or " << objectFile << endl;
			exit(3);
		}

		if (as.assemble(assembly, objectCode)) {
			cout << "Assembler Error\n";
			assembly.close();
			objectCode.close();
			exit(4);
		}
		assembly.close();
		objectCode.close();

		objectCode.open(objectFile.c_str(), ios::in);
		if (!objectCode.is_open()) {
			cout << "Couldn't open " << objectFile << endl;
			exit(5);
		}

		if (programcounter != 1)
			base = vm.programlines;
		else
			base = 0;

		vm.loadobjectcode(objectCode); // load object code into memory 
		limit = vm.limit;
		PCB *p = new PCB(name, base, limit); // create a pointer to the PCB we must keep track of the base and also limit as more programs come in
		jobs.push_back(p);
		// open filestreams for pcb
		string inFile = name + ".in";
		string outFile =  name + ".out";
		string stackFile = name + ".st";
		//fstream in, out, st;
		p->in.open(inFile.c_str(), ios::in);
		p->out.open(outFile.c_str(), ios::out);
		p->stack.open(stackFile.c_str(), ios::in|ios::out); // our file must be read and write
		if (!p->in.is_open() or !p->out.is_open() or !p->stack.is_open()) {
			cout << "Couldn't open " << inFile << " and/or " << outFile << "and/or" << stackFile << endl;
			exit(6);
		}
		getline(assemble, line);
		programcounter++;
	}
	//Set the readyQ
	list<PCB *>::iterator it; // We create an iterator in order to push the content from jobs into the readyQ
	it = jobs.begin();
	for(it; it != jobs.end(); it++)
		readyQ.push(*it);

	//intlize run
		run();
		
}

void OS::run()
{
	while (!readyQ.empty()) {
		running = readyQ.front();
		running->out.open((running->prog + ".out").c_str(), ios::out);
		readyQ.pop();
		loadState();
 		vm.run(running->in,running->out, running->stack);
		running->out.close();
			
		contextSwitch();
	}
	//load first program 
	//start running
	//save its state
	//load second pr
}
void OS::saveState()
{
	running->pc = vm.pc;
	running->r[0] = vm.r[0];
	running->r[1] = vm.r[1];
	running->r[2] = vm.r[2];
	running->r[3] = vm.r[3];
	running->ir = vm.ir;
	running->sr = vm.sr;
	running->sp = vm.sp;
	running->base = vm.base;
	running->limit = vm.limit;
	if (vm.sp < 256)
	{
		ofstream progstack;
		progstack.open(("./" + running->prog + ".st").c_str(), ios::in); // open the .st file and assign all vm stack into .st file
		if (progstack.is_open())
		{
			for (int i = vm.sp; i < 256; i++)
			{
				progstack << vm.mem[i];
				progstack << "\r\n";
			}
		}
	}
}

void OS::contextSwitch() 
{
	vm.clock += CONTEXT_SWITCH_TIME; // each context switch costs 5 clock ticks
	saveState();
	if (vm.sr != 0177477)
		readyQ.push(running);
}

void OS::loadState()
{
	vm.sp = running->sp;
	vm.pc = running->pc;
	vm.base = running->base;
	vm.limit = running->limit;
	vm.sr = running->sr;
	vm.r[0] = running->r[0];
	vm.r[1] = running->r[1];
	vm.r[2] = running->r[2];
	vm.r[3] = running->r[3];
	if (running->sp < 256)
	{
		fstream progstack;
		progstack.open(("./" + running->prog + ".st").c_str());
		if (progstack.is_open())
		{
			int data;
			stack<int> readstack;
			progstack >> data;

			while (!progstack.eof())
			{
				readstack.push(data);
				progstack >> data;
			}
			int i = 255;
			while (!readstack.empty() || vm.sp == i) // pop the stack until empty to fill vm mem
			{
				vm.mem[i] = readstack.top();
				readstack.pop();
				i--;
			}
			progstack.close();
			ofstream clear;
			clear.open(("./" + running->prog + ".st").c_str(), ios::trunc); // this will remove the contets of the .st file
			clear.close();
		}
	}
}

int main(int argc, char *argv[])
{
	OS os;
	int x;
	cin >> x;
}



