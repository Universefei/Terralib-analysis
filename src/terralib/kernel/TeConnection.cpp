// TerraLib
#include "TeConnection.h"

TeConnection::TeConnection() :
id_(0),
host_(""),
user_(""),
password_(""),
database_(""),
dbmsName_(""),
portNumber_(-1)
{}

TeConnection::~TeConnection()
{}	

unsigned int TeConnection::getId() const
{
    return id_;
}

void TeConnection::setId(const unsigned int& id)
{
    id_ = id;
}

std::string	TeConnection::getUser() const
{
    return user_;
}

void TeConnection::setUser(const std::string& user)
{
    user_ = user;
}

std::string TeConnection::getHost() const
{
    return host_;
}

void TeConnection::setHost(const std::string& host)
{
    host_ = host;
}

std::string TeConnection::getPassword() const
{
    return password_;
}

void TeConnection::setPassword(const std::string& psw)
{
    password_ = psw;
}

std::string	TeConnection::getDBMS() const
{
    return dbmsName_;
}

void TeConnection::setDBMS(const std::string& name)
{
    dbmsName_ = name;
}

std::string TeConnection::getDatabaseName() const
{
    return database_;
}

void TeConnection::setDatabaseName(const std::string& name)
{
    database_ = name;
}

int	TeConnection::getPortNumber() const
{
    return portNumber_;
}

void TeConnection::setPortNumber(const int& portNumber)
{
    portNumber_ = portNumber;
}
