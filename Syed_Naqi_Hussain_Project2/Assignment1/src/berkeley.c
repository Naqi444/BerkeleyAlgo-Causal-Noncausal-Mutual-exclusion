#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG_SIZE 1024
char sendtime[MAX_MSG_SIZE];
char recvtime[MAX_MSG_SIZE];

#define TRUE 1
#define SLEEP_DELAY 2



//Initializing the server with address and port number

struct sockaddr_in setup_server_params(const char *hostname, uint16_t port)
{
  struct sockaddr_in serverParams;
  memset(&serverParams, '\0', sizeof(struct sockaddr_in));

  serverParams.sin_family = AF_INET;
  serverParams.sin_addr.s_addr = htonl(INADDR_ANY);
  serverParams.sin_port = htons(port);

  return serverParams;
}

int socket_setup()
{
  int sockfd = -1;
  int opt = TRUE;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0)
  {
    printf("\n Error: Could not create a socket \n");
  }

  //set master socket to allow multiple connections

  if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt))<0)
  {
    printf("error at setsockopt");
  }
  return sockfd;
}

int socket_bind(int sockfd, struct sockaddr_in serv_addr)
{
  int recvValue = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

  if (recvValue < 0)
  {
    printf("Error binding the socket");
  }
  return recvValue;
}

int server_connect(int sockfd, struct sockaddr_in serv_addr, int port)
{
  memset(&serv_addr, '\0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
  {
    printf("Error connecting server to the IP address");
  }
  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
    }
  return sockfd;
}

void socket_server_start(int sockfd, int clock, int* clockref)
{
  //accept call blocks till a client is connected
  int connfd = accept(sockfd, (struct sockaddr*)NULL, NULL);
  //close(sockfd);

  //printf ("\nRequest accepted\n");

  printf ("\t: : Waiting for a New Request\n");

  int numBytes = read(connfd,recvtime, sizeof(recvtime)-1);
  if (numBytes<0)
  {
    printf("Error reading from the socket");
    //printf("Data received from the client: %s \n", recvtime);
  }
  char* token = strtok(recvtime, " ");

  if (strstr(token, "your_clock_value:"))
  {
    //printf("label for clock_value_request\n");
    token = strtok(NULL, " ");

    // data for the client
    printf("Preparing response....\n");
    memset(sendtime, '\0',sizeof(sendtime));
    sprintf(sendtime, "%d", clock);
    numBytes = write(connfd, sendtime, strlen(sendtime));
    if (numBytes < 0)
    {
      printf("\nerror sending response\n");
    }
    printf("\nMy clock value: %d\n",clock);
    printf("\nMy current clock value sent\n");
  }
  if (strstr(token, "set_clock:"))
  {
    //printf("Label for set clock:");
    token = strtok(NULL, " "); //fetch the next token in recvtime to update my clock
    int offset = atoi(token); //convert new clock value to int
    clock = clock +offset; //set new value for clock
    *clockref = clock;

    //Data for client
    printf("Preparing response...\n");
    memset(sendtime,'\0',sizeof(sendtime));
    sprintf(sendtime, "%d",clock);
    numBytes = write(connfd, sendtime, strlen(sendtime));
    if (numBytes<0)
    {
      printf("\n Error sending response");
    }
    printf("My clock value: %d \n", clock);
    printf("Response: My  clock value synchronized!\n");
  }
  //printf("My clock Value: %d\n", clock);
  memset(recvtime, '\0', sizeof(recvtime)); //after retrieving value, reset recvtime for next request
}

void set_clock(int sockfd, int value)
{
  printf("Sending new clock offset request...\n");
  memset(recvtime, '\0',sizeof(recvtime));

  printf("new clock offset: %d\n", value);
  sprintf(sendtime, "set_clock: %d", value);
  int numBytes = write(sockfd, sendtime, strlen(sendtime));
  if (numBytes < 0)
  {
    printf("Error sending request\n");
  }
  printf("Clock offset request: sent!\n");
  memset(recvtime, '\0', sizeof(recvtime));
  numBytes = read(sockfd, recvtime, sizeof(recvtime)-1);
  if (numBytes < 0)
  {
    printf("Error reading from the socket\n");
  }

  printf("Process response for clock offset: %s\t", recvtime);
  printf("sleeping... \n\n");
  sleep(SLEEP_DELAY);
  //close(sockfd);
}

