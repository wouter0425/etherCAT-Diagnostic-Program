#include "Master.h"

Master::Master(void)
{
	this->SlaveCount = 0;
}

Master::~Master(){
}

void Master::init_slaves(void)
{
	//Expected master slaves
	this->SlaveCount = send_command(ppmaccomm, "ECAT[0].SlaveCount");

	std::string cmd = "ecat slaves";
	std::string reply = "";
	std::string buffer = "";
	int i = 0;
	int slave_iter = 0;

	//Set (relative)position and alias
	int ret = ppmaccomm->PowerPMACcontrol_sendCommand(cmd, reply);
	Sleep(1000);
	while (reply[i] != '\0')
	{
		if (i == 0 || (reply[i] == '\n' && reply[i + 1] != '\0'))
		{
			//Create slave
			this->slaves.push_back(Slave());			

			//Get the Alias
			while (reply[i] != ':') { i++; }
			while (reply[i] != ' ') { i--; }
			i++;
			while (reply[i] != ':')
			{
				buffer += reply[i];
				i++;
			}
			slaves[slave_iter].Set_Alias(std::stoi(buffer, 0));
			buffer.clear();
			i++;

			//Get the relative position
			while (reply[i] != ' ')
			{
				buffer += reply[i];
				i++;
			}
			slaves[slave_iter].Set_Rel_Pos(std::stoi(buffer, 0));
			buffer.clear();
			slave_iter++;
		}
		i++;
	}

	//Assign number of slaves to the master
	for (int i = 0; i < this->slaves.size(); i++)
	{	
		for (int j = 0; j < this->SlaveCount; j++)
		{
			char it_j = j + ASCII_OFFSET;

			cmd.clear();
			cmd = "ECAT[0].Slave[].Alias";
			cmd.insert(14, 1, it_j);
			int buffer_alias = send_command(ppmaccomm, cmd);

			cmd.clear();
			cmd = "ECAT[0].Slave[].Position";
			cmd.insert(14, 1, it_j);
			int buffer_position = send_command(ppmaccomm, cmd);

			if(this->slaves[i].Get_Alias() == buffer_alias && this->slaves[i].Get_Rel_Pos())
			{
				slaves[i].Set_Master_Index(j);
				char slave_it = this->slaves[i].Get_Master_Index() + ASCII_OFFSET;

				//DL Status register
				cmd = "L0=ecatregreadwrite(0,,1,$110,0,2);L0";
				cmd.insert(22, 1, slave_it);
				this->slaves[i].Set_DL_Status(send_command(ppmaccomm, cmd));

				//Get the ecat slave port classes		
				cmd = "L0=ecatregreadwrite(0,,1,$7,0,1);L0";
				cmd.insert(22, 1, slave_it);
				this->slaves[i].Set_Port_Descriptor(send_command(ppmaccomm, cmd));

				this->slaves[i].init_ports();
				break;
			}
		}
	}
}

void Master::init_error_registers(void)
{
	for (int i = 0; i < this->slaves.size(); i++)
	{		
		char slave_it = this->slaves[i].Get_Master_Index() + ASCII_OFFSET;
		std::string cmd = "L0=ecatregreadwrite(0,,0,$100,0,2);L0";
		cmd.insert(22, 1, slave_it);
		int res = send_command(ppmaccomm, cmd);		
	}
}

void Master::clear_error_registers(void)
{	
	int temp;
	std::string cmd = "";
	
	for (int i = 0; i < this->slaves.size(); i++)
	{
		char slave_it = this->slaves[i].Get_Master_Index() + ASCII_OFFSET;

		//Clear error register 0x0300 to 0x030B
		cmd = "L0=ecatregreadwrite(0,,0,$300,0,1);L0";
		cmd.insert(22, 1, slave_it);
		temp = send_command(ppmaccomm, cmd);		

		//Clear error register 0x030C
		cmd = "L0=ecatregreadwrite(0,,0,$30C,0,1);L0";
		cmd.insert(22, 1, slave_it);
		temp = send_command(ppmaccomm, cmd);

		//Clear error register 0x030D to 0x030E
		cmd = "L0=ecatregreadwrite(0,,0,$30D,0,1);L0";
		cmd.insert(22, 1, slave_it);
		temp = send_command(ppmaccomm, cmd);

		//Clear error register 0x0310 to 0x0313
		cmd = "L0=ecatregreadwrite(0,,0,$310,0,1);L0";
		cmd.insert(22, 1, slave_it);
		temp = send_command(ppmaccomm, cmd);
	}
}

