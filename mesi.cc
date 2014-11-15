#include "mesi.h"

mesi_bus::mesi_bus(vector<Cache> *caches, int num_proc)
{
    p_caches = caches;
    num_processors = num_proc;
}

bool mesi_bus::check(ulong addr, int proc_id)
{
    cacheLine * line;
    for (int i=0;i<num_processors;i++)
    {
        if (i==proc_id)
            continue;

        line = p_caches->at(i).findLine(addr);
        if(line !=NULL) //Hit in one of the other caches
            return true;
    }
    return false;
}

void mesi_bus::access(ulong addr, int proc_id, int bus_tran)
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

        if(old_state==EXCLUSIVE)
        {
            switch(bus_tran)
            {
                case BUS_RD:
                    line->setFlags(SHARED);
                    p_caches->at(i).intervene();
                    break;
                    
                case BUS_RDX:
                    line->setFlags(INVALID);
                    p_caches->at(i).invalidate();
                    break;
            }
        }

        else if(old_state == SHARED)
        {
            switch(bus_tran)
            {
                case BUS_RD:
                    if(Debug) cout << "P" << i << " old " << old_state 
                        << " new " << line->getFlags() << endl;
                    break;
                    
                case BUS_RDX:
                    line->setFlags(INVALID);
                    p_caches->at(i).invalidate();
                    break;
                    
                case BUS_UPGR:
                    line->setFlags(INVALID);
                    p_caches->at(i).invalidate();
                    break;
            }
        }

        else if(old_state==MODIFIED)
        {
            switch(bus_tran)
            {
                case BUS_RD:
                    line->setFlags(SHARED);
                    p_caches->at(i).writeBack();
                    p_caches->at(i).memory();
                    p_caches->at(i).flush();
                    p_caches->at(i).intervene();
                    break;

                case BUS_RDX:
                    line->setFlags(INVALID);
                    p_caches->at(i).writeBack();
                    p_caches->at(i).memory();
                    p_caches->at(i).flush();
                    p_caches->at(i).invalidate();
            }
        }
    }
}

    
