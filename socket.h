#ifndef _SOCKET_H_
#define _SOCKET_H_

void socket_set_non_blocking(int sockfd);
int socket_create_and_bind(char *port);
void socket_start_listening(int sockfd);

#endif	/* _SOCKET_H_ */
