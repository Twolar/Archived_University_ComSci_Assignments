//159.334 - Networks
// CLIENT: prototype for assignment 2. 
//Note that this progam is not yet cross-platform-capable
// This code is different than the one used in previous semesters...
//************************************************************************/
//RUN WITH: Rclient_UDP 127.0.0.1 1235 0 0 
//RUN WITH: Rclient_UDP 127.0.0.1 1235 0 1 
//RUN WITH: Rclient_UDP 127.0.0.1 1235 1 0 
//RUN WITH: Rclient_UDP 127.0.0.1 1235 1 1 
//************************************************************************/

//---
#if defined __unix__ || defined __APPLE__
	#include <unistd.h>
	#include <errno.h>
	#include <stdlib.h>
	#include <stdio.h> 
	#include <string.h>
	#include <sys/types.h> 
	#include <sys/socket.h> 
	#include <arpa/inet.h>
	#include <netinet/in.h> 
	#include <netdb.h>
    #include <iostream>
#elif defined _WIN32 


	//Ws2_32.lib
	#define _WIN32_WINNT 0x501  //to recognise getaddrinfo()

	//"For historical reasons, the Windows.h header defaults to including the Winsock.h header file for Windows Sockets 1.1. The declarations in the Winsock.h header file will conflict with the declarations in the Winsock2.h header file required by Windows Sockets 2.0. The WIN32_LEAN_AND_MEAN macro prevents the Winsock.h from being included by the Windows.h header"
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif

	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <stdio.h> 
	#include <string.h>
	#include <stdlib.h>
	#include <iostream>
#endif

#include "myrandomizer.h"

using namespace std;

#define WSVERS MAKEWORD(2,0)
#define BUFFER_SIZE 80  //used by receive_buffer and send_buffer
                        //the BUFFER_SIZE has to be at least big enough to receive the packet
#define SEGMENT_SIZE 78
//segment size, i.e., if fgets gets more than this number of bytes it segments the message into smaller parts.

#define GENERATOR 0x8005 //0x8005, generator for polynomial division
//for the assignments you can copy and past CRCpolynomial() directly in your C code.
unsigned int CRCpolynomial(char *buffer){
	unsigned char i;
	unsigned int rem=0x0000;
    unsigned int bufsize=strlen(buffer);
	
	while(bufsize--!=0){
		for(i=0x80;i!=0;i/=2){
			if((rem&0x8000)!=0){
				rem=rem<<1;
				rem^=GENERATOR;
			} else{
	   	       rem=rem<<1;
		    }
	  		if((*buffer&i)!=0){
			   rem^=GENERATOR;
			}
		}
		buffer++;
	}
	rem=rem&0xffff;
	return rem;
}

void extractTokens(char *str,  unsigned int &CRC, char *command, int &packetNumber){
	CRC = 0;
	strcpy(command, "");
	packetNumber = 0;
	char * pch;
     
  int tokenCounter=0;
  //printf ("Splitting string \"%s\" into tokens:\n\n",str);
  
  while (1)
  {
  	
	 if(tokenCounter ==0){ 
       pch = strtok (str, " ,.-'\r\n'");
    } else {
		 pch = strtok (NULL, " ,.-'\r\n'");
	 }
	 if(pch == NULL) break;
	 //printf ("Token[%d], with %d characters = %s\n",tokenCounter,int(strlen(pch)),pch);
    switch(tokenCounter){
		case 0: 
			CRC = atoi(pch); //copy CRC to the variable
	        //printf("\tcode = %d\n",CRC);
			break;

      	case 1: //command = new char[strlen(pch)];
			strcpy(command, pch);
			// printf("\tcommand = %s, %d characters\n", command, int(strlen(command)));
            break;			
		case 2: 
			packetNumber = atoi(pch);
		    break;		
    }	
	  
	 tokenCounter++;
  }

}

void filterOutLineEndingCharacter(char *str){
	for(size_t i =0; i< strlen(str); i++){
		if((str[i] == '\r') ||(str[i] == '\n')){
			str[i] = '\0';
		}
	}
}




WSADATA wsadata;
const int ARG_COUNT=5;
//---
int numOfPacketsDamaged=0;
int numOfPacketsLost=0;
int numOfPacketsUncorrupted=0;

int packets_damagedbit=0;
int packets_lostbit=0;


/////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
//*******************************************************************
// Initialization
//*******************************************************************
   struct sockaddr_storage localaddr, remoteaddr;
   char portNum[NI_MAXSERV];
   struct addrinfo *result = NULL;
   struct addrinfo hints;
	
   memset(&localaddr, 0, sizeof(localaddr));  //clean up
   memset(&remoteaddr, 0, sizeof(remoteaddr));//clean up  
   randominit();
   SOCKET s;
   char send_buffer[BUFFER_SIZE],receive_buffer[BUFFER_SIZE];
   int n,bytes,addrlen;
   int ackCount;
   bool canSend = true;
   clock_t startTime;
	addrlen=sizeof(struct sockaddr);
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; //AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	
//********************************************************************
// WSSTARTUP
//********************************************************************
   if (WSAStartup(WSVERS, &wsadata) != 0) {
      WSACleanup();
      printf("WSAStartup failed\n");
   }
