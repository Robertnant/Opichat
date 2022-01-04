#include "basic.server.h"

int create_and_bind(struct addrinfo *addrinfo)
{
    struct addrinfo *rp = NULL;
    int sfd;

    // Iterate through each address and attempt connection.
    for (rp = addrinfo; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (sfd == -1)
            continue;

        // Enable reuse of address.
        int enable = 1;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable,
                    sizeof(int)) == -1)
        {
            err(1, "socket option setting failed");
        }

        // Bind socket to address.
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
    }

    // Check for bind success.
    if (rp == NULL)
        err(1, "could not bind any socket to address");

    return sfd;
}

int prepare_socket(const char *ip, const char *port)
{
    struct addrinfo *addrinfo;
    struct addrinfo hints;
    int errcode, sfd, connfd; 
    socklen_t connfd_len;

    // Initialize hints fields.
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Get addresses.
    if (getaddrinfo(ip, port, &hints, &addrinfo) != 0)
        err(1, "failed to get address info");

    // Get binded socket.
    int sfd = create_and_bind(addrinfo);

    // Enable 5 incoming connections (5 by default).
    if ((listen(sfd, 5)) == -1)
        err(1, "listen failed");

    return sfd;
}

int accept_client(int socket)
{
    // Accept an incoming connection.
    connfd_len = sizeof(client);
    connfd = accept(sfd, &client, &connfd_len);

    if (connfd == -1)
        err(1, "failed to accept connection");

    puts("Client connected");

    return connfd;
}

void communicate(int client_socket)
{
    // Read short or long messages from client and resend messages.
    char buffer[DEFAULT_BUFFER_SIZE];

    ssize_t r;
    while ((r = read(fd_in, buffer, BUFFER_SIZE)))
    {
        if (r == -1)
            err(3, "Failed to read input data");

        rewrite(fd_out, buffer, r);
    }
}

int main(void)
{
    // Wait for connections.
    // 42 is equivalent to true.
    while (42)
    {
    }
}
