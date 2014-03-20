#include "TeBayesFunctions.h"

// TeGlobalEmpiricalBayes				// Function declaration
bool TeGlobalEmpiricalBayes(TeSTStatInstanceSet *rs, double rate)
{
	TeSTStatInstanceSet::iterator it = rs->begin();
	
	long int Narea=0;	// number of areas
	long int count;		// Counter
	double m_hat;		// the global rate
	double n_;			// the mean population in risk
	double s2=0;		// s2 = sum(n(i)(r(i) - m_hat)^2)/n
	double sum_n=0;		//
	double sum_y=0;		//
	vector<double> r;	// the rate for each area
	vector<double> n;	// the population in each area
	vector<double> y;	// the cases in each area
	double aux;			// auxiliary variable

	try{

		while(it != rs->end())
		{
			(*it).getDoubleProperty(0,aux);
			n.push_back(aux);
			(*it).getDoubleProperty(1,aux);
			y.push_back(aux);
			++Narea;
			r.push_back(y[Narea-1]/n[Narea-1]);
			sum_n+=n[Narea-1];
			sum_y+=y[Narea-1];
			++it;
		
			// Check if the population is smaller than the 
			// number of cases
			//if(n[Narea-1]<=y[Narea-1])
			//	throw TeGlobalBayesException(BAYES_GREATER_CASES);
			if(n[Narea-1]<=0)
				throw TeGlobalBayesException(BAYES_NULL_POPULATION);
		}

		m_hat = sum_y/sum_n;
		n_	  = sum_n/Narea;

		for(count=0;count<Narea;count++)
		{
			s2+= n[count]*(r[count]-m_hat)*(r[count]-m_hat);
		}
		s2=s2/sum_n;

		it = rs->begin();
		aux = s2 - (m_hat/n_);
	
		if(aux<=0)
		{
            aux = 0;
        }

		double Theta_i, Ci, pop, cases;
		while(it != rs->end())
		{
			(*it).getDoubleProperty(0,pop);
			(*it).getDoubleProperty(1,cases);
			if ((aux == 0) && (m_hat == 0))
                Ci = 1;
            else 
			    Ci = (aux)/(aux + (m_hat/pop));
			Theta_i = Ci*(cases/pop) + (1-Ci)*m_hat;
			(*it).setDoubleProperty(2,Theta_i*rate);
			++it;
		}

	}catch(TeGlobalBayesException e)
		{
/*			switch(e.getErrorCode()){
			case BAYES_GREATER_CASES:
			break;
			case BAYES_NULL_POPULATION:
			break;
			}*/
      e.getErrorCode();
			return(false);
		}

	return(true);
}




