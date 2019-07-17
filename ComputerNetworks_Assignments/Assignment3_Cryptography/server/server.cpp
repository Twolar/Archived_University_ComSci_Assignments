//////////////////////////////////////////////////////////////////////////////////////////////
// TCP CrossPlatform SERVER v.1.0 (towards IPV6 ready)
// compiles using GCC 
//
//
// References: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
//             http://long.ccaba.upc.edu/long/045Guidelines/eva/ipv6.html#daytimeServer6
//             Andre Barczak's tcp server codes
//
// Author: Napoleon Reyes, Ph.D.
//         Massey University, Albany  
//
//////////////////////////////////////////////////////////////////////////////////////////////

#define USE_IPV6 true

#if defined __unix__ || defined __APPLE__
  #include <unistd.h>
  #include <errno.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <sys/types.h>  //// Structures and functions used for socket API
  #include <sys/socket.h> // Structures and functions used for socket API
  #include <arpa/inet.h>
  #include <netdb.h>      //used for domain/DNS hostname lookup (e.g getnameinfo())
  #include <iostream>
#elif defined _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h> //required by getaddrinfo() and special constants
  #include <stdlib.h>
  #include <stdio.h>
  #include <iostream>
  #define WSVERS MAKEWORD(2,2) /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
                    //The high-order byte specifies the minor version number; 
                    //the low-order byte specifies the major version number.

  WSADATA wsadata; //Create a WSADATA object called wsadata. 
#endif
#include "bigInt.h" //big number library

//////////////////////////////////////////////////////////////////////////////////////////////

#define SECRET_PASSWORD "334"
#define DEFAULT_PORT "1234" 

using namespace std;

//*******************************************************************
void printBuffer(const char *header, char *buffer){
  cout << "------" << header << "------" << endl;
  for(unsigned int i=0; i < strlen(buffer); i++){
    if(buffer[i] == '\r'){
       cout << "buffer[" << i << "]=\\r" << endl; 
    } else if(buffer[i] == '\n'){
       cout << "buffer[" << i << "]=\\n" << endl; 
    } else {   
       cout << "buffer[" << i << "]=" << buffer[i] << endl;
    }
  }
  cout << "---" << endl;
}

/******************************************
* a function used to encrypt or decrypt a character using RSA
* @param x the character
* @param n the n value
* @param e the private or public key 
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
* a function used to decrypt a character 
* that has been encrypted using RSA and CBC
* @param r the nonce or previous c value
* @param c the encrypted character
* @param d the private key 
* @param n the n value
* @return long the result
*****************************************/
long reverseRSA_CBC(long r, long c, long d, long n){
	unsigned long temp = encryptionDecryption(c, d, n);
	long result = temp ^ r;
	return result;
}

/******************************************
* a function used to calculate a value for d 
* using the extended Euclidean algorithm
* @param e the e value
* @param z the z value
* @return long the result
*****************************************/
int calculateD(int e, int z){
  //maybe change the size of the variables
  int x[80];
  int y[80];
  int w[80];
  int k[80];
  // init the first 2 steps
  x[0] = 1;
  x[1] = 0;
  y[0] = 0;
  y[1] = 1;
  w[0] = z;
  w[1] = e;
  k[0] = 0; 
  k[1] = z/e;
  int i = 2;
  while(true){
    x[i] = x[i-2] - (k[i-1] *x[i-1]);
    y[i] = y[i-2] - (k[i-1] *y[i-1]);
    w[i] = w[i-2] - (k[i-1] *w[i-1]);
    k[i] = w[i-1]/w[i];
    if(w[i] == 1){
      break;
    }
    i++;
  }
  //just to double check
  if(((z*x[i]) +(e*y[i])) == 1){
    if(y[i] <0){
      y[i] = y[i] + z;
    }
    return y[i];
  }
  return -1;
}

/******************************************
* a function used to calculate the greatest common divisor
* @param e the e value
* @param z the z value
* @return long the result
*****************************************/
long gcd(long e, long z){
  long dividend = z;
  long divisor = e;
  long remainder = -1;
  while(true){
    remainder = dividend % divisor;
    if(remainder == 0){
      return divisor;
    }
    dividend = divisor;
    divisor = remainder;
  }
}

