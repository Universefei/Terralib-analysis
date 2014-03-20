/************************************************************************************
TerraView - visualization and exploration of geographical databases
using TerraLib.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.
This file is part of TerraView. TerraView is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

You should have received a copy of the GNU General Public License
along with TerraView.
The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The software provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use of
this program and its documentation.
*************************************************************************************/

/*! \file PluginsParametersContainer.h
   \brief A container class to store multiple types os object copies.
*/


#ifndef PLUGINPARAMETERSCONTAINER_H
  #define PLUGINPARAMETERSCONTAINER_H

  #include "../../kernel/TeSharedPtr.h"
  #include "../../kernel/TeMutex.h"

#include "TeSPLDefines.h"
  
  #include <time.h>
  #include <vector>

  /**
    * @brief Node interface.
    * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
    */    
  class TLSPL_DLL PPCNodeInterface {
    public :
    
      /**
        * @brief Default Constructor.
        */        
      PPCNodeInterface() {};
      
      /**
        * @brief Default Destructor.
        */        
      virtual ~PPCNodeInterface() {};
      
      /**
        * @brief Copies the the current node instance by calling
        * the operator= from the contained object instance.
        * @return A pointer to the new node copy.
        */            
      virtual TeSharedPtr< PPCNodeInterface > clone() const = 0;
      
    private :
    
      /**
      * @brief operator= overload.
      *
      * @param ext_instance External instance reference.
      * @return The external instance reference.
      */
      const PPCNodeInterface& operator=( 
        const PPCNodeInterface& ext_instance );        
  };


  /**
    * @brief node class.
    * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
    */       
  template< typename ObjectT >
  class PPCNode : public PPCNodeInterface {
    public :
    
      /**
        * @brief Default Constructor.
        */        
      PPCNode() {};
      
      /**
        * @brief Default Destructor.
        */        
      ~PPCNode() {};
      
      /**
        * @brief Copies the the current node instance by calling
        * the operator= from the contained object instance.
        * @return A pointer to the new node copy.
        */              
      TeSharedPtr< PPCNodeInterface > clone() const
      {
        PPCNode< ObjectT >* new_node_ptr =
          new PPCNode< ObjectT >;
          
        TeSharedPtr< ObjectT > new_obj_ptr( new ObjectT );
        ( *new_obj_ptr )= ( *obj_ptr_ );
        
        new_node_ptr->setObjPtr( new_obj_ptr );
        
        TeSharedPtr< PPCNodeInterface > new_nodei_ptr(
          new_node_ptr );
        
        return new_nodei_ptr;
      };
      
      /**
        * @brief Set the internal object pointer.
        * @param ptr Object pointer.
        */        
      void setObjPtr( const TeSharedPtr< ObjectT >& ptr ) 
      { 
        obj_ptr_ = ptr;
      };
      
      /**
        * @brief Get the internal object pointer.
        * @return The object pointer.
        */        
      TeSharedPtr< ObjectT > getObjPtr() const
      { 
        return obj_ptr_; 
      };
      
    protected :
      
      /**
        * @brief The internal object pointer.
        */
      TeSharedPtr< ObjectT > obj_ptr_;

  }; 

 /**
  * @brief A container class to store multiple types os object copies.
  * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
  * @note This is a thread-safe class.
  */
  template< typename PluginsParametersContainerKeyT >
  class PluginsParametersContainer {
  
    public :
    
      /** @typedef TeSharedPtr< PluginsParametersContainer< PluginsParametersContainerKeyT > > pointer 
          Type definition for a instance pointer */
      typedef TeSharedPtr< PluginsParametersContainer< PluginsParametersContainerKeyT > > pointer;
      
      /** @typedef const TeSharedPtr< PluginsParametersContainer< PluginsParametersContainerKeyT > > const_pointer 
          Type definition for a const instance pointer */
      typedef const TeSharedPtr< PluginsParametersContainer< PluginsParametersContainerKeyT > > const_pointer;
      
      /**
       * @brief Default Constructor
       */
      PluginsParametersContainer();
      
      /**
       * @brief Alternative Constructor
       * @param external External reference.
       */
      PluginsParametersContainer( 
        const PluginsParametersContainer< PluginsParametersContainerKeyT >& external );      

      /**
       * @brief  Default Destructor
       */
      ~PluginsParametersContainer();
                           
      /**
       * @brief Operator == overload.
       *
       * @param ext_instance External instance reference.
       * @return true if this instance have the same internal
       * update time that the external instance.
       */
      bool operator==( 
        const PluginsParametersContainer< PluginsParametersContainerKeyT >& ext_instance ) const;
      
      /**
       * @brief Operator != overload.
       *
       * @param ext_instance External instance reference.
       * @return true if this instance don't have the same internal
       * update time that the external instance.
       */
      bool operator!=( 
        const PluginsParametersContainer< PluginsParametersContainerKeyT >& ext_instance ) const;
      
      /**
       * @brief operator= overload.
       *
       * @param ext_instance External instance reference.
       * @return The external instance reference.
       */
      const PluginsParametersContainer< PluginsParametersContainerKeyT >& operator=( 
        const PluginsParametersContainer< PluginsParametersContainerKeyT >& ext_instance );
      
      /**
       * @brief Clear all contents.
       *
       */
      void clear();
                         
      /**
       * @brief Store a object copy.
       *
       * @param obj_key Object key.
       * @param obj_reference Object instance.
       */                         
      template< typename ObjectT >
      void store( const PluginsParametersContainerKeyT& obj_key,
                         const ObjectT& obj_reference );
      
      /**
       * @brief Retrive a copy of a stored object.
       *
       * @param obj_key Object key.
       * @param obj_reference Object instance.
       * @return true if OK, false if the parameter was no found or error.
       */      
      template< typename ObjectT >
      bool retrive( const PluginsParametersContainerKeyT& obj_key,
        ObjectT& obj_reference ) const;
        
    protected :
        
      /** @typedef typename std::pair< PluginsParametersContainerKeyT, TeSharedPtr< PPCNodeInterface > > VectorKeyT
        Internal vector key type definition. */
      typedef typename std::pair< PluginsParametersContainerKeyT, 
        TeSharedPtr< PPCNodeInterface > > VectorKeyT;

      /** @typedef typename std::vector< VectorKeyT > VectorT
        Internal vector type definition. */
      typedef typename std::vector< VectorKeyT > VectorT;
      
      /** @brief The internal vector instance. */
      VectorT objects_vector_;    
      
      /**
       * @brief The last update time.
       */
      time_t last_up_time_;
      
      /**
       * @brief This instance locking mutex.
       */    
      mutable TeMutex this_lock_instance_;      

      /**
       * @brief Updates the internal last update time.
       *
       * @note Needed by comparison between two multi containers.
       */
      void update_time();            

};


