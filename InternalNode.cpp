#include <iostream>
#include <climits>
#include "InternalNode.h"

using namespace std;

InternalNode::InternalNode(int ISize, int LSize,
                           InternalNode *p, BTreeNode *left, BTreeNode *right) :
BTreeNode(LSize, p, left, right), internalSize(ISize)
{
    keys = new int[internalSize]; // keys[i] is the minimum of children[i]
    children = new BTreeNode* [ISize];
} // InternalNode::InternalNode()


BTreeNode* InternalNode::addPtr(BTreeNode *ptr, int pos)
{ // called when original was full, pos is where the node belongs.
    if(pos == internalSize)
        return ptr;
    
    BTreeNode *last = children[count - 1];
    
    for(int i = count - 2; i >= pos; i--)
    {
        children[i + 1] = children[i];
        keys[i + 1] = keys[i];
    } // shift things to right to make room for ptr, i can be -1!
    
    children[pos] = ptr;  // i will end up being the position that it is inserted
    keys[pos] = ptr->getMinimum();
    ptr->setParent(this);
    return last;
} // InternalNode:: addPtr()


void InternalNode::addToLeft(BTreeNode *last)
{
    ((InternalNode*)leftSibling)->insert(children[0]);
    
    for(int i = 0; i < count - 1; i++)
    {
        children[i] = children[i + 1];
        keys[i] = keys[i + 1];
    }
    
    children[count - 1] = last;
    keys[count - 1] = last->getMinimum();
    last->setParent(this);
    if(parent)
        parent->resetMinimum(this);
} // InternalNode::ToLeft()


void InternalNode::addToRight(BTreeNode *ptr, BTreeNode *last)
{
    ((InternalNode*) rightSibling)->insert(last);
    if(ptr == children[0] && parent)
        parent->resetMinimum(this);
} // InternalNode::addToRight()



void InternalNode::addToThis(BTreeNode *ptr, int pos)
{  // pos is where the ptr should go, guaranteed count < internalSize at start
    int i;
    
    for(i = count - 1; i >= pos; i--)
    {
        children[i + 1] = children[i];
        keys[i + 1] = keys[i];
    } // shift to the right to make room at pos
    
    children[pos] = ptr;
    keys[pos] = ptr->getMinimum();
    count++;
    ptr->setParent(this);
    
    if(pos == 0 && parent)
        parent->resetMinimum(this);
} // InternalNode::addToThis()



int InternalNode::getMaximum() const
{
    if(count > 0) // should always be the case
        return children[count - 1]->getMaximum();
    else
        return INT_MAX;
}  // getMaximum();


int InternalNode::getMinimum()const
{
    if(count > 0)   // should always be the case
        return children[0]->getMinimum();
    else
        return 0;
} // InternalNode::getMinimum()


InternalNode* InternalNode::insert(int value)
{  // count must always be >= 2 for an internal node
    int pos; // will be where value belongs
    
    for(pos = count - 1; pos > 0 && keys[pos] > value; pos--);
    
    BTreeNode *last, *ptr = children[pos]->insert(value);
    if(!ptr)  // child had room.
        return NULL;
    
    if(count < internalSize)
    {
        addToThis(ptr, pos + 1);
        return NULL;
    } // if room for value
    
    last = addPtr(ptr, pos + 1);
    
    if(leftSibling && leftSibling->getCount() < internalSize)
    {
        addToLeft(last);
        return NULL;
    }
    else // left sibling full or non-existent
        if(rightSibling && rightSibling->getCount() < internalSize)
        {
            addToRight(ptr, last);
            return NULL;
        }
        else // both siblings full or non-existent
            return split(last);
} // InternalNode::insert()


void InternalNode::insert(BTreeNode *oldRoot, BTreeNode *node2)
{ // Node must be the root, and node1
    children[0] = oldRoot;
    children[1] = node2;
    keys[0] = oldRoot->getMinimum();
    keys[1] = node2->getMinimum();
    count = 2;
    children[0]->setLeftSibling(NULL);
    children[0]->setRightSibling(children[1]);
    children[1]->setLeftSibling(children[0]);
    children[1]->setRightSibling(NULL);
    oldRoot->setParent(this);
    node2->setParent(this);
} // InternalNode::insert()


void InternalNode::insert(BTreeNode *newNode)
{ // called by sibling so either at beginning or end
    int pos;
    
    if(newNode->getMinimum() <= keys[0]) // from left sibling
        pos = 0;
    else // from right sibling
        pos = count;
    
    addToThis(newNode, pos);
} // InternalNode::insert(BTreeNode *newNode)


void InternalNode::print(Queue <BTreeNode*> &queue)
{
    int i;
    
    cout << "Internal: ";
    for (i = 0; i < count; i++)
        cout << keys[i] << ' ';
    cout << endl;
    
    for(i = 0; i < count; i++)
        queue.enqueue(children[i]);
    
} // InternalNode::print()


