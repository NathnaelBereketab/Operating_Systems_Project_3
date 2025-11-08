/*
 * Authors: Nathnael Bereketab and Jonathan Abeje
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// User node structure
struct node {
   char username[30];
   int socket;
   struct node *next;
};

// Room structure
struct room {
   char name[30];
   struct user_in_room *users;  // List of users in this room
   struct room *next;
};

// Users within a room
struct user_in_room {
   int socket;
   char username[30];
   struct user_in_room *next;
};

// Direct Message connections
struct dm_connection {
   int user1_socket;
   int user2_socket;
   struct dm_connection *next;
};

/////////////////// USERLIST //////////////////////////

//insert node at the first location
struct node* insertFirstU(struct node *head, int socket, char *username);

//find a node with given username
struct node* findU(struct node *head, char* username);

//find a node with given socket
struct node* findUserBySocket(struct node *head, int socket);

//delete a user from the list
struct node* deleteUser(struct node *head, int socket);


/////////////////// ROOMS //////////////////////////

// Create a new room
struct room* createRoom(struct room *rooms_head, char *roomName);

// Find a room by name
struct room* findRoom(struct room *rooms_head, char *roomName);

// Add user to a room
void addUserToRoom(struct room *rooms_head, char *roomName, int socket, char *username);

// Remove user from a specific room
void removeUserFromRoom(struct room *rooms_head, char *roomName, int socket);

// Remove user from all rooms
void removeUserFromAllRooms(struct room *rooms_head, int socket);


/////////////////// DIRECT MESSAGES //////////////////////////

// Add a DM connection between two users
struct dm_connection* addDM(struct dm_connection *dm_head, int user1_socket, int user2_socket);

// Remove a DM connection between two users
struct dm_connection* removeDM(struct dm_connection *dm_head, int user1_socket, int user2_socket);

// Check if two users have a DM connection
bool hasDM(struct dm_connection *dm_head, int user1_socket, int user2_socket);

// Remove all DM connections for a user
struct dm_connection* removeAllDMForUser(struct dm_connection *dm_head, int socket);
