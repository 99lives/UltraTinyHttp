/*
 *  UltraTinyHttpD.cpp
 *
 * 
 *  A trivial  and incomplete http implementation to enable serving of local files via marmalade
 *  See http://tinyhttpd.sourceforge.net/ for orginal 
 *  
 *  Created by peteh on 9/08/2011.
 *
 */

#include "UltraTinyHttpd.h"

/*
 ***  Static thread shim
 */
static void *ST_Thread(void *arg)
{
	//Listen();		
	while (true) {		
		((UltraTinyHttpd *)arg)->listening();
		//sleep(1);		
	}	
	pthread_exit(NULL);	  
	return NULL;	
} 

/*
***  Static callback shim
*/
static int32 ST_AcceptCallback(s3eSocket* sock, void* sysData, void* userData)
{
	((UltraTinyHttpd *)userData)->AcceptCallback(sock, sysData, userData);    
    return 0;
}


//Constructor
UltraTinyHttpd::UltraTinyHttpd() {

	//TODO - move these to an initalization list	
	g_SocketIsConnected = false;    //Flag that keeps track of connect status
	g_Listening = false;
	g_Sock = NULL;            //Socket for send/receive operation
	g_AcceptedSocket = NULL;
	
}


//Destructor
UltraTinyHttpd::~UltraTinyHttpd() {
	//TODO 	
}

void UltraTinyHttpd::start() {
	Listen();
	pthread_create(&thread, NULL, ST_Thread, this);
}

void UltraTinyHttpd::listening() {
	s3eSocket* newSocket = s3eSocketAccept(g_Sock, &addr, ST_AcceptCallback, this);
	if (newSocket)
		IncommingConnection(newSocket, &addr);
	if (g_AcceptedSocket)
	{
		accept_request(g_AcceptedSocket);
		g_AcceptedSocket= NULL;
		/*
		char buf[1024];
		int rtn = s3eSocketRecv(g_AcceptedSocket, buf, 1024, 0);
		if (rtn > 0)
		{
			buf[rtn] = '\0';
			s3eDebugTracePrintf("recieved %d bytes", rtn);
			s3eDebugTracePrintf("recieved %s ", buf);
			//snprintf(g_ErrorString, 256, "`x666666Got Data: '%s'", buf);
			//s3eSocketSend(g_AcceptedSocket, buf, rtn, 0);
			
		}
		 */

		
		
	}
}
void UltraTinyHttpd::IncommingConnection(s3eSocket* newSocket, s3eInetAddress* address)
{
    s3eDebugTracePrintf("IncommingConnection: %p %s", newSocket, s3eInetToString(address));
    g_AcceptedSocket = newSocket;
}




int32 UltraTinyHttpd::AcceptCallback(s3eSocket* sock, void* sysData, void* userData)
{
    s3eInetAddress addr;
    s3eSocket* newSocket = s3eSocketAccept(g_Sock, &addr, NULL, NULL);
    if (newSocket)
        IncommingConnection(newSocket, &addr);
    return 0;
}
void UltraTinyHttpd::Listen()
{
    g_Sock = s3eSocketCreate(S3E_SOCKET_TCP, 0);
    if (g_Sock == NULL)
    {
        strcpy(g_ErrorString,"`x666666Creation of socket failed");
        return;
    }
	
    // look up address
    
    memset(&addr, 0, sizeof(addr));
    addr.m_Port = s3eInetHtons(LISTEN_PORT);
    addr.m_IPAddress = 0;
	
    if (s3eSocketBind(g_Sock, &addr, S3E_TRUE) != S3E_RESULT_SUCCESS)
    {
        strcpy(g_ErrorString,"`x666666Bind failed");
        return;
    }
	
    if (s3eSocketListen(g_Sock, 10) != S3E_RESULT_SUCCESS)
    {
        strcpy(g_ErrorString,"`x666666Listen failed");
        return;
    }
	
    g_Listening = true;
    s3eSocket* newSocket = s3eSocketAccept(g_Sock, &addr, ST_AcceptCallback, this);
    if (newSocket)
        IncommingConnection(newSocket, &addr);
}


