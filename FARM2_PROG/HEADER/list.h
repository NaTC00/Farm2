//
// Created by Natalia Ceccarini
//

#ifndef FARM2_LIST_H
#define FARM2_LIST_H


#include <pthread.h>

// a node in linked list
typedef struct fileNode
{
    long result;        // result
    char *filename;     // filename
    struct fileNode *next; // Pointer pointing towards next node
}FileNode;
typedef FileNode *FileNodePtr;



void insertNode(FileNodePtr *head, FileNodePtr newNodeFile);

void printList(FileNodePtr currentPtr);

FileNodePtr newNode(long data, int length, char *msg);

int isEmpty(FileNodePtr head);

void freeList(FileNodePtr *head);

#endif //FARM2_LIST_H
