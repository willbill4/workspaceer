//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Proxy.h"
#include "winsock2.h"

#pragma comment(lib, "ws2_32.lib")

#define LISTENPORT 808
//
#define PROXYVERSION 0x05
#define NOAUTHLOGIN 0x00
#define GSSAPI 0x01
#define AUTHLOGIN 0x02
#define NOMETHOD 0xff
//
#define BACKUP 100
#define MAXDATALEN 65535
//
#define CMD_CONNECT 0x01
#define CMD_BIND 0x02
#define CMD_UDP 0x03
//
#define ATYP_IPV4 0x01
#define ATYP_DOMAIN 0x03
#define ATYP_IPV6 0x04
//
#define REP_SUCCESS 0x00
#define REP_FAILED 0x01
#define REP_CANNOT_CONNECT 0x02
#define REP_NET_CANNOTTO 0x03
#define REP_PC_CANNOTTO 0x04
#define REP_CONNECT_DENIED 0x05
#define REP_TTL_TIMEOUT 0x06
#define REP_CMD_WRONG 0x07
#define REP_ATYP_WRONG 0x08
//
#define FD_NOEVENT 0
#define ANY_SIZE 100
#define MAX_ADAPTER_NAME 128
/*
· X’09’ – X’FF’ 未定义
*/
/*
· X’03’ -- X’7F’ 由IANA分配
· X’80’ -- X’FE’ 为私人方法所保留的
*/
typedef struct _CLIENTINFO
{
	SOCKET clientsock;
	SOCKET udpsock;
	sockaddr_in remotesock;
}CLIENTINFO,*LPCLIENTINFO;
CLIENTINFO TcpClientInfo,UdpClientInfo;
//
typedef struct _SOCKINFO{
	SOCKET sourcesock;
	SOCKET destsock;
}SOCKINFO,*LPSOCKINFO;
SOCKINFO sockinfo;
//
typedef struct _MIB_TCPROW{ 
	DWORD dwState; 
	DWORD dwLocalAddr; 
	DWORD dwLocalPort; 
	DWORD dwRemoteAddr; 
	DWORD dwRemotePort; 
}MIB_TCPROW,*PMIB_TCPROW; 

typedef struct _MIB_TCPTABLE{ 
	DWORD dwNum_Of_Entries; 
	MIB_TCPROW TCP_Table[ANY_SIZE]; 
}MIB_TCPTABLE,*PMIB_TCPTABLE; 

typedef struct _MIB_UDPROW { 
	DWORD dwLocalAddr; 
	DWORD dwLocalPort; 
} MIB_UDPROW, *PMIB_UDPROW; 

typedef struct _MIB_UDPTABLE { 
	DWORD dwNumEntries; 
	MIB_UDPROW table[ANY_SIZE]; 
} MIB_UDPTABLE, *PMIB_UDPTABLE; 

typedef struct _MIB_TCPROW_EX 
{ 
	DWORD dwState; 
	DWORD dwLocalAddr; 
	DWORD dwLocalPort; 
	DWORD dwRemoteAddr; 
	DWORD dwRemotePort; 
	DWORD dwProcessId; 
} MIB_TCPROW_EX, *PMIB_TCPROW_EX; 

typedef struct _MIB_TCPTABLE_EX 
{ 
	DWORD dwNumEntries; 
	MIB_TCPROW_EX table[ANY_SIZE]; 
} MIB_TCPTABLE_EX, *PMIB_TCPTABLE_EX; 

typedef struct _MIB_UDPROW_EX 
{ 
	DWORD dwLocalAddr; 
	DWORD dwLocalPort; 
	DWORD dwProcessId; 
} MIB_UDPROW_EX, *PMIB_UDPROW_EX; 

typedef struct _MIB_UDPTABLE_EX 
{ 
	DWORD dwNumEntries; 
	MIB_UDPROW_EX table[ANY_SIZE]; 
} MIB_UDPTABLE_EX, *PMIB_UDPTABLE_EX; 
//
int Socks5ListenPort;
SOCKET Socks5ListenSock;
bool CanSockProxy;
//

