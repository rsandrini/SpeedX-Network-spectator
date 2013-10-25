#include <iostream>
#define bSendSize    256
#define bReceiveSize 4096

class RedeTcp {

	char SendBuffer[bSendSize];
	char ReceiveBuffer[bReceiveSize];

	int Socket;

	unsigned int In, Out;

	public:
		RedeTcp();
		~RedeTcp();

		bool createServer(short porta);
		bool connectServer(char* ip, short porta);
		bool disconnect();

		void update();
		bool receiveMessage(void* messageContent, int size);
		bool sendMessage(void* messageContent, int size = -1);
};
