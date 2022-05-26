#include "slave.h"

Slave::Slave(void)
{	
	this->Alias = -1;
	this->Position = -1;
	this->Rel_Pos = -1;
	this->Master_Index = -1;
	this->DL_Status = -1;
	this->Port_Descriptor = -1;
	this->ECU_Error_Counter = 0;
}

Slave::~Slave() {};

void Slave::init_ports(void)
{	
	for (int i = 0; i < 4; i++)
	{
		if ((this->Get_Port_Descriptor() >> (i * 2)) & 3 != 0)
		{
			this->ports.push_back(Port(i));
		}
	}

	int mask_1 = 0b0000000000010000;				//physical link		
	int mask_2 = 0b0000000100000000;				//Loop port
	int mask_3 = 0b0000001000000000;				//Communication 

	for (int i = 0; i < this->ports.size(); i++)
	{
		//Physical link
		this->ports[i].Set_Physical_Link(this->Get_DL_Status() & (mask_1 << (1 * this->ports[i].Get_Nr())));

		//Loop port
		this->ports[i].Set_Loop(this->Get_DL_Status() &(mask_2 << (2 * this->ports[i].Get_Nr())));

		//Communication
		this->ports[i].Set_Communication(this->Get_DL_Status() & (mask_3 << (2 * this->ports[i].Get_Nr())));
	}
}

void Slave::Get_Error_Counters(void)
{
	std::string cmd = "";
	std::string iter = "";
	std::string reg = "";
	int mask_first = 0xFF;	
	int ret;

	for (int i = 0; i < this->ports.size() && this->Get_Master_Index() != 7; i++)
	{		
		//Get invalid frame counter (first byte) and rx error counter (second byte)
		iter = std::to_string(this->Master_Index);														
		reg = std::to_string(ERR_OFFSET + (i*2));														
		cmd = "L0=ecatregreadwrite(0,,1,,0,2);L0";													
		cmd.insert(22, iter);
		cmd.insert(26, reg);
		ret = send_command(ppmaccomm, cmd);
		this->ports[i].Set_Frame_Error_Counter(ret & mask_first);									
		ret = ret >> 8;																				
		this->ports[i].Set_RX_Error_Counter(ret & mask_first);										
		
		//Get the forwarded RX error counter
		reg = std::to_string(ERR_OFFSET + i + 8);													
		cmd = "L0=ecatregreadwrite(0,,1,,0,2);L0";													
		cmd.insert(22, iter);
		cmd.insert(26, reg);
		ret = send_command(ppmaccomm, cmd);
		this->ports[i].Set_Frame_Error_Counter(ret);									

		//Get the lost link counter
		reg = std::to_string(ERR_OFFSET + i + 16);													
		cmd = "L0=ecatregreadwrite(0,,1,,0,2);L0";													
		cmd.insert(22, iter);
		cmd.insert(26, reg);
		ret = send_command(ppmaccomm, cmd);
		this->ports[i].Set_Lost_Link_Counter(ret);

		//Get the ECU error counter
		reg = std::to_string(ERR_OFFSET + 12);
		cmd = "L0=ecatregreadwrite(0,,1,,0,1);L0";
		cmd.insert(22, iter);
		cmd.insert(26, reg);
		ret = send_command(ppmaccomm, cmd);
		this->Set_ECU_Error_Counter(ret);
	}	
}
