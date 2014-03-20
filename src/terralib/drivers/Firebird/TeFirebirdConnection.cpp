// TerraLib
#include "TeFirebirdConnection.h"

TeFirebirdConnection::TeFirebirdConnection() :
TeConnection(),
fbConnection_(0)
{}

TeFirebirdConnection::~TeFirebirdConnection()
{}

void TeFirebirdConnection::setFBConnection(IBPP::Database* conn)
{
    fbConnection_ = conn;
}

IBPP::Database* TeFirebirdConnection::getFBConnection() const
{
    return fbConnection_;
}
