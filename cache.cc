/*******************************************************
                          cache.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
#include <iomanip>
using namespace std;

int Debug = 0;

Cache::Cache(int s,int a,int b )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;
    interventions = invalidations = mem = c2c_transfers = flushes = 0;
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
   
   /**create a two dimentional cache, sized as vector<vector<Cache>> cache(sets)(assoc)**/ 
   cache.resize(sets);
   for(i=0; i<sets; i++)
   {
      cache.at(i).resize(assoc);
      for(j=0; j<assoc; j++) 
      {
	   cache.at(i).at(j).invalidate();
      }
   }      
   
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
//Returns hit or miss from the cache.
void Cache::Access(ulong addr,uchar op)
{
	currentCycle++;/*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
        	
	if(op == 'w') writes++;
	else          reads++;
	
	cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
        if(Debug) cout << "MISS" << endl;
		if(op == 'w') writeMisses++;
		else readMisses++;
    }
    return;
}

int Cache::update_proc_MSI(ulong addr,uchar op)
{
    int bus_tran;
	cacheLine * line = findLine(addr);
    if(line!=NULL)
    {    
		updateLRU(line);
        if(op == 'w'&&line->getFlags()==SHARED)
        {
            //Initiate us transaction based on previus state
            //Send BUSRDX
            bus_tran = BUS_RDX;
            line->setFlags(MODIFIED);
            memory();
            //Do not do anything if read when shared
            //Do not do anything if read/write when modified
            //
        }
        else
            bus_tran = NONE;

        if (Debug) cout << "new "<< line->getFlags() << " " << endl;
    }

    else
    {
        cacheLine *newline = fillLine(addr);
        memory(); //Line fill memory transaction
        //Write back(Eviction) Counters updated in the above functions
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

int Cache::update_proc_MESI(ulong addr,uchar op,bool bus_chk)
{
    int bus_tran;
	cacheLine * line = findLine(addr);
    if(line!=NULL)  //HIT
    {    
		updateLRU(line);
        bus_tran = NONE;

        if(op == 'w')
        {
            if(line->getFlags()==SHARED)
            {
                //Initiate us transaction based on previus state
                //Send BUS_UPGR
                bus_tran = BUS_UPGR;
                line->setFlags(MODIFIED);
                //Do not do anything if read when shared
                //Do not do anything if read/write when modified
                //Do not do anything if read when exclusive
            }
            else if(line->getFlags() == EXCLUSIVE)
            {
                line->setFlags(MODIFIED);
            }
        }

        if (Debug) cout << "new "<< line->getFlags() << " " << endl;
    }

    //TODO update for MESI MISS
    else
    {
        cacheLine *newline = fillLine(addr);
        //Counters updates in the above functions
        newline->setTag(calcTag(addr));
   		if(op == 'w') 
        {
            newline->setFlags(MODIFIED);    
   		    bus_tran = BUS_RDX;
   		    if(bus_chk) c2c();
            else memory();
        }
        else if(op == 'r')
        {
            if (bus_chk)
            {
                newline->setFlags(SHARED);
                c2c();
            }
            else
            {
                newline->setFlags(EXCLUSIVE);
                memory();
            }

            bus_tran = BUS_RD;
        }
    if (Debug) cout << "new " << newline->getFlags() << endl;
    }
    return bus_tran;
}

int Cache::update_proc_Dragon(ulong addr,uchar op,bool bus_chk)
{
    int bus_tran;
	cacheLine * line = findLine(addr);
    if(line!=NULL)  //HIT
    {    
		updateLRU(line);
        bus_tran = NONE;

        //Do not do anything if read for any state
        //Do not do anything if write when modified
        
        if(op == 'w')
        {
            if(line->getFlags() == EXCLUSIVE)
            {
                line->setFlags(MODIFIED);
            }
            else if(line->getFlags()==SHARED_C)
            {
                //Initiate us transaction based on previus state
                //Send BUS_UPGR
                if(bus_chk) line->setFlags(SHARED_M);
                else line->setFlags(MODIFIED);

                bus_tran = BUS_UPD;
            }
            else if(line->getFlags() == SHARED_M)
            {
                if(!bus_chk) line->setFlags(MODIFIED);
                bus_tran = BUS_UPD;
            }
        }

        if (Debug) cout << "new "<< line->getFlags() << " " << endl;
    }
    else
    {
        cacheLine *newline = fillLine(addr);
        //Counters updates in the above functions
        newline->setTag(calcTag(addr));
   		if(op == 'w') 
        {
   		    if(bus_chk)
            {
                newline->setFlags(SHARED_M); 
                bus_tran = BUS_RD_UPD;
                memory();
            }
            else 
            {
                newline->setFlags(MODIFIED);
                bus_tran = BUS_RD;
                memory();
            }
   		    
        }
        else if(op == 'r')
        {
            if (bus_chk)
            {
                newline->setFlags(SHARED_C);
                memory();
            }
            else
            {
                newline->setFlags(EXCLUSIVE); 
                memory();
            }

            bus_tran = BUS_RD;
        }
    if (Debug) cout << "new " << newline->getFlags() << endl;
    }
    return bus_tran;
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache.at(i).at(j).isValid())
	        if(cache.at(i).at(j).getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache.at(i).at(pos)); 
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
      if(cache.at(i).at(j).isValid() == 0) return &(cache.at(i).at(j));     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache.at(i).at(j).getSeq() <= min) { victim = j; min = cache.at(i).at(j).getSeq();}
   } 
   assert(victim != assoc);
   
   return &(cache.at(i).at(victim));
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
    if(Debug && victim->isValid()) cout << "EVICT " << victim->getTag()<< endl;
   //TODO change dirty to modified?
   if(victim->getFlags() == MODIFIED||victim->getFlags() == SHARED_M)  
   {
       if(Debug) cout << "WRITE BACK" << endl;
        writeBack();
        memory();
   }

   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/
   return victim;   	
}

void Cache::printStats(int i)
{ 
	cout << "============ Simulation results (Cache " << i << ") ============" << endl;
	/****print out the rest of statistics here.****/
	/****follow the ouput file format**************/
	cout << "01. number of reads: 				    " << dec <<  reads << endl;
    cout << "02. number of read misses: 			" << dec << readMisses << endl;
    cout << "03. number of writes: 				    " << dec << writes << endl;
    cout << "04. number of write misses:			" << dec << writeMisses << endl;
    cout << "05. total miss rate: 				    " 
        << setprecision(2) << fixed 
        << (float)(readMisses + writeMisses)*100/(reads+writes) << "%" << endl;
    cout << "06. number of writebacks: 			    " << dec << writeBacks << endl;
    cout << "07. number of cache-to-cache transfers:    " << c2c_transfers << endl;
    cout << "08. number of memory transactions: 	    " << mem << endl; 
    cout << "09. number of interventions: 			" << interventions << endl;
    cout << "10. number of invalidations: 			" << invalidations << endl;
    cout << "11. number of flushes: 				" << flushes << endl;
}

void Cache::printCacheBlk (ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   cout << (uint)tag << endl;
   for(j=0; j<assoc; j++)
   {
       cout << hex << (uint)cache.at(i).at(j).getTag() << " " << cache.at(i).at(j).getFlags() << " | ";
   }
   cout << endl;
}


