#ifndef PORT_H
#define	PORT_H

#include "PowerPMACcontrol.h"
#include "Ssh_Func.h"
#include <string>

typedef struct Link {
	int s;									//Slave
	int p;									//Port
}Link;

class Port {
private:
	int Nr;
	Link link;

	//Name									ESC Register
	bool Loop;								//0x0110:0x0111
	bool Physical_Link;						//0x0110:0x0111
	bool Communication;						//0x0110:0x0111
	int Frame_Error_Counter;				//0x0300:0x0307
	int RX_Error_Counter;
	int Physical_Error_Counter;				//0x0300:0x0307
	int Forwarded_Error_Counter;			//0x0308:0x030B
	int Lost_Link_Counter;					//0x0310:0x0313
public:
	Port(void);
	Port(int x);
	~Port();	

	int Get_Nr(void) { return this->Nr; }
	bool Get_Loop(void) { return this->Loop; }
	bool Get_Physical_Link(void) { return this->Physical_Link; }
	bool Get_Communication(void) { return this->Communication; }
	Link Get_Link(void) { return this->link; }
	int Get_Frame_Error_Counter(void) { return this->Frame_Error_Counter; }
	int Get_RX_Error_counter(void) { return this->RX_Error_Counter; }
	int Get_Physical_Error_Counter(void) { return this->Physical_Error_Counter; }
	int Get_Forwarded_Error_Counter(void) { return this->Forwarded_Error_Counter; }
	int Get_Lost_Link_Counter(void) { return this->Lost_Link_Counter; }

	void Set_Nr(int x) { this->Nr = x; }
	void Set_Loop(bool x) { this->Loop = x; }
	void Set_Physical_Link(bool x) { this->Physical_Link = x; }
	void Set_Communication(bool x) { this->Communication = x; }
	void Set_Link(int x, int y) { this->link.s = x; this->link.p = y; }
	void Set_Frame_Error_Counter(int x) { this->Frame_Error_Counter = x; }
	void Set_RX_Error_Counter(int x) { this->RX_Error_Counter = x; }
	void Set_Physical_Error_Counter(int x) { this->Physical_Error_Counter = x; }
	void Set_Forwarded_Error_Counter(int x) { this->Forwarded_Error_Counter = x; }
	void Set_Lost_Link_Counter(int x) { this->Lost_Link_Counter = x; }
};
#endif