//*******************************************************************
//	Dealing with user's arguments
//*******************************************************************
   if (argc != ARG_COUNT) {
	   printf("USAGE: Rclient_UDP remote_IP-address remoteport allow_corrupted_bits(0 or 1) allow_packet_loss(0 or 1)\n");
	   exit(1);
   }
	
	int iResult=0;
	
	sprintf(portNum,"%s", argv[2]);
	iResult = getaddrinfo(argv[1], portNum, &hints, &result);
   
   packets_damagedbit=atoi(argv[3]);
   packets_lostbit=atoi(argv[4]);
   if (packets_damagedbit < 0 || packets_damagedbit > 1 || packets_lostbit< 0 || packets_lostbit>1){
	   printf("USAGE: Rclient_UDP remote_IP-address remoteport allow_corrupted_bits(0 or 1) allow_packet_loss(0 or 1)\n");
	   exit(0);
   }
   
//*******************************************************************
//CREATE CLIENT'S SOCKET 
//*******************************************************************
   s = INVALID_SOCKET; 
   s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

   if (s == INVALID_SOCKET) {
      printf("socket failed\n");
   	exit(1);
   }
    //nonblocking option
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the 
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled; 
	// If iMode != 0, non-blocking mode is enabled.
   u_long iMode=1;
	
   iResult=ioctlsocket(s,FIONBIO,&iMode);
   if (iResult != NO_ERROR){
         printf("ioctlsocket failed with error: %d\n", iResult);
		 closesocket(s);
		 WSACleanup();
		 exit(0);
   }

   cout << "==============<< UDP CLIENT >>=============" << endl;
   cout << "channel can damage packets=" << packets_damagedbit << endl;
   cout << "channel can lose packets=" << packets_lostbit << endl;
	
//*******************************************************************
//SEND A TEXT FILE 
//*******************************************************************
   int counter=0;
   char temp_buffer[BUFFER_SIZE];
   bool sending = false;
   bool closeSending = false;
   char closeFLAG[BUFFER_SIZE];
   // bool breakWhile = false;
   FILE *fin=fopen("data_for_transmission.txt","rb"); //original
	
