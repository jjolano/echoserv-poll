// echoserv-poll by jjolano

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef __PS3__
#include <net/net.h>
#include <sysmodule/sysmodule.h>
#include <ppu-types.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#endif

int running = 1;
int list_s;

void cleanup(void)
{
	close(list_s);
	#ifdef __PS3__
	netDeinitialize();
	#endif
}

int main(void)
{
	printf("echoserv-poll by jjolano\n");
	
	char message[8192];
	struct sockaddr_in sa;
	socklen_t len;
	nfds_t nfds;
	int ret;
	struct pollfd *fds = NULL;
	
	#ifdef __PS3__
	netInitialize();
	#endif
	
	atexit(cleanup);
	
	printf("creating socket...\n");
	
	if((list_s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	printf("binding address...\n");
	
	sa.sin_family = AF_INET;
	sa.sin_port = htons(8888);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(list_s, (struct sockaddr *)&sa, sizeof(sa)) == -1)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}
	
	printf("listening...\n");
	
	if(listen(list_s, 5) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	
	nfds = 1;
	fds = (struct pollfd *)calloc(1, nfds * sizeof(struct pollfd));
	fds->fd = list_s;
	fds->events = POLLIN | POLLPRI;
	
	while(running)
	{
		int i;
		
		#ifdef __PS3__
		// prevent hanging
		ret = poll(fds, nfds, 2000);
		#else
		ret = poll(fds, nfds, -1);
		#endif
		
		if(ret == -1)
		{
			perror("poll");
			exit(EXIT_FAILURE);
		}
		
		for(i = 0; (i < nfds) && (ret > 0); i++)
		{
			if(!(fds + i)->revents)
			{
				// nothing happened?
				continue;
			}
			
			printf("i=%d, revents=0x%x, ret=%d, nfds=%d\n", i, (fds + i)->revents, ret, nfds);
			ret--;
			
			if(((fds + i)->fd == list_s) && ((fds + i)->revents & POLLIN))
			{
				// connection incoming, accept it
				// (fds + nfds)->fd
				printf("new incoming connection - accepting...\n");
				
				len = sizeof(sa);
				fds = (struct pollfd *)realloc(fds, (nfds + 1) * sizeof(struct pollfd));
				(fds + nfds)->fd = accept(list_s, (struct sockaddr *)&sa, &len);
				
				if((fds + nfds)->fd == -1)
				{
					// bad connection - drop it
					perror("accept");
					close((fds + nfds)->fd);
					fds = (struct pollfd *)realloc(fds, nfds * sizeof(struct pollfd));
					continue;
				}
				
				(fds + nfds)->events = POLLIN | POLLPRI | POLLRDBAND | POLLRDNORM;
				
				nfds++;
				continue;
			}
			if((fds + i)->revents & POLLNVAL)
			{
				printf("fd%d: invalid socket - freeing resource\n", (fds + i)->fd);
				memcpy(fds + i, fds + i + 1, (nfds - i) * sizeof(struct pollfd));
				nfds--;
				fds = (struct pollfd *)realloc(fds, nfds * sizeof(struct pollfd));
				continue;
			}
			if((fds + i)->revents & (POLLHUP | POLLERR))
			{
				printf("fd%d: peer reset connection\n", (fds + i)->fd);
				close((fds + i)->fd);
				memcpy(fds + i, fds + i + 1, (nfds - i) * sizeof(struct pollfd));
				nfds--;
				fds = (struct pollfd *)realloc(fds, nfds * sizeof(struct pollfd));
				continue;
			}
			if((fds + i)->revents & (POLLRDNORM | POLLPRI | POLLRDBAND))
			{
				ret = recv((fds + i)->fd, message, sizeof(message), 0);
				
				printf("fd%d: recv: %d\n", (fds + i)->fd, ret);
				
				if(ret <= 0)
				{
					if(ret == 0)
					{
						printf("peer disconnected\n");
						close((fds + i)->fd);
					}
					else
					{
						perror("recv");
						close((fds + i)->fd);
					}
					
					memcpy(fds + i, fds + i + 1, (nfds - i) * sizeof(struct pollfd));
					nfds--;
					fds = (struct pollfd *)realloc(fds, nfds * sizeof(struct pollfd));
					continue;
				}
				
				printf("recv: %.*s\n", ret, message);
				
				if(strncmp(message, "exit", 4) == 0)
				{
					running = 0;
				}
				
				// echo back
				send((fds + i)->fd, message, ret, 0);
				continue;
			}
		}
	}
	
	free(fds);
	return 0;
}

