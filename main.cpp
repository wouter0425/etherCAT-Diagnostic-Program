#include "libssh2Driver.h"
#include "PowerPMACcontrol.h"
#include <iostream>
#include <string>
#include "argParser.h"
#include <windows.h>
#include "Master.h"
#include "Slave.h"
#include "Port.h"
#include "Ssh_Func.h"

void validation_test_1(void);
void validation_test_2(void);

int main(int argc, char *argv[])
{		
	//validation tests, uncomment rest of main to use
	//validation_test_1();
	//validation_test_2();	
	
	
	//Setup the SSH connection parameters		
	argParser args(argc, argv);
	con pmac_con;
	pmac_con.u_ipaddr = args.getIp();
	pmac_con.u_user = args.getUser();
	pmac_con.u_passw = args.getPassw();
	pmac_con.u_port = args.getPort();
	pmac_con.u_nominus2 = args.getNominus2();

	//Setup the SSH connection	
	connect_pmac(ppmaccomm, pmac_con);
	
	//Reset
	ecat_reset(ppmaccomm);	
	
	//Init master, it's slaves and the slave ports and the error registers
	Master mymaster;	
	mymaster.init_slaves();	
	mymaster.init_error_registers();	

	//Clear error registers at startup
	mymaster.clear_error_registers();

	//Get the topology
	int next_id = 0;
	mymaster.map_topology(0, next_id);	

	//Boolean used for determining if error check must be done
	bool state = false;		
	
	//Automatic mode detects missing slaves, Manual mode skips the detection
	printf("\nSelect monitoring mode: A(automatic), M(manual)\n");
	char c = getchar();
	
	//Boolean used to determine of errors needs to be located
	bool check = false;

	if (c == 'M') {
		printf("Press S to start the detection\n");
		while (c != 'S') {
			c = getchar();			
		}
		check = mymaster.detect_ecat_error();		
		mymaster.locate_ecat_error();
		
	}
	else if (c == 'A') {
		while (1) {			
			check = mymaster.detect_ecat_error();
			if (check == true) {
				mymaster.locate_ecat_error();
			}
			Sleep(INTERVAL);
		}
	}
	
	delete ppmaccomm;	
	
	return 0;
}