//In text mode, carriage return–linefeed combinations 
//are translated into single linefeeds on input, and 
//linefeed characters are translated to carriage return–linefeed combinations on output. 
	
   if(fin==NULL){
	   printf("cannot open data_for_transmission.txt\n");
	   closesocket(s);
   	   WSACleanup();
	   exit(0);
   } else {
	   printf("data_for_transmission.txt is now open for sending\n");
	}
   while (1){
		memset(send_buffer, 0, sizeof(send_buffer));//clean up the send_buffer before reading the next line
		if (!feof(fin) || sending || closeSending) {
			if(canSend){

				if(!sending){
					fgets(send_buffer,SEGMENT_SIZE,fin); 
				}else{
					strcpy(send_buffer, temp_buffer);
				}							//get one line of data from the file
				int i =0;
				while (i <SEGMENT_SIZE){
					if (send_buffer[i] == '\n') { /*end on a LF*/
						send_buffer[i] = '\0';
						break;
					}
					if (send_buffer[i] == '\r') /*ignore CRs*/
						send_buffer[i] = '\0';
					i++;
				}
				sending = true;
				char bufferCrc[BUFFER_SIZE];
				sprintf(bufferCrc,"PACKET %d ",counter);
				strcat(bufferCrc, send_buffer);
				unsigned int crc = CRCpolynomial(bufferCrc); 			//get the crc for the buffer that is going to be sent
				sprintf(temp_buffer,"%d PACKET %d ", crc, counter);  	//create packet header with crc and sequence number
				counter++;
				strcat(temp_buffer,send_buffer);   				//append data to packet header
				strcpy(send_buffer,temp_buffer);   				//the complete packet
				memset(temp_buffer, 0, sizeof(temp_buffer)); 	//clean up temp_buffer before reusing it
				strcpy(temp_buffer,send_buffer); 				//copy send_buffer back to temp to hold for retransmission (if packet loss)
				printf("\n======================================================\n");
				cout << "calling send_unreliably, to deliver data of size " << strlen(send_buffer) << endl;
				send_unreliably(s,send_buffer,(result->ai_addr)); //send the packet to the unreliable data channel
				startTime = clock(); 							  // START TIMER
				Sleep(1);  										  //sleep for 1 millisecond
				canSend = false;
			}
			float timePassed = ((float)(clock() - startTime)/CLOCKS_PER_SEC);
			if(timePassed >= 1){
				send_unreliably(s,temp_buffer,(result->ai_addr)); //send the packet to the unreliable data channel (use temp_buffer as send_buffer gets reset every while instance)
				startTime = clock(); 							  
				Sleep(1); 										  //sleep for 1 millisecond
			}																			
//********************************************************************
//RECEIVE
//********************************************************************

			addrlen = sizeof(remoteaddr); //IPv4 & IPv6-compliant
			bytes = recvfrom(s, receive_buffer, 78, 0,(struct sockaddr*)&remoteaddr,&addrlen);
//********************************************************************
//IDENTIFY server's IP address and port number.     
//********************************************************************   
			if(bytes > 0){	   
				char serverHost[NI_MAXHOST]; 
    			char serverService[NI_MAXSERV];	
    			memset(serverHost, 0, sizeof(serverHost));
    			memset(serverService, 0, sizeof(serverService));
    			getnameinfo((struct sockaddr *)&remoteaddr, addrlen,
                			 serverHost, sizeof(serverHost),
                  			 serverService, sizeof(serverService),
                  			 NI_NUMERICHOST);
				printf("\nReceived a packet of size %d bytes from <<<UDP Server>>> with IP address:%s, at Port:%s\n",bytes,serverHost, serverService); 	   
			}
//********************************************************************
//PROCESS REQUEST
//********************************************************************
			//Remove trailing CR and LN
			if(bytes != SOCKET_ERROR){	
				n=0;
				while (n<bytes){
					n++;
					if ((bytes < 0) || (bytes == 0)) break;	
					if (receive_buffer[n] == '\n') { /*end on a LF*/
						receive_buffer[n] = '\0';
						break;
					}
					if (receive_buffer[n] == '\r') /*ignore CRs*/
					receive_buffer[n] = '\0';
				}
				printf("RECEIVED --> %s, %d elements\n",receive_buffer, int(strlen(receive_buffer)));
				unsigned int serverCrc;
				char line[BUFFER_SIZE];
				char command[BUFFER_SIZE];
				if (closeSending){
					char ackChar[BUFFER_SIZE];
					sscanf(receive_buffer, "%d %s %s", &serverCrc, command, ackChar);
					memset(receive_buffer, 0, sizeof(receive_buffer));
					strcpy(receive_buffer, command);
					strcat(receive_buffer, " ");
					strcat(receive_buffer, ackChar);
				}else{
					int ackNum;
					strcpy(line, receive_buffer);
					extractTokens(line, serverCrc, command, ackNum);
					memset(receive_buffer, 0, sizeof(receive_buffer));
					strcpy(receive_buffer, command);
					strcat(receive_buffer, " ");
					char chNum[sizeof(counter)];
					itoa(ackNum, chNum, 10);
					strcat(receive_buffer, chNum);
				}
				unsigned int clientCrc = CRCpolynomial(receive_buffer);
				
				if((strncmp(receive_buffer, "NAK", 3) == 0)||(clientCrc != serverCrc)){ //crc not working properly
					send_unreliably(s,temp_buffer,(result->ai_addr)); //send the packet to the unreliable data channel (use temp_buffer as send_buffer gets reset every while instance)
					sending = true;
					startTime = clock(); 							  // START TIMER
					Sleep(1);  										  //sleep for 1 millisecond
				}else{
					sscanf(receive_buffer, "ACK %d", &ackCount); // extract ackCount from the receive_buffer string
					sscanf(receive_buffer, "ACK %s", closeFLAG); // extract closeFLAG from the receive_buffer string
					// Check to see if received ACK = counter-1 
					if((ackCount == (counter - 1))){ 	// (counter-1 because counter is increased instantly after the packet header has been created.)
						canSend = true; 				// change can send to true because previous packet ACK has been received.
						sending = false;
						memset(temp_buffer, 0, sizeof(temp_buffer)); //clean up the temp_buffer after packet has been ACKed
					}
					if (strncmp(closeFLAG, "CLOSE", 5) == 0){
						//Received close ack
						break;
					}
				}
			}			
		} else {
			fclose(fin);
			printf("End-of-File reached. \n");
			memset(send_buffer, 0, sizeof(send_buffer)); 
			char test[BUFFER_SIZE];
			strcpy(test, "CLOSE");
			int crc = CRCpolynomial(test);
			sprintf(send_buffer,"%d CLOSE", crc); //send a CLOSE command to the RECEIVER (Server)
			memset(temp_buffer, 0, sizeof(temp_buffer)); // Clean up temp_buffer before re using it
			strcpy(temp_buffer,send_buffer); //copy send_buffer back to temp to hold for retransmission (if packet loss)
			printf("\n======================================================\n");
			send_unreliably(s,send_buffer,(result->ai_addr));
			closeSending = true;
			canSend = false;
		}
   } //while loop
//*******************************************************************
//CLOSESOCKET   
//*******************************************************************
   closesocket(s);
   printf("Closing the socket connection and Exiting...\n");
   cout << "==============<< STATISTICS >>=============" << endl;
   cout << "numOfPacketsDamaged=" << numOfPacketsDamaged << endl;
   cout << "numOfPacketsLost=" << numOfPacketsLost << endl;
   cout << "numOfPacketsUncorrupted=" << numOfPacketsUncorrupted << endl;

   exit(0);
}
	
