// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pwd.h>
#define PORT 8080

int main(int argc, char *argv[])
{
    int server_fd, new_socket, valread, child_status;
    struct sockaddr_in address;
    struct passwd *pwd;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    char *flag = "true";
    char num_string[5];

    printf("Current process pid: %d\n", getpid());
    //printf("Address of printf=%p\n", (void *)printf);

    if (strcmp(argv[2], flag) == 0)
    {
        printf("Current process pid in child after exec: %d\n", getpid());

        pwd = getpwnam("nobody");
        printf("Nobody UID: %ld\n", (long)pwd->pw_uid);
        printf("Original UID is %d\n", getuid());

        if (setuid(pwd->pw_uid) == -1)
        {
            perror("setuid(nobody)");
            exit(EXIT_FAILURE);
        }

        printf("UID after setuid is %ld\n", (long)getuid());

        printf("In child server_fd: %s\n", argv[1]);

        char const *a = argv[1];
        int num = atoi(a);

        if ((new_socket = accept(num, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        valread = read(new_socket, buffer, 1024);
        printf("Read %d bytes: %s\n", valread, buffer);
        send(new_socket, hello, strlen(hello), 0);
        printf("Hello message sent\n");
        return 0;
    }

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    printf("Initial server_fd %d\n", server_fd);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Could not fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        printf("Current process pid in child: %d\n", getpid());

        sprintf(num_string, "%d", server_fd);
        char *paramList[] = {argv[0], num_string, flag, NULL};
        execv(argv[0], paramList);
        perror("exec failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if (waitpid(pid, &child_status, 0) < 0)
    {
        perror("Could not wait");
        exit(EXIT_FAILURE);
    }

    return 0;
}
