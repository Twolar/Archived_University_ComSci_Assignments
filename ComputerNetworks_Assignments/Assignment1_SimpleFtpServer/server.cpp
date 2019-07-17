//=======================================================================================================================
//ACTIVE FTP SERVER Start-up Code for Assignment 1 (WinSock 2)

//This code gives parts of the answers away but this implementation is only IPv4-compliant. 
//Remember that the assignment requires a full IPv6-compliant FTP server that can communicate with a built-in FTP client either in Windows 10 or Ubuntu Linux.
//Firstly, you must change parts of this program to make it IPv6-compliant (replace all data structures that work only with IPv4).
//This step would require creating a makefile, as the IPv6-compliant functions require data structures that can be found only by linking with the appropritate library files. 
//The sample TCP server codes will help you accomplish this.

//OVERVIEW
//The connection is established by ignoring USER and PASS, but sending the appropriate 3 digit codes back
//only the active FTP mode connection is implemented (watch out for firewall issues - do not block your own FTP server!).

//The ftp LIST command is fully implemented, in a very naive way using redirection and a temporary file.
//The list may have duplications, extra lines etc, don't worry about these. You can fix it as an exercise, 
//but no need to do that for the assignment.
//In order to implement RETR you can use the LIST part as a startup.  RETR carries a filename, 
//so you need to replace the name when opening the file to send.


//=======================================================================================================================
//Ws2_32.lib
#define _WIN32_WINNT 0x501  //to recognise getaddrinfo()

//"For historical reasons, the Windows.h header defaults to including the Winsock.h header file for Windows Sockets 1.1. The declarations in the Winsock.h header file will conflict with the declarations in the Winsock2.h header file required by Windows Sockets 2.0. The WIN32_LEAN_AND_MEAN macro prevents the Winsock.h from being included by the Windows.h header"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


//159.334 - Networks
//single threaded server
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

//#pragma directives offer a way for each compiler to offer machine- and operating system-specific features
// Need to link with Ws2_32.lib
//#pragma comment (lib, "Ws2_32.lib") 

#define USERNAME "napoleon"   	//authorized user 
#define SECRET_PASSWORD "334"

#define WSVERS MAKEWORD(2,2) /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
                             //The high-order byte specifies the minor version number; 
									  //the low-order byte specifies the major version number.

#define USE_IPV6 false

#define DEFAULT_PORT "1234" 

WSADATA wsadata; //Create a WSADATA object called wsadata. 

