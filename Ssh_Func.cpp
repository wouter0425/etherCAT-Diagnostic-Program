#include "Ssh_Func.h"
#include <string>

PowerPMACcontrol *ppmaccomm = new PowerPMACcontrol();
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int send_command(PowerPMACcontrol *ppmaccomm, std::string cmd)
{
	int result = 0;
	cmd += "\0";
	std::string reply = "";
	int ret = ppmaccomm->PowerPMACcontrol_sendCommand(cmd, reply);
	
	int counter = 0;
	while (reply.compare("nan") == 0 || ret < 0 || reply.empty() == true){
		int ret = ppmaccomm->PowerPMACcontrol_sendCommand(cmd, reply);
		if (counter >= 5) {
			ecat_reset(ppmaccomm);
		}
		if (ret == 0){
			break;
		}
		counter++;
	}

	if (reply[0] == '$'){
		reply.erase(0, 1);
		result = std::stoi(reply, 0, 16);		
	}
	else{
		result = std::stoi(reply, 0);
	}	
	return result;
}

void connect_pmac(PowerPMACcontrol *ppmaccomm, con mycon)
{
	int counter = 0;	
	int ret = ppmaccomm->PowerPMACcontrol_connect(mycon.u_ipaddr.c_str(), mycon.u_user.c_str(), mycon.u_passw.c_str(), mycon.u_port.c_str(), mycon.u_nominus2);
	while (ret != 0)
	{
		printf("Failed connecting attempt[%d] at %s\n", counter, mycon.u_ipaddr.c_str());
		ret = ppmaccomm->PowerPMACcontrol_connect(mycon.u_ipaddr.c_str(), mycon.u_user.c_str(), mycon.u_passw.c_str(), mycon.u_port.c_str(), mycon.u_nominus2);
		counter++;
	}

	printf("Connected to Power PMAC OK at %s\n", mycon.u_ipaddr.c_str());
	printf("---------------------------------------------------------------------------\n");
	Sleep(1000);
}

void read_slaves(PowerPMACcontrol *ppmaccomm)
{
	std::string cmd = "ecat slaves";
	std::string reply = "";
	int slave_pos;
	int slave_alias;

	reply = "0 VID=$00000002 PC=$044D2C52 51:0 PREOP + EK1101 EtherCAT-Koppler (2A E-Bus)\n1 VID=$00000002 PC=$044D2C52 43:0 PREOP + EK1101 EtherCAT-Koppler (2A E-Bus)";
	int i = 0;
	std::string buffer = "";

	while (reply[i] != '\0')
	{
		if (i == 0 || reply[i] == '\n')
		{
			//Maak slave		
			//Get the station position
			while (reply[i] != ' ')
			{
				buffer += reply[i];
				i++;
			}
			slave_pos = std::stoi(buffer, 0);			
			buffer.clear();

			//Get the Alias
			while (reply[i] != ':') { i++; }
			while (reply[i] != ' ') { i--; }
			i++;
			while (reply[i] != ':')
			{
				buffer += reply[i];			
				i++;
			}
			slave_alias = std::stoi(buffer, 0);
			buffer.clear();

			i++;			
			printf("Slave position:\t%d\n", slave_pos);
			printf("Slave alias:\t%d\n", slave_alias);
		}
		i++;
	}		
}

void ecat_state(PowerPMACcontrol *ppmaccomm, bool state)
{
	std::string reply = "";	
	int ret;

	if (state == false)
	{
		ret = ppmaccomm->PowerPMACcontrol_sendCommand("ECAT[0].Enable=0", reply);
	}
	else if (state == true)
	{
		ret = ppmaccomm->PowerPMACcontrol_sendCommand("ECAT[0].Enable=1", reply);
	}
	return;
}

void ecat_reset(PowerPMACcontrol *ppmaccomm)
{
	std::string reply = "";
	int ret;
	ret = ppmaccomm->PowerPMACcontrol_sendCommand("ecat reset", reply);	
	Sleep(5000);
	return;
}

//By far my most ugly written code ever
void clear_screan(void)
{
	for (int i = 0; i < 10; i++)
	{
		printf("\n\n\n\n\n\n\n\n\n\n");
	}
}