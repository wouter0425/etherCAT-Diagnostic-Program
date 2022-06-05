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
	vector<Link> data_path;

public:
	Master(void);
	~Master();	
	
	vector<Slave> slaves;	

	int get_slavecount(void) { return this->SlaveCount; }	
	Link get_data_path(int x) { return this->data_path[x]; }
	int get_path_size(void) { return this->data_path.size(); }
	
	void set_slavecount(int x) { this->SlaveCount = x; }
	void set_data_path(Link x) { this->data_path.push_back(x); }
	
	void init_slaves(void);	
	void init_error_registers(void);
	void clear_error_registers(void);
	void map_topology(int id, int& next_id);		
	void print_ecat_data(void);
	bool detect_ecat_error(void);
	void locate_ecat_error(void);
};



#endif

