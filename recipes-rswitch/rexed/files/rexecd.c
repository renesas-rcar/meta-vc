
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <netinet/in.h>
#include <signal.h>


#define BUFSIZE  0x1000

static char sok_buf[BUFSIZE];
static char ex_buf[BUFSIZE];



static void execute(const char *command, const int connfd)
{
	FILE *fe;
	char *s;

	fe = popen(command, "r");
	if (fe != NULL) {
		while (1) {
			s = fgets(ex_buf, sizeof(ex_buf), fe);
			if (s == NULL)
				break;
//			if (write(connfd, ex_buf, strlen(ex_buf)));
		}
		fclose(fe);
	}
}


//----------------------------------------------------------
int main(int argv, char **args)
{
	unsigned socketNumber;
	int mysocket;
	struct sockaddr_in serv_addr;
	int connfd;
	char *s;

	if (argv != 2 && argv != 3) {
		fprintf(stderr, "Remote Execution Deamon (rexec) Christian Mardmoeller 8/2016 V0.1\n");
		fprintf(stderr, "Invalid parameters\n"
						"usage: %s [PORT] [-s (optional)]\n"
						"\t-s\tsingle command mode\n", args[0]);
		return -EINVAL; /* Invalid argument */
	}
	sscanf(args[1], "%d", &socketNumber);

	//Register a signal handler to prevent kill of server when client closes
	signal(SIGPIPE, SIG_IGN);

	//Create and bind the socket to the mentioned address
	if ((mysocket = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) < 0) {
		fprintf(stderr, "ERROR: Creation of socket failed: %d (%s)\n", errno, strerror(errno));
		return mysocket;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(socketNumber);

	if (bind(mysocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
		fprintf(stderr, "ERROR: Failed to bind socket %d: %d (%s)\n", socketNumber, errno, strerror(errno));
		return -EPERM;
	}
	fprintf(stderr, "Bind on %d done.\n", socketNumber);

	if (listen(mysocket, 5) == -1) { //only 1 connection at time allowed
		fprintf(stderr, "ERROR: Failed to listen on socket %d: %d (%s)\n", socketNumber, errno, strerror(errno));
		return -EPERM;
	}
	printf("Listen on %d done.\n", socketNumber);


	while (1) {
		//Accept the connection from client
		if ((connfd = accept(mysocket, (struct sockaddr*)NULL, NULL)) < 0) {
			fprintf(stderr, "ERROR: Failed to accept the connection: %d (%s)\n", errno, strerror(errno));
			return -EPERM;
		}
		else {
			printf("Connection from client accepted.\n");

			s = sok_buf;
			while (1) {
				if (read(connfd, s, 1) != 1) {
					printf("  Closed by client\n");
					close(connfd);
					break;
				}
				if (*s == '\n' || (s-sok_buf+1) >= sizeof(sok_buf)) {
					*s = 0;
					printf("  get '%s'\n", sok_buf);
					execute(sok_buf, connfd);
					s = sok_buf;
					if(argv == 3 && !strncmp(args[2],"-s",2))
					{
						printf("  Closed by rexecd server\n");
						close(connfd);
						break;
					}
				}
				else {
					s++;
				}
			}
		}
	}
	return 0;
}