//********************************************************************
//MAIN
//********************************************************************
int main(int argc, char *argv[]) {
//********************************************************************
// INITIALIZATION
//********************************************************************
	     int err = WSAStartup(WSVERS, &wsadata);

		 if (err != 0) {
		      WSACleanup();
			  // Tell the user that we could not find a usable WinsockDLL
		      printf("WSAStartup failed with error: %d\n", err);
		      exit(1);
		 }
		//********************************************************************
		// set the socket address structure.
		//********************************************************************
		struct sockaddr_storage clientAddress; //IPV6
	
		char clientHost[NI_MAXHOST]; 
		char clientService[NI_MAXSERV];
   		SOCKET s,ns;
   		SOCKET ns_data, s_data_act;
   		char send_buffer[200],receive_buffer[200];
   		int n,bytes,addrlen;
		char portNum[NI_MAXSERV];
		char username[80]; 
		char password[80];
		struct addrinfo *result = NULL;
		struct addrinfo hints;
		int iResult;
		struct sockaddr_in local_data_addr_act;
		char localService[NI_MAXSERV];
		struct addrinfo localHints;
		struct addrinfo *local_result = NULL;
		bool isBinary = true;
		memset(&hints, 0, sizeof(struct addrinfo));
		memset(&localHints, 0, sizeof(struct addrinfo));
		if(USE_IPV6){
			hints.ai_family = AF_INET6;
		}else{
			hints.ai_family = AF_INET; 
		}

		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE; 		// For wildcard IP address 
                             				//setting the AI_PASSIVE flag indicates the caller intends to use 
											//the returned socket address structure in a call to the bind function. 
        ns_data=INVALID_SOCKET;
		int active=0;
		printf("\n===============================\n");
		printf("     159.334 FTP Server");
		printf("\n===============================\n");

		//********************************************************************
		//SOCKET
		//********************************************************************
		// Resolve the local address and port to be used by the server
		if(argc==2){	 
	 	iResult = getaddrinfo(NULL, argv[1], &hints, &result);  //converts human-readable text strings representing hostnames or IP addresses 
	                                                        	//into a dynamically allocated linked list of struct addrinfo structures
																//IPV4 & IPV6-compliant
	 	sprintf(portNum,"%s", argv[1]);
	 	printf("\nargv[1] = %s\n", argv[1]); 	
		} else {
    		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result); //converts human-readable text strings representing hostnames or IP addresses 
	                                                            		//into a dynamically allocated linked list of struct addrinfo structures
																		//IPV4 & IPV6-compliant
	 		sprintf(portNum,"%s", DEFAULT_PORT);
	 		printf("\nUsing DEFAULT_PORT = %s\n", portNum); 
		}

		if (iResult != 0) {
    		printf("getaddrinfo failed: %d\n", iResult);
    		WSACleanup();
    		return 1;
		}	 

		//********************************************************************
		// STEP#1 - Create welcome SOCKET
		//********************************************************************

		s = INVALID_SOCKET; //socket for listening
		// Create a SOCKET for the server to listen for client connections

		s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

		//check for errors in socket allocation
		if (s == INVALID_SOCKET) {
    		printf("Error at socket(): %d\n", WSAGetLastError());
    		freeaddrinfo(result);
    		WSACleanup();
    		exit(1);//return 1;
		}
		//********************************************************************
		
		//********************************************************************
		//STEP#2 - BIND the welcome socket
		//********************************************************************

		// bind the TCP welcome socket to the local address of the machine and port number
    	iResult = bind( s, result->ai_addr, (int)result->ai_addrlen);

    	if (iResult == SOCKET_ERROR) {
        	printf("bind failed with error: %d\n", WSAGetLastError());
       	 	freeaddrinfo(result);
        	closesocket(s);
        	WSACleanup();
        	return 1;
    	}
	 
	 	freeaddrinfo(result); //free the memory allocated by the getaddrinfo 
	                       //function for the server's address, as it is 
	                       //no longer needed
		//********************************************************************
		//********************************************************************
		//STEP#3 - LISTEN on welcome socket for any incoming connection
		//********************************************************************
		if (listen( s, SOMAXCONN ) == SOCKET_ERROR ) {
     		printf( "Listen failed with error: %d\n", WSAGetLastError() );
     		closesocket(s);
     		WSACleanup();
     		exit(1);
   		} else {
			printf("\n<<<SERVER>>> is listening at PORT: %s\n", portNum);
		}
		//********************************************************************
		//INFINITE LOOP
		//********************************************************************

		 //====================================================================================
		 while (1) {//Start of MAIN LOOP
		 //====================================================================================
			addrlen = sizeof(clientAddress); //IPv4 & IPv6-compliant
		
			//********************************************************************
			//NEW SOCKET newsocket = accept
			//********************************************************************
      
	   		ns = INVALID_SOCKET;

			//********************************************************************	
			// STEP#4 - Accept a client connection.  
			//	accept() blocks the iteration, and causes the program to wait.  
			//	Once an incoming client is detected, it returns a new socket ns
			// exclusively for the client.  
			// It also extracts the client's IP address and Port number and stores
			// it in a structure.
			//********************************************************************
	
			ns = accept(s,(struct sockaddr *)(&clientAddress),&addrlen); //IPV4 & IPV6-compliant
	
			if (ns == INVALID_SOCKET) {
		 		printf("accept failed: %d\n", WSAGetLastError());
		 		closesocket(s);
		 		WSACleanup();
		 		return 1;
			} else {
				printf("\nA <<<CLIENT>>> has been accepted.\n");
				//strcpy(clientHost,inet_ntoa(clientAddress.sin_addr)); //IPV4
				//sprintf(clientService,"%d",ntohs(clientAddress.sin_port)); //IPV4
				memset(clientHost, 0, sizeof(clientHost));
				memset(clientService, 0, sizeof(clientService));
      			getnameinfo((struct sockaddr *)&clientAddress, addrlen, clientHost, sizeof(clientHost), clientService, sizeof(clientService), NI_NUMERICHOST);
     			printf("\nConnected to <<<Client>>> with IP address:%s, at Port:%s, through SERVER's Port:%s\n",clientHost, clientService, portNum);
			}	

			//********************************************************************
			//Respond with welcome message
			//*******************************************************************
			 sprintf(send_buffer,"220 FTP Server ready. \r\n");
			 bytes = send(ns, send_buffer, strlen(send_buffer), 0);

			//********************************************************************
			//COMMUNICATION LOOP per CLIENT
			//********************************************************************

			 while (1) {
				 n = 0;
				 //********************************************************************
			     //PROCESS message received from CLIENT
			     //********************************************************************
				 
				 while (1) {
				//********************************************************************
				//RECEIVE MESSAGE AND THEN FILTER IT
				//********************************************************************
				     bytes = recv(ns, &receive_buffer[n], 1, 0);//receive byte by byte...

					 if ((bytes < 0) || (bytes == 0)) break;
					 if (receive_buffer[n] == '\n') { /*end on a LF*/
						 receive_buffer[n] = '\0';
						 break;
					 }
					 if (receive_buffer[n] != '\r') n++; /*Trim CRs*/
				 //=======================================================
				 } //End of PROCESS message received from CLIENT
				 //=======================================================
				 if ((bytes < 0) || (bytes == 0)) break;

				 printf("<< DEBUG INFO. >>: the message from the CLIENT reads: '%s\\r\\n' \n", receive_buffer);

				//********************************************************************
				//PROCESS COMMANDS/REQUEST FROM USER
				//********************************************************************
				 
				 if (strncmp(receive_buffer,"USER",4)==0)  {
				 	//********************************************************************
					// FTP Server Command: USER
					// Allows the client to enter a username used by the server
					//********************************************************************
				 	sscanf(receive_buffer, "USER %s", username);
					printf("Logging in... \n");
					sprintf(send_buffer,"331 Username okay, need password.\r\n");
					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					if (bytes < 0) break;

				 }else if (strncmp(receive_buffer,"PASS",4)==0)  {
				 	//********************************************************************
					// FTP Server Command: PASS
					// Allows the client to enter a password for their username
					//********************************************************************
				 	sscanf(receive_buffer, "PASS %s", password);
					 if(strcmp(username, USERNAME) == 0){

					 	if(strcmp(password, SECRET_PASSWORD) == 0){
					 		sprintf(send_buffer,"230 Authorised User logged in, proceed.\r\n");
					 		printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
					 		bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 		if (bytes < 0) break;
					 	}else{
					 		sprintf(send_buffer,"430 Invalid password for username %s\r\n", USERNAME);
					 		printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
					 		bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 		if (bytes < 0) break;
					 	}

					 }else{
					 	sprintf(send_buffer,"230 Public User logged in, proceed.\r\n");
					 	printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
					 	bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 	if (bytes < 0) break;
					}
				 }else if (strncmp(receive_buffer,"SYST",4)==0)  {

				 	//********************************************************************
					// FTP Server Command: SYST
					// Returns infomation about the OS
					//********************************************************************
					 printf("Information about the system \n");
					 sprintf(send_buffer,"215 Windows 64-bit\r\n");
					 printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
					 bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 if (bytes < 0) break;

				 }else if(strncmp(receive_buffer,"TYPE",4)==0){

				 	//********************************************************************
					// FTP Server Command: TYPE
					// Specifies the type for file transfer
					//********************************************************************
				 	 char dataType;
                     sscanf(receive_buffer, "TYPE %c", &dataType);
                     if(dataType == 'I'){
                        isBinary = true;
                        sprintf(send_buffer,"250 File mode binary successful\r\n");
					 	printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
					 	bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 	if (bytes < 0) break;
                     } else {
                        isBinary = false;
                        sprintf(send_buffer,"250 File mode ascii successful\r\n");
					 	printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
					 	bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 	if (bytes < 0) break;
                   }

				 }else if (strncmp(receive_buffer,"OPTS",4)==0)  {
				 	//********************************************************************
					// FTP Server Command: OPTS
					// unrecognised command
					//********************************************************************

					 printf("502 Unrecognised Command (not implemented)\n");
					 sprintf(send_buffer,"502 Command not implemented.\r\n");
					 printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
					 bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 if (bytes < 0) break;

				 }else if (strncmp(receive_buffer,"QUIT",4)==0)  {
				 	//********************************************************************
					// FTP Server Command: QUIT
					// Used to disconnect the client from the server
					//********************************************************************

					 printf("Quit \n");
					 sprintf(send_buffer,"221 Service closing control connection. Logged out if appropriate.\r\n");
					 printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
					 bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 if (bytes < 0) break;
					 closesocket(ns);

				 }else if(strncmp(receive_buffer,"EPRT",4)==0){
				 	//********************************************************************
					// FTP Server Command: EPRT
					// Connects to the clients port 
					// RFC 2428 Compliant (pg. 2-3)
					//********************************************************************

				 	 int ip_v;
					 char ip_decimal[100];
					 printf("===================================================\n");
					 printf("\n\tActive FTP mode, the client is listening... \n");
					 active=1; //flag for active connection
					 //remove the delimiters 
					 int i =0;
					 while(receive_buffer[i] != '\0'){
					 	if(receive_buffer[i] == '|'){
					 		receive_buffer[i] = ' ';
					 	}
					 	i++;
					 }
					 int scannedItems = sscanf(receive_buffer, "EPRT %d %s %s",&ip_v, ip_decimal, localService);
					 if(scannedItems < 3) {
		         	    sprintf(send_buffer,"501 Syntax error in parameters or arguments.\r\n");
						printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);
			            break;
		             }
					 printf("\tCLIENT's IP is %s\n", ip_decimal);  //IPv6
					 printf("\tCLIENT's Port is %s\n", localService);
					 printf("===================================================\n");
 					 localHints.ai_family = AF_INET6;
					 localHints.ai_socktype = SOCK_STREAM;
					 localHints.ai_protocol = IPPROTO_TCP;
					 iResult = getaddrinfo(NULL, localService, &localHints, &local_result);
					 if (iResult != 0) {
    					printf("getaddrinfo failed: %d\n", iResult);
    					WSACleanup();
    					return 1;
					 }
					 s_data_act = INVALID_SOCKET;
					 s_data_act = socket(local_result->ai_family, local_result->ai_socktype, local_result->ai_protocol); 
					 if (s_data_act == INVALID_SOCKET) {
      					printf("socket has failed.\n");
						freeaddrinfo(local_result);
						WSACleanup();
   						return 1;
   					 }

					 if ((connect(s_data_act, local_result->ai_addr, local_result->ai_addrlen)) != 0){
						 printf("trying connection in %s %s\n", ip_decimal, localService);
						 printf("connect error = %d",WSAGetLastError());
						 sprintf(send_buffer, "425 Can't open data connection.\r\n");
						 bytes = send(ns, send_buffer, strlen(send_buffer), 0);
						 printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
						 closesocket(s_data_act);
						 return 1;
					 }
					 else {
						 sprintf(send_buffer, "200 EPRT Command Successful\r\n");
						 bytes = send(ns, send_buffer, strlen(send_buffer), 0);
						 printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
						 printf("Connected to client\n");
					 }
				 }else if(strncmp(receive_buffer,"PORT",4)==0) {
				 	//********************************************************************
					// FTP Server Command: PORT
					// Connects to the clients port (ipv4 only)
					//********************************************************************

					 s_data_act = socket(AF_INET, SOCK_STREAM, 0);
					 int act_port[2];
					 int act_ip[4], port_dec;
					 char ip_decimal[40];
					 printf("===================================================\n");
					 printf("\n\tActive FTP mode, the client is listening... \n");
					 active=1;//flag for active connection
					 int scannedItems = sscanf(receive_buffer, "PORT %d,%d,%d,%d,%d,%d",
							&act_ip[0],&act_ip[1],&act_ip[2],&act_ip[3],
					      &act_port[0],&act_port[1]);
					 if(scannedItems < 6) {
		         	    sprintf(send_buffer,"501 Syntax error in parameters or arguments.\r\n");
						printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
						bytes = send(ns, send_buffer, strlen(send_buffer), 0);
			            break;
		             }
					 local_data_addr_act.sin_family=AF_INET;//local_data_addr_act  //ipv4 only
					 sprintf(ip_decimal, "%d.%d.%d.%d", act_ip[0], act_ip[1], act_ip[2],act_ip[3]);
					 printf("\tCLIENT's IP is %s\n",ip_decimal);  //IPv4 format
					 local_data_addr_act.sin_addr.s_addr=inet_addr(ip_decimal);  //ipv4 only
					 port_dec=act_port[0];
					 port_dec=port_dec << 8;
					 port_dec=port_dec+act_port[1];
					 printf("\tCLIENT's Port is %d\n",port_dec);
					 printf("===================================================\n");
					 local_data_addr_act.sin_port = htons(port_dec); //ipv4 only
					 if (connect(s_data_act, (struct sockaddr *)&local_data_addr_act, (int) sizeof(struct sockaddr)) != 0){
						 printf("trying connection in %s %d\n",inet_ntoa(local_data_addr_act.sin_addr),ntohs(local_data_addr_act.sin_port));
						 sprintf(send_buffer, "425 Can't open data connection.\r\n");
						 printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
						 bytes = send(ns, send_buffer, strlen(send_buffer), 0);
						 closesocket(s_data_act);
					 }
					 else {
						 sprintf(send_buffer, "200 PORT Command Successful\r\n");
						 bytes = send(ns, send_buffer, strlen(send_buffer), 0);
						 printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
						 printf("Connected to client\n");
					 }

				 }else if (strncmp(receive_buffer,"RETR",4)==0)  {
				 	//********************************************************************
					// FTP Server Command: RETR
					// Used to transfer files from the server to the client
					//********************************************************************	

                	char fname[120]; 
                	sscanf(receive_buffer, "RETR %s", fname);
                	FILE* fp;	
    				// HANDLES BINARY OR ASCII FILE TRANSFER (CHECKS FILE BEVORE SENDING STATUS)
                	if(isBinary){
                		fp = fopen(fname, "rb");
                		if (fp == NULL){
                			sprintf(send_buffer,"550 Requested action not taken. File unavailable (e.g., file not found, no access).\r\n");
                			printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
                			bytes = send(ns, send_buffer, strlen(send_buffer), 0);
                			if (bytes < 0) break;
                			break;
                		} else { sprintf(send_buffer,"150 File status okay; about to open binary data connection.\r\n"); }
                	}else{
                		fp = fopen(fname, "r");
                		if (fp == NULL){
                			sprintf(send_buffer,"550 Requested action not taken. File unavailable (e.g., file not found, no access).\r\n");
                			printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
                			bytes = send(ns, send_buffer, strlen(send_buffer), 0);
                			if (bytes < 0) break;
                			break;
                		} else { sprintf(send_buffer,"150 File status okay; about to open ascii data connection.\r\n"); }
                	}
                	printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
                	bytes = send(ns, send_buffer, strlen(send_buffer), 0);
                	if (bytes < 0) break;

    				if (fseek(fp, 0, SEEK_END) != 0) {
        				fclose(fp);
        				break;
   					}
   					long int sizeOfFile = ftell(fp);
   				 	if (fseek(fp, 0, SEEK_SET) != 0) {
        				fclose(fp);
        				break;
    				}
    				long int sizeOfNewFile = 0;
    				char buffer[4096];
    				for (;;) {
        				size_t read = fread(buffer, 1, sizeof(buffer), fp); // read = sizeof(buffer)
        				sizeOfNewFile +=read;
        				if (read == 0) {
            				fclose(fp);
            				if(sizeOfFile == sizeOfNewFile){

                				sprintf(send_buffer,"226 File transfer complete. \r\n");
                				printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
                				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
            					break;
            				}else{
            					if(isBinary){
            						fclose(fp);
                					sprintf(send_buffer,"450 Complete file transfer failed (Original File Size: %ld bytes vs New File Size: %ld bytes)\r\n", sizeOfFile, sizeOfNewFile);
                					printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
                					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
                					break;
                				}else{
                					sprintf(send_buffer,"226 File transfer complete. \r\n");
                					printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
                					bytes = send(ns, send_buffer, strlen(send_buffer), 0);
            						break;
                				}
            				}
       				 	}
						for (size_t sent = 0; sent < read;) {
            				int numOfBytes = send(s_data_act, buffer + sent, read - sent, 0);
            				if (numOfBytes < 0) {
                				fclose(fp);
                				sprintf(send_buffer,"450 Requested file action not taken \r\n");
                				printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
                				bytes = send(ns, send_buffer, strlen(send_buffer), 0);
                				break;
            				}
           					sent += numOfBytes;
        				}
    				}
                 	closesocket(s_data_act);
				}else if ( (strncmp(receive_buffer,"LIST",4)==0) || (strncmp(receive_buffer,"NLST",4)==0)||(strncmp(receive_buffer,"PWD",4)==0))   {
					//********************************************************************
					// FTP Server Command: LIST
					// Gets the current directory of the server
					//********************************************************************

					 //system("ls > tmp.txt");//change that to 'dir', so windows can understand
					 system("dir > tmp.txt");
					 FILE *fin=fopen("tmp.txt","r");//open tmp.txt file
					 //sprintf(send_buffer,"125 Transfering... \r\n");
					 if (fin == NULL){
                			sprintf(send_buffer,"550 Requested action not taken. File unavailable (e.g., file not found, no access).\r\n");
                			printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
                			bytes = send(ns, send_buffer, strlen(send_buffer), 0);
                			if (bytes < 0) break;
                			break;
                		} else { sprintf(send_buffer,"150 Here comes the directory listing.\r\n"); }
					 printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
					 bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 char temp_buffer[80];
					 while (!feof(fin)){
						 fgets(temp_buffer,78,fin);
						 sprintf(send_buffer,"%s",temp_buffer);
						 if (active==0) send(ns_data, send_buffer, strlen(send_buffer), 0);
						 else send(s_data_act, send_buffer, strlen(send_buffer), 0);
					 }
					 fclose(fin);
					 //sprintf(send_buffer,"250 File transfer completed... \r\n");
					 sprintf(send_buffer,"226 Directory sent OK \r\n");
					 printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
					 bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 if (active==0 )closesocket(ns_data);
					 else closesocket(s_data_act);
					 //OPTIONAL, delete the temporary file
					 system("del tmp.txt");
				 }else{
					printf("502 Unrecognised Command (not implemented)\n");
				 	sprintf(send_buffer,"502 Command not implemented.\r\n");
				 	printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
				 	bytes = send(ns, send_buffer, strlen(send_buffer), 0);
				 	if (bytes < 0) break;
				 }
                 //---	    
			 //=================================================================================	 
			 }//End of COMMUNICATION LOOP per CLIENT
			 //=================================================================================
			 
			//********************************************************************
			//CLOSE SOCKET
			//********************************************************************
			 closesocket(ns);
			 printf("DISCONNECTED from %s and listening for another client\n",clientHost);
			 /**
			 sprintf(send_buffer, "221 Service closing control connection. Logged out if appropriate.2\r\n");
			 printf("<< DEBUG INFO. >>: REPLY sent to CLIENT: %s\n", send_buffer);
			 bytes = send(ns, send_buffer, strlen(send_buffer), 0);
			 **/
			 
		 //====================================================================================
		 } //End of MAIN LOOP
		 //====================================================================================
		 closesocket(s);
		 printf("\nSERVER SHUTTING DOWN...\n");
		 exit(0);
}

