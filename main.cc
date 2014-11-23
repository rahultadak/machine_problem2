/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <fstream>
using namespace std;

#include "cache.h"
#include "msi.h"
#include "mesi.h"
#include "dragon.h"

int main(int argc, char *argv[])
{
    if(!Debug)
    {
        cout << "===== 506 Personal information =====" << endl;
        cout << "Rahul Tadakamadla" << endl;
        cout << "rtadaka@ncsu.edu" << endl;
        cout << "Section ECE 506-001" << endl;
    }
	
	ifstream trace;

	if(argv[1] == NULL){
		 cout << "input format: ";
		 cout << "./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> " << endl;
		 exit(0);
        }

	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/

	
	//****************************************************//
	//*******print out simulator configuration here*******//
	cout << "===== 506 SMP Simulator onfiguration =====" << endl;
	cout << "L1_SIZE:               " << cache_size << endl;
	cout << "L1_ASSOC:              " << cache_assoc << endl;
	cout << "L1_BLOCKSIZE:          " << blk_size << endl;
	cout << "NUMBER OF PROCESSORS:  " << num_processors << endl;
	cout << "COHERENCE PROTOCOL:    ";
	if(protocol==0) cout << "MSI" << endl;
    else if(protocol==1) cout << "MESI" << endl;
    else cout << "DRAGON" << endl;
    cout << "TRACE FILE:            " << argv[6] << endl;
	//****************************************************//

 
	//*********************************************//
        //*****create an array of caches here**********//
    //Instantiating the caches and bus object pointers
    vector<Cache> p_caches(num_processors,Cache(cache_size,cache_assoc,blk_size));
    msi_bus* MSI;
    mesi_bus* MESI;
    dragon_bus* Dragon;
    //Creating bus objects as required
    switch(protocol)
    {
        case 0:
            MSI = new msi_bus(&p_caches, num_processors);
            break;
        
        case 1:
            MESI = new mesi_bus(&p_caches, num_processors);
            break;

        case 2:
            Dragon = new dragon_bus(&p_caches, num_processors);
            break;
    }

    //Trace file check
	trace.open(argv[6]);
	if(trace.fail())
	{   
		cout << "Trace file problem" << endl;
		exit(1);
	}

	string strIn;
	//Get first line of trace
	getline(trace,strIn);
	Transaction tran;
	bool bus_chk;
	int bus_tran;
	int tran_cnt=0;

	//Beginning trace read and bus execution loop
	while (!trace.eof())
    {
        tran_cnt++;
        //Transaction object definition
        tran.setAttr(strIn);
        if(Debug) cout << tran_cnt << ". " << hex << tran.getAddr() << endl;
        if(Debug) cout << "P" << tran.proc_id() << " proc ";

        //Cache access, check if hit or miss and update miss counters
        p_caches.at(tran.proc_id()).Access(tran.getAddr(),tran.tranType());
	   
	    if(Debug) p_caches.at(tran.proc_id()).printCacheBlk(tran.getAddr());

	    //Based on type of bus, execute transactions on the specific bus
	    switch (protocol)
        {
            case 0:
                //Update processor state, bet the bus transaction posted
                bus_tran = p_caches.at(tran.proc_id()).update_proc_MSI(tran.getAddr(),tran.tranType());
                if(Debug) cout << "BUS "<< bus_tran << endl;
                //Complete bus side of transaction
                MSI->access(tran.getAddr(),tran.proc_id(),bus_tran);
                break;
            
            case 1:
                //Check if copy of the data present in any other cache
                bus_chk = MESI->check(tran.getAddr(),tran.proc_id());
                //Update processor state, bet the bus transaction posted
                bus_tran = p_caches.at(tran.proc_id()).update_proc_MESI(tran.getAddr(),tran.tranType(),bus_chk);
                if(Debug) cout << "BUS "<< bus_tran << endl;
                //Complete bus side of transaction
                MESI->access(tran.getAddr(),tran.proc_id(),bus_tran);
                break;
            
            case 2:
                //Check if copy of the data present in any other cache
                bus_chk = Dragon->check(tran.getAddr(),tran.proc_id());
                //Update processor state, bet the bus transaction posted
                bus_tran = p_caches.at(tran.proc_id()).update_proc_Dragon(tran.getAddr(),tran.tranType(),bus_chk);
                if(Debug) cout << "BUS "<< bus_tran << endl;
                //Complete bus side of transaction
                Dragon->access(tran.getAddr(),tran.proc_id(),bus_tran);
                break;
        }

        if(Debug)
        {
	        p_caches.at(tran.proc_id()).printCacheBlk(tran.getAddr());
	        cout << endl;
        }
        //Get next line of trace
	    getline(trace,strIn);
    }

	//********************************//
	//print out all caches' statistics //
	//********************************//
	for (int i;i<num_processors;i++)
    {
        p_caches.at(i).printStats(i);
    }
	
}
