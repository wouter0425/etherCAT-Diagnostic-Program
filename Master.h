#ifndef MASTER_H
#define MASTER_H

#include <vector>
#include <string>
#include "Slave.h"
#include "Port.h"
#include "Ssh_Func.h"
#include "defines.h"
#include <algorithm>
#include <fstream>

class Master {
private:
	int SlaveCount;

public:
	Master(void);
	~Master();
	
	vector<Link> data_path;
	vector<Slave> slaves;	

	int get_slavecount(void) { return this->SlaveCount; }	
	void set_slavecount(int x) { this->SlaveCount = x; }
	
	void init_slaves(void);	
	void init_error_registers(void);
	void clear_error_registers(void);
	void map_topology(int id, int& next_id);		
	void print_ecat_data(void);
	bool detect_ecat_error(void);
	void locate_ecat_error(void);
};



#endif

