/*
 * Authors: Nathnael Bereketab and Jonathan Abeje
 */
 
#include "server.h"

int chat_serv_sock_fd; //server socket

/////////////////////////////////////////////
// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE

int numReaders = 0; // keep count of the number of readers

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // mutex lock
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;  // read/write lock

/////////////////////////////////////////////

char const *server_MOTD = "Thanks for connecting to the BisonChat Server.\n\nchat>";

struct node *head = NULL;
struct room *rooms_head = NULL;  // NEW: rooms list
struct dm_connection *dm_head = NULL;  // NEW: DM connections list


// Thread safety functions
void start_read() {
    pthread_mutex_lock(&mutex);
    numReaders++;
    if(numReaders == 1) {
        pthread_mutex_lock(&rw_lock);
    }
    pthread_mutex_unlock(&mutex);
}

void end_read() {
    pthread_mutex_lock(&mutex);
    numReaders--;
    if(numReaders == 0) {
        pthread_mutex_unlock(&rw_lock);
    }
    pthread_mutex_unlock(&mutex);
}

void start_write() {
    pthread_mutex_lock(&rw_lock);
}

void end_write() {
    pthread_mutex_unlock(&rw_lock);
}


int main(int argc, char **argv) {

   signal(SIGINT, sigintHandler);
    
   //////////////////////////////////////////////////////
   // Create the default room (Lobby) for all clients
   //////////////////////////////////////////////////////
   start_write();
   rooms_head = createRoom(rooms_head, DEFAULT_ROOM);
   end_write();
   
   printf("Default room '%s' created.\n", DEFAULT_ROOM);

   // Open server socket
   chat_serv_sock_fd = get_server_socket();

   // step 3: get ready to accept connections
   if(start_server(chat_serv_sock_fd, BACKLOG) == -1) {
      printf("start server error\n");
      exit(1);
   }
   
   printf("Server Launched! Listening on PORT: %d\n", PORT);
    
   //Main execution loop
   while(1) {
      //Accept a connection, start a thread
      int new_client = accept_client(chat_serv_sock_fd);
      if(new_client != -1) {
         pthread_t new_client_thread;
         pthread_create(&new_client_thread, NULL, client_receive, (void *)&new_client);
      }
   }

   close(chat_serv_sock_fd);
}


int get_server_socket() {
    int opt = TRUE;   
    int master_socket;
    struct sockaddr_in address; 
    
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    //bind the socket to localhost port 8888  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   

   return master_socket;
}


int start_server(int serv_socket, int backlog) {
   int status = 0;
   if ((status = listen(serv_socket, backlog)) == -1) {
      printf("socket listen error\n");
   }
   return status;
}


int accept_client(int serv_sock) {
   int reply_sock_fd = -1;
   socklen_t sin_size = sizeof(struct sockaddr_storage);
   struct sockaddr_storage client_addr;

   if ((reply_sock_fd = accept(serv_sock,(struct sockaddr *)&client_addr, &sin_size)) == -1) {
      printf("socket accept error\n");
   }
   return reply_sock_fd;
}


/* Handle SIGINT (CTRL+C) */
void sigintHandler(int sig_num) {
   printf("\n\nServer shutting down gracefully...\n");

   start_write();
   
   //////////////////////////////////////////////////////////
   // Close all client sockets and free user list
   //////////////////////////////////////////////////////////
   printf("--------CLOSING ACTIVE USERS--------\n");
   struct node *current = head;
   while(current != NULL) {
      printf("Closing connection for user: %s (socket %d)\n", current->username, current->socket);
      close(current->socket);
      struct node *temp = current;
      current = current->next;
      free(temp);
   }
   
   // Free all rooms
   printf("--------FREEING ROOMS--------\n");
   struct room *currentRoom = rooms_head;
   while(currentRoom != NULL) {
      printf("Freeing room: %s\n", currentRoom->name);
      
      // Free all users in this room
      struct user_in_room *currentUser = currentRoom->users;
      while(currentUser != NULL) {
         struct user_in_room *tempUser = currentUser;
         currentUser = currentUser->next;
         free(tempUser);
      }
      
      struct room *tempRoom = currentRoom;
      currentRoom = currentRoom->next;
      free(tempRoom);
   }
   
   // Free all DM connections
   printf("--------FREEING DM CONNECTIONS--------\n");
   struct dm_connection *currentDM = dm_head;
   while(currentDM != NULL) {
      struct dm_connection *tempDM = currentDM;
      currentDM = currentDM->next;
      free(tempDM);
   }
   
   end_write();

   close(chat_serv_sock_fd);
   printf("Server shutdown complete.\n");
   exit(0);
}
