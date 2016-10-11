#include <iostream>
#include <climits>
#include "LeafNode.h"
#include "InternalNode.h"
#include "QueueAr.h"

using namespace std;


LeafNode::LeafNode(int LSize, InternalNode *p,
                   BTreeNode *left, BTreeNode *right) : BTreeNode(LSize, p, left, right)
{
    values = new int[LSize];
}  // LeafNode()

void LeafNode::addToLeft(int value, int last)
{
    leftSibling->insert(values[0]);
    
    for(int i = 0; i < count - 1; i++)
        values[i] = values[i + 1];
    
    values[count - 1] = last;   //reach the maximum of the list
    if(parent)
        parent->resetMinimum(this); //find minumun of all children
} // LeafNode::ToLeft()

void LeafNode::addToRight(int value, int last)
{
    rightSibling->insert(last);
    
    if(value == values[0] && parent)
        parent->resetMinimum(this);
} // LeafNode::addToRight()

void LeafNode::addToThis(int value)
{
    int i;
    
    for(i = count - 1; i >= 0 && values[i] > value; i--)
        values[i + 1] = values[i];
    
    values[i + 1] = value;
    count++;
    
    if(value == values[0] && parent)
        parent->resetMinimum(this);
} // LeafNode::addToThis()


void LeafNode::addValue(int value, int &last)
{
    int i;
    
    if(value > values[count - 1])
        last = value;
    else
    {
        last = values[count - 1];
        
        for(i = count - 2; i >= 0 && values[i] > value; i--)
            values[i + 1] = values[i];
        // i may end up at -1
        values[i + 1] = value;
    }
} // LeafNode:: addValue()


int LeafNode::getMaximum()const
{
    if(count > 0)  // should always be the case
        return values[count - 1];
    else
        return INT_MAX;
} // getMaximum()


int LeafNode::getMinimum()const
{
    if(count > 0)  // should always be the case
        return values[0];
    else
        return 0;
    
} // LeafNode::getMinimum()


LeafNode* LeafNode::insert(int value)
{
    int last;
    
    if(count < leafSize) //not full, add element, and keep it sorted
    {
        addToThis(value);
        return NULL;
    } // if room for value
    
    addValue(value, last);
    
    if(leftSibling && leftSibling->getCount() < leafSize) //have space in left
    {
        addToLeft(value, last);
        return NULL;
    }
    else // left sibling full or non-existent
        if(rightSibling && rightSibling->getCount() < leafSize)
        {
            addToRight(value, last);
            return NULL;
        }
        else // both siblings full or non-existent
            return split(value, last);
}  // LeafNode::insert()

void LeafNode::print(Queue <BTreeNode*> &queue)
{
    cout << "Leaf: ";
    for (int i = 0; i < count; i++)
        cout << values[i] << ' ';
    cout << endl;
} // LeafNode::print()

LeafNode* LeafNode::remove(int value)
{   // To be written by students
    int position;
    
    for(position = 0; position < count && values[position] != value; position++);
    
    if(position < count)
    {
        --count;    //while the position increasing, count'll be decreasing
        
        for(int i = position; i < count; i++)
        {
            values[i] = values[i + 1]; // shift up to find the remove value
        }
        
        //node drops minimum size
        if( count < (leafSize+1)/2)
        {
            if (leftSibling) //try to borowing from left sibling.if not, merges with the left sibling

            {
                if (leftSibling-> getCount() > (leafSize+1)/2 )                 {
                    insert(leftSibling->getMaximum());  //get the maximum value in The left leaf
                    leftSibling->remove(values[0]);     //remove that value in the left leaf
                    
                    if(parent)
                        parent->resetMinimum(this);     //maintain the minimum of the leaf
                    
                    return NULL;
                } // if can borrow from left sibling
                
                else // must merge with left sibling
                {
                    for(int i = 0; i < count; i++)
                        leftSibling->insert(values[i]);
                    
                    leftSibling->setRightSibling(rightSibling);
                    
                    if(rightSibling)
                        rightSibling->setLeftSibling(leftSibling);
                    
                    return this;
                }
            } //end left sibling
            
            else
                if (rightSibling)   //try to borowing from right sibling
                {
                    if (rightSibling-> getCount() > (leafSize+1)/2 )
                    {
                        insert(rightSibling->getMinimum());      //get the minimum value from the right
                        rightSibling->remove(values[count - 1]); //delete the value
                        
                        if(position == 0)
                            parent->resetMinimum(this);          //maintain the minimum
                        return NULL;
                    }
                    
                    else // must merge with right sibling
                    {
                        for(int i = 0; i < count; i++)
                            rightSibling->insert(values[i]);
                        
                        rightSibling->setLeftSibling(leftSibling);
                        
                        if(leftSibling)
                            leftSibling->setRightSibling(rightSibling);
                        
                        return this; // merged out of existence sentinel
                    }
                } //end else if
        } //end  count
        
        if(position == 0  && parent)
            parent->resetMinimum(this);
    }  // if value found
    
    return NULL;  // unmerged
}  // LeafNode::remove()



LeafNode* LeafNode::split(int value, int last)
{
    LeafNode *ptr = new LeafNode(leafSize, parent, this, rightSibling);
    
    if(rightSibling)
        rightSibling->setLeftSibling(ptr);
    
    rightSibling = ptr;
    
    for(int i = (leafSize + 1) / 2; i < leafSize; i++)
        ptr->values[ptr->count++] = values[i];
    
    ptr->values[ptr->count++] = last;
    count = (leafSize + 1) / 2;
    
    if(value == values[0] && parent)
        parent->resetMinimum(this);
    return ptr;
} // LeafNode::split()

