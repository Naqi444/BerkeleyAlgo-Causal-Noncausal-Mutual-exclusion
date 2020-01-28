#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <queue>
#include <list>

using namespace std;

#define REQUEST 1
#define RELEASE 2
#define FINISH 3
#define OK 4

queue<long> q;
list<long> l;

int critical = 0;
pthread_mutex_t lock1, lock2;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

void *NewClient(void *newsockfd)
{
  int request, n;
  long *ptr = (long*)newsockfd;
  long sock = *ptr;
  if(sock<0)
    cout<<"Error Connecting to Client"<<endl;
  cout << " Connected to Client Number: " <<sock<<endl;

  while (1)
  {
    n = read(sock, &request,sizeof(int));   //Reading client's message
    if (n<0)
    {
      cout << "****Error reading the socket" << endl;
    }
    if (request==REQUEST)
    {
      cout << "Resource requested by Client <" << sock << ">" << endl;
      pthread_mutex_lock(&lock1);
      q.push(sock);
      pthread_mutex_unlock(&lock1);
    }
    else if (request==RELEASE)
    {
      pthread_mutex_lock(&lock2);
      if (critical==1)
      {
        critical = 0;
        cout << "File released by process <" << sock << ">" << endl;
        pthread_cond_signal(&cv);
      }
      pthread_mutex_unlock(&lock2);
    }
    else if (request==FINISH)
    {
      cout << "Process <" << sock << ">" << "Exiting" << endl;
      break;
    }
  }

  for(list<long> :: iterator s = l.begin(); s!= l.end();)
  {
    if (*s==sock)
      s = l.erase(s);
    else s++;
  }

  close(sock);
  pthread_exit(NULL);
}

void *GiveAccess(void *arg)     //Thread function whihc server uses to grant access  to shared file
{
  long req_grant;
  int request,n;
  cout << "Granting Access....." << endl;
  while(1)
  {
    if (l.empty())
    {
      break;
    }
    pthread_mutex_lock(&lock2);
    if (critical==1)
    {
      pthread_cond_wait(&cv, &lock2);
    }
    pthread_mutex_lock(&lock1);
    if (!q.empty())
    {
      critical = 1;     // Marking the critical section as taken
      req_grant = q.front();
      q.pop();
      request = OK;
      n = send(req_grant, &request, sizeof(int), 0);
        if (n<0)
          cout << " Error writing to the socket" << endl;
      cout << "Permission granted to process <" << req_grant << ">" << endl;
    }
    pthread_mutex_unlock(&lock1);
    pthread_mutex_unlock(&lock2);
  }
  cout << "Exiting the Access Granting process" << endl;
}

int main(int argc, char *argv[])
{
  int t=0;
  long sockfd, newsockfd[10], portno;
  socklen_t clilen;

  struct sockaddr_in serv_addr, cli_addr;

  pthread_t threads[10];   //threads for handling clint requests
  pthread_t access;

  if (argc<2)
  {
    cout << "Please provide the port number" << endl;
    exit(1);
  }

if(pthread_mutex_init(&lock1, NULL)!=0)
{
  cout << "Lock 1 mutex initialization failed" << endl;
}

if (pthread_mutex_init(&lock2, NULL) != 0)
{
  cout << "Conditional variable mutex init has failed" << endl;
}
sockfd = socket(AF_INET, SOCK_STREAM, 0);

if (sockfd <0)
{
  cout << "Error opening socket"<<endl;;
}

bzero((char *) &serv_addr, sizeof(serv_addr));
portno = atoi(argv[1]);
serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = INADDR_ANY;
serv_addr.sin_port = htons(portno);

int reuse = 1;
if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse))<0)
  cout << "setsockopt failed" << endl;

if (bind (sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
{
  cout << "Error binding the socket" << endl;
}
listen(sockfd,10);
clilen = sizeof(cli_addr);

cout << "This is the Co-ordinator!!!" << endl;
cout << "Enter the total number of processes in the system: ";
cin >> t;

int sock_index = 0;

for (int i=0; i<t; i++)
{
  newsockfd[sock_index] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  l.push_back(newsockfd[sock_index]);
  pthread_create(&threads[i], NULL, NewClient, (void *)&newsockfd[sock_index]);
  sock_index = sock_index +1;
}

pthread_create(&access, NULL, GiveAccess, NULL);

for (int i=0;i<t; i++)
{
  int rx = pthread_join(threads[i], NULL);
  if (rx)
  {
    cout << " Error in joining the thread" << endl;
    cout << "Error: " << rx << endl;
    exit(-1);
  }
}

pthread_mutex_destroy(&lock1);
pthread_mutex_destroy(&lock2);

close (sockfd);
return 0;
pthread_exit(NULL);
}
