//server in c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>


void read_file_to_client(int fd){
  FILE *file = fopen("index.html", "rb");
  if(!file)
    {
      perror("File error");
      return;
    }

  //size of the file calculator
  struct stat st;
  stat("index.html", &st);
  int size = st.st_size;
  
  char header[512];
  snprintf(header, sizeof(header),
	   "HTTP/1.1 200 OK\r\n"
	   "Content-Type: text/html\r\n"
	   "Content-Length: %d\r\n"
	   "Connection: close\r\n\r\n",
	   size);
  send(fd, header, strlen(header), 0);

  char buffer[1024];
  size_t bytes_read;

  //sending file to browser
  while((bytes_read = fread(buffer, 1, sizeof(buffer), file))>0){
    if (send(fd, buffer, bytes_read, 0) == -1) {
      perror("send");
      break;
    }

  }

  fclose(file);
  
}


void data_recieved(int fd)
{

  char path[1024];// this is the variable which will recieve data
  int len;
  ssize_t dataRecieved;

  memset(path, 0, sizeof(path));
	 
  len = sizeof(path);
  dataRecieved = recv(fd, path, len, 0);
  
  if(dataRecieved < 0){
    perror("Error recieving data from client");

  }

  if(strncmp(path, "GET / ", 5) == 0){
   
    read_file_to_client(fd);
  }else{
    const char *not_found = "HTTP/1.1 404 Not Found\r\n"
      "Content-Type: text/plain\r\n"
      "Connection: close\r\n\r\n"
      "File not found";
    send(fd, not_found, strlen(not_found), 0);
    return;
  }

}

void* handle_client_thread(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);  // If passed as malloc'ed pointer

    data_recieved(client_fd);
    // datasend(client_fd);

    close(client_fd);
    return NULL;
}


int main(){

  // socket connection setup
  int fd = socket(AF_INET,  SOCK_STREAM, 0);
    
  // checking the error
  if(fd < 0)
    {
      perror("The connection failed");
    }

  //binding the server
  struct sockaddr_in address;
   address.sin_family = AF_INET;
   address.sin_port =htons(8080);
   address.sin_addr.s_addr = htonl(INADDR_ANY);
  
   // resue of address
  int reuse = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    printf("SO_REUSEADDR failed: %s \n", strerror(errno));
    return 1;
  }

   
  if(bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
      perror("successfully connected");
    }

  if(listen(fd,3) < 0){
    perror("listen Failed ..");
  }

  while(1)
    {
      int accept_fd = accept(fd,NULL,NULL);

      int* pclient = malloc(sizeof(int));
      *pclient = accept_fd;

      pthread_t tid;
      pthread_create(&tid, NULL, handle_client_thread, pclient);
      pthread_detach(tid);  // auto-cleanup thread
    }
 
}

