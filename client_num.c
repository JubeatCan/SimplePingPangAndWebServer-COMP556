#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <math.h>

/* simple client, takes two parameters, the server domain name,
   and the server port number */

int main(int argc, char **argv)
{

  if (argc != 5)
  {
    perror("4 parameters are required:\nhostname\nport\nsize\ncount\n");
    abort();
  }
  /* our client socket */
  int sock;
  int i;

  /* variables for identifying the server */
  unsigned int server_addr;
  struct sockaddr_in sin;
  struct addrinfo *getaddrinfo_result, hints;

  /* convert server domain name to IP address */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; /* indicates we want IPv4 */

  if (getaddrinfo(argv[1], NULL, &hints, &getaddrinfo_result) == 0)
  {
    server_addr = (unsigned int)((struct sockaddr_in *)(getaddrinfo_result->ai_addr))->sin_addr.s_addr;
    freeaddrinfo(getaddrinfo_result);
  }

  /* server port number */
  unsigned short server_port = atoi(argv[2]);
  unsigned short _size = atoi(argv[3]);
  int _count = atoi(argv[4]);

  char *buffer, *sendbuffer;
  char *dummyData;
  dummyData = (char *)malloc((_size - 10) * sizeof(char));
  // int size = 500;
  // int count;
  // int num;

  /* allocate a memory buffer in the heap */
  /* putting a buffer on the stack like:

         char buffer[500];

     leaves the potential for
     buffer overflow vulnerability */
  buffer = (char *)malloc(_size);
  if (!buffer)
  {
    perror("failed to allocated buffer");
    abort();
  }

  sendbuffer = (char *)malloc(_size);
  if (!sendbuffer)
  {
    perror("failed to allocated sendbuffer");
    abort();
  }

  /* create a socket */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    perror("opening TCP socket");
    abort();
  }

  /* fill in the server's address */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = server_addr;
  sin.sin_port = htons(server_port);

  /* connect to the server */
  if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
    perror("connect to server failed");
    abort();
  }

  /* everything looks good, since we are expecting a
     message from the server in this example, let's try receiving a
     message from the socket. this call will block until some data
     has been received */
  // count = recv(sock, buffer, size, 0);
  // if (count < 0)
  //   {
  //     perror("receive failure");
  //     abort();
  //   }

  /* in this simple example, the message is a string, 
     we expect the last byte of the string to be 0, i.e. end of string */
  // if (buffer[count-1] != 0)
  //   {
  //     /* In general, TCP recv can return any number of bytes, not
  //  necessarily forming a complete message, so you need to
  //  parse the input to see if a complete message has been received.
  //        if not, more calls to recv is needed to get a complete message.
  //     */
  //     printf("Message incomplete, something is still being transmitted\n");
  //   }
  // else
  //   {
  //     printf("Here is what we got: %s", buffer);
  //   }

  for (i = 0; i < _size - 10; i++)
  {
    dummyData[i] = 'A';
  }

  struct timeval _timeval;
  struct timeval _timeval2;

  time_t send_tv_sec;
  long int send_tv_usec;
  time_t server_tv_sec;
  long int server_tv_usec;
  // time_t recv_tv_sec;
  long int recv_tv_usec;
  long thousand = 1000 * 1000;
  FILE *f;
  f = fopen("data.csv", "a");

  double result[_count];
  // Send timestamp PING
  for (i = 0; i < _count; i++)
  {

    gettimeofday(&_timeval, NULL);
    send_tv_sec = _timeval.tv_sec;
    send_tv_usec = _timeval.tv_usec;
    *(unsigned short *)sendbuffer = (unsigned short)htons(_size);
    *(time_t *)(sendbuffer + 2) = (time_t)htonl(send_tv_sec);
    *(long int *)(sendbuffer + 6) = (long int)htonl(send_tv_usec);
    strncpy(sendbuffer + 10, dummyData, _size - 10);

    //send all
    int sentSizeCount = 0;
    int tempSizeToSent = _size;
    int _sent_count = 0;
    _size = (unsigned short)htons(*(unsigned short *)(sendbuffer));
    // printf("%d\n", _size);
    while (sentSizeCount != _size)
    {
      // printf("Send size: %d\n", _size);
      _sent_count = send(sock, sendbuffer + sentSizeCount, tempSizeToSent, 0);
      if (_sent_count <= 0)
      {
        continue;
      }
      // printf("AlreadySend: %d\n", _sent_count);
      sentSizeCount += _sent_count;
      tempSizeToSent = _size - sentSizeCount;
    }

    // printf("Send Closed \n");
    // send(sock, sendbuffer, _size, 0);

    //Receive The Whole Frame
    int receivedSizeCount = 0;
    int tempSizeToReceive = _size;
    int _receive_count = 0;

    while (receivedSizeCount != _size)
    {
      _receive_count = recv(sock, buffer + receivedSizeCount, tempSizeToReceive, 0);
      if (_receive_count <= 0)
      {
        continue;
      }
      receivedSizeCount += _receive_count;
      tempSizeToReceive = _size - receivedSizeCount;
    }
    
    // _receive_count = recv(sock, buffer, _size, 0);

    // if (_receive_count < 0) {
    //   perror("Receive failure");
    //   abort();
    // } else if (_receive_count != _size) {
    //   perror("Incomplete");
    //   abort();
    // }

    server_tv_sec = (time_t)htonl(*(time_t *)(buffer + 2));
    server_tv_usec = (long int)htonl(*(long int *)(buffer + 6));

    
    gettimeofday(&_timeval2, NULL);
    recv_tv_usec = _timeval2.tv_usec;

    // printf("Server Received Timestamp Sec:  %ld\n", server_tv_sec);
    // printf("Server Received Timestamp MSec: %ld\n", server_tv_usec);
    // printf("First diff Sec: %ld\n", server_tv_sec - send_tv_sec);
    // printf("First diff MSec: %lf\n", (server_tv_usec - send_tv_usec) * 1.0 / 1000);
    // printf("2-way delay in Msec: %lf\n", (recv_tv_usec - send_tv_usec) * 1.0 / 1000);
    // printf("Bandwidth with error (Mbps): %lf\n", (_size * 2 * 8 * 1.0 / ((recv_tv_usec - send_tv_usec) * 1.0 / pow(10, 6))) / thousand);
    // printf("10Gbps delay (Msec): %lf\n", _size * 2 * 8 * 1.0 / (thousand * 10));
    // printf("----------------------------------\n");
    result[i] = (recv_tv_usec - send_tv_usec) * 1.0 / 1000;
  }
  free(buffer);
  free(sendbuffer);
  free(dummyData);
  close(sock);

  double accumulate = 0;
  for (i = 0; i < _count; i++) {
    accumulate += result[i];
  }

  fprintf(f, "%d, %lf\n", _size ,accumulate/_count);
  fclose(f);
  return 0;
}