BTreeNode* InternalNode::remove(int value)
{  // to be written by students
    int position; // will be where value belongs
    
    for(position = count - 1; position > 0 && keys[position] > value; position--); //search and move the position, you already know this in Leafnode
    
    BTreeNode  *ptr = children[position]->remove(value);
    
    if(ptr) //if ptr is where the value be removed
    {
        delete ptr;
        count--;
        
        for( int i = position; i< count; i++) //shift to the left because ptr already deleted
        {
            children[i] = children[i + 1];
            keys[i] = keys[i + 1];   //keys[i] is the minimum of children[i]
        }
        
         if(count < (internalSize + 1) / 2 || count == 1) //this keep track of borrowing and merging which minimal code to InterNode:remove() to call LeafNode::remove() (step 2 and 3)
        { //so you have 2 cases as in LeafNode
            if(leftSibling)
            {
                if(leftSibling->getCount() > (internalSize + 1) / 2)  //if you can borrow from left sibling (its more than haft size full)
                {
                    insert(((InternalNode*)leftSibling)->removeChild(leftSibling->getCount() - 1)); //borrow the sibling at the end first and then delete it
                    
                    if(parent)
                        parent->resetMinimum(this); //control the minimum of the left sibling after each borrowing
                    
                    return NULL;
                }
                
                else // must merge with left sibling
                {
                    for(int i = 0; i < count; i++)
                        ((InternalNode*)leftSibling)->insert(children[i]); //get the first value inserted at the right sibling( the smallest one) and add to the end of the left sibling and so on
                    leftSibling->setRightSibling(rightSibling); //right sibling values are now all merge and set to left
                    
                    if(rightSibling)
                        rightSibling->setLeftSibling(leftSibling); //and if there is another sibling on the right of the one that already merged, it moves and becomes the left one of itself because it stealed the position of the old deleted sibling
                    
                    return this;
                }
            }
            
            else
                if(rightSibling)
                {
                    if(rightSibling->getCount() > (internalSize + 1) / 2) // if can borrow from right siblingz (its more than haft size full)
                    {
                        insert(((InternalNode*)rightSibling)->removeChild(0)); //borrow the sibling at the beginning first and then delete it
                        
                        if(position == 0)
                            parent->resetMinimum(this);  //control the minimum of the right sibling after each borrowing
                        return NULL;
                    }
                    
                    else // must merge with right sibling if not enough value to borrow
                    {
                        for(int i = count - 1; i >= 0; i--) //get the last value inserted at the left sibling( the largest one) and add to the beginning of the right sibling.Keeping add to the beginning-> thats why i--
                            
                            ((InternalNode*)rightSibling)->insert(children[i]);
                        
                        rightSibling->setLeftSibling(leftSibling); //left sibling values are now all merge and set to the right
                        
                        if(leftSibling)
                            leftSibling->setRightSibling(rightSibling); //and if there is another sibling on the left of the one that already merged, it moves and becomes the right one of itself because it stealed the position of the old deleted sibling
                        
                        return this; // merged out of existence sentinel
                    } // else must merge
  
                }  // if below minimum
        }
        
        if(parent && position == 0 && count > 0)
            parent->resetMinimum(this);
    }
    
    if(count == 1 && parent == NULL)  // this step notify the Btree for an internal node with only one value
        return children[0];
    
    return NULL; // no merge
}

BTreeNode* InternalNode::removeChild(int position) //if the child is a non Null value. This is how it deletes the value that we use in removeRightSibling and removeLeftSibling
{
    BTreeNode *ptr = children[position];
    count--;
    
    for(int i = position; i < count; i++)
    {
        children[i] = children[i + 1];
        keys[i] = keys[i + 1];
    }
    
    if(position == 0 && parent)
        parent->resetMinimum(this);
    
    return ptr;
}

void InternalNode::resetMinimum(const BTreeNode* child)
{
    for(int i = 0; i < count; i++)
        if(children[i] == child)
        {
            keys[i] = children[i]->getMinimum();
            if(i == 0 && parent)
                parent->resetMinimum(this);
            break;
        }
} // InternalNode::resetMinimum()


InternalNode* InternalNode::split(BTreeNode *last)
{
    InternalNode *newptr = new InternalNode(internalSize, leafSize,
                                            parent, this, rightSibling);
    
    
    if(rightSibling)
        rightSibling->setLeftSibling(newptr);
    
    rightSibling = newptr;
    
    for(int i = (internalSize + 1) / 2; i < internalSize; i++)
    {
        newptr->children[newptr->count] = children[i];
        newptr->keys[newptr->count++] = keys[i];
        children[i]->setParent(newptr);
    }
    
    newptr->children[newptr->count] = last;
    newptr->keys[newptr->count++] = last->getMinimum();
    last->setParent(newptr);
    count = (internalSize + 1) / 2;
    return newptr;
} // split()


