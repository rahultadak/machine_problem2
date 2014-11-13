/*******************************************************
                          cache.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
using namespace std;

int Debug = 1;

Cache::Cache(int s,int a,int b )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;

   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
   //*******************//
 
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
		tagMask <<= 1;
        tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache.resize(sets);
   for(i=0; i<sets; i++)
   {
      cache[i].resize(assoc);
      for(j=0; j<assoc; j++) 
      {
	   cache[i][j].invalidate();
      }
   }      
   
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
//Returns hit or miss from the cache.
bool Cache::Access(ulong addr,uchar op)
{
	currentCycle++;/*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
        	
	if(op == 'w') writes++;
	else          reads++;
	
	cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
		if(op == 'w') writeMisses++;
		else readMisses++;
		return false;
    }
    return true;
}

int Cache::update_proc_MSI(ulong addr,uchar op, bool hit)
{
    int bus_tran;
	cacheLine * line = findLine(addr);
    if(hit)
    {
        if(op == 'w'&&line->getFlags()==SHARED)
        {
            //Initiate us transaction based on previus state
            //Send BUSRDX
            bus_tran = BUS_RDX;
            line->setFlags(MODIFIED);
            //Do not do anything if read when shared
            //Do not do anything if read/write when modified
            //
        }
        else
            bus_tran = NONE;

        if (Debug) cout << "new "<< line->getFlags() << " " << endl;
    }

    //TODO Need to add functionality when miss
    else
    {
        cacheLine *newline = fillLine(addr);
        newline->setTag(calcTag(addr));

   		if(op == 'w') 
        {
            newline->setFlags(MODIFIED);    
   		    bus_tran = BUS_RDX;
        }
        else if(op == 'r')
        {
            newline->setFlags(SHARED);
            bus_tran = BUS_RD;
        }
    if (Debug) cout << "new " << newline->getFlags() << endl;
    }
    return bus_tran;
}

//        //TODO need to change access function here to send bus transaction
//		cacheLine *newline = fillLine(addr);
//   		if(op == 'w') newline->setFlags(DIRTY);    
//		
//	}
//	else
//	{
//		/**since it's a hit, update LRU and update dirty flag**/
//		updateLRU(line);
//		if(op == 'w') line->setFlags(DIRTY);
//	}
//}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	        if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   //TODO change dirty to modified?
   if(victim->getFlags() == MODIFIED || SHARED_M) writeBack(addr);

   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/
   return victim;   	
}

void Cache::printStats()
{ 
	cout << "===== Simulation results      =====" << endl;
	/****print out the rest of statistics here.****/
	/****follow the ouput file format**************/
}
