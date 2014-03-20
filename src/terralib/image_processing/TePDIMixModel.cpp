#include "TePDIMixModel.hpp"
#include "TePDIMixModelStratFactory.hpp"

TePDIMixModel::TePDIMixModel()
{
}


TePDIMixModel::~TePDIMixModel()
{
}

void TePDIMixModel::ResetState(const TePDIParameters& params)
{
	TePDIParameters dummy_params = params;
}


bool TePDIMixModel::CheckParameters(const TePDIParameters& parameters) const
{
	std::string mixmodel_type;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("mixmodel_type", mixmodel_type), "Missing parameter : mixmodel_type");

	TePDIMixModelStrategy::pointer mixmodel_strategy(TePDIMixModelStratFactory::make(mixmodel_type, parameters));
	
	TEAGN_TRUE_OR_RETURN(mixmodel_strategy.isActive(), "Invalid strategy");

	return mixmodel_strategy->CheckParameters(parameters);
}


bool TePDIMixModel::RunImplementation()
{
	std::string mixmodel_type;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("mixmodel_type", mixmodel_type), "Missing parameter : mixmodel_type");
	
	TePDIMixModelStrategy::pointer mixmodel_strategy(TePDIMixModelStratFactory::make(mixmodel_type, params_));
	
	TEAGN_TRUE_OR_RETURN(mixmodel_strategy.isActive(), "Invalid strategy");
	
	return mixmodel_strategy->Apply(params_);
}
