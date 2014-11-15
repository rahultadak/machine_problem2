#include "dragon.h"

dragon_bus::dragon_bus(vector<Cache> *caches, int num_proc)
{
    p_caches = caches;
    num_processors = num_proc;
}

bool dragon_bus::check(ulong addr, int proc_id)
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

void dragon_bus::access(ulong addr, int proc_id, int bus_tran)
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
                    line->setFlags(SHARED_C);
                    p_caches->at(i).intervene();
                    break;

                case BUS_RD_UPD:
                    line->setFlags(SHARED_C);
                    p_caches->at(i).intervene();
                    break;
            }
        }

        else if(old_state == SHARED_C)
        {
            switch(bus_tran)
            {
                case BUS_RD:
                    if(Debug) cout << "P" << i << " old " << old_state 
                        << " new " << line->getFlags() << endl;
                    break;
                    
                case BUS_RD_UPD:
                    break;
                    
                case BUS_UPD:
                    break;
            }
        }

        else if(old_state == SHARED_M)
        {
            switch(bus_tran)
            {
                case BUS_RD:
                    if(Debug) cout << "P" << i << " old " << old_state 
                        << " new " << line->getFlags() << endl;
                    p_caches->at(i).flush();
                    break;
                    
                case BUS_RD_UPD:
                    p_caches->at(i).flush();
                    line->setFlags(SHARED_C);
                    break;
                    
                case BUS_UPD:
                    line->setFlags(SHARED_C);
                    break;
            }
        }

        else if(old_state==MODIFIED)
        {
            switch(bus_tran)
            {
                case BUS_RD:
                    line->setFlags(SHARED_M);
                    p_caches->at(i).flush();
                    p_caches->at(i).intervene();
                    break;
                
                case BUS_RD_UPD:
                    line->setFlags(SHARED_C);
                    p_caches->at(i).flush();
                    p_caches->at(i).intervene();
                    break;
            }
        }
    }
}

    
