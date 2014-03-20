// TerraManager include files
#include "TerraManagerException.h"

TeMANAGER::TerraManagerException::TerraManagerException()
	: TeException(UNKNOWN_ERROR_TYPE)
{
}

TeMANAGER::TerraManagerException::TerraManagerException(const std::string& errorMessage, const std::string& errorCode)
	: TeException(UNKNOWN_ERROR_TYPE, errorMessage), errorCode_(errorCode)
{
}

TeMANAGER::TerraManagerException::~TerraManagerException()
{
}

const std::string& TeMANAGER::TerraManagerException::getErrorCode() const
{
	return errorCode_;
}

const std::string& TeMANAGER::TerraManagerException::getErrorMessage() const
{
	return userText_;
}

void TeMANAGER::TerraManagerException::setError(const std::string& errorMessage, const std::string& errorCode)
{
	userText_ = errorMessage;
	errorCode_ = errorCode;
}

void TeMANAGER::TerraManagerException::reset()
{
	userText_ = "";
	errorCode_ = "";
}

