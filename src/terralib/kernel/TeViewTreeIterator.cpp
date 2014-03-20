/*
** ---------------------------------------------------------------
** TeViewTreeIterator.cpp - 
**
*/

/*
** ---------------------------------------------------------------
** Includes:
*/

#include "TeViewTreeIterator.h"


/*
** ---------------------------------------------------------------
** Methods implementations:
*/

void TeViewTreeIterator::next()
{
  do
  {
    StackElem& elem = TravStack.top();
    elem.ind++;

    if(elem.ind >= (int)(elem.tree->size()))
    {
      if(TravStack.size() <= 1) break; // reached the end of the list

      TravStack.pop();
      continue;
    }

    TeViewNode* node = elem.tree->retrieve(elem.ind);
//    if(node->type() == TeTHEME) break;   // reached a leaf
    if(node->type() != TeTREE) break;

    TravStack.push(StackElem((TeViewTree*)node, -1));
    break;
  }while(1);
}


void TeViewTreeRevIterator::next()
{
  do
  {
    StackElem& elem = TravStack.top();
    elem.ind--;

    if(elem.ind < 0)
    {
      if(TravStack.size() <= 1) break; // reached the end of the list

      TravStack.pop();
      continue;
    }

    TeViewNode* node = elem.tree->retrieve(elem.ind);
//    if(node->type() == TeTHEME) break;   // reached a leaf
    if(node->type() != TeTREE) break;  // reached a leaf

    TravStack.push(StackElem((TeViewTree*)node));
    break;
  }while(1);
}

/*!
  Move to the next leaf on the tree.
*/
void TeViewTreeIterator::nextLeaf()
{
  do
  {
    StackElem& elem = TravStack.top();
    elem.ind++;

    if(elem.ind >= (int)(elem.tree->size()))
    {
      if(TravStack.size() <= 1) break; // reached the end of the list

      TravStack.pop();
      continue;
    }

    TeViewNode* node = elem.tree->retrieve(elem.ind);
    //if(node->type() == TeTHEME) break;   // reached a leaf
    if(node->type() != TeTREE) break;   // reached a leaf

    TravStack.push(StackElem((TeViewTree*)node, -1));

  }while(1);
}

/*!
  Move to the next leaf on the tree.
*/
void TeViewTreeRevIterator::nextLeaf()
{
  do
  {
    StackElem& elem = TravStack.top();
    elem.ind--;

    if(elem.ind < 0)
    {
      if(TravStack.size() <= 1) break; // reached the end of the list

      TravStack.pop();
      continue;
    }

    TeViewNode* node = elem.tree->retrieve(elem.ind);
//    if(node->type() == TeTHEME) break;   // reached a leaf
    if(node->type() != TeTREE) break;   // reached a leaf

    TravStack.push(StackElem((TeViewTree*)node));

  }while(1);
}


/*!
  Move to the next node in the tree skipping the current
  node children.
*/
void TeViewTreeIterator::skipChildren()
{
  StackElem& elem = TravStack.top();
  if(elem.ind == -1) elem.ind = elem.tree->size();
  next();
}

void TeViewTreeRevIterator::skipChildren()
{
  StackElem& elem = TravStack.top();
  if(elem.ind == (int)(elem.tree->size())) elem.ind = -1;
  next();
}
