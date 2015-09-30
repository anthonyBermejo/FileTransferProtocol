//    SERVER TCP PROGRAM
// revised and tidied up by
// J.W. Atwood
// 1999 June 30
// There is still some leftover trash in this code.

/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h>
#include <iostream>
#include <windows.h>



using namespace std;

//port data types

#define REQUEST_PORT 0x7070

int port = REQUEST_PORT;

//socket data types
SOCKET s;

SOCKET s1;
SOCKADDR_IN sa;      // filled by bind
SOCKADDR_IN sa1;     // fill with server info, IP, port
union {
	struct sockaddr generic;
	struct sockaddr_in ca_in;
}ca;

int calen = sizeof(ca);

//buffer data types
char szbuffer[128];

char *buffer;
int ibufferlen;
int ibytesrecv;

int ibytessent;

//host data types
char localhost[21];

HOSTENT *hp;

//wait variables
int nsa1;
int r, infds = 1, outfds = 0;
struct timeval timeout;
const struct timeval *tp = &timeout;

fd_set readfds;

//others
HANDLE test;

DWORD dwtest;

//reference for used structures

/*  * Host structure

struct  hostent {
char    FAR * h_name;             official name of host *
char    FAR * FAR * h_aliases;    alias list *
short   h_addrtype;               host address type *
short   h_length;                 length of address *
char    FAR * FAR * h_addr_list;  list of addresses *
#define h_addr  h_addr_list[0]            address, for backward compat *
};

* Socket address structure

struct sockaddr_in {
short   sin_family;
u_short sin_port;
struct  in_addr sin_addr;
char    sin_zero[8];
}; */

int main(void){

	WSADATA wsadata;

	try{
		if (WSAStartup(0x0202, &wsadata) != 0){
			cout << "Error in starting WSAStartup()\n";
		}
		else{
			buffer = "WSAStartup was suuccessful\n";
			WriteFile(test, buffer, sizeof(buffer), &dwtest, NULL);
		}

		//Display info of local host

		gethostname(localhost, 20);
		cout << "ftpd_tcp starting at host: [" << localhost << "]" << endl
			<< "waiting to be contacted for trasnferring files..." << endl;

		if ((hp = gethostbyname(localhost)) == NULL) {
			cout << "gethostbyname() cannot get local host info?"
				<< WSAGetLastError() << endl;
			exit(1);
		}

		//Create the server socket
		if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			throw "can't initialize socket";
		// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM 


		//Fill-in Server Port and Address info.
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);


		//Bind the server port

		if (bind(s, (LPSOCKADDR)&sa, sizeof(sa)) == SOCKET_ERROR)
			throw "can't bind the socket";

		//Successfull bind, now listen for client requests.

		if (listen(s, 10) == SOCKET_ERROR)
			throw "couldn't  set up listen on socket";

		FD_ZERO(&readfds);

		char clientName[64];
		char fileName[64];
		char transferDirection[8];
		char userName[64];
		string temp;

		while (1)

		{

			FD_SET(s, &readfds);  //always check the listener

			if (!(outfds = select(infds, &readfds, NULL, NULL, tp))) {}

			else if (outfds == SOCKET_ERROR) throw "failure in Select";

			else if (FD_ISSET(s, &readfds))  cout << "got a connection request" << endl;

			//Found a connection request, try to accept. 

			if ((s1 = accept(s, &ca.generic, &calen)) == INVALID_SOCKET)
				throw "Couldn't accept connection\n";

			//Fill in szbuffer from accepted request.
			if ((ibytesrecv = recv(s1, szbuffer, 128, 0)) == SOCKET_ERROR)
				throw "Receive error in server program\n";

			//Interpret request from client
			string tempBuffer(szbuffer);
			size_t pos = tempBuffer.find(",");
			strcpy(clientName, tempBuffer.substr(0, pos).c_str());

			tempBuffer = tempBuffer.substr(pos + 1);
			pos = tempBuffer.find(",");
			strcpy(fileName, tempBuffer.substr(0, pos).c_str());

			tempBuffer = tempBuffer.substr(pos + 1);
			pos = tempBuffer.find(",");
			strcpy(transferDirection, tempBuffer.substr(0, pos).c_str());

			tempBuffer = tempBuffer.substr(pos + 1);
			pos = tempBuffer.find("\\");
			strcpy(userName, tempBuffer.substr(0,pos).c_str());

			// Length of buffer
			const int BUFFER_LENGTH = 512;

			// GET - Send file to client
			if (strcmp(transferDirection, "get") == 0) {

				cout << "User \"" << userName << "\" requested file " << fileName << " to be sent." << endl
					<< "Sending file to " << clientName << endl << endl;

				FILE * file = fopen(fileName, "r");
				if (file == NULL) {
					cout << "File not found" << endl << endl;
				}
				char buffer[BUFFER_LENGTH];

				int fileBlockSize = 0;

				while ((fileBlockSize = fread(buffer, sizeof(char), BUFFER_LENGTH, file)) > 0)
				{
					if (send(s1, buffer, fileBlockSize, 0) < 0)
						cout << "Failed to send file" << endl << endl;
				}

				cout << "File sent to client" << endl << endl;
			}
			// PUT - Receive file from client
			else if (strcmp(transferDirection, "put") == 0) {

				cout << "User \"" << userName << "\" requested file " << fileName << " to be received." << endl
					<< "Receiving file from " << clientName << endl << endl;

				char* fname = fileName;
				char buffer[BUFFER_LENGTH];
				FILE *file = fopen("textf.txt", "a");

				if (file == NULL)
					cout << "File " << fname << "cannot be opened" << endl << endl;
				else
				{
					int fileBlockSize = 0;
					while ((fileBlockSize = recv(s1, buffer, BUFFER_LENGTH, 0)) > 0)
					{
						int writeSize = fwrite(buffer, sizeof(char), fileBlockSize, file);
						if (writeSize < fileBlockSize)
						{
							cout << "Failed to retrieve file from client" << endl << endl;
						}
						if (fileBlockSize == 0 || fileBlockSize != BUFFER_LENGTH)
						{
							break;
						}
					}
					if (fileBlockSize < 0)
					{
						cout << "Error retrieving file from client" << endl << endl;
					}
					else
						cout << "File was received" << endl << endl;

					fclose(file);
				}
			}
			// LIST - List files available for transfer
			else if (strcmp(transferDirection, "list") == 0) {
				system("dir /b >> dir.txt");
			}
			else {
				cout << "Invalid transfer direction" << endl << endl;
			}

		}//wait loop

	} //try loop

	//Display needed error message.

	catch (char* str) { cerr << str << WSAGetLastError() << endl; }

	//close Client socket
	closesocket(s1);

	//close server socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */
	WSACleanup();
	return 0;
}



