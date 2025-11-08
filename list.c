/*
 * Authors: Nathnael Bereketab and Jonathan Abeje
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

/////////////////// USERLIST FUNCTIONS //////////////////////////

//insert link at the first location
struct node* insertFirstU(struct node *head, int socket, char *username) {
    
   if(findU(head,username) == NULL) {
           
       //create a link
       struct node *link = (struct node*) malloc(sizeof(struct node));

       link->socket = socket;
       strcpy(link->username,username);
       
       //point it to old first node
       link->next = head;

       //point first to new first node
       head = link;
 
   }
   else
       printf("Duplicate: %s\n", username);
   return head;
}

//find a link with given user
struct node* findU(struct node *head, char* username) {

   //start from the first link
   struct node* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
    while(strcmp(current->username, username) != 0) {
	
      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
	
   //if username found, return the current Link
   return current;
}

//find a user with given socket
struct node* findUserBySocket(struct node *head, int socket) {
   struct node* current = head;

   if(head == NULL) {
      return NULL;
   }

   while(current != NULL) {
      if(current->socket == socket) {
         return current;
      }
      current = current->next;
   }
   
   return NULL;
}

//delete a user from the list
struct node* deleteUser(struct node *head, int socket) {
   struct node *current = head;
   struct node *previous = NULL;
   
   if(head == NULL) {
      return NULL;
   }
   
   // If head node is the one to delete
   if(current->socket == socket) {
      head = current->next;
      free(current);
      return head;
   }
   
   // Search for the node to delete
   while(current != NULL && current->socket != socket) {
      previous = current;
      current = current->next;
   }
   
   // If found, delete it
   if(current != NULL) {
      previous->next = current->next;
      free(current);
   }
   
   return head;
}


/////////////////// ROOM FUNCTIONS //////////////////////////

// Create a new room
struct room* createRoom(struct room *rooms_head, char *roomName) {
   // Check if room already exists
   if(findRoom(rooms_head, roomName) != NULL) {
      printf("Room '%s' already exists\n", roomName);
      return rooms_head;
   }
   
   // Create new room
   struct room *newRoom = (struct room*) malloc(sizeof(struct room));
   strcpy(newRoom->name, roomName);
   newRoom->users = NULL;
   newRoom->next = rooms_head;
   
   return newRoom;
}

// Find a room by name
struct room* findRoom(struct room *rooms_head, char *roomName) {
   struct room *current = rooms_head;
   
   while(current != NULL) {
      if(strcmp(current->name, roomName) == 0) {
         return current;
      }
      current = current->next;
   }
   
   return NULL;
}

// Add user to a room
void addUserToRoom(struct room *rooms_head, char *roomName, int socket, char *username) {
   struct room *room = findRoom(rooms_head, roomName);
   
   if(room == NULL) {
      printf("Room '%s' not found\n", roomName);
      return;
   }
   
   // Check if user already in room
   struct user_in_room *current = room->users;
   while(current != NULL) {
      if(current->socket == socket) {
         printf("User already in room\n");
         return;
      }
      current = current->next;
   }
   
   // Add user to room
   struct user_in_room *newUser = (struct user_in_room*) malloc(sizeof(struct user_in_room));
   newUser->socket = socket;
   strcpy(newUser->username, username);
   newUser->next = room->users;
   room->users = newUser;
}

// Remove user from a specific room
void removeUserFromRoom(struct room *rooms_head, char *roomName, int socket) {
   struct room *room = findRoom(rooms_head, roomName);
   
   if(room == NULL) {
      return;
   }
   
   struct user_in_room *current = room->users;
   struct user_in_room *previous = NULL;
   
   while(current != NULL) {
      if(current->socket == socket) {
         if(previous == NULL) {
            room->users = current->next;
         } else {
            previous->next = current->next;
         }
         free(current);
         return;
      }
      previous = current;
      current = current->next;
   }
}

// Remove user from all rooms
void removeUserFromAllRooms(struct room *rooms_head, int socket) {
   struct room *currentRoom = rooms_head;
   
   while(currentRoom != NULL) {
      removeUserFromRoom(rooms_head, currentRoom->name, socket);
      currentRoom = currentRoom->next;
   }
}


/////////////////// DIRECT MESSAGE FUNCTIONS //////////////////////////

// Add a DM connection between two users
struct dm_connection* addDM(struct dm_connection *dm_head, int user1_socket, int user2_socket) {
   // Check if connection already exists
   if(hasDM(dm_head, user1_socket, user2_socket)) {
      printf("DM connection already exists\n");
      return dm_head;
   }
   
   // Create new DM connection
   struct dm_connection *newDM = (struct dm_connection*) malloc(sizeof(struct dm_connection));
   newDM->user1_socket = user1_socket;
   newDM->user2_socket = user2_socket;
   newDM->next = dm_head;
   
   return newDM;
}

// Remove a DM connection between two users
struct dm_connection* removeDM(struct dm_connection *dm_head, int user1_socket, int user2_socket) {
   struct dm_connection *current = dm_head;
   struct dm_connection *previous = NULL;
   
   while(current != NULL) {
      if((current->user1_socket == user1_socket && current->user2_socket == user2_socket) ||
         (current->user1_socket == user2_socket && current->user2_socket == user1_socket)) {
         
         if(previous == NULL) {
            dm_head = current->next;
         } else {
            previous->next = current->next;
         }
         free(current);
         return dm_head;
      }
      previous = current;
      current = current->next;
   }
   
   return dm_head;
}

// Check if two users have a DM connection
bool hasDM(struct dm_connection *dm_head, int user1_socket, int user2_socket) {
   struct dm_connection *current = dm_head;
   
   while(current != NULL) {
      if((current->user1_socket == user1_socket && current->user2_socket == user2_socket) ||
         (current->user1_socket == user2_socket && current->user2_socket == user1_socket)) {
         return true;
      }
      current = current->next;
   }
   
   return false;
}

// Remove all DM connections for a user
struct dm_connection* removeAllDMForUser(struct dm_connection *dm_head, int socket) {
   struct dm_connection *current = dm_head;
   struct dm_connection *previous = NULL;
   
   while(current != NULL) {
      if(current->user1_socket == socket || current->user2_socket == socket) {
         struct dm_connection *temp = current;
         
         if(previous == NULL) {
            dm_head = current->next;
            current = dm_head;
         } else {
            previous->next = current->next;
            current = current->next;
         }
         free(temp);
      } else {
         previous = current;
         current = current->next;
      }
   }
   
   return dm_head;
}
