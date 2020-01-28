#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

#define REQUEST 1
#define RELEASE 2
#define FINISH 3

using namespace std;


int main(int argc, char *argv[])
{
  long sockfd,portno,n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  int request;
  char buffer[256];

  if (argc<2)
  {
    cout << "Enter the port number please" << endl;
    exit(0);
  }

  portno = atoi(argv[1]);

  sockfd = socket (AF_INET, SOCK_STREAM,0);

  if (sockfd<0)
    cout << "Error opening socket" <<  endl;

  server = gethostbyname("localhost");

  if (server == NULL)
  {
    cout << "Server not available" << endl;
    exit (0);
  }

  bzero((char *) &serv_addr,sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy ((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
  if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
    cout << "Error connecting" << endl;

  int j = 0;

  while (j<5)
  {
    request = REQUEST;
    n= write (sockfd,&request, sizeof(int));
    if (n<0)
    {
      cout << "Error writing to the socket" << endl;
    }
    cout << "Requesting to access the file" << endl;
    n = read(sockfd,&request, sizeof(int));
    if (n<0)
    {
      cout << "Error writing to socket" << endl;
    }
    cout << " Access granted for file" << endl;

    ifstream shared_file ("Counter.txt");

    string line;
    int cnt;

    if (shared_file.is_open())
    {
      getline (shared_file, line);
      istringstream ss(line);
      ss >> cnt;
      shared_file.close();
    }

    else cout << "Not able to open the file" <<endl;

    sleep(1);

    cout << "Initial Counter Value: " << cnt << endl;
    cnt++;

    ofstream my_shared_file("Counter.txt");
    if (my_shared_file.is_open())
    {
      my_shared_file << cnt;
      my_shared_file.close();
    }

    else cout << "Unable to open file";

    ifstream shared_file1("Counter.txt");

    if(shared_file1.is_open())
    {
      getline (shared_file1, line);
      istringstream ss1(line);
      ss1 >> cnt;
      shared_file1.close();
    }
    else cout << "Not able to open file" << endl;
    cout << "Updated Counter Value : " << cnt << endl;
    request = RELEASE;

    n = write (sockfd, &request, sizeof(int));    // Sending message to release the file
    if (n<0)
      cout << "Error writing to the socket" << endl;

    cout << "Released the file \n "<< endl;
    j++;
  }
  request = FINISH;
  n = write (sockfd, &request, sizeof(int));  // Sending request to access file
  cout << "Work done, closing the connection " << endl;
  if (n<0)
    cout << "Error writing to socket" <<endl;

    close (sockfd);
    return 0;
}