void validation_test_1(void)
{
	//Number of slaves, expected by the master
	Master mymaster;
	//Simulation master expects 9 slaves
	mymaster.set_slavecount(9);			

	//Buffer to store parts of the string result
	std::string buffer = "";

	int i = 0;
	int slave_iter = 0;

	//Create simulation reply for the 'ecat slaves' command
	std::string reply =
		"0 VID=$00000002 PC=$044D2C52 0:0 PREOP + SLAVE0\n"
		"1 VID=$00000002 PC=$044D2C52 1:0 PREOP + SLAVE1\n"
		"2 VID=$00000002 PC=$044D2C52 2:0 PREOP + SLAVE2\n"
		"3 VID=$00000002 PC=$044D2C52 3:0 PREOP + SLAVE3\n"
		"4 VID=$00000002 PC=$044D2C52 0:1 PREOP + SLAVE4\n"
		"5 VID=$00000002 PC=$044D2C52 4:0 PREOP + SLAVE5\n"
		"6 VID=$00000002 PC=$044D2C52 5:0 PREOP + SLAVE6\n"
		"7 VID=$00000002 PC=$044D2C52 6:0 PREOP + SLAVE7\n"
		"8 VID=$00000002 PC=$044D2C52 7:0 PREOP + SLAVE8\n";
		
	while (reply[i] != '\0')
	{
		if (i == 0 || (reply[i] == '\n' && reply[i + 1] != '\0'))
		{
			//Create slave
			mymaster.slaves.push_back(Slave());

			//Get the Alias
			while (reply[i] != ':') { i++; }
			while (reply[i] != ' ') { i--; }
			i++;
			while (reply[i] != ':')
			{
				buffer += reply[i];
				i++;
			}
			mymaster.slaves[slave_iter].Set_Alias(std::stoi(buffer, 0));
			buffer.clear();
			i++;

			//Get the relative position
			while (reply[i] != ' ')
			{
				buffer += reply[i];
				i++;
			}
			mymaster.slaves[slave_iter].Set_Rel_Pos(std::stoi(buffer, 0));
			buffer.clear();
			slave_iter++;
		}
		i++;
	}

	//Struct to store simulation ECATConfig.cfg file data
	typedef struct ecatconfig {
		int alias;
		int pos;
	}ecatconfig;

	//simulation data of the ECATConfig.cfg file
	ecatconfig configslaves[9];
	configslaves[0] = { 0, 0 };
	configslaves[1] = { 1, 0 };
	configslaves[2] = { 3, 0 };
	configslaves[3] = { 2, 0 };
	configslaves[4] = { 0, 1 };
	configslaves[5] = { 4, 0 };
	configslaves[6] = { 5, 0 };
	configslaves[7] = { 6, 0 };
	configslaves[8] = { 7, 0 };

	//Simulation data for the port registers
	mymaster.slaves[0].Set_Port_Descriptor(255);
	mymaster.slaves[0].Set_DL_Status(43760);

	mymaster.slaves[1].Set_Port_Descriptor(15);
	mymaster.slaves[1].Set_DL_Status(23088);

	mymaster.slaves[2].Set_Port_Descriptor(63);
	mymaster.slaves[2].Set_DL_Status(27248);

	mymaster.slaves[3].Set_Port_Descriptor(15);
	mymaster.slaves[3].Set_DL_Status(22032);

	mymaster.slaves[4].Set_Port_Descriptor(15);
	mymaster.slaves[4].Set_DL_Status(22032);

	mymaster.slaves[5].Set_Port_Descriptor(207);
	mymaster.slaves[5].Set_DL_Status(39600);

	mymaster.slaves[6].Set_Port_Descriptor(15);
	mymaster.slaves[6].Set_DL_Status(22032);

	mymaster.slaves[7].Set_Port_Descriptor(15);
	mymaster.slaves[7].Set_DL_Status(22032);

	mymaster.slaves[8].Set_Port_Descriptor(15);
	mymaster.slaves[8].Set_DL_Status(22032);
	
	//Get the slave index and init the slave ports when found
	for (int i = 0; i < mymaster.slaves.size(); i++)
	{
		for (int j = 0; j < mymaster.get_slavecount(); j++)
		{
			int buffer_position = configslaves[j].pos;
			int buffer_alias = configslaves[j].alias;

			if (mymaster.slaves[i].Get_Alias() == buffer_alias && mymaster.slaves[i].Get_Rel_Pos() == buffer_position)
			{
				mymaster.slaves[i].Set_Slave_Index(j);

				//Init the slave ports
				mymaster.slaves[i].init_ports();
				break;
			}
		}
	}	

	//print the results of the validation test
	for (int i = 0; i < mymaster.slaves.size(); i++)
	{
		printf("Slave[%d]:\t\t\t\t\t \n", i);
		printf("Master index:\t\t\t\t\t%d\n", mymaster.slaves[i].Get_Slave_Index());
		printf("Alias:\t\t\t\t\t\t%d\n", mymaster.slaves[i].Get_Alias());
		printf("Position:\t\t\t\t\t%d\n", i);
		printf("Relative pos:\t\t\t\t\t%d\n", mymaster.slaves[i].Get_Rel_Pos());

		for (int j = 0; j < mymaster.slaves[i].ports.size(); j++)
		{
			printf("\tPort[%d].Nr\t\t\t\t%d\n", j, mymaster.slaves[i].ports[j].Get_Nr());
			printf("\tPort[%d].Physical_link:\t\t\t%d\n", j, mymaster.slaves[i].ports[j].Get_Physical_Link());
			printf("\tPort[%d].Loop port:\t\t\t%d\n", j, mymaster.slaves[i].ports[j].Get_Loop());
			printf("\tPort[%d].Communication:\t\t\t%d\n", j, mymaster.slaves[i].ports[j].Get_Communication());
		}
		printf("------------------------------------------------------------\n");
	}

	return;
}

