#pragma once
#ifndef ULTRA_TINY_HTTPD_H
#define ULTRA_TINY_HTTPD_H
/*
 *  UltraTinyHttpD.h
 *
 * 
 *  A trivial  and incomplete http implementation to enable serving of local files via airpay
 *  
 *  Created by peteh on 9/08/2011.
 *
 */

#include "s3eSocket.h"
#include "s3eOSReadString.h"
#include "s3eFile.h"

#include "s3e.h"

#include "IwDebug.h"
#include <stdio.h>
#include <ctype.h>

#include <stdlib.h> // for atoi
#include <string.h> // for strlcpy
#include <string>
#include <algorithm>
#import <unistd.h>
#include <sys/stat.h>
#include "pthread.h"


#define LISTEN_PORT  7777
#define SOCKET_TIMEOUT (30000)

#define SERVER_STRING "Server: kerbhttpd/0.1.0\r\n"
#define MSG_PEEK = 1024; //TODO whats this???!
	
class UltraTinyHttpd {
	public:
		UltraTinyHttpd();
		~UltraTinyHttpd();
		void Listen();
		void listening();
		void start();
		int32 AcceptCallback(s3eSocket* sock, void* sysData, void* userData);
	protected:
		void IncommingConnection(s3eSocket* newSocket, s3eInetAddress* address);
		
	
		char g_ErrorString[256];            //Contains operation results for display
		bool g_SocketIsConnected;			//Flag that keeps track of connect status	
		bool g_Listening;
		s3eSocket* g_Sock;					//Socket for send/receive operation
		s3eSocket* g_AcceptedSocket;

		void accept_request(s3eSocket* client);
		void bad_request(s3eSocket* client);
		void cat(s3eSocket* client, FILE *);
		void sendFile(s3eSocket* client, s3eFile *);
		void cannot_execute(s3eSocket* client);
		void error_die(const char *);
		int get_line(s3eSocket* sock, char *, int);
		void headers(s3eSocket* client, const char *);
		void not_found(s3eSocket* client);
		void serve_file(s3eSocket* client, const char *);
	
		void unimplemented(s3eSocket* socket);
	
		void send(s3eSocket* socket, char* buf, int32 size, int32 x);
	
	private:
		pthread_t thread; 
		s3eInetAddress addr;
		
	
};
	


#endif