/**********************************************************************/
/* A request has caused a call to accept() on the server port to
 * return.  Process the request appropriately.
 * Parameters: the socket connected to the client */
/**********************************************************************/
void UltraTinyHttpd::accept_request(s3eSocket* client)
{
	char buf[1024];
	int numchars;
	char method[255];
	char url[255];
	char path[512];
	size_t i, j;
	struct stat st;
	int cgi = 0;      /* becomes true if server decides this is a CGI
					   * program */
	char *query_string = NULL;
	
	numchars = get_line(client, buf, sizeof(buf));
	i = 0; j = 0;
	while (!isspace(buf[j]) && (i < sizeof(method) - 1))
	{
		method[i] = buf[j];
		i++; j++;
	}
	method[i] = '\0';
	
	if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	{
		unimplemented(client);
		return;
	}
	
	if (strcasecmp(method, "POST") == 0)
		cgi = 1;
	
	i = 0;
	while (isspace(buf[j]) && (j < sizeof(buf)))
		j++;
	while (!isspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
	{
		url[i] = buf[j];
		i++; j++;
	}
	url[i] = '\0';
	
	if (strcasecmp(method, "GET") == 0)
	{
		query_string = url;
		while ((*query_string != '?') && (*query_string != '\0'))
			query_string++;
		if (*query_string == '?')
		{
			cgi = 1;
			*query_string = '\0';
			query_string++;
		}
	}
	
	sprintf(path, "htdocs%s", url);
	if (path[strlen(path) - 1] == '/')
		strcat(path, "index.html");
	if (stat(path, &st) == -1) {
		while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
			numchars = get_line(client, buf, sizeof(buf));
		not_found(client);
	}
	else
	{
		if ((st.st_mode & S_IFMT) == S_IFDIR)
			strcat(path, "/index.html");
		if ((st.st_mode & S_IXUSR) ||
			(st.st_mode & S_IXGRP) ||
			(st.st_mode & S_IXOTH)    )
			cgi = 1;
		if (!cgi)
			serve_file(client, path);
		//else
		//	execute_cgi(client, path, method, query_string);
	}
	//TODO CLOSE!!!!
	//close(client);
	s3eSocketClose(client);
	
}

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
/**********************************************************************/
void UltraTinyHttpd::bad_request(s3eSocket* client)
{
	char buf[1024];
	
	sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "<P>Your browser sent a bad request, ");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "such as a POST without a Content-Length.\r\n");
	send(client, buf, sizeof(buf), 0);
}



/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
/**********************************************************************/
void UltraTinyHttpd::cannot_execute( s3eSocket* client)
{
	char buf[1024];
	
	sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
 * on value of errno, which indicates system call errors) and exit the
 * program indicating an error. */
/**********************************************************************/
void UltraTinyHttpd::error_die(const char *sc)
{
	perror(sc);
	exit(1);
}


/**********************************************************************/
/* Get a line from a socket, whether the line ends in a newline,
 * carriage return, or a CRLF combination.  Terminates the string read
 * with a null character.  If no newline indicator is found before the
 * end of the buffer, the string is terminated with a null.  If any of
 * the above three line terminators is read, the last character of the
 * string will be a linefeed and the string will be terminated with a
 * null character.
 * Parameters: the socket descriptor
 *             the buffer to save the data in
 *             the size of the buffer
 * Returns: the number of bytes stored (excluding null) */