void Master::map_topology(int id, int& next_id)
{	
	int num_ports = this->slaves[id].ports.size();

	if (num_ports == 2) {
		this->slaves[id].iterator.push_back(0);
		this->slaves[id].iterator.push_back(1);
	}	
	else if (num_ports == 3) {
		if (this->slaves[id].ports[2].Get_Nr() == 2) {
			this->slaves[id].iterator.push_back(0);
			this->slaves[id].iterator.push_back(1);
			this->slaves[id].iterator.push_back(2);
			
		}

		if (this->slaves[id].ports[2].Get_Nr() == 3) {
			this->slaves[id].iterator.push_back(0);
			this->slaves[id].iterator.push_back(2);
			this->slaves[id].iterator.push_back(1);
		}
	}	
	else if (num_ports == 4) {
		this->slaves[id].iterator.push_back(0);
		this->slaves[id].iterator.push_back(3);
		this->slaves[id].iterator.push_back(1);
		this->slaves[id].iterator.push_back(2);		
	}	
	if (id == 0)
	{
		this->slaves[id].ports[0].link.s = -1;
		this->slaves[id].ports[0].link.p = 0;
	}

	Link node = { id, 0 };
	this->data_path.push_back(node);

	for (int i = 1; i < this->slaves[id].iterator.size(); i++) {
		if (this->slaves[id].ports[this->slaves[id].iterator[i]].Get_Communication() == true) {							

			next_id++;				
			this->slaves[id].ports[this->slaves[id].iterator[i]].link.s = next_id;
			this->slaves[id].ports[this->slaves[id].iterator[i]].link.p = 0;

			this->slaves[next_id].ports[0].link.s = id;
			this->slaves[next_id].ports[0].link.p = this->slaves[id].iterator[i];

			int temp = next_id;
			this->map_topology(next_id, next_id);			
			
			this->data_path.push_back(this->slaves[temp].ports[0].link);
		}
		else if (this->slaves[id].ports[this->slaves[id].iterator[i]].Get_Communication() == false) {
			this->slaves[id].ports[this->slaves[id].iterator[i]].link.s = -1;
			this->slaves[id].ports[this->slaves[id].iterator[i]].link.p = -1;
		}				
	}	
}	

void Master::print_ecat_data(void)
{
	printf("\nEtherCAT DATA\n");
	printf("---------------------------------------------------------------------------\n");
	printf("Master Data: \n");	
	printf("Slavecount\t\t\t%d\n", this->SlaveCount);
	printf("---------------------------------------------------------------------------\n");
	for (int i = 0; i < this->slaves.size(); i++)
	{
		printf("Slave[%d]: \n", i);
		printf("Master index: %d\n", this->slaves[i].Get_Master_Index());
		printf("Alias: %d\n", this->slaves[i].Get_Alias());
		printf("Poistion: %d\n", this->slaves[i].Get_Position());
		printf("Comp pos: %d\n", this->slaves[i].Get_Rel_Pos());

		for (int j = 0; j < this->slaves[i].ports.size(); j++)
		{
			printf("Poort[%d]:\t slave[%d] \t poort[%d]\n", this->slaves[i].iterator[j], this->slaves[i].ports[this->slaves[i].iterator[j]].link.s, this->slaves[i].ports[this->slaves[i].iterator[j]].link.p);
			printf("\tPort[%d].Nr = %d\n", j, this->slaves[i].ports[j].Get_Nr());
			printf("\tPort[%d].Physical_link:\t\t\t%d\n", j, this->slaves[i].ports[j].Get_Physical_Link());
			printf("\tPort[%d].Loop port:\t\t\t%d\n", j, this->slaves[i].ports[j].Get_Loop());
			printf("\tPort[%d].Communication:\t\t\t%d\n", j, this->slaves[i].ports[j].Get_Communication());
		}
		printf("------------------------------------------------------------\n");
	}

	for (int i = 0; i < this->data_path.size(); i++) {
		int s = this->data_path[i].s;
		int p = this->data_path[i].p;

		printf("(%d, %d)\t", this->data_path[i].s, this->data_path[i].p);
		printf("%d\n", this->slaves[this->data_path[i].s].ports[this->data_path[i].p].Get_Nr());
	}
	return;
}

bool Master::detect_ecat_fault(void) {	
	bool check = false;
	Master temp_master;		
	temp_master.init_slaves();

	if (temp_master.slaves.size() != this->slaves.size()) {	
		check = true;
	}

	if (temp_master.slaves.size() == 0) {
		printf("No link between master and slave[0], unable to read any data from the etherCAT bus\n");
		return check;
	}	

	ecat_reset(ppmaccomm);	

	if (check == true) {
		for (int i = 0; i < this->slaves.size(); i++) {
			for (int j = 0; j < temp_master.slaves.size(); j++) {
				if (this->slaves[i].Get_Master_Index() == temp_master.slaves[j].Get_Master_Index()) {
					printf("Get error counters from slave: %d\n", i);
					this->slaves[i].Get_Error_Counters();					
				}
			}
		}			
	}	
	else {
		printf("No possible error detected\n");
	}

	return check;
}

void Master::locate_ecat_error(void) {
	//Port error indicators
	for (int i = 0; i < this->data_path.size(); i++)
	{
		int s = this->data_path[i].s;
		int p = this->data_path[i].p;
		printf("\nError detection at slave %d on port %d:\n", s, p);
		if (this->slaves[s].ports[p].Get_Physical_Error_Counter() > 0 || this->slaves[s].ports[p].Get_RX_Error_counter() > 0 || this->slaves[s].ports[p].Get_Forwarded_Error_Counter() > 0 || this->slaves[s].ports[p].Get_Lost_Link_Counter() > 0 || this->slaves[s].ports[p].Get_Forwarded_Error_Counter() > 0)
		{
			printf("Lost link counter:\t\t\t %d\n", this->slaves[s].ports[p].Get_Lost_Link_Counter());
			printf("Frame error(s):\t\t\t\t %d\n", this->slaves[s].ports[p].Get_RX_Error_counter());
			printf("Physical  error(s):\t\t\t %d\n", this->slaves[s].ports[p].Get_Physical_Error_Counter());
			printf("frames previously declared corrupt:\t %d\n\n", this->slaves[s].ports[p].Get_Forwarded_Error_Counter());
		}
		else {
			printf("\tNo error detected\n");
		}
	}
}