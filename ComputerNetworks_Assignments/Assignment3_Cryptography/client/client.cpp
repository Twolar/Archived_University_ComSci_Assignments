//////////////////////////////////////////////////////////////////////////////////////////////
// TCP CrossPlatform CLIENT v.1.0 (towards IPV6 ready)
// compiles using GCC 
//
//
// References: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
//             http://long.ccaba.upc.edu/long/045Guidelines/eva/ipv6.html#daytimeServer6
//             Andre Barczak's tcp client codes
//
// Author: Napoleon Reyes, Ph.D.
//         Massey University, Albany  
//
//////////////////////////////////////////////////////////////////////////////////////////////

#define USE_IPV6 true
#define DEFAULT_PORT "1234" 

#if defined __unix__ || defined __APPLE__
  #include <unistd.h>
  #include <errno.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h> //used by getnameinfo()
  #include <cstdio>
  #include <iostream>
#elif defined _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h> //required by getaddrinfo() and special constants
  #include <stdlib.h>
  #include <stdio.h>
  #include <cstdio>
  #include <iostream>
  #define WSVERS MAKEWORD(2,2) /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
                    //The high-order byte specifies the minor version number; 
                    //the low-order byte specifies the major version number.

  WSADATA wsadata; //Create a WSADATA object called wsadata. 
#endif
#include "bigInt.h" // big number library
#include<time.h>

//////////////////////////////////////////////////////////////////////////////////////////////


enum CommandName{USER, PASS, SHUTDOWN};


using namespace std;
/////////////////////////////////////////////////////////////////////

void printBuffer(const char *header, char *buffer){
	cout << "------" << header << "------" << endl;
	for(unsigned int i=0; i < strlen(buffer); i++){
		if(buffer[i] == '\r'){
		   cout << "buffer[" << i << "]=\\r" << endl;	
		} else if(buffer[i] == '\n'){
		   cout << "buffer[" << i << "]=\\n" << endl;	
		} else {   
		   cout << "buffer[" << i << "]=" << (long)buffer[i] << endl;
		}
	}
	cout << "---" << endl;
}

/******************************************
* a function used to encrypt or decrypt a character using RSA
* @param x the character
* @param n the n value
* @param e private or public key 
* @return long the result
*****************************************/
long encryptionDecryption(long x, long e, long n) {
	long y=1;//initialize y to 1, very important
	while (e >  0) {
		if (( e % 2 ) == 0) {
			x = (x*x) % n;
			e = e/2;
		}
		else {
			y = (x*y) % n;
			e = e-1;
		}
	}
	return y; //the result is stored in y
}

/******************************************
* a function used to encrypt a character using RSA and CBC
* @param c the nonce or previous encrpyted m value
* @param m the character
* @param e the public key 
* @param n the n value
* @return long the result
*****************************************/
long calculateRSA_CBC(long c, char m, long e, long n){
	unsigned long temp = (m ^ c);
	long result = encryptionDecryption((long)temp, e, n);
	return result;
}

/////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
//*******************************************************************
// Initialization
// What are the important data structures?
//*******************************************************************



   char portNum[NI_MAXSERV];

#if defined __unix__ || defined __APPLE__
   int s;
#elif defined _WIN32
   SOCKET s;
#endif

#define BUFFER_SIZE 200 
//remember that the BUFFESIZE has to be at least big enough to receive the answer from the server
#define SEGMENT_SIZE 70
//segment size, i.e., if fgets gets more than this number of bytes it segments the message
	//public key variables
   long serverPublicKeyE = 0; 
   long serverPublicKeyN = 0; 

   long caPublicKeyE = 11;      
   long caPublicKeyN = 31007;      
   long nonce;
   long c; 
   char temp[BUFFER_SIZE];
   char send_buffer[BUFFER_SIZE],receive_buffer[BUFFER_SIZE];
   int n,bytes;
	
   char serverHost[NI_MAXHOST]; 
   char serverService[NI_MAXSERV];

   char message[BUFFER_SIZE];
	
   //memset(&sin, 0, sizeof(sin));

#if defined __unix__ || defined __APPLE__
   //nothing to do here

#elif defined _WIN32
//********************************************************************
// WSSTARTUP
//********************************************************************

//********************************************************************
// WSSTARTUP
/*  All processes (applications or DLLs) that call Winsock functions must 
  initialize the use of the Windows Sockets DLL before making other Winsock 
  functions calls. 
  This also makes certain that Winsock is supported on the system.
*/
//********************************************************************
   int err;
  
   err = WSAStartup(WSVERS, &wsadata);
   if (err != 0) {
      WSACleanup();
    /* Tell the user that we could not find a usable */
    /* Winsock DLL.                                  */
      printf("WSAStartup failed with error: %d\n", err);
      exit(1);
   }
  
