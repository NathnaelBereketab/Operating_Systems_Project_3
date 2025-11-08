/*
 * Authors: Nathnael Bereketab and Jonathan Abeje
 */
 
#include "server.h"

#define DEFAULT_ROOM "Lobby"

// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;
extern struct room *rooms_head;
extern struct dm_connection *dm_head;

extern char *server_MOTD;

// Thread safety functions
extern void start_read();
extern void end_read();
extern void start_write();
extern void end_write();


/*
 *Main thread for each client.  Receives all messages
 *and passes the data off to the correct function.  Receives
 *a pointer to the file descriptor for the socket the thread
 *should listen on
 */

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

void *client_receive(void *ptr) {
   int client = *(int *) ptr;  // socket
  
   int received, i;
   char buffer[MAXBUFF], sbuffer[MAXBUFF];  //data buffer of 2K  
   char tmpbuf[MAXBUFF];  //data temp buffer of 1K  
   char cmd[MAXBUFF], username[20];
   char *arguments[80];

   struct node *currentUser;
    
   send(client  , server_MOTD , strlen(server_MOTD) , 0 ); // Send Welcome Message of the Day.

   // Creating the guest user name
   sprintf(username,"guest%d", client);
   
   start_write();
   head = insertFirstU(head, client , username);
   
   // Add the GUEST to the DEFAULT ROOM (i.e. Lobby)
   addUserToRoom(rooms_head, DEFAULT_ROOM, client, username);
   end_write();
   
   printf("New user connected: %s (socket %d) - Added to Lobby\n", username, client);

   while (1) {
       
      if ((received = read(client , buffer, MAXBUFF)) != 0) {
      
            buffer[received] = '\0'; 
            strcpy(cmd, buffer);  
            strcpy(sbuffer, buffer);
         
            /////////////////////////////////////////////////////
            // we got some data from a client

            // 1. Tokenize the input in buf (split it on whitespace)

            // get the first token 
             arguments[0] = strtok(cmd, delimiters);

            // walk through other tokens 
             i = 0;
             while( arguments[i] != NULL ) {
                arguments[++i] = strtok(NULL, delimiters); 
                if(arguments[i-1] != NULL) {
                    strcpy(arguments[i-1], trimwhitespace(arguments[i-1]));
                }
             } 

             // Arg[0] = command
             // Arg[1] = user or room
             
             /////////////////////////////////////////////////////
             // 2. Execute command


            if(strcmp(arguments[0], "create") == 0)
            {
               if(arguments[1] == NULL) {
                   sprintf(buffer, "Usage: create <room>\nchat>");
                   send(client , buffer , strlen(buffer) , 0 );
                   memset(buffer, 0, sizeof(buffer));
                   continue;
               }
               
               printf("create room: %s\n", arguments[1]); 
              
               start_write();
               rooms_head = createRoom(rooms_head, arguments[1]);
               end_write();
              
               sprintf(buffer, "Room '%s' created.\nchat>", arguments[1]);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "join") == 0)
            {
               if(arguments[1] == NULL) {
                   sprintf(buffer, "Usage: join <room>\nchat>");
                   send(client , buffer , strlen(buffer) , 0 );
                   memset(buffer, 0, sizeof(buffer));
                   continue;
               }
               
               printf("join room: %s\n", arguments[1]);  

               start_read();
               struct node *user = findUserBySocket(head, client);
               struct room *room = findRoom(rooms_head, arguments[1]);
               end_read();
               
               if(room == NULL) {
                   sprintf(buffer, "Room '%s' does not exist.\nchat>", arguments[1]);
                   send(client , buffer , strlen(buffer) , 0 );
               } else {
                   start_write();
                   addUserToRoom(rooms_head, arguments[1], client, user->username);
                   end_write();
                   
                   sprintf(buffer, "Joined room '%s'.\nchat>", arguments[1]);
                   send(client , buffer , strlen(buffer) , 0 );
               }
            }
            else if (strcmp(arguments[0], "leave") == 0)
            {
               if(arguments[1] == NULL) {
                   sprintf(buffer, "Usage: leave <room>\nchat>");
                   send(client , buffer , strlen(buffer) , 0 );
                   memset(buffer, 0, sizeof(buffer));
                   continue;
               }
               
               printf("leave room: %s\n", arguments[1]); 

               start_write();
               removeUserFromRoom(rooms_head, arguments[1], client);
               end_write();

               sprintf(buffer, "Left room '%s'.\nchat>", arguments[1]);
               send(client , buffer , strlen(buffer) , 0 );
            } 
            else if (strcmp(arguments[0], "connect") == 0)
            {
               if(arguments[1] == NULL) {
                   sprintf(buffer, "Usage: connect <user>\nchat>");
                   send(client , buffer , strlen(buffer) , 0 );
                   memset(buffer, 0, sizeof(buffer));
                   continue;
               }
               
               printf("connect to user: %s \n", arguments[1]);

               start_read();
               struct node *targetUser = findU(head, arguments[1]);
               end_read();
               
               if(targetUser == NULL) {
                   sprintf(buffer, "User '%s' not found.\nchat>", arguments[1]);
                   send(client , buffer , strlen(buffer) , 0 );
               } else if(targetUser->socket == client) {
                   sprintf(buffer, "Cannot connect to yourself.\nchat>");
                   send(client , buffer , strlen(buffer) , 0 );
               } else {
                   start_write();
                   dm_head = addDM(dm_head, client, targetUser->socket);
                   end_write();
                   
                   sprintf(buffer, "Connected to user '%s'.\nchat>", arguments[1]);
                   send(client , buffer , strlen(buffer) , 0 );
               }
            }
            else if (strcmp(arguments[0], "disconnect") == 0)
            {
               if(arguments[1] == NULL) {
                   sprintf(buffer, "Usage: disconnect <user>\nchat>");
                   send(client , buffer , strlen(buffer) , 0 );
                   memset(buffer, 0, sizeof(buffer));
                   continue;
               }
               
               printf("disconnect from user: %s\n", arguments[1]);
               
               start_read();
               struct node *targetUser = findU(head, arguments[1]);
               end_read();
               
               if(targetUser == NULL) {
                   sprintf(buffer, "User '%s' not found.\nchat>", arguments[1]);
                   send(client , buffer , strlen(buffer) , 0 );
               } else {
                   start_write();
                   dm_head = removeDM(dm_head, client, targetUser->socket);
                   end_write();
                   
                   sprintf(buffer, "Disconnected from user '%s'.\nchat>", arguments[1]);
                   send(client , buffer , strlen(buffer) , 0 );
               }
            }                  
            else if (strcmp(arguments[0], "rooms") == 0)
            {
                printf("List all the rooms\n");
              
                strcpy(buffer, "Available rooms:\n");
                
                start_read();
                struct room *currentRoom = rooms_head;
                while(currentRoom != NULL) {
                    strcat(buffer, "  - ");
                    strcat(buffer, currentRoom->name);
                    strcat(buffer, "\n");
                    currentRoom = currentRoom->next;
                }
                end_read();
              
                strcat(buffer, "chat>");
                send(client , buffer , strlen(buffer) , 0 );                            
            }   
            else if (strcmp(arguments[0], "users") == 0)
            {
                printf("List all the users\n");
              
                strcpy(buffer, "Connected users:\n");
                
                start_read();
                currentUser = head;
                while(currentUser != NULL) {
                    strcat(buffer, "  - ");
                    strcat(buffer, currentUser->username);
                    strcat(buffer, "\n");
                    currentUser = currentUser->next;
                }
                end_read();
                
                strcat(buffer, "chat>");
                send(client , buffer , strlen(buffer) , 0 );
            }                           
            else if (strcmp(arguments[0], "login") == 0)
            {
                if(arguments[1] == NULL) {
                    sprintf(buffer, "Usage: login <username>\nchat>");
                    send(client , buffer , strlen(buffer) , 0 );
                    memset(buffer, 0, sizeof(buffer));
                    continue;
                }
                
                printf("User login: %s\n", arguments[1]);
                
                start_write();
                
                // Check if username already taken
                struct node *existingUser = findU(head, arguments[1]);
                if(existingUser != NULL) {
                    end_write();
                    sprintf(buffer, "Username '%s' is already taken.\nchat>", arguments[1]);
                    send(client , buffer , strlen(buffer) , 0 );
                } else {
                    // Update username in user list
                    struct node *user = findUserBySocket(head, client);
                    char oldUsername[30];
                    strcpy(oldUsername, user->username);
                    strcpy(user->username, arguments[1]);
                    
                    // Update username in all rooms
                    struct room *currentRoom = rooms_head;
                    while(currentRoom != NULL) {
                        struct user_in_room *roomUser = currentRoom->users;
                        while(roomUser != NULL) {
                            if(roomUser->socket == client) {
                                strcpy(roomUser->username, arguments[1]);
                            }
                            roomUser = roomUser->next;
                        }
                        currentRoom = currentRoom->next;
                    }
                    
                    end_write();
                    
                    sprintf(buffer, "Username changed from '%s' to '%s'.\nchat>", oldUsername, arguments[1]);
                    send(client , buffer , strlen(buffer) , 0 );
                }
            } 
            else if (strcmp(arguments[0], "help") == 0 )
            {
                sprintf(buffer, "Available commands:\n");
                strcat(buffer, "  login <username> - login with username\n");
                strcat(buffer, "  create <room> - create a room\n");
                strcat(buffer, "  join <room> - join a room\n");
                strcat(buffer, "  leave <room> - leave a room\n");
                strcat(buffer, "  users - list all users\n");
                strcat(buffer, "  rooms - list all rooms\n");
                strcat(buffer, "  connect <user> - connect to user (DM)\n");
                strcat(buffer, "  disconnect <user> - disconnect from user\n");
                strcat(buffer, "  exit - exit chat\n");
                strcat(buffer, "chat>");
                send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0)
            {
                printf("User exiting: socket %d\n", client);
                
                start_write();
                
                // Remove user from all rooms
                removeUserFromAllRooms(rooms_head, client);
                
                // Remove all DM connections
                dm_head = removeAllDMForUser(dm_head, client);
                
                // Remove from user list
                head = deleteUser(head, client);
                
                end_write();
                
                close(client);
                pthread_exit(NULL);
            }                         
            else { 
                 /////////////////////////////////////////////////////////////
                 // 3. SMART MESSAGE ROUTING - send only to room members and DM connections
                 
                 start_read();
                 
                 // Get sender's information
                 struct node *sender = findUserBySocket(head, client);
                 if(sender == NULL) {
                     end_read();
                     memset(buffer, 0, sizeof(buffer));
                     continue;
                 }
                 
                 // Format the message
                 sprintf(tmpbuf,"\n::%s> %s\nchat>", sender->username, sbuffer);
                 strcpy(sbuffer, tmpbuf);
                 
                 // Create array to track unique recipients (avoid sending duplicates)
                 int recipients[max_clients];
                 int num_recipients = 0;
                 
                 // 1. Find all rooms sender is in, add all users in those rooms
                 struct room *currentRoom = rooms_head;
                 while(currentRoom != NULL) {
                     // Check if sender is in this room
                     struct user_in_room *roomUser = currentRoom->users;
                     bool senderInRoom = false;
                     
                     while(roomUser != NULL) {
                         if(roomUser->socket == client) {
                             senderInRoom = true;
                             break;
                         }
                         roomUser = roomUser->next;
                     }
                     
                     // If sender is in this room, add all other users in this room
                     if(senderInRoom) {
                         roomUser = currentRoom->users;
                         while(roomUser != NULL) {
                             if(roomUser->socket != client) {  // Don't add self
                                 // Check if already in recipients list
                                 bool alreadyAdded = false;
                                 for(int j = 0; j < num_recipients; j++) {
                                     if(recipients[j] == roomUser->socket) {
                                         alreadyAdded = true;
                                         break;
                                     }
                                 }
                                 
                                 if(!alreadyAdded) {
                                     recipients[num_recipients++] = roomUser->socket;
                                 }
                             }
                             roomUser = roomUser->next;
                         }
                     }
                     
                     currentRoom = currentRoom->next;
                 }
                 
                 // 2. Find all DM connections involving sender
                 struct dm_connection *currentDM = dm_head;
                 while(currentDM != NULL) {
                     int otherSocket = -1;
                     
                     if(currentDM->user1_socket == client) {
                         otherSocket = currentDM->user2_socket;
                     } else if(currentDM->user2_socket == client) {
                         otherSocket = currentDM->user1_socket;
                     }
                     
                     if(otherSocket != -1) {
                         // Check if already in recipients list
                         bool alreadyAdded = false;
                         for(int j = 0; j < num_recipients; j++) {
                             if(recipients[j] == otherSocket) {
                                 alreadyAdded = true;
                                 break;
                             }
                         }
                         
                         if(!alreadyAdded) {
                             recipients[num_recipients++] = otherSocket;
                         }
                     }
                     
                     currentDM = currentDM->next;
                 }
                 
                 // 3. Send message to all unique recipients
                 for(int j = 0; j < num_recipients; j++) {
                     send(recipients[j], sbuffer, strlen(sbuffer), 0);
                 }
                 
                 end_read();
          
            }
 
         memset(buffer, 0, sizeof(buffer));
      }
   }
   return NULL;
}