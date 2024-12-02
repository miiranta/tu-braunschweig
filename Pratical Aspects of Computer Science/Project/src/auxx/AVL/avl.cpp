#include <iostream>
#include <math.h>
#include "avl.h"

using namespace std;

// Constructor and Destructor
template< class TreeEntry >
AVL<TreeEntry>::AVL()
{  
    root = NULL;
}

template< class TreeEntry >
AVL<TreeEntry>::~AVL()
{  
    clear(root);
}

// Private methods
template< class TreeEntry >
bool AVL<TreeEntry>::empty()
{
  return (root == NULL);
}

template< class TreeEntry >
bool AVL<TreeEntry>::full()
{
  return false;
}

template< class TreeEntry >
void AVL<TreeEntry>::clear(TreePointer &t)
{
  if( t != NULL )
  { 
    clear( t->leftNode );
    clear( t->rightNode );
    delete t;

    nodesCount--;
  }
}

template< class TreeEntry >
int AVL<TreeEntry>::nodes(TreePointer &t)
{ 
  if(t == NULL)
     return 0;
  else
    return 1 + nodes(t->leftNode) + nodes(t->rightNode);
}

template< class TreeEntry >
int AVL<TreeEntry>::leaves(TreePointer &t)
{  
    if(t == NULL)
        return 0;
    else
        if(t->leftNode == NULL && t->rightNode == NULL)
            return 1;
        else
            return leaves(t->leftNode) + leaves(t->rightNode);
}

template< class TreeEntry >
int AVL<TreeEntry>::height(TreePointer &t)
{ 
    if(t == NULL)
        return -1;
    else
    {   int L,R;
        L = height(t->leftNode);
        R = height(t->rightNode);
        if(L>R) return L+1; else return R+1;
    }
}

template< class TreeEntry >
int AVL<TreeEntry>::smallestHeight(TreePointer &t)
{ 
    float base = 2, no_elements = (float) nodes(t);
    int hmin = ceil( (log(no_elements + 1) / log(base)) - 1 ); 

    return hmin;
}

// Public methods
template< class TreeEntry >
TreeEntry AVL<TreeEntry>::insert(TreeEntry x)
{ 
    bool h = false;
    nodesCount++;
    return insert(x, root, h);
}

template< class TreeEntry >
TreeEntry AVL<TreeEntry>::insert(TreeEntry x, TreePointer &pA, bool &h)
{ 
    TreePointer pB, pC;

    //Insert
    if(pA == NULL){  

        pA = new TreeNode;
        h = true;
        pA->entry = x;
        pA->leftNode = pA->rightNode = NULL;
        pA->bal = 0;

    }else if(x.key < pA->entry.key){ 

        insert(x, pA->leftNode, h);

        //LEFT got bigger
        if(h){ 
            switch (pA->bal){ 
                case -1: pA->bal = 0; h = false; break;
                case 0: pA->bal = +1; break;
                case +1: pB = pA->leftNode;
                
                //LL
                if(pB->bal == +1){
                    pA->leftNode = pB->rightNode; pB->rightNode = pA;
                    pA->bal = 0; pA = pB;
                
                //LR
                }else{ 
                    pC = pB->rightNode; pB->rightNode = pC->leftNode;
                    pC->leftNode = pB; pA->leftNode = pC->rightNode;
                    pC->rightNode = pA;
                    if(pC->bal == +1) pA->bal = -1; else pA->bal = 0;
                    if(pC->bal == -1) pB->bal = +1; else pB->bal = 0;
                    pA = pC;
                }
                pA->bal = 0; h = false;
            }
        }

    }else if(x.key > pA->entry.key){ 

        insert(x, pA->rightNode, h);

        //RIGHT got bigger
        if(h){ 
            switch (pA->bal){ 
                case +1: pA->bal = 0; h = false; break;
                case 0: pA->bal = -1; break;
                case -1: pB = pA->rightNode;
                
                //RR
                if(pB->bal == -1){
                    pA->rightNode = pB->leftNode; pB->leftNode = pA;
                    pA->bal = 0; pA = pB;

                //RL
                }else{  
                    pC = pB->leftNode; pB->leftNode = pC->rightNode;
                    pC->rightNode = pB; pA->rightNode = pC->leftNode;
                    pC->leftNode = pA;
                    if(pC->bal == -1) pA->bal = +1; else pA->bal = 0;
                    if(pC->bal == +1) pB->bal = -1; else pB->bal = 0;
                    pA = pC;
                }
                pA->bal = 0; h = false;
            }
        }

    //Found  > Update entry
    }else{ 
        pA->entry = x;
        return pA->entry;
    } 

    return x;
}

template< class TreeEntry >
TreeEntry AVL<TreeEntry>::search(TreeEntry x)
{ 
    x.notFound = true;

    TreePointer pA = root;
    while(pA != NULL)
    { 
        if(x.key < pA->entry.key) pA = pA->leftNode;
        else if(x.key > pA->entry.key) pA = pA->rightNode;

        //Found
        else if(x.key == pA->entry.key)
        {
            return pA->entry;
        }
        
    }

    return x;
}

template< class TreeEntry >
int AVL<TreeEntry>::size()
{ 
    return nodesCount;
}

template< class TreeEntry >
void AVL<TreeEntry>::status()
{ 
    cout << "-------" << endl;
    cout << "No. of nodes: " << nodes(root) << endl;
    cout << "No. of leaves: " << leaves(root) << endl;
    cout << "Height: " << height(root) << endl;
    cout << "Min Height: " << smallestHeight(root) << endl;
    cout << "Leaves: " << leaves(root) << endl;
    cout << "-------" << endl;
}

template< class TreeEntry >
void AVL<TreeEntry>::clear()
{ 
    clear(root);
    return;
}