//********************************************************************
/* Confirm that the WinSock DLL supports 2.2.        */
/* Note that if the DLL supports versions greater    */
/* than 2.2 in addition to 2.2, it will still return */
/* 2.2 in wVersion since that is the version we      */
/* requested.                                        */
//********************************************************************
    printf("\n\n<<<TCP (CROSS-PLATFORM, IPv6-ready) CLIENT, by nhreyes>>>\n");  

    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        exit(1);
    }
    else{
         
        printf("\nThe Winsock 2.2 dll was initialised.\n");
    }

#endif


//********************************************************************
// set the socket address structure.
//
//********************************************************************
struct addrinfo *result = NULL;
struct addrinfo hints;
int iResult;


//ZeroMemory(&hints, sizeof (hints)); //alternatively, for Windows only
memset(&hints, 0, sizeof(struct addrinfo));


if(USE_IPV6){
   hints.ai_family = AF_INET6;  
} else { //IPV4
   hints.ai_family = AF_INET;
}

hints.ai_socktype = SOCK_STREAM;
hints.ai_protocol = IPPROTO_TCP;
//hints.ai_flags = AI_PASSIVE;// PASSIVE is only for a SERVER	
	
//*******************************************************************
//	Dealing with user's arguments
//*******************************************************************
 
   
	
	//if there are 3 parameters passed to the argv[] array.
   if (argc == 3){ 
		//sin.sin_port = htons((u_short)atoi(argv[2])); //get Remote Port number
	    sprintf(portNum,"%s", argv[2]);
	    iResult = getaddrinfo(argv[1], portNum, &hints, &result);
	   //iResult = getaddrinfo("0:0:0:0:0:0:0:1", portNum, &hints, &result); //works! test only!
	} else {
		//sin.sin_port = htons(1234); //use default port number
	    printf("USAGE: ClientWindows IP-address [port]\n"); //missing IP address
		sprintf(portNum,"%s", DEFAULT_PORT);
		printf("Default portNum = %s\n",portNum);
		printf("Using default settings, IP:127.0.0.1, Port:1234\n");
		iResult = getaddrinfo("127.0.0.1", portNum, &hints, &result);
	}
	
	if (iResult != 0) {
		 printf("getaddrinfo failed: %d\n", iResult);
#if defined _WIN32
         WSACleanup();
#endif  
		 return 1;
   }	 
		
//*******************************************************************
//CREATE CLIENT'S SOCKET 
//*******************************************************************

#if defined __unix__ || defined __APPLE__
  	s = -1;
#elif defined _WIN32
 	s = INVALID_SOCKET;
#endif

	//s = socket(PF_INET, SOCK_STREAM, 0);
	s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	
#if defined __unix__ || defined __APPLE__
  	if (s < 0) {
      printf("socket failed\n");
      freeaddrinfo(result);
  	}
#elif defined _WIN32
  //check for errors in socket allocation
  	if (s == INVALID_SOCKET) {
      printf("Error at socket(): %d\n", WSAGetLastError());
      freeaddrinfo(result);
      WSACleanup();
      exit(1);//return 1;
  	}
#endif

   //sin.sin_family = AF_INET;
//~ //*******************************************************************
//~ //GETHOSTBYNAME
//~ //*******************************************************************
   //~ if ((h=gethostbyname(argv[1])) != NULL) {
      //~ memcpy(&sin.sin_addr,h->h_addr,h->h_length); //get remote IP address
   //~ } else if ((sin.sin_addr.s_addr = inet_addr(argv[1])) == INADDR_NONE) {
      //~ printf("An error occured when trying to translate to IP address\n");
		//~ WSACleanup();
   	//~ exit(1);
   //~ }
