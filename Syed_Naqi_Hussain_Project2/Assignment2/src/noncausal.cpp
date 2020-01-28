#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <netdb.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iterator>

char *filename;
using namespace std;



int counter=1, nop; //nop is the number of processes in the distributed system
int nconn = 1;

int vect[5];
int pid;  // This variable is used to store  the current process ID
pthread_mutex_t lock;
list<string> q; // This is a queue in order to store messages if need be

struct server
{
  int conn_pid;
  long socket_fd;
  long port;
  struct sockaddr_in new_serv_addr;
  struct hostent *new_server;
};

struct server p[10];

int NonCausalYorN(string tmpstr)
{
  int atemp[10];
  int index;

  stringstream s1(tmpstr);

  for (int i=0;i<nop;i++)
  {
    s1 >> atemp[i];
  }

  s1 >> index;

  cout << "\nReceived Sender's Index value: " << index << endl;

  //int flag=1;

  for (int i=0;i<nop;i++)
  {
    if(atemp[i]>vect[i])
    {
      vect[i]=atemp[i];
    }
  }
  return index;
}
/*int CheckQueue()
{
  int flag=0;
  for (list<string> :: iterator s = q.begin(); s !=q.end();)
  {
    int result = CausalYorN(*s);
    if(result>0)
    {
      flag=1;
      cout << "\nMessage delivered from queue for process: " <<" " << result << endl;

      vect [result-1]++;


      cout <<"\n----------------------------------------------" << endl;
      cout << "Value of updated vector clock  is:"<<endl;
      for (int i=0;i<nop;i++)
      {
        cout<<vect[i];
      }
      cout << endl;

      cout << "==============================================" << endl;

      s=q.erase(s);
      break;
    }
    else
    {
      s++;
    }
  }
  return flag;
}*/
void *SendCast(void *)
{
  char buffer[256];
  int n;
  string buffer1;
	char re[256];
	double ts, first;
	int mtp;
  int ans;
	//int n;
	char *tkn;
	double sec=.5;
  //cout << "Enter 1 to multicast:";
  //cin >> ans;
  //if (ans==1)
  //{
  //Reading the input file
	cout << "Reading the input file"<<endl;
  fstream file;
	//file.open("input" + std::to_string(myProcID) + "txt",ios::in | ios::out);
	file.open(filename,ios::in | ios::out);
	if(file.is_open())
	{//file.open("Input1.txt",ios::in | ios::out);
		while (getline(file,buffer1))
		{
			int i=0;
			first +=1;
			//int ids[256]={0};
			const char *y = buffer1.c_str();
			strcpy(re,y);
			tkn=strtok(re," ");
			if (tkn)
			{
				ts=atoi(tkn);
				cout << "Message timestamp is:"<<" "<<ts<<endl;
			}
			/*tkn=strtok(NULL," ");
			if (tkn)
			{
				mtp=atoi(tkn);
				cout<< "Group consists of following processes :" << mtp<< endl;
			}
			tkn=strtok(NULL," ");*/
			/*int x=mtp;
			int c,d;
			while(x!=0)
			{
				d=x%10;
				x=x/10;
				ids[i]=d;
				//cout << "Processes in the group are:" << ids[i]<<endl;
				c++;
			}*/

			while (first!=ts)
			{
				if (first > ts)
				{
					sleep ((first-ts)*sec);
					first = first +1;
				}
				else
				{
					sleep((ts-first)*sec);
					first=first+1;
				}
			}

			if(first==ts)
			{
				cout << "Sending multicast to every process:"<<endl;
    //while(1)
  //{
    //int ans;
    //cout << "Enter 1 for multicasting:";
    //cin >> ans;
    //if(ans==1)
    //{
    //  int z = 0;
    //  while (z<100)   //In order to realize buffering of messages a large number of multicasting mssages should be sent
    //  {
      //  z++;
        vect[pid-1]++;

        for (int i =0;i<nop;i++)
        {
          if (i==(pid-1))
          {
            continue;       //To avoid this process sending multicasting to itself
          }
          else
          {
            //for (int j=0;j<c;j++)
            //{
              //if (i+1==ids[j]);
              //{
                bzero(buffer,256);
                stringstream s;
                string tmpstr;
                s.str("");
                s<<vect[0];

                tmpstr = s.str();
                strcpy(buffer,tmpstr.c_str());
                strcat(buffer," ");
                for (int h=1; h<nop;h++)
                {
                  s.str("");
                  s<<vect[h];

                  tmpstr = s.str();
                  strcat(buffer,tmpstr.c_str());

                  strcat(buffer," ");
                }
                s.str("");
                s<<pid;     //Appending the process ID as the multicasting message has the process ID too

                tmpstr = s.str();
                strcat(buffer,tmpstr.c_str());

                n=send(p[i].socket_fd,buffer,sizeof(buffer),0);
                if(n<0)
                {
                  cout << ("Error writing to socket");
                }
              //}
            //}
            srand(time(0));
            sleep(rand()%5);
            /*cout<< "\nMessage sent to all other processes from:" << " " <<pid<< "process::"<<endl;
            cout << "\n---------------------------------------\n";
            cout << "Value of vector clock sent is:"<<endl;
            for (int i=0;i<nop-1;i++)
            {
              cout<<vect[i]<<",";
            }
            cout << vect[nop-1] << endl;
            cout << "================================================"<<endl;*/
          }
        }
        cout<< "\nMessage sent to all other processes from:" << " " <<pid<<endl;
        cout << "\n---------------------------------------\n";
        cout << "Value of vector clock sent is:"<<endl;
        for (int i=0;i<nop-1;i++)
        {
          cout<<vect[i]<<",";
        }
        cout << vect[nop-1] << endl;
        cout << "================================================"<<endl;
      }
    }
  }
  else cout << "Unable to open the input file" << endl;
}
void *RecvCast(void *sockfd)
{
  char buffer[256];
  int n;
  long socket_fd = (long)sockfd;
  list<string> q;

  while(1)
  {
    bzero(buffer,256);

    srand(time(0));
    sleep(rand()%6);

    int rx = recv(socket_fd,buffer,sizeof(buffer),0); //Receive the vector clock of all processes sending a multicast
    if (rx<0)
    {
      cout << "####Error reading from socket" << endl;
    }
    else
    {
      stringstream s;
      string tmpstr;
      s.str("");
      s<<buffer;

      tmpstr = s.str();

      int index = NonCausalYorN(tmpstr);

        cout << "Multicast message received from process:" << " " << index << endl;

        vect[index-1]++;

        cout << "\n---------------------------------------------" << endl;

        cout << "Value of updated vector clock  is:"<<endl;
        for (int i=0;i<nop-1;i++)
        {
          cout<<vect[i]<<",";
        }
        cout << vect[nop-1] << endl;

        cout << "==============================================" << endl;
      }
    }
  }


