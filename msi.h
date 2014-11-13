#include "cache.h"
#include <vector>

class msi_bus{
    private:
        vector<Cache> *p_caches;
        int num_processors;

    public:
        msi_bus(vector<Cache> *caches, int num_proc);
        void access(ulong addr, int proc_id, int bus_tran);
};
