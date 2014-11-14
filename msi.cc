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
        
        //Invalid state
        if (line == NULL)
        {
            if (Debug) cout << "P" << i << " MISS" << endl; 
            continue;
        }

        int old_state = line->getFlags();
        if(bus_tran == BUS_RD)
        {
            //BUS_RD Shared state
            if(old_state == SHARED)
            {
                if(Debug) cout << "P" << i << " old " << old_state 
                    << " new " << line->getFlags() << endl;
            }

            //BUS_RD Modified state
            else
            {
                line->setFlags(SHARED);
                p_caches->at(i).memory();
                p_caches->at(i).writeBack();
                p_caches->at(i).flush();
                p_caches->at(i).intervene();

                if(Debug) cout << "P" << i << " old " << old_state 
                    << " new " << line->getFlags() << endl;
            }
        }

        else if(bus_tran == BUS_RDX)
        {
            line->invalidate();
            p_caches->at(i).invalidate();
            if(old_state == MODIFIED)
            {
                p_caches->at(i).memory();
                p_caches->at(i).writeBack();
                p_caches->at(i).flush();
            }
            if(Debug) cout << "P" << i << " old " << old_state 
                << " new " << line->getFlags() << endl;
        }
    }
}

    
