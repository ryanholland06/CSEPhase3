#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdlib>

#include "VirtualMachine.h"

using namespace std;

void VirtualMachine::load(fstream& objectCode, int base, int & limit)
{
    for (limit = base; objectCode >> mem[limit]; limit++);
}

void VirtualMachine::run(int time_slice, fstream & in, fstream & out)
{
    const int debug = false;
    int opcode, rd, i, rs, constant, addr, j;

    int interrupt = clock + time_slice;
    while (pc < base+limit and clock < interrupt) {
        ir = mem[pc];
        pc++;
        opcode = (ir&0xf800)>>11;
        rd = (ir&0x600)>>9;
        i = (ir&0x100)>>8;
        rs = (ir&0xc0)>>6;
        addr = ir&0xff;
        constant = addr;
        if (ir&0x80) constant |= 0xffffff00; // if neg sign extend 
    
        clock++;
    	
        if (opcode == 0) { /* load loadi */
            if (i) r[rd] = constant;
            else {
                if (addr >= limit) {
                    sr &= 0xffffff1f; sr |= 0x60; // ERROR: set return status to 011
                    return;
                }
                r[rd] = mem[addr+base];
                clock += 3;
            }
        } else if (opcode == 1) { /* store */
            if (addr >= limit) {
                sr &= 0xffffff1f; sr |= 0x60; // ERROR: set return status to 011
                return;
            }
            mem[addr+base] = r[rd];
            clock += 3;
        } else if (opcode == 2) { /* add addi */
            int sign1 = (r[rd]&0x8000)>>15;
            int sign2;
            if (i) {
                sign2 = (constant&0x8000)>>15;
                r[rd] = r[rd] + constant;
            } else {
                sign2 = (r[rs]&0x8000)>>15;
                r[rd] = r[rd] + r[rs];
            }
            // set CARRY
            if (r[rd]&0x10000) sr |= 01;
            else sr &= 0xfffffffe;
            // set OVERFLOW
            if (sign1 == sign2 and sign1 != (r[rd]&0x8000)>>15)
                sr |= 0x10;
            else
                sr &= 0xf;
            // sign extend
            if (r[rd]&0x8000) r[rd] |= 0xffff0000;
            else r[rd] &= 0xffff;
        } else if (opcode == 3) { /* addc addci */
            if (i)
                r[rd] = r[rd] + constant + sr&01;
            else
                r[rd] = r[rd] + r[rs] + sr&01;
            if (r[rd]&0x10000) sr |= 01;
            else sr &= 0xfffffffe;
            if (r[rd]&0x8000) r[rd] |= 0xffff0000;
            else r[rd] &= 0xffff;
        } else if (opcode == 4) { /* sub subi */
            int sign1 = (r[rd]&0x8000)>>15;
            int sign2;
            if (i) {
                sign2 = (constant&0x8000)>>15;
                r[rd] = r[rd] - constant;
            } else {
                sign2 = (r[rs]&0x8000)>>15;
                r[rd] = r[rd] - r[rs];
            }
            // set CARRY
            if (r[rd]&0x10000) sr |= 01;
            else sr &= 0xfffffffe;
            // set OVERFLOW
            if (sign1 != sign2 and sign2 == (r[rd]&0x8000)>>15)
                sr |= 0x10;
            else
                sr &= 0xf;
            // sign extend
            if (r[rd]&0x8000) r[rd] |= 0xffff0000;
            else r[rd] &= 0xffff;
        } else if (opcode == 5) { /* subc subci */
            if (i)
                r[rd] = r[rd] - constant - sr&01;
            else
                r[rd] = r[rd] - r[rs] - sr&01;
            if (r[rd]&0x10000) sr |= 01;
            else sr &= 0xfffffffe;
            if (r[rd]&0x8000) r[rd] |= 0xffff0000;
            else r[rd] &= 0xffff;
        } else if (opcode == 6) { /* and andi */
            if (i) r[rd] = r[rd] & constant;
            else r[rd] = r[rd] & r[rs];
            r[rd] &= 0xffff;
        } else if (opcode == 7) { /* xor xori */
            if (i) r[rd] = r[rd] ^ constant;
            else r[rd] = r[rd] ^ r[rs];
            r[rd] &= 0xffff;
        } else if (opcode == 8) { /* compl */
            r[rd] = ~r[rd];
            r[rd] &= 0xffff;
        } else if (opcode == 9) { /* shl */
            r[rd] <<= 1;
            if (r[rd]&0x10000) sr |= 01;
            else sr &= 0xfffffffe;
        } else if (opcode == 10) { /* shla */
            r[rd] <<= 1;
            if (r[rd]&0x10000) {
                sr |= 01;
                r[rd] |= 0x8000;
            } else {
                sr &= 0xfffffffe;
                r[rd] &= 0x7fff;
            }
            if (r[rd]&0x8000) r[rd] |= 0xffff0000;
            else r[rd] &= 0xffff;
        } else if (opcode == 11) { /* shr */
            r[rd] &= 0xffff;
            if (r[rd]&01) sr |= 01;
            else sr &= 0xfffffffe;
            r[rd] >>= 1;
        } else if (opcode == 12) { /* shra */
            if (r[rd]&01) sr |= 01;
            else sr &= 0xfffffffe;
            r[rd] >>= 1;
        } else if (opcode == 13) { /* compr  compri */
            sr &= 0xffffffe1;
            if (i) {
                if (r[rd]<constant) sr |= 010;
                if (r[rd]==constant) sr |= 04;
                if (r[rd]>constant) sr |= 02;
            } else {
                if (r[rd]<r[rs]) sr |= 010;
                if (r[rd]==r[rs]) sr |= 04;
                if (r[rd]>r[rs]) sr |= 02;
            }
        } else if (opcode == 14) { /* getstat */
            r[rd] = sr;
        } else if (opcode == 15) { /* putstat */
            sr = r[rd];
        } else if (opcode == 16) { /* jump */
            pc = addr+base;
            if (addr >= limit) {
                sr &= 0xffffff1f; sr |= 0x40; // Out-of-bound Reference
                return;
            }
        } else if (opcode == 17) { /* jumpl */
            if (sr & 010) pc = addr+base;
            if (addr >= limit) {
                sr &= 0xffffff1f; sr |= 0x40; // Out-of-bound Reference
                return;
            }
        } else if (opcode == 18) { /* jumpe */
            if (sr & 04) pc = addr+base;
            if (addr >= limit) {
                sr &= 0xffffff1f; sr |= 0x40; // Out-of-bound Reference
                return;
            }
        } else if (opcode == 19) { /* jumpg */
            if (sr & 02) pc = addr+base;
            if (addr >= limit) {
                sr &= 0xffffff1f; sr |= 0x40; // Out-of-bound Reference
                return;
            }
        } else if (opcode == 20) { /* call */
            if (addr >= limit) {
                sr &= 0xffffff1f; sr |= 0x40; // Out-of-bound Reference
                return;
            }
            if (sp < total_limit+6) {
                cerr << "Stack Overflow sp = " << sp << endl;
                sr &= 0xffffff1f; sr |= 0x60; // ST Over
                return;
            }
            mem[--sp] = pc;
            for (j=0; j<4; j++)
                mem[--sp] = r[j];
            mem[--sp] = sr;
            pc = addr+base;
            clock += 3;
        } else if (opcode == 21) { /* return */
            if (sp > 256-6) {
                cerr << "Stack Underflow sp = " << sp << endl;
                sr &= 0xffffff1f; sr |= 0x80; // ST Under
                return;
            }
            sr = mem[sp++];
            for (j=3; j>=0; j--)
                r[j] = mem[sp++];
            pc = mem[sp++];
            clock += 3;
        } else if (opcode == 22) { /* read */
            in >> r[rd];
            clock++;
            sr &= 0xfffff01f; 
            sr |= 0xc0; // Read
            sr |= rd<<8; // Register
            return;
        } else if (opcode == 23) { /* write */
            out << r[rd] << endl;
            clock++;
            sr &= 0xfffff01f; 
            sr |= 0xe0; // Write
            sr |= rd<<8; // Register
            return;
        } else if (opcode == 24) { /* halt */
            sr &= 0xffffff1f; sr |= 0x20; // Halt
            return;
        } else if (opcode == 25) { /* noop */
            /* do nothing */
        } else {
            cerr << "Bad opcode = " << opcode << endl;
            sr &= 0xffffff1f; sr |= 0xa0; // Bad Opcode
            return;
        }
        
        //cout << mem[pc -1] << endl;
        
        if (debug) {
            printf("ir=%d op=%d rd=%d i=%d rs=%d const=%d addr=%d\n", ir, opcode, rd, i, rs, constant, addr);
            printf("pc=%d base=%d limit=%d r[0]=%d r[1]=%d r[2]=%d r[3]=%d sr=%d sp=%d clock=%d\n", pc, base, limit, r[0], r[1], r[2], r[3], sr, sp, clock);
        }
    }
    
    if (debug) {
        for (j=0; j<total_limit; j++) {
            printf("%8d", mem[j]);
            if ((j%8)==7) printf("\n");
        }
        cout << endl;
    }
    if (pc >= base+limit) {
        sr &= 0xffffff1f; sr |= 0x40; // Out-of-bound Reference
    } else sr &= 0xffffff1f; // Time Slice
} /* main */