template< typename PluginsParametersContainerKeyT >
PluginsParametersContainer< PluginsParametersContainerKeyT >::PluginsParametersContainer()
{
  last_up_time_ = 0;
}


template< typename PluginsParametersContainerKeyT >
PluginsParametersContainer< PluginsParametersContainerKeyT >::PluginsParametersContainer( 
  const PluginsParametersContainer< PluginsParametersContainerKeyT >& external )
{
  last_up_time_ = 0;
  
  operator=( external );
}


template< typename PluginsParametersContainerKeyT >
PluginsParametersContainer< PluginsParametersContainerKeyT >::~PluginsParametersContainer()
{
  clear();
}


template< typename PluginsParametersContainerKeyT >
void PluginsParametersContainer< PluginsParametersContainerKeyT >::clear()
{
  this_lock_instance_.lock();
  
  objects_vector_.clear();
  
  this_lock_instance_.unLock();
}


template< typename PluginsParametersContainerKeyT >
bool PluginsParametersContainer< PluginsParametersContainerKeyT >::operator==( 
  const PluginsParametersContainer< PluginsParametersContainerKeyT >& ext_instance ) const
{
  if( last_up_time_ == ext_instance.last_up_time_ ) {
    return true;
  } else {
    return false;
  }
}


template< typename PluginsParametersContainerKeyT >
bool PluginsParametersContainer< PluginsParametersContainerKeyT >::operator!=( 
  const PluginsParametersContainer< PluginsParametersContainerKeyT >& ext_instance ) const
{
  if( last_up_time_ == ext_instance.last_up_time_ ) {
    return false;
  } else {
    return true;
  }
}


