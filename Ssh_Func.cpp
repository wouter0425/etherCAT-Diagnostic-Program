#include "Ssh_Func.h"
#include <string>

PowerPMACcontrol *ppmaccomm = new PowerPMACcontrol();
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int send_command(PowerPMACcontrol *ppmaccomm, std::string cmd)
{
	int result = 0;
	std::string reply = "";
	int ret = ppmaccomm->PowerPMACcontrol_sendCommand(cmd, reply);
	
	int counter = 0;

	//If value is not a number (nan) or there is an error code (ret != 0), or there is no reply, try again 5 times
	while (reply.compare("nan") == 0 || ret < 0 || reply.empty() == true){
		int ret = ppmaccomm->PowerPMACcontrol_sendCommand(cmd, reply);
		if (counter >= 5) {
			//Reset after 5 attempts
			ecat_reset(ppmaccomm);
		}
		counter++;
	}

	//Remove $ symbol if present, is the first symbol for hex return values
	if (reply[0] == '$'){
		reply.erase(0, 1);

		//create int from string containing the hex value
		result = std::stoi(reply, 0, 16);		
	}
	else{
		//create int from string containing the dec value
		result = std::stoi(reply, 0);
	}	
	return result;
}

void connect_pmac(PowerPMACcontrol *ppmaccomm, con mycon)
{
	int counter = 0;	
	//Connect to the PMAC
	int ret = ppmaccomm->PowerPMACcontrol_connect(mycon.u_ipaddr.c_str(), mycon.u_user.c_str(), mycon.u_passw.c_str(), mycon.u_port.c_str(), mycon.u_nominus2);
	
	//If not able to connect, try again and print the number of attempts 
	while (ret != 0)
	{
		printf("Failed connecting attempt[%d] at %s\n", counter, mycon.u_ipaddr.c_str());
		ret = ppmaccomm->PowerPMACcontrol_connect(mycon.u_ipaddr.c_str(), mycon.u_user.c_str(), mycon.u_passw.c_str(), mycon.u_port.c_str(), mycon.u_nominus2);
		counter++;
	}

	printf("Connected to Power PMAC OK at %s\n", mycon.u_ipaddr.c_str());
	printf("---------------------------------------------------------------------------\n");	
}

void ecat_reset(PowerPMACcontrol *ppmaccomm)
{
	std::string reply = "";
	int ret;
	//send command to reset the pmac
	ret = ppmaccomm->PowerPMACcontrol_sendCommand("ecat reset", reply);	

	//Wait estimated time for bus to reset
	Sleep(5000);
	return;
}