#ifndef SSH_FUNC_H
#define SSH_FUNC_H
#include "libssh2Driver.h"
#include "PowerPMACcontrol.h"
#include <iostream>
#include <string>
#include "argParser.h"
#include "defines.h"
#include <vector>
#include <fstream>

using namespace PowerPMACcontrol_ns;
extern PowerPMACcontrol *ppmaccomm;
extern HANDLE hConsole;

//Struct for connecting to the pmac
typedef struct _con_ {
	std::string u_ipaddr;
	std::string u_user;
	std::string u_passw;
	std::string u_port;
	bool u_nominus2;
}con;

int send_command(PowerPMACcontrol* ppmaccomm, std::string cmd);
void connect_pmac(PowerPMACcontrol *ppmaccomm, con mycon);
void ecat_reset(PowerPMACcontrol *ppmaccomm);

#endif 


