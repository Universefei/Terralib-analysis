// TerraLib
#include "TeConnectionPool.h"
#include "TeDatabase.h"
#include "TeDatabaseFactory.h"
#include "TeDatabaseFactoryParams.h"
#include "TeException.h"

TeConnectionPool::TeConnectionPool(const unsigned int& maxConnections, const unsigned int& maxIdle, const clock_t& maxWait) :
maxConnections_(maxConnections),
maxIdle_(maxIdle),
maxWait_(maxWait),
ticket_(0),
connectionsSeq_(0)
{}

TeConnectionPool::~TeConnectionPool()
{
   clear();
}

bool TeConnectionPool::initialize(const std::string& user, const std::string& password, const std::string& host, 
                                  const std::string& dbName, const std::string& dbmsName, const int& portNumber)
{

    if(!databases_.empty())
        return true;

    // Builds the database params
    params_.user_ = user;
    params_.password_ = password;
    params_.host_ = host;
    params_.database_ = dbName;
	params_.dbms_name_ = dbmsName;
	params_.port_ = portNumber;

    // Try connect to the informed database
    TeDatabase* db = TeDatabaseFactory::make(params_);
    if(db == 0 || !db->isConnected()) {
    	delete db;
    	return false;
    }

    delete db;

    // Create the set of databases
    for(unsigned int i = 1; i < maxIdle_; ++i)
    {
    	this->createNewConnection();
    }

    return true;
}

TeConnection* TeConnectionPool::getConnection()
{
    mutexLock_.lock();

    if(freeConns_.empty())
    {
        // There are not free connections!

        // Gets a tickets and...
        unsigned int myTicket = getTicket();
        // ... wait on the queue
        requestQueue_.push_back(myTicket);

        mutexLock_.unLock();
        //clock_t start = clock();
        clock_t endwait = clock () + maxWait_ * CLOCKS_PER_SEC;
        while(clock() < endwait) // Start counting...
        {
        	mutexLock_.lock();
            if(!freeConns_.empty()) // Are there free connections?
            {
            	if(myTicket == requestQueue_[0]) // Is it me?
                {
					TeConnection* c = getFreeConnection();
					freeTickets_.insert(myTicket); // Give back the ticket and...
					requestQueue_.erase(requestQueue_.begin()); // ... get off the queue!
					mutexLock_.unLock();
					return c;
                }
            } else {
				if(databases_.size()<maxConnections_) {
					this->createNewConnection();
					TeConnection* c = getFreeConnection();
					freeTickets_.insert(myTicket); // Give back the ticket and...
					for(unsigned int i = 0; i < requestQueue_.size(); ++i)
					{
						if(requestQueue_[i] == myTicket)
						{
							requestQueue_.erase(requestQueue_.begin() + i);  // Get off (angry?!) the queue...
							break;
						}
					}
					//requestQueue_.erase(requestQueue_.begin()); // ... get off the queue!
					mutexLock_.unLock();
					return c;
				}
            }

            mutexLock_.unLock();
            // waiting...
            #ifdef WIN32    
                Sleep(5);
            #else
                sleep(5);
            #endif
        }

        // Time out exceeded!
        mutexLock_.lock();
        for(unsigned int i = 0; i < requestQueue_.size(); ++i)
        {
            if(requestQueue_[i] == myTicket)
            {
                requestQueue_.erase(requestQueue_.begin() + i);  // Get off (angry?!) the queue...
                break;
            }
        }
        freeTickets_.insert(myTicket); // Give back the ticket and... go home... =/
        mutexLock_.unLock();
        
        return 0;
    }

    TeConnection* c = getFreeConnection();

    mutexLock_.unLock();
    
    return c;
}

void TeConnectionPool::releaseConnection(TeConnection* conn)
{
    mutexLock_.lock();

    unsigned int id = conn->getId();

    if(databases_.size()>maxIdle_) {
    	deleteConnection(id);
    } else {
    	freeConns_.insert(id);
    }
    delete conn;

    mutexLock_.unLock();
}

void TeConnectionPool::setTimeOut(const clock_t t)
{
	maxWait_ = t;
}

TeConnection* TeConnectionPool::getFreeConnection()
{
    unsigned int id = *freeConns_.begin();
    freeConns_.erase(freeConns_.begin());

    TeDatabase* db = databases_[id];
    TeConnection* c = db->getConnection();
    c->setId(id);
    
    return c;
}

unsigned int TeConnectionPool::getTicket()
{
    if(freeTickets_.empty()) {
    	++ticket_;
    	return ticket_;
    }

    unsigned int t = *freeTickets_.begin();
    freeTickets_.erase(freeTickets_.begin());
    return t;
}

void TeConnectionPool::clear()
{
	std::map<unsigned int, TeDatabase*>::iterator it =	databases_.begin();
	while (it != databases_.end()) {
		delete it->second;
		it++;
	}

    databases_.clear();
    freeConns_.clear();
    requestQueue_.clear();

    ticket_ = 0;
    freeTickets_.clear();
}

void TeConnectionPool::createNewConnection() {
    TeDatabase* db = TeDatabaseFactory::make(params_);

    databases_[connectionsSeq_]=db;
    freeConns_.insert(connectionsSeq_);
    connectionsSeq_++;
}

void TeConnectionPool::deleteConnection(unsigned int& id) {
	std::map<unsigned int, TeDatabase*>::iterator it =	databases_.begin();
	while (it != databases_.end()) {
		if(it->first==id) {
			delete it->second;
			databases_.erase(it);
			break;
		}
		it++;
	}

}
