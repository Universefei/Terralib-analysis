#ifndef  __TERRALIB_INTERNAL_QTTERRAMETHODS_H
#define  __TERRALIB_INTERNAL_QTTERRAMETHODS_H
/***
 * Implementa suporte para as funcoes do TerraStat
 * Objetivo -- soh nao manter a parte de geracao de eventos,
 * etc no ui, que podera ser reaproveitado.
 ****/

#include "TeDatabase.h"
#include "TeKernelParams.h"
#include <qapplication.h>

#include "TViewDefines.h"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class TeAppTheme;
class TeQtGrid;

class TVIEW_DLL TeSkaterMethod : public QWidget {

	Q_OBJECT

protected:
  //Method params
  TeTheme*			regTheme_;    //Tema de regioes
  vector<string>	attrVector_;  //Atributos a serem utilizados
  bool				hasPop_;       //Usa populacao
  string			attrPop_;     //Atributo de populacao
  int				type_;
  int				minPop_;
  int				nClusters_;
  string			tableName_;
  string			resName_;

  string			aggrAttr_;	//aggregate attribute

public:

	//! Constructor
	TeSkaterMethod():  regTheme_(0), hasPop_(false), attrPop_(""), 
		nClusters_(0), tableName_(""), resName_(""), aggrAttr_("")
	{ }
		
	bool setTheme(string name, TeView* view) {
    regTheme_ = view->get(name);
    return (regTheme_ != NULL);
  }

  void setAttrVector(const vector<string>& vec) {
    attrVector_ = vec;
  }

  void setAggrAttr(const string& attrName) 
  {
    aggrAttr_ = attrName;
  }

  void setHasPop(bool hp, const string& pop) {
    hasPop_ = hp;
    attrPop_ = pop;
  }

  void setType(int t, int mp, int nc) {
    type_ = t; minPop_= mp; nClusters_= nc;
  }

  void setResName(string r, string t) {
    resName_ = r;
    tableName_ = t;
  }

  //Aplica o metodo skater
  bool apply(vector<double>& het);

};

#endif
