#include <iostream>
#include "list.h"

template< class entryType >
List< entryType >::List(){
    head = NULL;
    count = 0;
}

template< class entryType >
List< entryType >::~List(){
    clear();
}

template< class entryType >
bool List< entryType >::empty(){
    return (head == NULL);
}

template< class entryType >
bool List< entryType >::full(){
    return false;
}

template< class entryType >
void List< entryType >::setPosition(int p, listPointer &current){
    int i;

    current = head;

    for(i=2; i<=p; i++){
        current = current->nextNode;
    }
}

template< class entryType >
void List< entryType >::insert(entryType in, int p){  
    listPointer newNode, current;

    p = p + 1; //I like starting at 0

    if(p<1 || p>count+1){
        return;
    }

    newNode = new listNode;
    newNode->entry = in;

    if(p==1){
        newNode->nextNode = head;
        head = newNode;
    }else{
        setPosition(p-1, current);
        newNode->nextNode = current->nextNode;
        current->nextNode = newNode;
    }

    count++;
}

template< class entryType >
void List< entryType >::remove(entryType &out, int p){  
    listPointer node, current;

    p = p + 1;

    if(p<1 || p>count){
        return;
    }

    if(p==1){
        node = head;
        head = head->nextNode;
    }else{
        setPosition(p-1, current);
        node = current->nextNode;
        current->nextNode = node->nextNode;
    }

    out = node->entry;
    delete node;

    count--;
}

template< class entryType >
void List< entryType >::replace(entryType in, int p){
    listPointer current;

    p = p + 1;

    if(p<1 || p>count){
        return;
    }

    setPosition(p, current);
    current->entry = in;
}

template< class entryType >
void List< entryType >::retrieve(entryType &out, int p){
    listPointer current;

    p = p + 1;

    if(p<1 || p>count){
        return;
    }

    setPosition(p, current);
    out = current->entry;
}

template< class entryType >
void List< entryType >::clear(){
    listPointer node;

    while(head != NULL){
        node = head;
        head = head->nextNode;
        delete node;
    }

    count = 0;
}

template< class entryType >
int List< entryType >::size(){
    return count;
}

template< class entryType >
int List< entryType >::search(entryType in){
    //linear search
    int p = 1;
    listPointer current = head;

    while(current != NULL && current->entry.key != in.key){
        current = current->nextNode;
        p++;
    }

    return (current == NULL ? -1 : p - 1);
}