//*******************************************************************
//CONNECT
//*******************************************************************
   //~ if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
      //~ printf("connect failed\n");
		//~ WSACleanup();
   	//~ exit(1);
   //~ }
	
	 if (connect(s, result->ai_addr, result->ai_addrlen) != 0) {
        printf("connect failed\n");
		freeaddrinfo(result);
#if defined _WIN32
        WSACleanup();
#endif 
   	    exit(1);
   } else {
		//~ printf("connected to server.\n");
		//~ struct sockaddr_in sa;
      //~ char ipstr[INET_ADDRSTRLEN];
		
		// store this IP address in sa:
      //inet_pton(AF_INET, result->ai_addr, &(sa.sin_addr));
		
		//-----------------------------------
		//~ void *addr;
		char ipver[80];
		
		// Get the pointer to the address itself, different fields in IPv4 and IPv6
		if (result->ai_family == AF_INET)
		{
			// IPv4
			//~ struct sockaddr_in *ipv4 = (struct sockaddr_in *)result->ai_addr;
			//~ addr = &(ipv4->sin_addr);
			strcpy(ipver,"IPv4");
		}
		else if(result->ai_family == AF_INET6)
		{
			// IPv6
			//~ struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)result->ai_addr;
			//~ addr = &(ipv6->sin6_addr);
			strcpy(ipver,"IPv6");
		}
			
		printf("\nConnected to <<<SERVER>>> with IP address: %s, %s at port: %s\n", argv[1], ipver,portNum);
		
		//--------------------------------------------------------------------------------
	   //getnameinfo() can be used to extract the IP address of the SERVER, in case a hostname was
		//              supplied by the user instead.

#if defined __unix__ || defined __APPLE__     
       int returnValue;
#elif defined _WIN32      
       DWORD returnValue;
#endif

		memset(serverHost, 0, sizeof(serverHost));
	    memset(serverService, 0, sizeof(serverService));
		
        //int addrlen = sizeof (struct sockaddr); 
        // int addrlen = sizeof (*(result->ai_addr)); 

        returnValue=getnameinfo((struct sockaddr *)result->ai_addr, /*addrlen*/ result->ai_addrlen,
               serverHost, sizeof(serverHost),
               serverService, sizeof(serverService), NI_NUMERICHOST);

		// returnValue=getnameinfo((struct sockaddr *)result->ai_addr, /* sizeof(*(result->ai_addr)) */&addrlen,
  //              serverHost, sizeof(serverHost),
  //              serverService, sizeof(serverService),
	 //            NI_NUMERICHOST);


		//~ getnameinfo(result->ai_addr, sizeof(*(result->ai_addr)),
               //~ serverHost, sizeof(serverHost),
               //~ serverService, sizeof(serverService),
	            //~ NI_NAMEREQD); //works only if the DNS can resolve the hostname; otherwise, will result in an error

		if(returnValue != 0){

#if defined __unix__ || defined __APPLE__     
           printf("\nError detected: getnameinfo() failed with error\n");
#elif defined _WIN32      
           printf("\nError detected: getnameinfo() failed with error#%d\n",WSAGetLastError());
#endif       
	       exit(1);

	    } else{
		   printf("\nConnected to <<<SERVER>>> extracted IP address: %s, %s at port: %s\n", serverHost, ipver,/* serverService */ portNum);  //serverService is nfa
	    	//printf("\nConnected to <<<SERVER>>> extracted IP address: %s, at port: %s\n", serverHost, serverService);
	    }
		//--------------------------------------------------------------------------------
		//recieve the public key from the server
		while (1) {
			bytes = recv(s, &receive_buffer[n], 1, 0);
			#if defined __unix__ || defined __APPLE__  
				if ((bytes == -1) || (bytes == 0)) {
			         printf("recv failed\n");
			         exit(1);
			    }   
		    #elif defined _WIN32      
		        if ((bytes == SOCKET_ERROR) || (bytes == 0)) {
			      	printf("recv failed\n");
		         	exit(1);
		        }
			#endif       
		    if (receive_buffer[n] == '\n') {  /*end on a LF*/
		        receive_buffer[n] = '\0';
		        break;
		    }
		    if (receive_buffer[n] != '\r') n++;   /*ignore CR's*/
		}
    	printf("MSG RECEIVED --->>>: %s\n",receive_buffer);
    	if (strncmp(receive_buffer,"publickey",9) == 0){
	    	long encryptedE, encryptedN;
	    	sscanf(receive_buffer, "publickey (%ld,%ld)", &encryptedE, &encryptedN);
	    	serverPublicKeyE = encryptionDecryption(encryptedE, caPublicKeyE, caPublicKeyN);
	    	serverPublicKeyN = encryptionDecryption(encryptedN, caPublicKeyE, caPublicKeyN);
	    	memset(&receive_buffer,0,BUFFER_SIZE);
	    	memset(&send_buffer,0,BUFFER_SIZE);
	    	strcpy(send_buffer, "ACK 226 public key received\n");
	    	bytes = send(s, send_buffer, strlen(send_buffer),0);
			printf("\nMSG SENT     --->>>: %s\n",send_buffer);
    	}
		memset(&send_buffer,0,BUFFER_SIZE);
		// send the nonce to the server
		srand(time(NULL));
		// set range between 10 and the encrypted value of 127
		long lower = 10;
		long upper = encryptionDecryption(127, serverPublicKeyE, serverPublicKeyN);
		nonce = rand()% upper; 
		if(nonce < lower){
			nonce += lower;
		}
		long encryptedNonce = encryptionDecryption(nonce, serverPublicKeyE, serverPublicKeyN);
		itoa(encryptedNonce, temp, 10);
		strcat(send_buffer, "nonce ");
		strcat(send_buffer, temp);
		strcat(send_buffer, "\n");
		bytes = send(s, send_buffer, strlen(send_buffer),0);
		printf("\nMSG SENT     --->>>: %s\n",send_buffer);
		// receive nonce ACK
		n = 0;
	    while (1) {
	//*******************************************************************
	//RECEIVE
	//*******************************************************************
	        bytes = recv(s, &receive_buffer[n], 1, 0);

#if defined __unix__ || defined __APPLE__  
		    if ((bytes == -1) || (bytes == 0)) {
	            printf("recv failed\n");
	         	exit(1);
	        }   
      
#elif defined _WIN32      
            if ((bytes == SOCKET_ERROR) || (bytes == 0)) {
	            printf("recv failed\n");
	         	exit(1);
	        }
#endif

	         
	         if (receive_buffer[n] == '\n') {  /*end on a LF*/
	            receive_buffer[n] = '\0';
	            break;
	         }
	         if (receive_buffer[n] != '\r') n++;   /*ignore CR's*/
	      }
	      
	      printf("MSG RECEIVED --->>>: %s\n",receive_buffer);
  }
	
