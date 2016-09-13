#define _GNU_SOURCE

#include "SortedList.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int opt_yield;

void SortedList_insert(SortedList_t* list, SortedListElement_t* element)
{
  //Points to first node after header 
  SortedListElement_t* current = list->next;
  //Points to header
  SortedListElement_t* previous = list;

  if (current == NULL) //List is empty
    {
      if (opt_yield & INSERT_YIELD) //If --yield=i option
	pthread_yield();
      list->next = element;
      element->next = NULL;
      element->prev = list;
      return;
    }

  while (current != NULL) //Iterate through  until end of list
    {
      if (strcmp(element->key, current->key) <= 0)
	break;
      previous = current;
      current = current->next;
    }

  if (opt_yield & INSERT_YIELD) //If --yield=i option
    pthread_yield();

  if (current != NULL)
    element->next = current; //Make current the new node's next pointer
  else
    element->next = NULL;
  element->prev = previous; //Make current's previous the previous of the new node
  previous->next = element; //Make node after element point to it
  if (current != NULL)
    current->prev = element; //Make node before element point to it
  
  return;
}

int SortedList_delete(SortedListElement_t* element)
{
  //Check if node is last in the list
  if ((element->prev->next == element) && (element->next == NULL))
    {
      if (opt_yield & DELETE_YIELD) //If --yield=d option
	pthread_yield();
      
      element->prev->next = NULL;
      element->prev = NULL;
      return 0; //Deleted successfully
    }

  //Check to make sure next->prev and prev->next both point to element
  if ((element->next->prev != element) || (element->prev->next != element))
    return 1;

  if (opt_yield & DELETE_YIELD) //If --yield=d option
    pthread_yield();

  element->prev->next = element->next;
  element->next->prev = element->prev;
  return 0;
}


SortedListElement_t* SortedList_lookup(SortedList_t* list, const char* key)
{
  SortedListElement_t* current = list->next;

  while (current != NULL)
    {
      if (strcmp(current->key, key) == 0)
	{
	  if (opt_yield & SEARCH_YIELD) //If --yield=s option
	    pthread_yield();
	  return current;
	}
      current = current->next;
    }

  return NULL; //Key was not found
}

int SortedList_length(SortedList_t* list)
{
  SortedListElement_t* current = list->next;

  int count = 0;

  while (current != NULL)
    {
      count++;
      if (opt_yield & SEARCH_YIELD) //If --yield=s option
	pthread_yield();
      current = current->next;
    }

  if (count >= 0) //Return count if greater than zero
    return count;
  else
    return -1; //List is corrupted
}