void *LookforProcs(void *sockfd)
{
  //cout << "Waiting for other processes to connect"<< endl;
  int i=0,n;
  char buffer[256];
  socklen_t clilen;
  long newsockfd[10]; // Defining an array here to save the socket file descriptors of all the other processed whihc connect to this processes
  struct sockaddr_in cli_addr;
  clilen = sizeof(cli_addr);

  struct server p1[10];

  while(counter < nop)    //counter starts from 1 and goes till N , so that all the processes connect to each other
  {
    newsockfd[i]=accept((long)sockfd, (struct sockaddr*) &cli_addr, &clilen);
    bzero(buffer,256);

    stringstream s1,s2,s3;
    s1 << p[0].conn_pid;
    string tmpstr1 = s1.str();   //This converts the process ID of this process to a string
    strcpy(buffer,tmpstr1.c_str());  //This converts the string to const char * in order to transfer it onto a socket

    n=send (newsockfd[i],buffer,strlen(buffer),0);  //Sending this process's ID to other processes
    if(n<0)
    {
      printf("ERROR writing to the socket");
    }  //Sending this process's ID to other processes connected with this one.

    bzero(buffer,256);
    recv(newsockfd[i], buffer,255,0); //This statement receives the process ID of the connected client/process

    s2 << buffer;
    string tmpstr2 = s2.str();

    p1[i].conn_pid= atoi(tmpstr2.c_str());  //Saving the client/process connected ID in integer format onto the temporary array

    cout << "Connected to Process with ID:" << p1[i].conn_pid<< ",";

    n = send(newsockfd[i],"ID received", 11,0);
    if (n<0) printf("Error writing to the socket");

    bzero (buffer,256);
    recv (newsockfd[i],buffer,255,0); //Reading the port number of the process/client connected

    s3 << buffer;
    string tmpstr3 = s3.str();

    p1[i].port = atoi(tmpstr3.c_str());

    cout << "Port no:" << p1[i].port << endl;

    n = send (newsockfd[i],"Port received",13,0);
    if(n<0) printf("Error writing to socket");

    p1[i].socket_fd = newsockfd[i]; //Saves the socket file descriptor of each of the process connected

    i++;
    counter++;
  }

  //this code transfers the details of the processes connected above into the main datastructure
  for (int j=0;j<i;j++)
  {
    p[nconn]=p1[j];
    nconn++;
  }
}

