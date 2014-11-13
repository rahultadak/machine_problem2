#include "msi.h"

msi_bus::msi_bus(vector<Cache> *caches, int num_proc)
{
    p_caches = caches;
    num_processors = num_proc;
}

void msi_bus::access(ulong addr, int proc_id, int bus_tran)
{
    cacheLine *line;
    for (int i=0;i<num_processors;i++)
    {
        if (i==proc_id)
            continue;

        line = p_caches->at(i).findLine(addr);
        if (line == NULL)
        {
            if (Debug) cout << "P" << i << " MISS" << endl; 
            continue;
        }
        int old_state = line->getFlags();
        if(bus_tran == BUS_RD)
        {
            if(old_state == SHARED)
            {
                if(Debug) cout << "P" << i << " old " << old_state 
                    << " new " << line->getFlags() << endl;
            }
            else
            {
                line->setFlags(SHARED);
                p_caches->at(i).writeBack(addr);
                if(Debug) cout << "P" << i << " old " << old_state 
                    << " new " << line->getFlags() << endl;
            }
        }
    }
}

    
