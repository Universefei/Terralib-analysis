/*
TerraLib - a library for developing GIS applications.
Copyright  2001, 2002, 2003 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular
purpose. The library provided hereunder is on an "as is" basis, and the
authors have no obligation to provide maintenance, support, updates,
enhancements, or modifications.
In no event shall INPE be held liable to any party
for direct, indirect, special, incidental, or consequential damages arising
out of the use of this library and its documentation.
*/

#ifndef TEPDIALGORITHM_HPP
  #define TEPDIALGORITHM_HPP

  #include "TePDIParameters.hpp"
  #include "TePDIPIManager.hpp"
  #include "TePDIDefines.hpp"
  
  #include "../kernel/TeSharedPtr.h"
  #include "../kernel/TeProgress.h"

/**
 * @class TePDIAlgorithm
 * @brief The base class for digital image processing algorithms.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup TePDIGeneralAlgoGroup
 */
  class PDI_DLL TePDIAlgorithm
  {
    public :
      /** @typedef TeSharedPtr< TePDIAlgorithm > pointer
       * Type definition for an algorithm instance pointer. 
       */
      typedef TeSharedPtr< TePDIAlgorithm > pointer;
      
      /** @typedef const TeSharedPtr< TePDIAlgorithm > const_pointer
       * Type definition for a const algorithm instance pointer. 
       */
      typedef const TeSharedPtr< TePDIAlgorithm > const_pointer;
      
      /**
       * @brief Default Destructor
       */
      virtual ~TePDIAlgorithm();

      /**
       * @brief Applies the algorithm following the current state and
       * internal stored parameters.
       *
       * @return true if OK. false on error.
       */
      bool Apply();
      
      /**
       * @brief Applies the algorithm following the state defined by
       * the supplied parameters.
       * @param params The new supplied parameters.
       * @return true if OK. false on error.
       */
      bool Apply( const TePDIParameters& params );      

      /**
       * @brief Reset the internal state with new supplied parameters.
       *
       * @param params The new supplied parameters.
       * @return true if parameters OK, false on error.
       */
      bool Reset( const TePDIParameters& params );

      /**
       * @brief Checks if the supplied parameters fits the requirements of each
       * PDI algorithm implementation.
       *
       * @note Error log messages must be generated. No exceptions generated.
       *
       * @param parameters The parameters to be checked.
       * @return true if the parameters are OK. false if not.
       */
      virtual bool CheckParameters( 
        const TePDIParameters& parameters ) const = 0;

      /**
       * @brief Returns a reference to the current internal parameters.
       *
       * @return A reference to the current internal parameters.
       */
      const TePDIParameters& GetParameters() const;
      
      /**
       * @brief Returns a default object.
       *
       * @return A default object.
       */
      static TePDIAlgorithm* DefaultObject( const TePDIParameters& params );
      
      /**
       * @brief Enable / Disable the progress interface.
       * @param enabled Flag to enable ( true ) or disable ( false ).
       */
      void ToggleProgInt( bool enabled );        

    protected :
      /**
       * @brief Internal parameters reference
       */
      mutable TePDIParameters params_;
      
      /**
       * @brief A flag to indicate when the algorithm should initiate
       * a progress interface.
       */
      bool progress_enabled_;
       
      /**
       * @brief The internal progress interface.
       * @deprecated DEPRECATED - This object will be removed in the future 
       * (each algorithm must check the variable 
       * TePDIAlgorithm::progress_enabled_ and instantiate its own 
       * TePDIPIManager when necessary).
       */    
      TePDIPIManager pi_manager_;      
        
      /**
       * @brief Default Constructor
       */
      TePDIAlgorithm();

      /**
       * @brief Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      virtual bool RunImplementation() = 0;

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      virtual void ResetState( const TePDIParameters& params ) = 0;
      
      /**
       * @brief Checks if current internal parameters fits the requirements of each
       * PDI algorithm implementation.
       *
       * @note Error log messages must be generated. No exceptions generated.
       *
       * @return true if the internal parameters are OK. false if not.
       */
      bool CheckInternalParameters() const;
      
      /**
       * @brief Start the progress interface.
       *
       * @param message Progress window message.
       * @param steps Progress window total steps.
       * @deprecated DEPRECATED - This method will be removed in the future 
       * (each algorithm must check the variable 
       * TePDIAlgorithm::progress_enabled_ and instantiate its own 
       * TePDIPIManager when necessary).       
       */
      void StartProgInt( const std::string& message, 
        unsigned int steps );
        
      /**
       * @brief Update the current progress interface to the current step.
       *
       * @param step Current step.
       * @return true if the interface was canceled, false otherwise.
       * @deprecated DEPRECATED - This method will be removed in the future 
       * (each algorithm must check the variable 
       * TePDIAlgorithm::progress_enabled_ and instantiate its own 
       * TePDIPIManager when necessary).       
       */
      bool UpdateProgInt( unsigned int step );
      
      /**
       * @brief Increments the the current progress by one step.
       * @return true if the interface was canceled, false otherwise.
       * @deprecated DEPRECATED - This method will be removed in the future 
       * (each algorithm must check the variable 
       * TePDIAlgorithm::progress_enabled_ and instantiate its own 
       * TePDIPIManager when necessary).       
       */
      bool IncProgInt();
                
      /**
       * @brief Stop the progress interface.
       * @deprecated DEPRECATED - This method will be removed in the future 
       * (each algorithm must check the variable 
       * TePDIAlgorithm::progress_enabled_ and instantiate its own 
       * TePDIPIManager when necessary).       
       *
       */      
      void StopProgInt();
      
    private :
    
      /**
       * @brief Alternative constructor.
       *
       * @note Algorithms cannot be copied.
       */    
      TePDIAlgorithm( const TePDIAlgorithm& ) {};
    
      /**
       * @brief Operator = overload.
       *
       * @note Algorithms cannot be copied.
       *
       * @param external External algorithm reference.
       * @return A const Algorithm reference.
       */
      const TePDIAlgorithm& operator=( 
        const TePDIAlgorithm& external );
  };

#endif //TEPDIALGORITHM_HPP
