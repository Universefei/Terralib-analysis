/** \file TerraManagerException.h
  *	\brief This file contains an exception class to be used by TerraManager.
  * \author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
  */

#ifndef  __TERRAMANAGER_INTERNAL_TERRAMANAGEREXCEPTION_H
#define  __TERRAMANAGER_INTERNAL_TERRAMANAGEREXCEPTION_H

// TerraManager include files
#include "TerraManagerConfig.h"

// TerraLib include files
#include <TeException.h>

// STL include files
#include <string>

namespace TeMANAGER
{

	/** \class TerraManagerException
	  *	\brief This class should be used to handle errors in TerraManager.
	  *
	  * \sa TeException
	  */
	class TERRAMANAGEREXPORT TerraManagerException : public TeException
	{
		public:

			/** @name Constructor and Destructor Methods
			* Methods related to instantiation and destruction.
			*/
			//@{

			/** \brief Empty constructor.
			*/
			TerraManagerException();

			/** \brief Constructor with parameters.
				\param errorMessage Error message associated to this exception. (Input)
				\param errorCode Error code. (Input)
				*/
			TerraManagerException(const std::string& errorMessage, const std::string& errorCode);

			/** \brief Virtual destructor.
			*/
			virtual ~TerraManagerException();

			//@}

			/** @name Accessor methods
			* Methods used to set/get error properties.
			*/
			//@{

			/** \brief Returns errorCode value.
			*/
			const std::string& getErrorCode() const;

			/** \brief Returns error message value.
			*/
			const std::string& getErrorMessage() const;

			/** \brief Sets full error info, increasing the error count too.
				\param errorMessage Error message associated to this exception. (Input)
				\param errorCode Error code. (Input)
			*/
			void setError(const std::string& errorMessage, const std::string& errorCode = "");

			/** \brief Resets the object, cleaning all errors.
			*/
			void reset();

			//@}		

		protected:

			std::string errorCode_;		//!< Associated error code.
	};

}	// end namespace TeMANAGER

#endif	// __TERRAMANAGER_INTERNAL_TERRAMANAGEREXCEPTION_H


