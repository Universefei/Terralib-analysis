// TerraLib
#include "TePostgreSQLConnection.h"

TePostgreSQLConnection::TePostgreSQLConnection() :
TeConnection(),
pgConnection_(0)
{}

TePostgreSQLConnection::~TePostgreSQLConnection()
{}

void TePostgreSQLConnection::setPGConnection(TePGConnection* conn)
{
    pgConnection_ = conn;
}

TePGConnection* TePostgreSQLConnection::getPGConnection() const
{
    return pgConnection_;
}
