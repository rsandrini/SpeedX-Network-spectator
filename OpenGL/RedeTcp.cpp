#include "RedeTcp.h"
#ifdef _WIN32
#include <WinSock.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
#endif

RedeTcp::RedeTcp() {
	Socket = In = Out = 0;

#ifdef _WIN32
	WSADATA     wsaData;
	WORD        wVersion;

	/* procura uma DLL WinSock de versao 1.1 */
	wVersion = MAKEWORD(2,0);
	if(WSAStartup(wVersion,&wsaData) != 0) {
		exit(0);
	}
	if((LOBYTE(wsaData.wVersion) != 2) || (HIBYTE(wsaData.wVersion) != 0)) {
		WSACleanup();
		exit(0);
	}
#endif
}

RedeTcp::~RedeTcp() {
#ifdef _WIN32
	WSACleanup();
#endif
}

bool RedeTcp::createServer(short porta) {
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));

	/*somente um socket para escutar*/
	int socketListen = socket(AF_INET, SOCK_STREAM, 0);
	if(socketListen == -1) {
		return false;
	}

	int yes = 1;
#ifdef _WIN32
	if(setsockopt(socketListen, SOL_SOCKET, SO_REUSEADDR,(char*)&yes, sizeof(int)) == -1) {
		return false;
	}
#else
	if(setsockopt(socketListen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		return false;
	}
#endif

	serverAddress.sin_family      = AF_INET;
	serverAddress.sin_port        = htons(porta);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	bind(socketListen, (struct sockaddr*)&serverAddress, sizeof(serverAddress));  /*batizando o socket*/
	listen(socketListen, 1); /*aguarda apenas uma conexao*/
	Socket = accept(socketListen, (struct sockaddr*)NULL, NULL); /* NULLS 1: endereco do cliente, 2: tamanho da estrutura*/

#ifdef _WIN32
	closesocket(socketListen);
#else
	close(socketListen);
#endif

	if(Socket == -1) {
		return false;
	}

	/*Colocando Socket para nao bloqueante*/
#ifdef _WIN32
	u_long opt = 1;
	ioctlsocket(Socket, FIONBIO, &opt);
#else
	int opt = 1;
	ioctl(Socket, FIONBIO, &opt);
#endif

	return true;
}

bool RedeTcp::connectServer(char* ip, short porta) {
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));

	Socket = socket(AF_INET, SOCK_STREAM, 0);
	if(Socket == -1) {
		return false;
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port   = htons(porta);

#ifdef _WIN32
	serverAddress.sin_addr.s_addr = inet_addr(ip);
	if(serverAddress.sin_addr.s_addr == INADDR_NONE) {
		return false;
	}
#else
	/*preenchendo o endereco ip*/
	if(inet_pton(AF_INET, ip, &serverAddress.sin_addr)<=0) {
		return false;
	}
#endif

	if(connect(Socket,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		return false;
	}

	/*Colocando Socket para nao bloqueante*/
#ifdef _WIN32
	u_long opt = 1;
	ioctlsocket(Socket, FIONBIO, &opt);
#else
	int opt = 1;
	ioctl(Socket, FIONBIO, &opt);
#endif

	return true;
}

bool RedeTcp::disconnect() {
#ifdef _WIN32
	closesocket(Socket);
#else
	close(Socket);
#endif
	return true;
}

void RedeTcp::update() {
	int n = 0;
	do {
		if((In+1)%bReceiveSize == Out) /*buffer de recebimento cheio*/
			return;

		if(Out <= In)
			n = recv(Socket, ReceiveBuffer+In, bReceiveSize-In, 0);
		else
			n = recv(Socket, ReceiveBuffer+In, Out-In-1, 0);

		if(n > 0) { /*dados recebidos*/
			In = (In+n)%bReceiveSize; /*posiciona In no buffer circular de acordo com numero de bytes 'n' recebidos*/
		}
	} while(n > 0);
}

bool RedeTcp::receiveMessage(void* messageContent, int size) {
	char aux[bReceiveSize+1];

	if(Out == In) /*buffer vazio*/
		return false;

	int mSize = ReceiveBuffer[Out]; /*tamanho da mensagem recebida no buffer*/

	if(size <= 0 || mSize > size) /*garantia que cabe o buffer cabe em messageContent*/
		return false;

	if(In > Out) {
		if(In-Out > mSize) {
			memcpy(messageContent,ReceiveBuffer+Out+1,mSize);
			Out = (Out+mSize+1)%bReceiveSize;
			return true;
		}
	} else {
		/*testar se o tamanho do buffer (que deu a volta) é do tamanho do buffer que estamos procurando)*/
		if(bReceiveSize-Out+In > mSize) {
			if(bReceiveSize-Out > mSize) { /*tamanho até o final do buffer é maior que o necessário*/
				memcpy(messageContent,ReceiveBuffer+Out+1,mSize);
				Out = (Out+mSize+1)%bReceiveSize;
				return true;
			} else {
				/*o buffer deu a volta, copiamos até o seu final e mais o que falta no inicio*/
				unsigned int n = bReceiveSize-Out-1; /*quantidade de bytes copiados até o final do buffer*/
				memcpy(messageContent,ReceiveBuffer+Out+1,n);
				memcpy((char *)messageContent+n,ReceiveBuffer,mSize-n); /*copiando a quantidade de bytes restantes*/
				Out = (Out+mSize+1)%bReceiveSize;
				return true;
			}
		}

	}

	return false;
}

bool RedeTcp::sendMessage(void* messageContent, int size) {
	if(size == -1) {
		size = strlen((char*)messageContent)+1;
	}

	if(size <= 0 || size > bSendSize-1)
		return false;

	SendBuffer[0] = (unsigned char)size; /*colocando o tamanho do pacote no buffer*/
	memcpy(SendBuffer+1,messageContent,size);

	if(send(Socket, SendBuffer, size+1, 0) == -1) {
		return false;
	}

	return true;
}