//*******************************************************************
//Get input while user don't type "."
//*******************************************************************

	
	
	memset(&message,0,BUFFER_SIZE);
	memset(&send_buffer,0,BUFFER_SIZE);

	printf("\n--------------------------------------------\n");
	printf("you may now start sending commands to the <<<SERVER>>>\n");
	printf("\nType here:");

	
    if(fgets(message,SEGMENT_SIZE,stdin) == NULL){
		printf("error using fgets()\n");
		exit(1);
	}	
	c = nonce;
	for(size_t i =0; i<strlen(message); i++){
		c = calculateRSA_CBC(c, message[i], serverPublicKeyE, serverPublicKeyN);
		itoa(c,temp, 10);
		cout<<temp<<endl;
		strcat(send_buffer, temp);
		strcat(send_buffer, " ");
		
	}


    
	//while ((strncmp(send_buffer,".",1) != 0) && (strncmp(send_buffer,"\n",1) != 0)) {
	while ((strncmp(send_buffer,".",1) != 0)) {
		   send_buffer[strlen(send_buffer)-1]='\0';//strip '\n'
		   printBuffer("SEND_BUFFER", send_buffer);
		   printf("Message length: %d \n",(int)strlen(send_buffer));

		   
	       strcat(send_buffer,"\r\n");
	//*******************************************************************
	//SEND
	//*******************************************************************

	       bytes = send(s, send_buffer, strlen(send_buffer),0);
	       printf("\nMSG SENT     --->>>: %s\n",send_buffer);//line sent 
	       

#if defined __unix__ || defined __APPLE__     
          if (bytes == -1) {
	         printf("send failed\n");
    		 exit(1);
	      }
#elif defined _WIN32      
      	  if (bytes == SOCKET_ERROR) {
	         printf("send failed\n");
    		 WSACleanup();
	      	 exit(1);
	      }
#endif
	      
	      n = 0;
	      while (1) {
	//*******************************************************************
	//RECEIVE
	//*******************************************************************
	         bytes = recv(s, &receive_buffer[n], 1, 0);

#if defined __unix__ || defined __APPLE__  
		     if ((bytes == -1) || (bytes == 0)) {
	            printf("recv failed\n");
	         	exit(1);
	         }   
      
#elif defined _WIN32      
             if ((bytes == SOCKET_ERROR) || (bytes == 0)) {
	            printf("recv failed\n");
	         	exit(1);
	         }
#endif

	         
	         if (receive_buffer[n] == '\n') {  /*end on a LF*/
	            receive_buffer[n] = '\0';
	            break;
	         }
	         if (receive_buffer[n] != '\r') n++;   /*ignore CR's*/
	      }
	      
	     printf("MSG RECEIVED --->>>: %s\n",receive_buffer);			
		 //get another user input
	     memset(&send_buffer, 0, BUFFER_SIZE);
	     memset(&message, 0, BUFFER_SIZE);
         printf("\nType here:");
	     if(fgets(message,SEGMENT_SIZE,stdin) == NULL){
		   printf("error using fgets()\n");
		   exit(1);
	     }
	     c = nonce;
	     for(size_t i =0; i<strlen(message); i++){
		    c = calculateRSA_CBC(c, message[i], serverPublicKeyE, serverPublicKeyN);
			itoa(c,temp, 10);
			cout<<temp<<endl;
			strcat(send_buffer, temp);
			strcat(send_buffer, " ");
		 }
	     
		
	}
	printf("\n--------------------------------------------\n");
	printf("<<<CLIENT>>> is shutting down...\n");

//*******************************************************************
//CLOSESOCKET   
//*******************************************************************
#if defined __unix__ || defined __APPLE__
    close(s);//close listening socket
#elif defined _WIN32
    closesocket(s);//close listening socket
    WSACleanup(); /* call WSACleanup when done using the Winsock dll */  
#endif


   return 0;
}