char* get_clock (int sockfd, char* value)
{
  printf("Sending request...\n");
  memset(recvtime, '\0',sizeof(recvtime));

  strcpy(sendtime,"your_clock_value: 1001");

  int numBytes = write(sockfd, sendtime, strlen(sendtime));
  if(numBytes < 0)
  {
    printf("\nError sending data to socket\n");
  }
  printf ("Request: sent!\n");

  //Get response..
  memset(recvtime, '\0', sizeof(recvtime));
  numBytes = read(sockfd,recvtime,sizeof(recvtime)-1);
  if(numBytes<0)
  {
    printf("Error reading from the socket");
  }
  printf("Server response for clock value: %s\t",recvtime);
  printf("sleeping...\n\n");
  sleep(SLEEP_DELAY);
  //close(sockfd);
  return recvtime;
}

int main (int argc, char* argv[])
{
  uint32_t portList[3] = {6666,7777,8888};
  uint8_t clocks[3] = {0,0,0};
  int total = 0;

  size_t numberOfPorts = sizeof(portList)/sizeof(portList[0]);
  //printf("Number of Processes to send to: %zu\n", numberOfPorts);

  if (argc != 3)
  {
    printf("\n Usage: program port processID\n");
  }

  uint16_t port = atoi(argv[1]);

  time_t timer;

  srand((unsigned) time(&timer));

  //int clock = atoi(argv[2]);

  int clock = rand()%20; //generates a random number between 1 and  20
  //printf("\n%d",clock);
  //chec the user provided process ID, if it is number 1, then it is the requesting process
  if (atoi(argv[2])==1)
  {
    printf("\nI am the sending process\n\n");

    for (int processIndex =0; processIndex < numberOfPorts; ++processIndex)
    {
      struct sockaddr_in serverAddress = setup_server_params("127.0.0.1", portList[processIndex]);
      int sockfd = socket_setup();

      printf("Requesting clock value from Process %d with sockfd %d on port %d\n", processIndex+1, sockfd, portList[processIndex]);
      sockfd = server_connect(sockfd,serverAddress,portList[processIndex]);

      clocks[processIndex] = atoi(get_clock(sockfd, "your_clock_value"));
      total = total + clocks[processIndex];
    }
    //All clock values have been received at this point, so calculate synchronizing average
    int average = (total/numberOfPorts);

    printf("____________________\n\n");
    for (int processIndex= 0; processIndex<numberOfPorts; ++processIndex)
    {
      // Initiate server features
      struct sockaddr_in serverAddress = setup_server_params("127.0.0.1", portList[processIndex]);
      int sockfd = socket_setup();

      sockfd = server_connect(sockfd,serverAddress,portList[processIndex]);
      clock = average - clocks[processIndex];  //calculate clock offset
      printf("Sending new clock value offset to Process %d with sockfd %d on port %d\n", processIndex+1, sockfd, portList[processIndex]);
      set_clock(sockfd, clock);
    }
  }
  else
  {
    printf("I am the receiver process: \t Waiting for requests... \n\n");

    const int CONN_BACKLOG_NUM = 1; //Max length of queue pending connections for var sockfd

    //Initiate server Parameters
    struct sockaddr_in myAddr = setup_server_params("127.0.0.1",port);

    while (1)
    {
      int sockfd = socket_setup();
      //inform user of socket number used in send and receive commands
      printf("\nNew connection, socket fd is %d, ip is : %s, port: %d \n", sockfd, inet_ntoa(myAddr.sin_addr), ntohs(myAddr.sin_port));

      socket_bind(sockfd, myAddr);

      listen(sockfd, CONN_BACKLOG_NUM);

      printf("\nlistening on sockfd: %d on port: %d \n", sockfd,port);

      socket_server_start(sockfd, clock, &clock); //pass a ref to the processes clock to update later
      close(sockfd);
    }
  }
  return 0;
}
