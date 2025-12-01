//
//  Created by Natalia Ceccarini
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list.h>

// function to insert data in sorted position (ascendind order, key is result field)
void insertNode(FileNodePtr *head, FileNodePtr newNodeFile){
    FileNodePtr previousPtr = NULL;
    FileNodePtr currentPtr = *head;

    while (currentPtr != NULL && newNodeFile->result > currentPtr->result){
        previousPtr = currentPtr;
        currentPtr = currentPtr->next;
    }

    if(previousPtr == NULL){
        newNodeFile->next = *head;
        *head = newNodeFile;
    }
    else
    {
        previousPtr->next = newNodeFile;
        newNodeFile->next = currentPtr;
    }
}


// create a new Node with data and file_name fields as the args
FileNodePtr newNode(long data, int length, char *msg)
{
    FileNodePtr node = malloc(sizeof(FileNode));
    if(node != NULL){
        node->result = data;
        node->filename = malloc(sizeof(char)*length);
        strncpy(node->filename, msg, length);
        node->next = NULL;
        return node;
    }
    return NULL;
}

// function to print the linked list
void printList(FileNodePtr currentPtr)
{
    while(currentPtr != NULL)
    {
        printf("%ld %s\n", currentPtr->result, currentPtr->filename);
        currentPtr = currentPtr->next;
    }
}



int isEmpty(FileNodePtr head)
{
    return head == NULL;
}

// function to free the memory allocated for the list
void freeList(FileNodePtr *head)
{
    FileNodePtr current = *head;
    FileNodePtr temp;
    while (current != NULL)
    {
        temp = current;
        current = current->next;
        free(temp);

    }
}
