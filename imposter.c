/**
 * @file  imposter.c
 * @brief imPOSTer is a tool to diagnose SMTP
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAXLINE (1024)

ssize_t sock_read(int fd, void *ptr, size_t n);
ssize_t sock_write(int fd, const void *ptr, size_t n);
ssize_t sock_read_line(int fd, void *ptr, size_t maxlen);

/**
 * @fn    int main(int argc, char **argv)
 * @brief Entry point
 */
int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
    printf("ImPOSTer\n");

    struct hostent *host = gethostbyname(argv[1]);
    if (host) {
	printf("Hostname: %s\n", host->h_name);
	for(char **pp=host->h_aliases; *pp; pp++) {
	    printf("Alias:    %s\n", *pp);
	}
	for(char **pp=host->h_addr_list; *pp; pp++) {
	    char s[INET6_ADDRSTRLEN];
	    printf("Address:  %s\n",
		   inet_ntop(host->h_addrtype, *pp, s, sizeof(s)));
	}
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(25);
    memcpy(&server_addr.sin_addr,
	   *(struct in_addr **)host->h_addr_list,
	   sizeof(struct in_addr));
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
	char line[MAXLINE];
	printf("Connected\n");
	sock_read_line(sock_fd, line, MAXLINE);
	printf("line: [%s]\n", line);
    }
    close(sock_fd);
    
    return 0;
}

/**
 * @fn    ssize_t sock_read(int fd, void *ptr, size_t n)
 * @brief Read n bytes from a socket
 */
ssize_t sock_read(int fd, void *ptr, size_t n)
{
    size_t nleft = n;
    ssize_t nread = 0;
    char *cptr = (char *)ptr;
    while(nleft > 0) {
	nread = read(fd, cptr, nleft);
	if (nread < 0) {
	    if (errno == EINTR) {
		nread = 0;
	    } else {
		return -1;
	    }
	} else if (nread == 0) {
	    break;
	}
	nleft -= nread;
	cptr += nread;
    }
    return n - nleft;
}

/**
 * @fn    ssize_t sock_write(int fd, const void *ptr, size_t n)
 * @brief Write n bytes to a socket
 */
ssize_t sock_write(int fd, const void *ptr, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten = 0;
    const char *cptr = (const char *)ptr;
    while(nleft > 0) {
	nwritten = write(fd, cptr, nleft);
	if (nwritten <= 0) {
	    if (errno == EINTR) {
		nwritten = 0;
	    } else {
		return -1;
	    }
	}
	nleft -= nwritten;
	cptr += nwritten;
    }
    return n - nleft;
}

/**
 * @fn    ssize_t sock_read_line(int fd, void *ptr, size_t maxlen)
 * @brief Read a line from a socket (byte-by-byte)
 */
ssize_t sock_read_line(int fd, void *ptr, size_t maxlen)
{
    ssize_t n = -1;
    ssize_t rc = 0;
    char *cptr = (char *)ptr;
    char ch;
    for(n = 1; n < maxlen; n++) {
    again:
	rc = read(fd, &ch, 1);
	if (rc == 1) {
	    *cptr++ = ch;
	    if (ch == '\n') {
		break;
	    }
	} else if (rc == 0) {
	    if (n == 1) {
		return 0;
	    } else {
		break;
	    }
	} else {
	    if (errno == EINTR) {
		goto again;
	    }
	    return -1;
	}
    }
    *cptr = 0;
    return n;
}
