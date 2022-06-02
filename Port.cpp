#include "Port.h"

Port::Port(void)
{
	this->Nr = -1;
	this->Loop = NULL;
	this->Communication = NULL;
	this->Physical_Link = NULL;
	this->Frame_Error_Counter = 0;
	this->RX_Error_Counter = 0;
	this->Physical_Error_Counter = 0;
	this->Forwarded_Error_Counter = 0;
	this->Lost_Link_Counter = 0;
}

Port::Port(int x)
{
	this->Nr = x;
	this->Loop = NULL;
	this->Communication = NULL;
	this->Physical_Link = NULL;
	this->Frame_Error_Counter = 0;
	this->RX_Error_Counter = 0;
	this->Physical_Error_Counter = 0;
	this->Forwarded_Error_Counter = 0;
	this->Lost_Link_Counter = 0;
}

Port::~Port() {	

}

