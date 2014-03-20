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

#ifndef TEDTMALGORITHM_HPP
  #define TEDTMALGORITHM_HPP

  #include "TeDTMParameters.h"

/**
 * @brief This is the base class for DTM algorithms.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup TeDTMGeneralAlgoGroup
 */
  class DTM_DLL TeDTMAlgorithm
  {
    public :
      
      /**
       * @brief Default Destructor
       */
      virtual ~TeDTMAlgorithm();

      /**
       * @brief Applies the algorithm following the current state and
       * internal stored parameters.
       *
       * @return true if OK. false on error.
       */
      bool apply();

      /**
       * @brief Reset the internal state with new supplied parameters.
       *
       * @param params The new supplied parameters.
       * @return true if parameters OK, false on error.
       */
      bool reset( const TeDTMParameters& params );

      /**
       * @brief Checks if the supplied parameters fits the requirements of each
       * DTM algorithm implementation.
       *
       * @note Error log messages must be generated. No exceptions generated.
       *
       * @param parameters The parameters to be checked.
       * @return true if the parameters are OK. false if not.
       */
      virtual bool checkParameters( 
        const TeDTMParameters& parameters ) const = 0;

      /**
       * @brief Returns a reference to the current internal parameters.
       *
       * @return A reference to the current internal parameters.
       */
      const TeDTMParameters& getParameters() const;
      
      /**
       * @brief Enable / Disable the progress interface.
       *
       * @param enabled Flag to enable ( true ) or disable ( false ).
       */
      void toggleProgInt( bool enabled );        

    protected :
      /**
       * @brief Internal parameters reference
       */
      mutable TeDTMParameters params_;
      
      /**
       * @brief The internal progress interface enabled state.
       */
      bool progress_enabled_;
 
      /**
       * @brief Default Constructor
       */
      TeDTMAlgorithm();

      /**
       * @brief Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      virtual bool runImplementation() = 0;

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      virtual void resetState( const TeDTMParameters& params ) = 0;
      
      /**
       * @brief Checks if current internal parameters fits the requirements of each
       * DTM algorithm implementation.
       *
       * @note Error log messages must be generated. No exceptions generated.
       *
       * @return true if the internal parameters are OK. false if not.
       */
      bool checkInternalParameters() const;
                
    private :
    
      /**
       * @brief Alternative constructor.
       *
       * @note Algorithms cannot be copied.
       */    
      TeDTMAlgorithm( const TeDTMAlgorithm& ) {};
    
      /**
       * @brief Operator = overload.
       *
       * @note Algorithms cannot be copied.
       *
       * @param external External algorithm reference.
       * @return A const Algorithm reference.
       */
      const TeDTMAlgorithm& operator=( 
        const TeDTMAlgorithm& external );
  };

#endif //TEDTMALGORITHM_HPP