template< typename PluginsParametersContainerKeyT >
const PluginsParametersContainer< PluginsParametersContainerKeyT >& 
PluginsParametersContainer< PluginsParametersContainerKeyT >::operator=( 
  const PluginsParametersContainer< PluginsParametersContainerKeyT >& ext_instance )
{
  if( ( &ext_instance ) != this ) {
    this_lock_instance_.lock();
  
    objects_vector_.clear();
    
    ext_instance.this_lock_instance_.lock();
  
    typename VectorT::const_iterator objects_vector_it = 
      ext_instance.objects_vector_.begin();
    typename VectorT::const_iterator objects_vector_it_end = 
      ext_instance.objects_vector_.end();
    
    VectorKeyT auxkey;
          
    while( objects_vector_it != objects_vector_it_end ) {
      auxkey.first = objects_vector_it->first;
      auxkey.second = objects_vector_it->second->clone();
      
      objects_vector_.push_back( auxkey );
          
      ++objects_vector_it;
    }
  
    last_up_time_ = ext_instance.last_up_time_;
  
    ext_instance.this_lock_instance_.unLock();
    
    this_lock_instance_.unLock();
  }

  return *this;
}


template< typename PluginsParametersContainerKeyT >
void PluginsParametersContainer< PluginsParametersContainerKeyT >::update_time()
{
  last_up_time_ = time( 0 );
}


template< typename PluginsParametersContainerKeyT >
template< typename ObjectT >
void PluginsParametersContainer< PluginsParametersContainerKeyT >::store( 
  const PluginsParametersContainerKeyT& obj_key, const ObjectT& obj_reference )
{
  this_lock_instance_.lock();
  
  typename VectorT::iterator objects_vector_it = 
    objects_vector_.begin();
  typename VectorT::iterator objects_vector_it_end = 
    objects_vector_.end();
  
  PPCNode< ObjectT >* node_ptr = 0;
  
  while( objects_vector_it != objects_vector_it_end ) {
    if( objects_vector_it->first == obj_key ) {
      node_ptr = (PPCNode< ObjectT >*)
        objects_vector_it->second.nakedPointer();
    
      break;
    }
  
    ++objects_vector_it;
  }
  
  if( node_ptr == 0 ) {
    TeSharedPtr< ObjectT > new_obj_ptr( new ObjectT );
    (*new_obj_ptr) = obj_reference;
  
    node_ptr = new PPCNode< ObjectT >;
    node_ptr->setObjPtr( new_obj_ptr );
    
    VectorKeyT auxkey;
    auxkey.first = obj_key;
    auxkey.second = 
      TeSharedPtr< PPCNodeInterface >( 
        (PPCNodeInterface*)node_ptr );
  
    objects_vector_.push_back( auxkey );
  } else {
    (*( node_ptr->getObjPtr() ) ) = obj_reference;
  }
  
  update_time();
  
  this_lock_instance_.unLock();
}


template< typename PluginsParametersContainerKeyT >
template< typename ObjectT >
bool PluginsParametersContainer< PluginsParametersContainerKeyT >::retrive(
  const PluginsParametersContainerKeyT& obj_key, ObjectT& obj_reference ) const
{
  this_lock_instance_.lock();
  
  typename VectorT::const_iterator objects_vector_it = 
    objects_vector_.begin();
  typename VectorT::const_iterator objects_vector_it_end = 
    objects_vector_.end();
  
  PPCNode< ObjectT >* node_ptr = 0;
  
  while( objects_vector_it != objects_vector_it_end ) {
    if( objects_vector_it->first == obj_key ) {
      node_ptr = (PPCNode< ObjectT >*)
        objects_vector_it->second.nakedPointer();
    
      break;
    }
  
    ++objects_vector_it;
  }
  
  if( node_ptr == 0 ) {
    TEAGN_LOGWARN( "Trying to retrive an object not stored" );
    
    this_lock_instance_.unLock();
      
    return false;
  } else {
    obj_reference = ( *( node_ptr->getObjPtr() ) );
    
    this_lock_instance_.unLock();
    
    return true;
  }
}


#endif