int main (int argc, char *argv[])
{
  char buffer[256];
  long sockfd, newsockfd[10], portno;
  socklen_t clilen;

  struct sockaddr_in serv_addr, cli_addr;

  pthread_t sendcast, recvcast, newconns;

  if (argc<3)
  {
    printf("ERROR, please provide the port number also\n");
    exit(1);
  }
  if (pthread_mutex_init(&  lock, NULL) !=0)
  {
    cout << "Mutex initialization has failed" << endl;
  }

  filename=argv[2];
  ifstream file("NoOfProc.txt");

  string line;

  if(file.is_open())
  {
    getline (file,line);
    istringstream s(line);
    s >> nop;     //Saving the string stream value into the N ie no of processes
    file.close();
  }
  else cout << " Unable to open the file";

  sockfd = socket (AF_INET, SOCK_STREAM, 0);  //Creating a socket for this process to communicate

  if(sockfd<0)
    cout << "Error opening socket" << endl;

  bzero ((char*) &serv_addr,sizeof (serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr= INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  int reuse = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse))<0)   //to reuse socket addresses in case of failures
  {
    cout << "Setting socket to reuse failed" << endl;
  }

  #ifdef SO_REUSEPORT
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0)
		perror("setsockopt(SO_REUSEPORT) failed");
	#endif

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
  {
    cout << "Error in binding the socket" << endl;
  }

  listen(sockfd, 10);

  cout << "Enter your process ID:";
  cin >> pid;
  p[0].conn_pid = pid;
  p[0].port = portno;
  p[0].socket_fd = sockfd;

  int x = pthread_create(&newconns, NULL, LookforProcs,(void *)sockfd);

  cout <<"Press" << "\t" << "1: Connect to a machine" << endl;
  cout << "\t"<<"2: Just wait for new connections";

  int ans,n;
  cin >> ans;
  if (ans == 1)
  {
    cout << "Enter the number of machines you want to connect to: ";
    cin >> nconn;
    for (int i=1; i<=nconn;i++)
    {
      cout << "Enter the port number of the process you want to connect to: ";
      cin >> p[i].port;

      p[i].socket_fd = socket(AF_INET, SOCK_STREAM,0);
      //cout << "creating socket for connecting to specified client" << endl;
      if(p[i].socket_fd <0)
      {
        cout << "Error opening socket";
      }
      //cout << "socket created for connecting to specified client" << endl;
      p[i].new_server = gethostbyname("localhost");

      bzero((char *) &p[i].new_serv_addr, sizeof(p[i].new_serv_addr));
      p[i].new_serv_addr.sin_family = AF_INET;
      bcopy((char *)p[i].new_server->h_addr, (char *)&p[i].new_serv_addr.sin_addr.s_addr, p[i].new_server->h_length);
      p[i].new_serv_addr.sin_port = htons(p[i].port);
      if (connect(p[i].socket_fd, (struct sockaddr *) &p[i].new_serv_addr,sizeof(p[i].new_serv_addr))<0)
      {
        cout<< " Error connecting";
      }

      bzero(buffer,256);
      recv (p[i].socket_fd, buffer,sizeof(buffer),0); //Reading the process ID of the machine to which this process connect

      stringstream s1,s2,s3;
      s1 << buffer;
      string tmpstr1 = s1.str();

      p[i].conn_pid = atoi(tmpstr1.c_str());
      cout << "Connected to process with ID:" << p[i].conn_pid << endl;

      bzero(buffer,256);

      s2 << p[0].conn_pid;
      string tmpstr2 = s2.str();
      strcpy(buffer,tmpstr2.c_str());

      n = send(p[i].socket_fd,buffer,strlen(buffer),0);//Sending this server process ID to the processes it connected
      if (n<0)
      {
        cout << "*******Error writing to the socket";
      }

      bzero(buffer,256);
      n = recv(p[i].socket_fd,buffer,255,0);  //Recieving the acknowledgement of process ID sent

      bzero(buffer, 256);
      s3 << p[0].port;
      string tmpstr3 = s3.str();
      strcpy(buffer,tmpstr3.c_str());

      n = send (p[i].socket_fd, buffer, strlen(buffer), 0); //Sending this process port Number
      if (n<0)
      {
        cout << "Error writing to the socket" << endl;
      }

      bzero(buffer,256);
      n = recv (p[i].socket_fd,buffer,255,0);

      counter++;
    }
    nconn++;
  }
  while (counter < nop)
  {
    continue;
  }
  cout << "All connections are established"<< endl;
  cout << "Printing all the connected processes and their port no" <<endl;

  /*Sorting of process accoring to ID's can go here*/
  for(int i=0; i < (nop-1); i++)						// Sorting the processes according to their ID values
	{
		for(int j =0; j < (nop-i-1); j++)
		{
			if(p[j].conn_pid > p[j+1].conn_pid)
			{
				swap(p[j], p[j+1]);
			}
		}
	}
  //Printing the process details Below
  for (int i=0; i< nop; i++)
  {
    cout << "\t" << p[i].conn_pid << "\t" << p[i].port << endl;
  }

  cout << "\n ========================================================" << endl;

  //Below code is for creating multiple threads at each process to accept multicast

  for (int i=0;i<nop;i++)
  {
    if (i==pid -1)
    {
      continue;
    }
    else
    {
      pthread_create(&recvcast,NULL,RecvCast,(void*)p[i].socket_fd);
    }
  }

  //Below code is a thread used by this process to send multicast to other processes
  pthread_create(&sendcast, NULL,SendCast,NULL);

  while(1)
  {
    continue;
  }
}
