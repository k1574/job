#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

enum{
	SrvPort = 8000,
	MaxClients = 64,
};

struct Client {
	int sk;
	struct sockaddr_in *addr;
	struct Client *next;
};

int
main(int argc, char *argv[])
{
	char buf[512], *ip, *ipc;
	char nl[2] = "\n" ;
	int sk, ln, i;
	socklen_t dummy;
	struct sockaddr_in addr, *caddr;
	struct Client zero, *first, *pclient, *last;

	first = last = &zero ;

	if((ln = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(1);
	}

	addr.sin_family = AF_INET ;
	addr.sin_port = htons(SrvPort) ;
	addr.sin_addr.s_addr = htonl(INADDR_ANY) ;

	if(bind(ln, (struct sockaddr *)&addr, sizeof(addr))){
		perror("bind");
		exit(1);
	}

	listen(ln, 64);

	while(1){
		caddr = malloc(sizeof(struct sockaddr_in)) ;

		if(( sk = accept(ln, (struct sockaddr *)caddr, &dummy) ) < 0 ){
			perror("accept");
			continue;
		}

		last->next = malloc(sizeof(struct Client));
		last = last->next ;

		last->sk = sk ;
		last->next = 0 ;
		last->addr = caddr ;
		

		pclient = first->next ;
		while(pclient && pclient != last){
			ip = inet_ntoa(caddr->sin_addr) ;
			send(pclient->sk, ip, strlen(ip), 0);
			send(pclient->sk, nl, strlen(nl), 0);

			ip = inet_ntoa(pclient->addr->sin_addr) ;
			send(sk, ip, strlen(ip), 0);
			send(sk, nl, strlen(nl), 0);
			pclient = pclient->next ;
		}
	}
	
	return 0 ;
}

