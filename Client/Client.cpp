// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood
// 1999 June 30



char* getmessage(char *);



/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>

#include <winsock.h>
#include <stdio.h>
#include <iostream>

#include <string.h>

#include <windows.h>
#include <conio.h>

#include <lmcons.h>

using namespace std;

//user defined port number
#define REQUEST_PORT 0x7070;

int port = REQUEST_PORT;



//socket data types
SOCKET s;
SOCKADDR_IN sa;         // filled by bind
SOCKADDR_IN sa_in;      // fill with server info, IP, port



//buffer data types
char szbuffer[128];

char *buffer;

int ibufferlen = 0;

int ibytessent;
int ibytesrecv = 0;



//host data types
HOSTENT *hp;
HOSTENT *rp;

char localhost[21];


//other

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
	char server[64];
	char fileName[64];
	char transferDirection[8];

	try {

		if (WSAStartup(0x0202, &wsadata) != 0){
			cout << "Error in starting WSAStartup()" << endl;
		}
		else {
			buffer = "WSAStartup was successful\n";
			WriteFile(test, buffer, sizeof(buffer), &dwtest, NULL);
		}


		//Display name of local host.

		gethostname(localhost, 20);
		cout << "ftp_tcp starting on host: [" << localhost << "]" << endl;

		cout << "Type name of ftp server: " << flush;
		cin >> server;

		cout << "Type name of file to be transferred: " << flush;
		cin >> fileName;

		cout << "Type direction of transfer: " << flush;
		cin >> transferDirection;

		cout << endl << "Send request to " << server << ", waiting..." << endl;

		if ((hp = gethostbyname(localhost)) == NULL)
			throw "gethostbyname failed\n";

		if ((rp = gethostbyname(server)) == NULL)
			throw "remote gethostbyname failed\n";

		//Create the socket
		if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			throw "Socket failed\n";
		/* For UDP protocol replace SOCK_STREAM with SOCK_DGRAM */

		//Specify server address for client to connect to server.
		memset(&sa_in, 0, sizeof(sa_in));
		memcpy(&sa_in.sin_addr, rp->h_addr, rp->h_length);
		sa_in.sin_family = rp->h_addrtype;
		sa_in.sin_port = htons(port);

		//Connect Client to the server
		if (connect(s, (LPSOCKADDR)&sa_in, sizeof(sa_in)) == SOCKET_ERROR)
			throw "connect failed\n";

		char userName[UNLEN + 1];
		DWORD userNameSize = UNLEN + 1;

		GetUserNameA(userName, &userNameSize);

		sprintf_s(szbuffer, strcat(strcat(strcat(strcat(strcat(strcat(strcat(localhost, ","), fileName), ","), transferDirection), ","), userName), ""));

		ibytessent = 0;
		ibufferlen = strlen(szbuffer);
		ibytessent = send(s, szbuffer, ibufferlen, 0);
		if (ibytessent == SOCKET_ERROR)
			throw "Send failed\n";

		const int BUFFER_LENGTH = 512;
		char* fname = fileName;
		char buffer[BUFFER_LENGTH];
		FILE *file = fopen("textf.txt", "a");

		if (file == NULL)
			cout << "File " << fname << "cannot be opened" << endl;
		else
		{
			int fileBlockSize = 0;
			while ((fileBlockSize = recv(s, buffer, BUFFER_LENGTH, 0)) > 0)
			{
				int write_sz = fwrite(buffer, sizeof(char), fileBlockSize, file);
				if (write_sz < fileBlockSize)
				{
					cout << "Failed to retrieve file from server" << endl;
				}
				if (fileBlockSize == 0 || fileBlockSize != BUFFER_LENGTH)
				{
					break;
				}
			}
			if (fileBlockSize < 0)
			{
				cout << "Error retrieving file from sever" << endl;
			}
			cout << "File was received" << endl;
			fclose(file);
		}
	}

	//Display any needed error response.

	catch (char *str) { cerr << str << ":" << dec << WSAGetLastError() << endl; }

	//close the client socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */
	WSACleanup();
	_getch();
	return 0;
}




