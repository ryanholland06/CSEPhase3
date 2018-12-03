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
	VirtualMachine(): msize(256), rsize(4), clock(0) { mem.reserve(msize); r.reserve(rsize); }
	void load(fstream&, int base, int & limit);
	void run(int, fstream &, fstream &);
friend
	class OS;
friend
	class PT;
}; // VirtualMachine
////////////////////////////////////////////////////////////////////////////////////////////
class Row
{
    int frame;
    bool valid;
    bool modified;

friend
    class PT;

friend
    class OS;
};
///////////////////////////////////////////////////////////////////////////////////////
class PT
{
    vector<Row> page_table;
    
public:
    PT() { }
    PT(int size)
    {
        page_table = vector<Row>(size);
		int frame = -1;	// neg to count zero as well.
        for (int i = 0; i <= size; i++) 
        {
        	frame++;
        	page_table[i].frame = frame;
            page_table[i].valid = false;
            page_table[i].modified = false;
            
            //cout << frame << " | " << page_table[i].valid << " | " << page_table[i].modified << endl;
        }
    }
    void operator=(const PT & p)
    {
        page_table = p.page_table;
    }
    void operator=(PT && p)
    {
        swap(page_table, p.page_table);
    }

    int log_to_phys(int addr)
    {
        //assuming 8 word pages
        int page_num = addr/8;
        if (page_table[page_num].valid) {
            int frame = page_table[page_num].frame;
            int phys_addr = frame*8 + addr%8;
            return phys_addr;
        } 
        else // page fault	
            return -1;
    }
    void fill_frames(int fcount)
    {
    	for(int i = 0; i <= page_table.size(); i++){
    		if (fcount < 32)
    			page_table[i].frame += fcount;
    		else
    			break;
    	}
    }
friend
    class OS;

friend
    class VirtualMachine;
friend
	class PCB;
};

#endif
