#include "TePDIMixModelFactory.hpp"
#include "TePDIMixModel.hpp"
#include "../kernel/TeAgnostic.h"


TePDIMixModelFactory::TePDIMixModelFactory():TePDIAlgorithmFactory(std::string("TePDIMixModel"))
{
};

TePDIMixModelFactory::~TePDIMixModelFactory()
{
};

TePDIAlgorithm* TePDIMixModelFactory::build (const TePDIParameters& arg)
{
	TePDIAlgorithm* instance_ptr = new TePDIMixModel();
	
	TEAGN_TRUE_OR_THROW(instance_ptr->Reset(arg), "Invalid parameters");
	
	return instance_ptr;
}