/******************************************
* a function used to calculate e using the Euclidean algorithm
* @param z the z value
* @return long the result
*******************************************/
long calculateE(long z){
  long e;
  long result;
  long answer = -1;
  srand (time(NULL));
  while(true){
    e = rand()% z;
    result = gcd(e,z);
    if((result == 1)&&(e<z)){
      answer = e;
      break;
    }
  }
  return answer;
}

/******************************************
* a function used to calculate z
* @param p the p value
* @param q the q value
* @return long the result
*******************************************/
long calculateZ(long p, long q){
  return (p-1)*(q-1);
}

/******************************************
* a function used to calculate n
* @param p the p value
* @param q the q value
* @return long the result
*******************************************/
long calculateN(long p, long q){
  return p*q;
}

/******************************************
* a function used to check if a number is prime
* @param num a whole number
* @return bool the result
*******************************************/
bool isPrime(long num){
  for(long i = 2; i <= num / 2; ++i){
      if(num % i == 0){
        return false;
      }
  }
  return true;
}

/******************************************
* a function used to generate a random prime number for q
* @param p the p value
* @return long the result
*******************************************/
long generateQ(long p){
  long num = 0;
  srand (time(NULL));
  while(true){
    long lower = 1;
    num = rand()% 200; //between 1-199
    if(num < lower){
      num += lower;
    }
    if(isPrime(num)){
      if(p != num){
        return num;
      }
    }
  }
}

/******************************************
* a function used to generate a random prime number for p
* @return long the result
*******************************************/
long generateP(){
  long num = 0;
  srand (time(NULL));
  while(true){
    long lower = 1;
    num = rand()% 200; //between 1-199
    if(num < lower){
      num += lower;
    }
    if(isPrime(num)){
        return num;
    }
  }
}


