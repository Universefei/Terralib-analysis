#include "TePDIParameters.hpp"

#include "../kernel/TeAgnostic.h"

#include "../kernel/TeUtils.h"

TePDIParameters::TePDIParameters()
{
}


TePDIParameters::TePDIParameters( const TePDIParameters& external )
: TeMultiContainer< std::string >( external )
{
}


TePDIParameters::~TePDIParameters()
{
}


void TePDIParameters::Clear()
{
  TeMultiContainer< std::string >::clear();
}


void TePDIParameters::SetdecName( const std::string& dec_name )
{
  TeMultiContainer< std::string >::store( "dec_name", dec_name );
}


std::string TePDIParameters::decName() const
{
  std::string return_string;
  
  TEAGN_TRUE_OR_THROW( TeMultiContainer< std::string >::retrive( "dec_name", 
    return_string ), "Parameters descriptor not set" );
  
  return return_string;
}


bool TePDIParameters::operator==( const TePDIParameters& ext_instance ) const
{
  return TeMultiContainer< std::string >::operator==( ext_instance );
}


bool TePDIParameters::operator!=( const TePDIParameters& ext_instance ) const
{
  return TeMultiContainer< std::string >::operator!=( ext_instance );
}


const TePDIParameters& TePDIParameters::operator=( 
  const TePDIParameters& ext_instance )
{
  TeMultiContainer< std::string >::operator=( ext_instance );
  
  return ext_instance;
}