/**********************************************************************/
int UltraTinyHttpd::get_line(s3eSocket* sock, char *buf, int size)
{
	int i = 0;
	char c = '\0';
	int n;
	
	while ((i < size - 1) && (c != '\n'))
	{
		n = s3eSocketRecv(sock, &c, 1, 0);
		/* DEBUG printf("%02X\n", c); */
		if (n > 0)
		{
			if (c == '\r')
			{
				//sock->
				n = s3eSocketRecv(sock, &c, 1, 0);//MSG_PEEK);  //TODO figure out s3e verison
				
				/* DEBUG printf("%02X\n", c); */
				if ((n > 0) && (c == '\n'))
					s3eSocketRecv(sock, &c, 1, 0);
				else
					c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';
	
	return(i);
}

/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
 *             the name of the file */
/**********************************************************************/
void UltraTinyHttpd::headers(s3eSocket* client, const char *filename)
{
	char buf[1024];
	(void)filename;  /* could use filename to determine file type */
	std::string s = std::string(filename);	//lets do just that!
	std::string contentType;
	int32 pos=0;
	pos = s.find_last_of (".");
	s= s.substr(pos+1);												//grab extension
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);		//lowercase
	contentType = "Content-Type: ";
	
	if (s=="gif") {
		contentType += "image/gif";
	}else
	if (s=="png") {
		contentType += "image/png";
	}else		
	 contentType += "text/html";
	
	contentType += "\r\n";
	IwDebugTraceLinePrintf("extension : %s,  content type string %s:",s.c_str(), contentType.c_str());
	
	
	strcpy(buf, "HTTP/1.0 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	//sprintf(buf, contentType.c_str());
	//sprintf(buf, "Content-Type: image/gif\r\n");
	//send(client, buf, strlen(buf), 0);
	send(client, (char*)contentType.c_str(), contentType.size(), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void UltraTinyHttpd::not_found(s3eSocket* client)
{
	char buf[1024];
	
	sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "your request because the resource specified\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Send a regular file to the client.  Use headers, and report
 * errors to client if they occur.
 * Parameters: a pointer to a file structure produced from the socket
 *              file descriptor
 *             the name of the file to serve */
/**********************************************************************/
void UltraTinyHttpd::serve_file(s3eSocket* client, const char *filename)
{
	s3eFile *resource = NULL;
	int numchars = 1;
	char buf[1024];
	
	buf[0] = 'A'; buf[1] = '\0';
	while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
		numchars = get_line(client, buf, sizeof(buf));
	
	resource =  s3eFileOpen(filename, "rb");
	if (resource == NULL)
		not_found(client);
	else
	{
		headers(client, filename);
		sendFile(client, resource);
	}
	 s3eFileClose(resource);
}
/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
* WARNING THIS FUNCTION IS DANGEROUS IT DOES NO CHECKING OF SIZES!!!
*
 
/**********************************************************************/
void UltraTinyHttpd::sendFile( s3eSocket* client, s3eFile *resource)
{
	static char*       g_DataToBeRead;                 // Data which is being kRead from the file
	int32 fileSize = s3eFileGetSize(resource);
	g_DataToBeRead = (char*)s3eMallocBase(fileSize);
	IwDebugTraceLinePrintf("Filesize %d:", fileSize);
	// Read data from file
	s3eFileRead(&g_DataToBeRead[0], fileSize, 1, resource);

	send(client, g_DataToBeRead, fileSize, 0);
	s3eFreeBase(g_DataToBeRead);
}

/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
 * is named after the UNIX "cat" command, because it might have been
 * easier just to do something like pipe, fork, and exec("cat").
 * Parameters: the client socket descriptor
 *             FILE pointer for the file to cat */
/**********************************************************************/
void UltraTinyHttpd::cat( s3eSocket* client, FILE *resource)
{
	char buf[1024];
	
	fgets(buf, sizeof(buf), resource);
	while (!feof(resource))
	{
		send(client, buf, strlen(buf), 0);
		
		fgets(buf, sizeof(buf), resource);
	}
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
/**********************************************************************/
void UltraTinyHttpd::unimplemented(s3eSocket* client)
{
	char buf[1024];
	
	sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}


/**********************************************************************/
void UltraTinyHttpd::send(s3eSocket* socket, char* buf, int32 size, int32 x) {
	IwDebugTraceLinePrintf("sending %d: %s", size, buf);
	s3eSocketSend(socket,buf, size, x);
}