long GetSocketEventId(SOCKET remotesock){
	long EventId;
	HANDLE hevent;
	hevent=CreateEvent(NULL,0,0,0);
	WSANETWORKEVENTS socket_events;
	EventId=FD_NOEVENT;
	if(WSAEventSelect(remotesock,hevent,FD_ACCEPT|FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE)==SOCKET_ERROR) return EventId;
	WSAEnumNetworkEvents(remotesock,hevent,&socket_events);
	if(socket_events.lNetworkEvents!=0){
		switch(socket_events.lNetworkEvents){
		case FD_ACCEPT:EventId=FD_ACCEPT;break;
		case FD_CONNECT:EventId=FD_CONNECT;break;
		case FD_READ:EventId=FD_READ;break;
		case FD_WRITE:EventId=FD_WRITE;break;
		case FD_CLOSE:EventId=FD_CLOSE;break;
		case FD_OOB:EventId=FD_OOB;break;
		default:EventId=FD_NOEVENT;break;
		}
	}
	else EventId=FD_NOEVENT;
	return EventId;
}
//
unsigned long GetLocalIp()
{
	char IP[MAX_PATH],*ip;
	char pc_name[80];
	struct in_addr in;
	struct hostent *host;
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested=MAKEWORD(2,0);
	ip=IP;
	strcpy(ip,"Ip not get!");
	if(WSAStartup(wVersionRequested,&wsaData)) return 0;
	if(gethostname(pc_name,80)==SOCKET_ERROR){
		WSACleanup();
		return 0;
	}
	if(!(host=gethostbyname(pc_name))){
		WSACleanup();
		return 0;
	}
	in.s_addr=*((unsigned long *)host->h_addr_list[0]);
	strcpy(ip,inet_ntoa(in));
	WSACleanup();
	return in.s_addr;
}
//
unsigned long GetDomainIp(char domainname[250])
{
	char IP[MAX_PATH],*ip;
	struct in_addr in;
	struct hostent *host;
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested=MAKEWORD(2,0);
	ip=IP;
	strcpy(ip,"Ip not get!");
	if(WSAStartup(wVersionRequested,&wsaData)) return 0;
	if(!(host=gethostbyname(domainname))){
		WSACleanup();
		return 0;
	}
	in.s_addr=*((unsigned long *)host->h_addr_list[0]);
	strcpy(ip,inet_ntoa(in));
	WSACleanup();
	return in.s_addr;
}
//
int GetValidPort(){
	int port;
	HINSTANCE hdll;
	int CurrentUsePort[1000] = {0};
	int portnum = 0, i = 0;
	unsigned long table_len;
	MIB_TCPTABLE tcp_table;
	MIB_UDPTABLE udp_table;
	port=0;
	DWORD (*GetTcpTable)(PMIB_TCPTABLE,PDWORD,bool);
	DWORD (*GetUdpTable)(PMIB_UDPTABLE,PDWORD,bool);
	hdll=LoadLibrary("iphlpapi.dll");
	if(!hdll) return port;
	(FARPROC &)GetTcpTable=GetProcAddress(hdll,"GetTcpTable");
	(FARPROC &)GetUdpTable=GetProcAddress(hdll,"GetUdpTable");
	if(GetTcpTable){
		TRACE("get tcp table\r\n");
		table_len=(unsigned long)sizeof(tcp_table);
		GetTcpTable(&tcp_table,(PDWORD)&table_len,false);
	}
	if(GetUdpTable){
		TRACE("get udp table\r\n");
		table_len=sizeof(MIB_UDPTABLE);
		//GetUdpTable(&udp_table,&table_len,1);
	}
	FreeLibrary(hdll);
	return port;
}
//
int SendMethodToClient(SOCKET remotesock,unsigned char methodid){
	unsigned char data[2];
	data[0]=PROXYVERSION; 
	data[1]=methodid;
	while(!send(remotesock,(const char *)data,sizeof(data),0)) ;
	return 1;
}
//
int SendRepToClient(SOCKET remotesock,unsigned char Rep,unsigned long ipmark,int port){
	char data[10];
	unsigned long ip;
	data[0]=PROXYVERSION;
	data[1]=Rep;
	data[2]=0x00;
	data[3]=ATYP_IPV4;
	port=htons(port);
	memcpy((void *)&data[8],(const void *)&port,2);
	ip=ipmark;
	//ip=inet_addr("202.194.4.218");
	memcpy((void *)&data[4],(const void *)&ip,4);
	while(!send(remotesock,data,sizeof(data),0));
	return 1;
} 
//
UINT DeailThread(LPVOID info){
	LPSOCKINFO psockinfo;
	SOCKET sourcesock,destsock;
	char data[MAXDATALEN];
	long eventid;
	int datalen;
	psockinfo=(LPSOCKINFO)info;
	sourcesock=psockinfo->sourcesock;
	destsock=psockinfo->destsock;
	TRACE("deail thread ok!\r\n");
	while(true){
		eventid=GetSocketEventId(sourcesock);
		switch(eventid){
        case FD_CLOSE:
			TRACE("s fdclosed\r\n");
			closesocket(destsock);
			return 1;
			break;
        case FD_READ:
			TRACE("s fdread\r\n");
			datalen=recv(sourcesock,data,sizeof(data),0);
			if(datalen>0){
				while(!send(destsock,data,datalen,0));
				TRACE("send s to d\r\n");
			}
			break;
        default:break;
        }
		eventid=GetSocketEventId(destsock);
		switch(eventid){
        case FD_CLOSE:
			closesocket(sourcesock);
			TRACE("d fdclosed\r\n");
			return 1;
			break;
        case FD_READ:
			TRACE("d fd read\r\n");
			datalen=recv(destsock,data,sizeof(data),0);
			if(datalen>0){
				while(!send(sourcesock,data,datalen,0));
				TRACE("send d to s\r\n");
			}
			break;
        default:break;
        }
		Sleep(1);
	}
}
//
UINT ReciveThread(LPVOID info){
	LPSOCKINFO psockinfo;
	SOCKET sourcesock,destsock;
	char data[MAXDATALEN];
	long eventid;
	int datalen;
	psockinfo=(LPSOCKINFO)info;
	sourcesock=psockinfo->sourcesock;
	destsock=psockinfo->destsock;
	TRACE("deail recive thread ok!\r\n");
	while(true){
		eventid=GetSocketEventId(sourcesock);
		switch(eventid){
        case FD_CLOSE:
			TRACE("s fdclosed\r\n");
			closesocket(destsock);
			return 1;
			break;
        default:break;
        }
		eventid=GetSocketEventId(destsock);
		switch(eventid){
        case FD_CLOSE:
			closesocket(sourcesock);
			TRACE("d fdclosed\r\n");
			return 1;
			break;
        default:break;
        }
		datalen=recv(sourcesock,data,sizeof(data),0);
		if(datalen==0){
			closesocket(sourcesock);
			closesocket(destsock);
			TRACE("s fdclosed\r\n");
			break;
		}
		if(datalen>0){
			while(!send(destsock,data,datalen,0));
        }
		Sleep(1);
	}
	return 1;
}
//
UINT SendThread(LPVOID info){
	LPSOCKINFO psockinfo;
	SOCKET sourcesock,destsock;
	char data[MAXDATALEN];
	long eventid;
	int datalen;
	psockinfo=(LPSOCKINFO)info;
	sourcesock=psockinfo->sourcesock;
	destsock=psockinfo->destsock;
	TRACE("deail send thread ok!\r\n");
	while(true){
		eventid=GetSocketEventId(sourcesock);
		switch(eventid){
        case FD_CLOSE:
			TRACE("s fdclosed\r\n");
			closesocket(destsock);
			return 1;
			break;
        default:break;
        }
		eventid=GetSocketEventId(destsock);
		switch(eventid){
        case FD_CLOSE:
			closesocket(sourcesock);
			TRACE("d fdclosed\r\n");
			return 1;
			break;
        default:break;
        }
		datalen=recv(destsock,data,sizeof(data),0);
		if(datalen==0){
			closesocket(sourcesock);
			closesocket(destsock);
			TRACE("d fdclosed\r\n");
			break;
		}
		if(datalen>0){
			while(!send(sourcesock,data,datalen,0));
        }
		Sleep(1);
	}
	return 1;
}
//
UINT UdpDeailThread(LPVOID info){
	LPCLIENTINFO pclientinfo;
	sockaddr_in source_sock_addr,dest_sock_addr,from_sock_addr;
	SOCKET tcp_sourcesock,udp_listensock;
	char data[MAXDATALEN],senddata[MAXDATALEN];
	long eventid;
	int datalen,socklen,index,DestPort;
	unsigned char ATYP;
	char ip[250];
	pclientinfo=(LPCLIENTINFO)info;
	udp_listensock=pclientinfo->udpsock;
	tcp_sourcesock=pclientinfo->clientsock;
	source_sock_addr=pclientinfo->remotesock;
	TRACE("UDP deail recive thread ok!\r\n");
	while(true){
		eventid=GetSocketEventId(tcp_sourcesock);
		switch(eventid){
        case FD_CLOSE:
			TRACE("tcp_sourcesock fdclosed\r\n");
			closesocket(tcp_sourcesock);
			closesocket(udp_listensock);
			return 1;
			break;
        default:break;
        }
		socklen=sizeof(from_sock_addr);
		datalen=recvfrom(udp_listensock,data,sizeof(data),0,(sockaddr *)&from_sock_addr,&socklen);
		/*
		if(datalen==0){
		closesocket(sourcesock);
		closesocket(destsock);
		TRACE("s fdclosed\r\n");
		break;
		}
		*/
		if(datalen>0){
			if(from_sock_addr.sin_addr.S_un.S_addr!=source_sock_addr.sin_addr.S_un.S_addr){
				while(!sendto(udp_listensock,data,datalen,0,(sockaddr *)&source_sock_addr,sizeof(source_sock_addr)));
			}
			else if(datalen>8){
				ATYP=data[3];
				switch(ATYP){
				case ATYP_IPV4:
					memcpy((void *)&DestPort,(const void *)&data[8],2);
					//DestPort=110;
					ZeroMemory(ip,sizeof(ip));
					wsprintf(ip,"%d.%d.%d.%d",(unsigned char)data[4],(unsigned char)data[5],(unsigned char)data[6],(unsigned char)data[7]);
					//strcpy(ip,"mail.sdu.edu.cn");
					TRACE(ip);
					break;
				case ATYP_IPV6:
					closesocket(tcp_sourcesock);
					closesocket(udp_listensock);
					TRACE("ipv6 ok!\r\n");
					return 0;
					break;
				case ATYP_DOMAIN:
					index=data[4];
					memcpy((void *)&DestPort,&data[index+5],2);
					ZeroMemory(ip,sizeof(ip));
					memcpy((void *)ip,(const void *)&data[5],index);
					ip[index]='\0';
					TRACE(ip);
					break;
				default:
					closesocket(tcp_sourcesock);
					closesocket(udp_listensock);
					TRACE("default ok!\r\n");
					return 0;              
					break;
				}
				if(ATYP==ATYP_IPV4) memcpy((void *)senddata,(const void *)data[10],datalen-10);
				else memcpy((void *)senddata,(const void *)data[index+7],datalen-index-7);
				dest_sock_addr.sin_family=AF_INET;
				dest_sock_addr.sin_port=DestPort;
				if(ATYP==ATYP_IPV4) dest_sock_addr.sin_addr.S_un.S_addr=inet_addr(ip);
				else dest_sock_addr.sin_addr.S_un.S_addr=GetDomainIp(ip);
				while(!sendto(udp_listensock,senddata,datalen-index-7,0,(sockaddr *)&dest_sock_addr,sizeof(dest_sock_addr)));
			}  
        }
		Sleep(1);
	}
	return 1;
}
//
UINT ProxyServerThread(LPVOID info){
	LPCLIENTINFO pclientinfo;
	SOCKET ClientSock,connectsock,listensock,newsock;
	sockaddr_in remotesock_addr,sock_addr,localsock_addr;
	char data[MAXDATALEN];
	int datalen,methodlen,i,index;
	unsigned char CMD,ATYP;
	int DestPort,socklen;
	char ip[250];
	struct in_addr ipaddr;
	pclientinfo=(LPCLIENTINFO)info;
	ClientSock=pclientinfo->clientsock;
	remotesock_addr=pclientinfo->remotesock;
	CString remoteip;
	ipaddr.S_un.S_addr=remotesock_addr.sin_addr.S_un.S_addr;
	remoteip.Format("%s",inet_ntoa(ipaddr));
	TRACE(remoteip);
	TRACE(" connect ok...\r\n");
	CString temp;
	datalen=recv(ClientSock,(char *)data,sizeof(data),0);
	if(datalen>2){
		if(data[0]!=PROXYVERSION){
			TRACE("remote server send data no socks5 \r\n");
			SendMethodToClient(ClientSock,NOMETHOD);
			closesocket(ClientSock);
			return 1;
		}
		methodlen=data[1];
		for(i=2;i<(methodlen+2);i++){
			if(data[i]==NOAUTHLOGIN){
				TRACE("send method ok!\r\n");
				SendMethodToClient(ClientSock,NOAUTHLOGIN);
				break;
			}
		}
	}
	else{
		TRACE("remote server send data wrong!\r\n");
		SendMethodToClient(ClientSock,NOMETHOD);
		closesocket(ClientSock);
		return 1;
	}
	datalen=recv(ClientSock,(char *)data,sizeof(data),0);
	temp.Format("%d",datalen);
	TRACE(temp);
	if(datalen>=10){
		TRACE("get cmd ok!");
		CMD=data[1];
		ATYP=(unsigned char)data[3];
		switch(ATYP){
		case ATYP_IPV4:
			memcpy((void *)&DestPort,(const void *)&data[8],2);
			//DestPort=110;
			ZeroMemory(ip,sizeof(ip));
			wsprintf(ip,"%d.%d.%d.%d",(unsigned char)data[4],(unsigned char)data[5],(unsigned char)data[6],(unsigned char)data[7]);
			//strcpy(ip,"mail.sdu.edu.cn");
			TRACE(ip);
			break;
		case ATYP_IPV6:
			DestPort=*(int *)&data[20];
			SendRepToClient(ClientSock,REP_ATYP_WRONG,GetLocalIp(),0);
			closesocket(ClientSock);
			TRACE("ipv6 ok!\r\n");
			return 0;
			break;
		case ATYP_DOMAIN:
			index=data[4];
			memcpy((void *)&DestPort,&data[index+5],2);
			ZeroMemory(ip,sizeof(ip));
			memcpy((void *)ip,(const void *)&data[5],index);
			ip[index]='\0';
			TRACE(ip);
			break;
		default:
			SendRepToClient(ClientSock,REP_ATYP_WRONG,GetLocalIp(),0);
			TRACE("default ok!\r\n");
			closesocket(ClientSock);
			return 0;              
			break;
		}
		sock_addr.sin_family=AF_INET;
		sock_addr.sin_port=DestPort;
		if(ATYP==ATYP_DOMAIN) sock_addr.sin_addr.S_un.S_addr=GetDomainIp(ip);
		else sock_addr.sin_addr.S_un.S_addr=inet_addr(ip);   
		localsock_addr.sin_family=AF_INET;
		localsock_addr.sin_port=htons(8001);
		localsock_addr.sin_addr.S_un.S_addr=INADDR_ANY;  
		if(CMD==CMD_CONNECT){
			TRACE("to connect remote ip...\r\n");
			connectsock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			if(connect(connectsock,(const sockaddr *)&sock_addr,sizeof(sock_addr))==INVALID_SOCKET){
				SendRepToClient(ClientSock,REP_TTL_TIMEOUT,GetLocalIp(),0);
				closesocket(ClientSock);
				return 0;
			}
			socklen=sizeof(sock_addr);
			if(getsockname(connectsock,(sockaddr *)&sock_addr,&socklen)==SOCKET_ERROR)
				TRACE("get sock name err\r\n");           
			SendRepToClient(ClientSock,REP_SUCCESS,GetLocalIp(),ntohs(sock_addr.sin_port));
			sockinfo.sourcesock=ClientSock;
			sockinfo.destsock=connectsock;
			TRACE("connect ok...\r\n");
			AfxBeginThread(ReciveThread,(LPVOID)&sockinfo);  
			AfxBeginThread(SendThread,(LPVOID)&sockinfo);
			//AfxBeginThread(DeailThread,(LPVOID)&sockinfo);  
		}
		else if(CMD==CMD_BIND){
			TRACE("to bind remote ip...\r\n");
			listensock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			if(bind(listensock,(sockaddr *)&localsock_addr,sizeof(localsock_addr))==SOCKET_ERROR){
				SendRepToClient(ClientSock,REP_FAILED,GetLocalIp(),0);
				closesocket(ClientSock);
				TRACE("to bind remote failed...\r\n");
				return 0;
			}
			listen(listensock,10);                     
			TRACE("to bind remote ok...\r\n");
			SendRepToClient(ClientSock,REP_SUCCESS,GetLocalIp(),7895);
			newsock=accept(listensock,(sockaddr *)NULL,(int *)NULL);
			if(newsock==INVALID_SOCKET){
				SendRepToClient(ClientSock,REP_FAILED,GetLocalIp(),0);
				closesocket(ClientSock);
				TRACE("to accpet remote failed...\r\n");
				return 0;
			}
			SendRepToClient(ClientSock,REP_SUCCESS,GetLocalIp(),7895);
			sockinfo.sourcesock=ClientSock;
			sockinfo.destsock=newsock;
			TRACE("accept ok...\r\n");
			AfxBeginThread(ReciveThread,(LPVOID)&sockinfo);  
			AfxBeginThread(SendThread,(LPVOID)&sockinfo); 
		}
		else if(CMD==CMD_UDP){
			TRACE("to udp remote ...\r\n");
			TRACE("to bind remote ip...\r\n");
			listensock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
			if(bind(listensock,(sockaddr *)&localsock_addr,sizeof(localsock_addr))==SOCKET_ERROR){
				SendRepToClient(ClientSock,REP_FAILED,GetLocalIp(),0);
				closesocket(ClientSock);
				TRACE("to bind remote failed...\r\n");
				return 0;
			}                     
			TRACE("to bind remote ok...\r\n");
			SendRepToClient(ClientSock,REP_SUCCESS,GetLocalIp(),8001);
			UdpClientInfo.clientsock=ClientSock;
			UdpClientInfo.remotesock=sock_addr;
			UdpClientInfo.udpsock=listensock;
			TRACE("udp thread ok...\r\n");
			AfxBeginThread(UdpDeailThread,(LPVOID)&UdpClientInfo); 
		}
		else{
			SendRepToClient(ClientSock,REP_FAILED,GetLocalIp(),0);
			closesocket(ClientSock);
			return 0;
		}
   }
   return 1;
}
//
UINT StartSocks5Proxy(LPVOID info){
	SOCKET NewSock;
	int socklen;
	sockaddr_in serversock,remotesock_addr;
	serversock.sin_family=AF_INET;
	serversock.sin_addr.S_un.S_addr=INADDR_ANY;
	serversock.sin_port=htons(Socks5ListenPort);
	Socks5ListenSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(Socks5ListenSock==INVALID_SOCKET) return 0;
	if(bind(Socks5ListenSock,(const sockaddr *)&serversock,sizeof(serversock))==SOCKET_ERROR) return 0;
	listen(Socks5ListenSock,BACKUP);
	socklen=sizeof(remotesock_addr);
	TRACE("start sock5 proxy thread while\r\n");
	while(true){
		while(!CanSockProxy) Sleep(1);
		NewSock=accept(Socks5ListenSock,(sockaddr *)&remotesock_addr,&socklen);
		TRACE("waitting ok...\r\n");
		if(NewSock==INVALID_SOCKET){
			Sleep(1);
			continue;
		}
		ZeroMemory((void *)&TcpClientInfo,sizeof(CLIENTINFO));
		TcpClientInfo.clientsock=NewSock;
		TcpClientInfo.remotesock=remotesock_addr;
		TRACE("start proxy thread\r\n");
		AfxBeginThread(ProxyServerThread,(LPVOID)&TcpClientInfo);
		Sleep(100);
	}
	return 1;
}
//
CProxy::CProxy()
{
	WSADATA WsaData;
	WORD wsaVer;
	wsaVer=MAKEWORD(2,0);
	WsaStartupOk=false;
	CanSockProxy=false;
	//ListenPort=LISTENPORT;
	if(WSAStartup(wsaVer,&WsaData)!=SOCKET_ERROR) WsaStartupOk=true;
}

CProxy::~CProxy()
{
	if(WsaStartupOk){
		WSACleanup();
	}
}
int CProxy::StartProxy(int listenport)
{
	Socks5ListenPort=listenport;
	CanSockProxy=true;
	AfxBeginThread(StartSocks5Proxy,(LPVOID)NULL);
	return 1;
}


int CProxy::StopProxy()
{
	CanSockProxy=false;
	return 0;
}

