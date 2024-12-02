#include <iostream>

template< class TreeEntry >
class AVL
{ 
    public:

        AVL();
        ~AVL();
        TreeEntry search(TreeEntry x);
        TreeEntry insert(TreeEntry x);
        void status();
        void clear();
        int size();
    
    private:

        struct TreeNode; 
        typedef TreeNode *TreePointer;

        struct TreeNode
        { 
            TreeEntry entry;             
            TreePointer leftNode, rightNode; 
            int bal;
        };

        // vars
        TreePointer root;
        int nodesCount;

        // methods
        bool empty();
        bool full();
        void clear(TreePointer &t);

        TreeEntry insert(TreeEntry x, TreePointer &pA, bool &h);
        int smallestHeight(TreePointer &t); //Min height
        int nodes(TreePointer &t); //Amount of nodes
        int leaves(TreePointer &t); //Amount of leaves
        int height(TreePointer &t); //Height of the tree

}; 