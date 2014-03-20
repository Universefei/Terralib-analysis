/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright 2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/

/*! \file TeConnectionPool.h
    \brief This file contains a class that represents a pool of TerraLib Database connections.
    \author Douglas Uba <douglas@dpi.inpe.br>
    \author Mario Rocco Pettinati <mario@dpi.inpe.br>
*/

#ifndef  __TERRALIB_INTERNAL_CONNECTION_POOL_H
#define  __TERRALIB_INTERNAL_CONNECTION_POOL_H

// TerraLib
#include "TeConnection.h"
#include "TeDefines.h"
#include "TeMutex.h"
#include "TeDatabaseFactoryParams.h"
// STL
#include <ctime>
#include <set>
#include <vector>

// forward declarations
class TeDatabase;

class TL_DLL TeConnectionPool
{

public: 
	
    /** @name Constructor
     *  Initilizer methods.
     */
    //@{

    /*! \brief Constructor. 
        \param nConns The number of connections that will be controlled by the pool.
        \param timeOut Time limit to wait for a connection (in milliseconds).
    */
    TeConnectionPool(const unsigned int& maxConnections, const unsigned int& maxIdle=0, const clock_t& maxWait = 10000);

    /*! \brief Destructor. */
	~TeConnectionPool();

    //@}

    /*! \brief Intializes the pool of connections given a set of parameters. 
        \param user The database user.
        \param password The user password.
        \param host The database host.
        \param dbName The database name.
        \param dbmsName The database manager system name (i.e. PostgreSQL, Access).
        \param portNumber The database port.
    */
    bool initialize(const std::string& user, const std::string& password, const std::string& host, 
                    const std::string& dbName, const std::string& dbmsName, const int& portNumber);

    /*! \brief Gets a free connection that can be used for a TeDatabase instance.
        \note This method is thread-safe.
        \note The caller of this method will NOT take the ownership of the returned pointer.
        \note Case not exists a free connection, the caller will wait on a queue for one, respecting the time out.
        \note Case time out exceeds a NULL pointer will be returned.
        \note The connection should be repassed again to the pool after used using the method releaseConnection.
        Example: 
        {
            TeConnection* c = pool->getConnection();
            db->setConnection(c); // TeDatabase instance
            // do something...
            pool->releaseConnection(c);
        }
    */
    TeConnection* getConnection();

    
    /*! \brief Releases the given connection.
        \note This method is thread-safe.
        \note The given connection will be deleted.
    */
    void releaseConnection(TeConnection* conn);

    /** @name Access method.
    * Method to access the pool attrbiutes.
    */
    //@{

    /*! \brief Sets the time limit to wait for a connection (in milliseconds). */
    void setTimeOut(const clock_t t);

    //@}

private:

    /** @name Internal methods */
    //@{

    /*! \brief Gets a free connection that can be used for a TeDatabase instance. */
    TeConnection* getFreeConnection();

    /*! \brief Generates a number to control the get of connections. */
    unsigned int getTicket();

    /*! \brief Clear the connections and associates variables. */
    void clear();
    /*! \brief Create new connections on the database pool. */
    void createNewConnection();
	/*! \brief Delete connection on the pool with the requested id. */
    void deleteConnection(unsigned int& id);

    //@}

private:

    TeDatabaseFactoryParams params_;		 //!< Connection details to create new connections. (Set on initialization)
    unsigned int maxConnections_;            //!< The number of max connections controlled by this.
    unsigned int maxIdle_;       	         //!< The number of max connections idle to be hold by this.
    clock_t maxWait_;                        //!< Time limit to wait for a connection (in milliseconds).
    unsigned int ticket_;                    //!< A number used to control the get of connections.
    unsigned int connectionsSeq_; 	         //!< Sequence to generate connection id
    std::map<unsigned int, TeDatabase*> databases_;     //!< A map of database connections.
    std::set<unsigned int> freeConns_;       //!< Informs which connections are free.
    std::set<unsigned int> freeTickets_;     //!< Informs which tickets numbers are free.
    std::vector<unsigned int> requestQueue_; //!< A wait queue for get connections.

    TeMutex mutexLock_;                      //!< For threads control.
};

#endif // __TERRALIB_INTERNAL_CONNECTION_POOL_H
	