//*******************************************************************
//MAIN
//*******************************************************************
int main(int argc, char *argv[]) {
  long pValue = 173;
  long qValue = 149;
  long zValue = calculateZ(pValue,qValue);
  long nKey = calculateN(pValue,qValue);   
  long ePublicKey = 3;
  long dPrivateKey = calculateD(ePublicKey, zValue);
  
  //servers dCA
  //where:
  //p = 101
  //q = 307
  //z = 30600
  //e = 11
  long caPrivateKeyD = 47291;
  long caPrivateKeyN = 31007;
	long nonce = 0;
	int counter = 0;
//********************************************************************
// INITIALIZATION of the SOCKET library
//********************************************************************
   //struct sockaddr_in clientAddress;  //IPV4
	struct sockaddr_storage clientAddress; //IPV6-compatible
	char clientHost[NI_MAXHOST]; 
	char clientService[NI_MAXSERV];
	
#if defined __unix__ || defined __APPLE__
  int s, ns;
#elif defined _WIN32
  SOCKET s, ns;
#endif

#define BUFFER_SIZE 200 

  char send_buffer[BUFFER_SIZE],receive_buffer[BUFFER_SIZE];
  int  n,bytes,addrlen;
	char portNum[NI_MAXSERV];
	char username[80];
	char passwd[80];
	
  memset(&send_buffer,0,BUFFER_SIZE);
  memset(&receive_buffer,0,BUFFER_SIZE);

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

    printf("\n\n<<<TCP (CROSS-PLATFORM, IPv6-ready) SERVER, by nhreyes>>>\n");  
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
// struct addrinfo *ptr = NULL;
int iResult;


//********************************************************************
// STEP#0 - Specify server address information and socket properties
//********************************************************************
	 
//ZeroMemory(&hints, sizeof (hints)); //alternatively, for Windows only
memset(&hints, 0, sizeof(struct addrinfo));

if(USE_IPV6){
   hints.ai_family = AF_INET6;  
}	 else { //IPV4
   hints.ai_family = AF_INET;
}

//hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
hints.ai_socktype = SOCK_STREAM;
hints.ai_protocol = IPPROTO_TCP;
hints.ai_flags = AI_PASSIVE; // For wildcard IP address 
                             //setting the AI_PASSIVE flag indicates the caller intends to use 
									           //the returned socket address structure in a call to the bind function. 

// Resolve the local address and port to be used by the server
if(argc==2){	 
	 iResult = getaddrinfo(NULL, argv[1], &hints, &result); //converts human-readable text strings representing hostnames or IP addresses 
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

#if defined _WIN32
    WSACleanup();
#endif    
    return 1;
}	 


//********************************************************************

//********************************************************************
// STEP#1 - Create welcome SOCKET
//********************************************************************


#if defined __unix__ || defined __APPLE__
  s = -1;
#elif defined _WIN32
  s = INVALID_SOCKET;
#endif


// Create a SOCKET for the server to listen for client connections

s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

#if defined __unix__ || defined __APPLE__
  if (s < 0) {
      printf("socket failed\n");
      freeaddrinfo(result);
      exit(1);
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
  


//********************************************************************

	
//********************************************************************
//STEP#2 - BIND the welcome socket
//********************************************************************

// bind the TCP welcome socket to the local address of the machine and port number
   iResult = bind( s, result->ai_addr, (int)result->ai_addrlen);
    
     
//if error is detected, then clean-up
#if defined __unix__ || defined __APPLE__
   if (iResult == -1) {
      printf( "\nbind failed\n"); 
      freeaddrinfo(result);
      close(s);//close socket
#elif defined _WIN32
    if (iResult == SOCKET_ERROR) {
      printf("bind failed with error: %d\n", WSAGetLastError());
      freeaddrinfo(result);
      closesocket(s);
      WSACleanup();
#endif       
      return 1;
    }
	 
	 freeaddrinfo(result); //free the memory allocated by the getaddrinfo 
	                       //function for the server's address, as it is 
	                       //no longer needed
//********************************************************************
	 
/*
   if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) == SOCKET_ERROR) {
      printf("Bind failed!\n");
   }
*/
	
//********************************************************************
//STEP#3 - LISTEN on welcome socket for any incoming connection
//********************************************************************

#if defined __unix__ || defined __APPLE__
    if (listen( s, 5) == -1) {
#elif defined _WIN32
    if (listen( s, SOMAXCONN ) == SOCKET_ERROR ) {
#endif

	


#if defined __unix__ || defined __APPLE__
      printf( "\nListen failed\n"); 
      close(s); 
#elif defined _WIN32
      printf( "Listen failed with error: %d\n", WSAGetLastError() );
      closesocket(s);
      WSACleanup(); 
#endif   

      exit(1);

   } else {
		  printf("\n<<<SERVER>>> is listening at PORT: %s\n", portNum);
	 }
	
//*******************************************************************
//INFINITE LOOP
//********************************************************************
while (1) {  //main loop

      addrlen = sizeof(clientAddress); //IPv4 & IPv6-compliant
		
//********************************************************************
//NEW SOCKET newsocket = accept
//********************************************************************
#if defined __unix__ || defined __APPLE__
       ns = -1;
#elif defined _WIN32
       ns = INVALID_SOCKET;
#endif


//********************************************************************	
// STEP#4 - Accept a client connection.  
//	accept() blocks the iteration, and causes the program to wait.  
//	Once an incoming client is detected, it returns a new socket ns
// exclusively for the client.  
// It also extracts the client's IP address and Port number and stores
// it in a structure.
//********************************************************************
	
#if defined __unix__ || defined __APPLE__     
      ns = accept(s,(struct sockaddr *)(&clientAddress),(socklen_t*)&addrlen); //IPV4 & IPV6-compliant
#elif defined _WIN32      
      ns = accept(s,(struct sockaddr *)(&clientAddress),&addrlen); //IPV4 & IPV6-compliant
      // ns = accept(s,NULL, NULL); //IPV4 & IPV6-compliant - getnameinfo() will complain if you use this!
#endif


#if defined __unix__ || defined __APPLE__


  if (ns == -1) {
     printf("\naccept failed\n");
     close(s);
     
     return 1;

  } else {
      printf("\nA <<<CLIENT>>> has been accepted.\n");
    
      // strcpy(clientHost,inet_ntoa(clientAddress.sin_addr)); //IPV4
      // sprintf(clientService,"%d",ntohs(clientAddress.sin_port)); //IPV4
      // ---
      int returnValue;
      memset(clientHost, 0, sizeof(clientHost));
      memset(clientService, 0, sizeof(clientService));

      returnValue = getnameinfo((struct sockaddr *)&clientAddress, addrlen,
                    clientHost, sizeof(clientHost),
                    clientService, sizeof(clientService),
                    NI_NUMERICHOST);
      if(returnValue != 0){
        printf("\nError detected: getnameinfo() failed \n");
        exit(1);
      } else{
        printf("\nConnected to <<<CLIENT>>> with IP address:%s, at Port:%s\n",clientHost, clientService);
      }   

  } 

      
#elif defined _WIN32

  if (ns == INVALID_SOCKET) {
     printf("accept failed: %d\n", WSAGetLastError());
     closesocket(s);
     WSACleanup();
     return 1;

  } else {
    printf("\nA <<<CLIENT>>> has been accepted.\n");
    
    //strcpy(clientHost,inet_ntoa(clientAddress.sin_addr)); //IPV4
    //sprintf(clientService,"%d",ntohs(clientAddress.sin_port)); //IPV4
    //---

    DWORD returnValue;
    memset(clientHost, 0, sizeof(clientHost));
    memset(clientService, 0, sizeof(clientService));

    returnValue = getnameinfo((struct sockaddr *)&clientAddress, addrlen,
                    clientHost, sizeof(clientHost),
                    clientService, sizeof(clientService),
                    NI_NUMERICHOST);
    if(returnValue != 0){
       printf("\nError detected: getnameinfo() failed with error#%d\n",WSAGetLastError());
       exit(1);
    } else{
       printf("\nConnected to <<<Client>>> with IP address:%s, at Port:%s\n",clientHost, clientService);
    }
    //encrypted and send public keys using dCA keys
    long eValueEncrypt = encryptionDecryption(ePublicKey, caPrivateKeyD, caPrivateKeyN);
   	long nValueEncrypt = encryptionDecryption(nKey, caPrivateKeyD, caPrivateKeyN);
    sprintf(send_buffer, "publickey (%ld,%ld)\r\n", eValueEncrypt, nValueEncrypt);
		bytes = send(ns, send_buffer, strlen(send_buffer), 0);
		#if defined __unix__ || defined __APPLE__      
		         if ((bytes == -1) || (bytes == 0)) break;
		#elif defined _WIN32      
		         if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
		#endif
		printf("\nMSG SENT     --->>> :%s\n",send_buffer);
		memset(&send_buffer,0,BUFFER_SIZE);
    //receive ACK for public key
    while (1) {
            bytes = recv(ns, &receive_buffer[n], 1, 0);
     #if defined __unix__ || defined __APPLE__      
           if ((bytes == -1) || (bytes == 0)) break;    
     #elif defined _WIN32      
           if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
     #endif
            if (receive_buffer[n] == '\n') { /*end on a LF, Note: LF is equal to one character*/  
               receive_buffer[n] = '\0';
              break;
          }
        if (receive_buffer[n] != '\r') n++; /*ignore CRs*/
      }
      printBuffer("RECEIVE_BUFFER", receive_buffer);
      printf("\nMSG RECEIVED <<<--- :%s\n",receive_buffer);   
  } 


#endif 	


  
		
//********************************************************************		
//Communicate with the Client
//********************************************************************

		printf("\n--------------------------------------------\n");
	  	printf("the <<<SERVER>>> is waiting to receive commands.\n");
		//Clear user details
		memset(username,0,80);
		memset(passwd,0,80);

    while (1) {
         memset(receive_buffer, 0, BUFFER_SIZE);
         n = 0;
//********************************************************************
//RECEIVE one command (delimited by \r\n)
//********************************************************************
         while (1) {

            bytes = recv(ns, &receive_buffer[n], 1, 0);

#if defined __unix__ || defined __APPLE__      
      if ((bytes == -1) || (bytes == 0)) break;
      
#elif defined _WIN32      
      if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
#endif

            if (receive_buffer[n] == '\n') { /*end on a LF, Note: LF is equal to one character*/  
               receive_buffer[n] = '\0';
               break;
            }
            if (receive_buffer[n] != '\r') n++; /*ignore CRs*/
         }

//this will handle the case when the user quits (types '.')
#if defined __unix__ || defined __APPLE__      
      if ((bytes == -1) || (bytes == 0)) break;
#elif defined _WIN32      
      if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
 #endif

//********************************************************************
//PROCESS REQUEST
//********************************************************************      
         printBuffer("RECEIVE_BUFFER", receive_buffer);
         printf("\nMSG RECEIVED <<<--- :%s\n",receive_buffer);
         if(strncmp(receive_buffer,"nonce",5) == 0){
           	long encryptedNonce;
          	sscanf(receive_buffer, "nonce %ld", &encryptedNonce);
          	nonce = encryptionDecryption(encryptedNonce, dPrivateKey, nKey);
  			    memset(&send_buffer, 0, BUFFER_SIZE);
           	sprintf(send_buffer, "ACK 220 nonce OK\n");
           	memset(receive_buffer, 0, BUFFER_SIZE);
         }else{
	         char *ptr = strtok(receive_buffer, " ");
	         long cArray[BUFFER_SIZE];
	         memset(&cArray, 0, counter);
	         counter = 0;
	         for(counter = 0; ptr != NULL; counter++){
	         	if(counter == 0){
	         		cArray[counter] = nonce;
	         	}else{
	         		cArray[counter] = atoi(ptr);
	         		ptr = strtok(NULL, " ");
	         	}
	         }
           cout<<"decrypted message: ";
           memset(send_buffer, 0, BUFFER_SIZE);
           char decryptedBuffer[BUFFER_SIZE];
	         for(int i = 1; i<counter; i++){
				      long temp = reverseRSA_CBC(cArray[i-1], cArray[i], dPrivateKey, nKey);
				      cout<<(char)temp;
              decryptedBuffer[i-1] = (char)temp;
			     }
           decryptedBuffer[counter-2] = '\0';
			memset(&send_buffer, 0, BUFFER_SIZE);
      sprintf(send_buffer, "The Client typed '%s' - %d bytes of information\r\n", decryptedBuffer, n);
		}
        


      
//********************************************************************			 
         

//SEND
//********************************************************************         
			   bytes = send(ns, send_buffer, strlen(send_buffer), 0);
#if defined __unix__ || defined __APPLE__      
         if ((bytes == -1) || (bytes == 0)) break;
#elif defined _WIN32      
         if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
#endif
			   printf("\nMSG SENT     --->>> :%s\n",send_buffer);
         memset(&send_buffer,0,BUFFER_SIZE);
      }
//********************************************************************
//CLOSE SOCKET
//********************************************************************
		  
#if defined __unix__ || defined __APPLE__
      close(ns);
#elif defined _WIN32
      int iResult = shutdown(ns, SD_SEND);
      if (iResult == SOCKET_ERROR) {
         printf("shutdown failed with error: %d\n", WSAGetLastError());
         closesocket(ns);
         WSACleanup();
         exit(1);
      } 
//***********************************************************************
      closesocket(ns);   
#endif 
		
      printf("\nDisconnected from <<<CLIENT>>> with IP address:%s, Port:%s\n",clientHost, clientService);
		  printf("=============================================");
		
} //main loop
//***********************************************************************


#if defined __unix__ || defined __APPLE__
    close(s);//close listening socket
#elif defined _WIN32
    closesocket(s);//close listening socket
    WSACleanup(); /* call WSACleanup when done using the Winsock dll */ 
#endif
	
	 
    return 0;
}
