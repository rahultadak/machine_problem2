#include "cache.h"
#include <vector>

class dragon_bus{
    private:
        vector<Cache> *p_caches;
        int num_processors;

    public:
        dragon_bus(vector<Cache> *caches, int num_proc);
        bool check(ulong addr, int proc_id);
        void access(ulong addr, int proc_id, int bus_tran);
};