void validation_test_2(void)
{
	//initialize the classes with simulation data
	Master mymaster;
	mymaster.slaves.push_back(Slave());	//0
	mymaster.slaves[0].ports.push_back(Port(0));
	mymaster.slaves[0].ports.push_back(Port(1));
	mymaster.slaves[0].ports.push_back(Port(2));
	mymaster.slaves[0].ports.push_back(Port(3));
	mymaster.slaves[0].ports[0].Set_Communication(true);
	mymaster.slaves[0].ports[1].Set_Communication(true);
	mymaster.slaves[0].ports[2].Set_Communication(true);
	mymaster.slaves[0].ports[3].Set_Communication(true);


	mymaster.slaves.push_back(Slave());	//1
	mymaster.slaves[1].ports.push_back(Port(0));
	mymaster.slaves[1].ports.push_back(Port(1));
	mymaster.slaves[1].ports[0].Set_Communication(true);
	mymaster.slaves[1].ports[1].Set_Communication(true);

	mymaster.slaves.push_back(Slave());	//2
	mymaster.slaves[2].ports.push_back(Port(0));
	mymaster.slaves[2].ports.push_back(Port(1));
	mymaster.slaves[2].ports.push_back(Port(2));
	mymaster.slaves[2].ports[0].Set_Communication(true);
	mymaster.slaves[2].ports[1].Set_Communication(true);
	mymaster.slaves[2].ports[2].Set_Communication(true);

	mymaster.slaves.push_back(Slave());	//3
	mymaster.slaves[3].ports.push_back(Port(0));
	mymaster.slaves[3].ports.push_back(Port(1));
	mymaster.slaves[3].ports[0].Set_Communication(true);
	mymaster.slaves[3].ports[1].Set_Communication(false);

	mymaster.slaves.push_back(Slave());	//4
	mymaster.slaves[4].ports.push_back(Port(0));
	mymaster.slaves[4].ports.push_back(Port(1));
	mymaster.slaves[4].ports[0].Set_Communication(true);
	mymaster.slaves[4].ports[1].Set_Communication(false);

	mymaster.slaves.push_back(Slave());	//5
	mymaster.slaves[5].ports.push_back(Port(0));
	mymaster.slaves[5].ports.push_back(Port(1));
	mymaster.slaves[5].ports.push_back(Port(3));
	mymaster.slaves[5].ports[0].Set_Communication(true);
	mymaster.slaves[5].ports[1].Set_Communication(true);
	mymaster.slaves[5].ports[2].Set_Communication(true);

	mymaster.slaves.push_back(Slave()); //6
	mymaster.slaves[6].ports.push_back(Port(0));
	mymaster.slaves[6].ports.push_back(Port(1));
	mymaster.slaves[6].ports[0].Set_Communication(true);
	mymaster.slaves[6].ports[1].Set_Communication(false);

	mymaster.slaves.push_back(Slave()); //7
	mymaster.slaves[7].ports.push_back(Port(0));
	mymaster.slaves[7].ports.push_back(Port(1));
	mymaster.slaves[7].ports[0].Set_Communication(true);
	mymaster.slaves[7].ports[1].Set_Communication(false);

	mymaster.slaves.push_back(Slave()); //8
	mymaster.slaves[8].ports.push_back(Port(0));
	mymaster.slaves[8].ports.push_back(Port(1));
	mymaster.slaves[8].ports[0].Set_Communication(true);
	mymaster.slaves[8].ports[1].Set_Communication(false);

	//Map the topology
	int next_id = 0;
	mymaster.map_topology(0, next_id);
	
	//print the results of the validation test
	for (int i = 0; i < mymaster.slaves.size(); i++)
	{
		printf("slave: %d\n", i);
		for (int j = 0; j < mymaster.slaves[i].iterator.size(); j++)
		{
			Link temp_link = mymaster.slaves[i].ports[mymaster.slaves[i].iterator[j]].Get_Link();
			printf("Poort[%d]:\t slave[%d] \t poort[%d]\n", mymaster.slaves[i].iterator[j], temp_link.s, temp_link.p);
		}
		printf("\n");
	}
	printf("\n------------------------------------------------\n");
	
	for (int i = 0; i < mymaster.data_path.size(); i++) {
		int s = mymaster.data_path[i].s;
		int p = mymaster.data_path[i].p;

		printf("(%d, %d)\t", mymaster.data_path[i].s, mymaster.data_path[i].p);
		printf("%d\n", i);
	}
}
