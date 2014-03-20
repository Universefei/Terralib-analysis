/* $Id: TeViewTreeIterator.h 6480 2007-10-22 17:05:59Z juan $
** ---------------------------------------------------------------
** TeViewTreeIterator - 
**
*/

#ifndef TEVIEWTREEITERATOR_H
#define TEVIEWTREEITERATOR_H

/*
** ---------------------------------------------------------------
** Includes:
*/

// TerraLib includes
#include "TeException.h"
#include "TeViewNode.h"

// STL includes
#include <vector>
#include <stack>

/*
** ---------------------------------------------------------------
** Defines:
*/

/* 
  One stl module included by some TerraLib files define a macro named
  next which conflicts whith the TeViewTreeInterator::next method. So
  we choosed to undefine it.
*/
#undef next  


/*
** ---------------------------------------------------------------
** Class definition:
*/

class TL_DLL TeViewTreeIterator
{
public:
  TeViewTreeIterator(TeViewTree* root);

  void first();
  void firstLeaf();
  void next();
  void nextLeaf();
  void skipChildren();  //!< move to the next node skipping the current node childs

//  void Previous();

  bool isDone();

  TeViewNode* currentNode();
  int currentDepth();  //!< Returns the depth of the current node. The root has depth is 0.

protected:
private:
  struct StackElem{
    StackElem(TeViewTree* tr, int i){ tree = tr; ind = i; }
    TeViewTree* tree;
    int ind;           //!< index of the current child in the tree
  };

  std::stack<StackElem> TravStack; //!< traversal stack
};

class TL_DLL TeViewTreeRevIterator
{
public:
  TeViewTreeRevIterator(TeViewTree* root);

  void first();
  void firstLeaf();
  void next();
  void nextLeaf();
  void skipChildren();  //!< move to the next node skipping the current node childs

//  void Previous();

  bool isDone();

  TeViewNode* currentNode();
  int currentDepth();  //!< Returns the depth of the current node. The root has depth is 0.

protected:
private:
  struct StackElem{
    StackElem(TeViewTree* tr, int i){ tree = tr; ind = i; }
    StackElem(TeViewTree* tr){ tree = tr; ind = tr->size(); }
    TeViewTree* tree;
    int ind;           //!< index of the current child in the tree
  };

  std::stack<StackElem> TravStack; // traversal stack
};


/*
** ---------------------------------------------------------------
** Inline methods:
*/

inline TeViewTreeIterator::TeViewTreeIterator(TeViewTree* root)
{
  TravStack.push(StackElem(root, -1));
}

inline TeViewTreeRevIterator::TeViewTreeRevIterator(TeViewTree* root)
{
  TravStack.push(StackElem(root));
}

inline void TeViewTreeIterator::first()
{
  while(TravStack.size() > 1)
    TravStack.pop();
  TravStack.top().ind = -1;
}

inline void TeViewTreeRevIterator::first()
{
  while(TravStack.size() > 1)
    TravStack.pop();
  TravStack.top().ind = TravStack.top().tree->size();
}

inline void TeViewTreeIterator::firstLeaf()
{
  first();
  nextLeaf();
}

inline void TeViewTreeRevIterator::firstLeaf()
{
  first();
  nextLeaf();
}

inline bool TeViewTreeIterator::isDone()
{
  if(TravStack.size() > 1) return false;
  
  StackElem& elem = TravStack.top();
  if(elem.ind < (int)(elem.tree->size())) return false;
  
  return true;
}

inline bool TeViewTreeRevIterator::isDone()
{
  if(TravStack.size() > 1) return false;
  
  StackElem& elem = TravStack.top();
  if(elem.ind >= 0) return false;
  
  return true;
}

inline TeViewNode* TeViewTreeIterator::currentNode()
{
  StackElem& elem = TravStack.top();
  if(elem.ind == -1) return elem.tree;

  return elem.tree->retrieve(elem.ind);
}

inline TeViewNode* TeViewTreeRevIterator::currentNode()
{
  StackElem& elem = TravStack.top();
  if(elem.ind == (int)(elem.tree->size())) return elem.tree;

  return elem.tree->retrieve(elem.ind);  
}

inline int TeViewTreeIterator::currentDepth()
{
  return TravStack.size() - 1;
}

inline int TeViewTreeRevIterator::currentDepth()
{
  return TravStack.size() - 1;
}


#ifdef te__next_back__
#define next te__next_back__
#undef te__next_back__  // some stl module defines a macro named next that is generating conflics with 
#endif
#endif
