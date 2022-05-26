#ifndef SLAVE_H
#define	SLAVE_H

#include "PowerPMACcontrol.h"
#include "Ssh_Func.h"
#include "defines.h"
#include <string>
#include "Port.h"
#include <vector>
#include <list>
#include <fstream>

using namespace std;

class Slave {
private:
	int Alias;
	int Position;	
	int Rel_Pos;
	int Master_Index;
	int DL_Status;
	int Port_Descriptor;
	int ECU_Error_Counter;	

public:
	vector<Port> ports;
	vector<int> iterator;
	
	void init_ports(void);
	void Get_Error_Counters(void);
	
	Slave(void);
	~Slave();
		
	int Get_Alias(void) { return this->Alias; }
	int Get_Position(void) { return this->Position; }
	int Get_Master_Index(void) { return this->Master_Index; }
	int Get_DL_Status(void) { return this->DL_Status; }
	int Get_ECU_Error_Counter(void) { return this->ECU_Error_Counter; }
	int Get_Port_Descriptor(void) { return this->Port_Descriptor; }
	int Get_Rel_Pos(void) { return this->Rel_Pos; }

	void Set_Alias(int x) { this->Alias = x; }
	void Set_Position(int x) { this->Position = x; }
	void Set_Master_Index(int x) { this->Master_Index = x; }
	void Set_DL_Status(int x) { this->DL_Status = x; }
	void Set_ECU_Error_Counter(int x) { this->ECU_Error_Counter = x; }
	void Set_Port_Descriptor(int x) { this->Port_Descriptor = x; }
	void Set_Rel_Pos(int x) { this->Rel_Pos = x; }
};
#endif