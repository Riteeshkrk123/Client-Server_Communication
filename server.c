#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
int client_count=0;
void *processclient(void *arg)
{
    int conn = *((int *)arg);
    char buffer[1024];
    FILE *fp;

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        recv(conn, buffer, sizeof(buffer), 0);
        if (strcmp(buffer, "quit") == 0)
        {
            break;
        }

        // Process the command and send the result to the client
        printf("%s\n",buffer);
        fp = popen(buffer, "r");
        
        if (fp == NULL)
        {
            perror("popen failed");
            exit(EXIT_FAILURE);
        }
        memset(buffer, 0, sizeof(buffer));
        while (fgets(buffer, sizeof(buffer), fp))
        {
            send(conn, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
        }
        pclose(fp);
    }
    close(conn);
}

int conn;

int main()
{
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        conn = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        printf("New client connected\n");
         if (system("netstat -an | awk '$4 ~ /:8080$/ && $6 == \"ESTABLISHED\" && $5 !~ /:8080$/ {print $0}' | wc -l > count.txt")< 0)
  {
    printf ("\nCommand execution error \n");
    return -1;
  }
FILE *fp = fopen ("count.txt", "r");
char count_str[10];
fgets (count_str, 10, fp);
fclose (fp);
int count = atoi (count_str);
          printf("Number of clients connected: %d\n", count);
        pthread_t thread;
        pthread_create(&thread, NULL, processclient, (void *)&conn);
    }

    close(server_socket);
    return 0;
}
