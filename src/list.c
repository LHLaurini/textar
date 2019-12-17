#include "list.h"

#include <stdlib.h>

typedef struct Element
{
	struct Element* next;
	char data[];
} Element;

List* listCreate()
{
	Element* root = malloc(sizeof(Element));
	root->next = NULL;
	return (List*)root;
}

void* listAppend(List* list, size_t elementSize)
{
	void* element;
	void* nextElement;

	for (element = ((Element*)list)->data; (nextElement = listIterate(list, element)); element = nextElement)
	{
	}
	
	return listInsert(list, element, elementSize);
}

void* listInsert(List* list, const void* afterElement, size_t elementSize)
{
	Element** newElementPtr = &(((Element*)afterElement) - 1)->next;
	Element* nextElement = *newElementPtr;
	(*newElementPtr) = malloc(sizeof(Element) + elementSize);
	(*newElementPtr)->next = nextElement;
	return &(*newElementPtr)->data;
}

void* listIterate(List* list, const void* element)
{
	Element* next;

	if (element)
	{
		next = (((Element*)element) - 1)->next;
	}
	else
	{
		next = ((Element*)list)->next;
	}

	if (next)
	{
		return &next->data;
	}
	else
	{
		return NULL;
	}
}

void listFree(List** list)
{
	Element* element = (Element*)(*list);

	while (element)
	{
		Element* next = element->next;
		free(element);
		element = next;
	}

	*list = NULL;
}
