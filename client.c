#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#define PORT1 8080
#define PORT2 8090
int countServer1=0;
int countServer2=0;

char *validate(char *command)
{
    char *result = malloc(sizeof(char) * 1024);
    char *res[2];
    int i;   
    
    if (sscanf(command, "findfile %ms", &res[0]) == 1)
    {
        sprintf(result, "echo -e $(find ~/ -maxdepth 1 -type f -name %s -exec stat -c '%s\\t%%w\\t%%n' {} \\; | head -n 1)", res[0]);
        free(res[0]);
    }
  
    else if (sscanf(command, "sgetfiles %ms %ms%*c", &res[0], &res[1]) == 2)
    {
    	char cwd[4096];
	getcwd(cwd, sizeof(cwd));
	printf("cwd: %s\n",cwd);
	
        sprintf(result, "zip %s/temp.tar.gz $(find ~/ -maxdepth 1 -type f -size +%sc -size -%sc)",cwd, res[0], res[1]);
        //if(command[strlen(command)-1] == 'u') {
         // sprintf(result, "zip %s/temp.tar.gz $(find ~/ -maxdepth 1 -type f -size +%sc -size -%sc) \; echo 15secondwait \; sleep 15 \; unzip %s/temp.tar.gz;",cwd, res[0], res[1],cwd);
        //}
        //else{
          //sprintf(result, "zip %s/temp.tar.gz $(find ~/ -maxdepth 1 -type f -size +%sc -size -%sc)",cwd, res[0], res[1]);
        //}
        free(res[0]);
        free(res[1]);
    }
    else if (sscanf(command, "dgetfiles %ms %ms%*c", &res[0], &res[1]) == 2)
    {
        sprintf(result, "zip temp.tar.gz $(find ~/ -maxdepth 1 -type f -newermt \"%s\" ! -newermt \"%s\")", res[0], res[1]);
        free(res[0]);
        free(res[1]);
    }
    else if (sscanf(command, "getfiles %[^\n]", result) == 1)
    {
        char *token;
        char files[1024] = "";
        int count = 0;
        token = strtok(result, " ");
        while (token != NULL)
        {
            char tmp[256];
            if (count > 0)
            {
                strcat(files, "-o ");
            }
            sprintf(tmp, "-name %s ", token);
            strcat(files, tmp);
            count++;
            token = strtok(NULL, " ");
        }
        sprintf(result, "zip temp.tar.gz $(find ~/ -maxdepth 1 -type f %s)", files);
    }
    else if (sscanf(command, "gettargz %[^\n]", result) == 1)
    {
        char *token;
        char files[1024] = "";
        int count = 0;
        token = strtok(result, " ");
        while (token != NULL)
        {
            char tmp[256];
            if (count > 0)
            {
                strcat(files, "-o ");
            }
            sprintf(tmp, "-iname \"*.%s\" ", token);
            strcat(files, tmp);
            count++;
            token = strtok(NULL, " ");
        }
        sprintf(result, "zip temp.tar.gz $(find ~/ -maxdepth 1 -type f %s)", files);
    }
    else if (strcmp(command, "quit") == 0)
    {
        //sprintf(result, "%s", command);

    }
    else
    {
        sprintf(result, "Invalid Command");
    }
    return result;
}

int main()
{
    
    int client_socket;
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;

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

if (system("netstat -an | awk '$4 ~ /:8090$/ && $6 == \"ESTABLISHED\" && $5 !~ /:8090$/ {print $0}' | wc -l > count1.txt")< 0)
  {
    printf ("\nCommand execution error \n");
    return -1;
  }
FILE *fp1 = fopen ("count1.txt", "r");
char count_str1[10];
fgets (count_str1, 10, fp);
fclose (fp1);
int count1 = atoi (count_str1);


printf("count:%d \t count1:%d\n",count,count1);
if (count<4)
  {
    printf("in port1\n");
    server_addr.sin_port = htons (PORT1);
  }
else if(count>=4 && count1<4)
  {
    printf("in port2\n");
    server_addr.sin_port = htons (PORT2);
  }
  else if((count1+count)%2==0) {
     printf("in port1\n");
    server_addr.sin_port = htons (PORT1);
  }
  else {
     printf("in port2\n");
    server_addr.sin_port = htons (PORT2);
  }
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));
   
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        char command[1024];
        char output[1024];
        char *result;

        printf("Enter command: ");
        fgets(command, 1024, stdin);
        
        if (command[strlen(command) - 1] == '\n')
        {
            command[strlen(command) - 1] = '\0';
        }
        if(strcmp(command,"quit") == 0) {
            printf("connection closed");;
            break;
        } 
        result = validate(command);
        send(client_socket, result, strlen(result), 0);
        memset(output, 0, sizeof(output));
        recv(client_socket, output, sizeof(output), 0);
        printf("%s", output);
        memset(output, 0, sizeof(output));
        // while (recv(client_socket, output, sizeof(output), 0) > 0)
        // {
        //     printf("%s", output);
        //     memset(output, 0, sizeof(output));
        // }
    }

    close(client_socket);
    return 0;
}

