#include "Master.h"

Master::Master(void)
{
	this->SlaveCount = 0;
}

Master::~Master(){

}

void Master::init_slaves(void)
{
	//Number of slaves, expected by the master
	this->set_slavecount(send_command(ppmaccomm, "ECAT[0].SlaveCount"));

	std::string cmd = "ecat slaves";		//string for the command
	std::string reply = "";					//string to store the reply	
	std::string buffer = "";				//Buffer to store parts of the string result

	int i = 0;								//Iterator to loop through the command result
	int slave_iter = 0;						//Iterator for the slaves


	//Get the result of the 'ecat slaves' command
	int ret = ppmaccomm->PowerPMACcontrol_sendCommand(cmd, reply);	
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

	//Get the slave index and init the slave ports when found
	for (int i = 0; i < this->slaves.size(); i++)
	{	
		for (int j = 0; j < this->get_slavecount(); j++)
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
				slaves[i].Set_Slave_Index(j);
				char slave_it = this->slaves[i].Get_Slave_Index() + ASCII_OFFSET;

				//DL Status register
				cmd = "L0=ecatregreadwrite(0,,1,$110,0,2);L0";
				cmd.insert(22, 1, slave_it);
				this->slaves[i].Set_DL_Status(send_command(ppmaccomm, cmd));

				//Get the ecat slave port classes		
				cmd = "L0=ecatregreadwrite(0,,1,$7,0,1);L0";
				cmd.insert(22, 1, slave_it);
				this->slaves[i].Set_Port_Descriptor(send_command(ppmaccomm, cmd));

				//Init the slave ports
				this->slaves[i].init_ports();
				break;
			}
		}
	}
}

void Master::init_error_registers(void)
{
	//Set DL Control register value to 0 so error registers can be used
	for (int i = 0; i < this->slaves.size(); i++)
	{		
		char slave_it = this->slaves[i].Get_Slave_Index() + ASCII_OFFSET;
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
		char slave_it = this->slaves[i].Get_Slave_Index() + ASCII_OFFSET;

		//Clear error register 0x0300 to 0x030B: RX, invalid and forwarded frame errors
		cmd = "L0=ecatregreadwrite(0,,0,$300,0,1);L0";
		cmd.insert(22, 1, slave_it);
		temp = send_command(ppmaccomm, cmd);		

		//Clear error register 0x030C: EPU error counter
		cmd = "L0=ecatregreadwrite(0,,0,$30C,0,1);L0";
		cmd.insert(22, 1, slave_it);
		temp = send_command(ppmaccomm, cmd);
		
		//Clear error register 0x0310 to 0x0313: lost link counter
		cmd = "L0=ecatregreadwrite(0,,0,$310,0,1);L0";
		cmd.insert(22, 1, slave_it);
		temp = send_command(ppmaccomm, cmd);
	}
}

void Master::map_topology(int id, int& next_id)
{	
	//Determine the port iterator of a slave, this is not necessarily consecutive
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

	//Set link between master and the first slave at port 0
	if (id == 0){
		
		this->slaves[id].ports[0].Set_Link(-1, 0);
	}

	//Add port 0 of the current slave to the datapath
	Link node = { id, 0 };
	this->data_path.push_back(node);

	for (int i = 1; i < this->slaves[id].iterator.size(); i++) {
		if (this->slaves[id].ports[this->slaves[id].iterator[i]].Get_Communication() == true) {					

			//increment at what slave we currently are
			next_id++;				

			//The current port, of the current slave, is connected to port 0 of the next slave
			//Set the link for the current port to port 0 of the next slave			
			this->slaves[id].ports[this->slaves[id].iterator[i]].Set_Link(next_id, 0);

			//Set the link for port 0 of the next slave
			this->slaves[next_id].ports[0].Set_Link(id, this->slaves[id].iterator[i]);

			//Store the next_id or else it will change due to being passed by reference
			int temp = next_id;

			//recursively call the map_topology function
			this->map_topology(next_id, next_id);			
			
			//Add the previous slave port when the slave is not connected to another slave
			this->data_path.push_back(this->slaves[temp].ports[0].Get_Link());
		}
		else if (this->slaves[id].ports[this->slaves[id].iterator[i]].Get_Communication() == false) {
			//Set link to s = -1 and p = -1 for slaves that are not connected to another slave
			this->slaves[id].ports[this->slaves[id].iterator[i]].Set_Link(-1, -1);
		}				
	}	
}	

void Master::print_ecat_data(void)
{
	printf("\nEtherCAT DATA\n");
	printf("---------------------------------------------------------------------------\n");
	printf("Master Data: \n");	
	printf("Slavecount\t\t\t%d\n", this->get_slavecount());
	printf("---------------------------------------------------------------------------\n");
	for (int i = 0; i < this->slaves.size(); i++)
	{
		printf("Slave[%d]: \n", i);
		printf("Master index: %d\n", this->slaves[i].Get_Slave_Index());
		printf("Alias: %d\n", this->slaves[i].Get_Alias());
		printf("Poistion: %d\n", i);
		printf("Comp pos: %d\n", this->slaves[i].Get_Rel_Pos());

		for (int j = 0; j < this->slaves[i].ports.size(); j++)
		{
			Link temp_link = this->slaves[i].ports[this->slaves[i].iterator[j]].Get_Link();
			printf("Poort[%d]:\t slave[%d] \t poort[%d]\n", this->slaves[i].iterator[j], temp_link.s, temp_link.p);
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

bool Master::detect_ecat_error(void) {
	bool check = false;

	//Create and init a new master with slaves
	Master temp_master;		
	temp_master.init_slaves();

	//Compare the size of the original number of slaves and the current number of slaves
	if (temp_master.slaves.size() != this->slaves.size()) {	
		check = true;
	}

	//If there are no slaves there could be a cable problem between the PMAC and the first slave
	if (temp_master.slaves.size() == 0) {
		printf("No link between master and slave[0], unable to read any data from the etherCAT bus\n");
		return check;
	}	

	//Reset the master stack so we can read the error counter registers
	ecat_reset(ppmaccomm);	

	if (check == true) {
		//Make sure the correct error registers are obtained, trying to obtain error registers that aren't there 
		//is impossible
		for (int i = 0; i < this->slaves.size(); i++) {
			for (int j = 0; j < temp_master.slaves.size(); j++) {
				if (this->slaves[i].Get_Slave_Index() == temp_master.slaves[j].Get_Slave_Index()) {
					this->slaves[i].Get_Error_Registers();					
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