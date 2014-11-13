/*******************************************************
                          cache.h
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include "states.h"

extern int Debug;

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

class cacheLine 
{
protected:
   ulong tag;
   ulong Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong seq; 
 
public:
   cacheLine()            { tag = 0; Flags = INVALID; }
   ulong getTag()         { return tag; }
   ulong getFlags()			{ return Flags;}
   ulong getSeq()         { return seq; }
   void setSeq(ulong Seq)			{ seq = Seq;}
   void setFlags(ulong flags)			{  Flags = flags;}
   void setTag(ulong a)   { tag = a; }
   void invalidate()      { tag = 0; Flags = INVALID; }//useful function
   bool isValid()         { return ((Flags) != INVALID); }
};

class Cache
{
private:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks;

   //******///
   //add coherence counters here///
   //******///

   vector< vector<cacheLine> > cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)  { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag)   { return (tag << (log2Blk));}
   
public:
    ulong currentCycle;  
     
    Cache(int,int,int);
    //~Cache() { delete cache;}
   
    //TODO remove later
    void cache_addr()
    {
        cout << "cache " << &cache << endl;
        cout << "size " << &size << endl;   
    }

   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM(){return readMisses;} ulong getWM(){return writeMisses;} 
   ulong getReads(){return reads;}ulong getWrites(){return writes;}
   ulong getWB(){return writeBacks;}
   
   void writeBack(ulong)   {writeBacks++;}
   bool Access(ulong,uchar);
   void printStats();
   void updateLRU(cacheLine *);

   //Protocol Functions
   int update_proc_MSI(ulong addr, uchar op, bool hit);

   //******///
   //add other functions to handle bus transactions///
   //******///

};

class Transaction{
    private:
        ulong addr;
        int pid;
        uchar rw;
        
    public:
        uchar tranType() { return rw;}
        int proc_id() { return pid; }
        void setAttr(const string &x) 
        {
            istringstream buf(x);
            buf >> pid;
            buf >> rw;
            buf>>hex>>addr;
        }
        int getAddr() { return addr; }

};
 

#endif
