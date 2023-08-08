//#include "reactorSinglecb.h"
//#include "reactor_multicb.h"
#include "reactor_bf.h"


int main(int argc, char** argv)
{
#if 0
	printf("%s 向你问好!\n", "Reactor");
	int serFd = initSock(9999);

	ntyReactor* reactor = (ntyReactor*)malloc(sizeof(ntyReactor));
	ntyReactor_init(reactor);

	ntyReactor_addListener(reactor, serFd, accept_cb);
	ntyreactor_run(reactor);

	ntyreactor_destory(reactor);
	close(serFd);
#elif 0
	int  connfd, n;

	int listenfd = init_server(9999);
	nreactor_set_event(listenfd, accept_callback, ACCEPT_CB, NULL);


	reactor_loop(listenfd);
#elif 1
	unsigned short port = SERVER_PORT; // listen 8888
	if (argc == 2) {
		port = atoi(argv[1]);
	}
	struct ntyreactor* reactor = (struct ntyreactor*)malloc(sizeof(struct ntyreactor));
	ntyreactor_init(reactor);

	int i = 0;
	int sockfds[PORT_COUNT] = { 0 };
	for (i = 0; i < PORT_COUNT; i++) {
		sockfds[i] = init_sock(port + i);
		ntyreactor_addlistener(reactor, sockfds[i], accept_cb);
	}


	ntyreactor_run(reactor);

	ntyreactor_destory(reactor);

	for (i = 0; i < PORT_COUNT; i++) {
		close(sockfds[i]);
	}

	free(reactor);
#endif
	return 0;
}