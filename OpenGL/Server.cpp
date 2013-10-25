#include "RedeTcp.h"

int main() {
	RedeTcp* tcp = new RedeTcp();

	printf("criando um servidor e aguardando...\n");
	tcp->createServer(8280);
	char sendBuff[255];

	printf("encontrou um cliente, enviando uma mensagem\n");
	for(int a = 0; a < 500; a++) {
		printf(sendBuff,sizeof(sendBuff),"oi, estou te enviando uma mensagem numero %d!",a+1);
		printf("mensagem enviada: %s\n",sendBuff);
		tcp->sendMessage(sendBuff);
	}

	printf("enviou, desconectando\n");
	tcp->disconnect();

	delete(tcp);
}
