#ifndef  __TERRALIB_INTERNAL_QTTERRASTAT_H
#define  __TERRALIB_INTERNAL_QTTERRASTAT_H
/***
 * Implementa suporte para as funcoes do TerraStat
 * Objetivo -- soh nao manter a parte de geracao de eventos,
 * etc no ui, que podera ser reaproveitado.
 ****/

#include "TeDatabase.h"
#include "TeKernelParams.h"
#include <qmessagebox.h>
#include <terraViewBase.h>

#include "TViewDefines.h"

#define STAT_PRECISION   12
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class TeSTElementSet;
class TeSTStatInstanceSet;

/**
 * Grava uma coluna na tabela de atributos associada a um STOSet.
 * Nome da coluna eh o mesmo nome da propriedade
 **/
TVIEW_DLL bool insertPropertyColumn(TeSTElementSet& stoSet,
                          const string& tableName,
                          const string& columnName);

TVIEW_DLL bool insertPropertyColumn(TeSTStatInstanceSet& stSet,
                          int idxDoubleProp,
                          TeTheme* theme,
                          const string& tableName,
                          const string& columnName);

/***
 * Cria um STOSet a partir de um tema e um conjunto de atributos,
 * colocando todas as tabelas do TEMA
 ***/
TVIEW_DLL bool createSTOSetFromTheme(TeTheme* theme, 
                              vector<string>& attributes,
                              TeSTElementSet* rs);


/***
 * Cria um STOSet a partir de um tema e um conjunto de atributos,
 * colocando todas as tabelas do TEMA
 ***/
TVIEW_DLL bool createSTOStatSetFromTheme(TeTheme* theme, 
                               vector<string>& attributes,
                               int additionalDouble,
                               TeSTStatInstanceSet* rs);

 
enum TeKernelErrorType {Uerror, Error, Warning};


///Retorna legenda do kernel
TVIEW_DLL bool getKernelLegend(TeKernelParams& par, vector<string>& legend);

/** Classe para aplicar o metodo de Kernel **/
class TVIEW_DLL TeKernelMethod : public QWidget {

	Q_OBJECT

protected:
    TeTheme*       evTheme1_;
    TeTheme*       evTheme2_;
    TeTheme*       regTheme_;
    TeTheme*       rasterTheme_;


    /**
     * Pointers to containers -- void since different types may apply
     * should dispose them
     **/
    void*          evSet1_;
    void*          evSet2_;
    void*          regSet_;
    int            precision_;
    TeLayer*       gridLayer_;
    bool           justOnePolygon_;
    TePolygon      boundary_;

    bool           call();
    bool           generateEventSet(int which);
    bool           generateSupportSet();
    void           error(TeKernelErrorType type, string message, 
						QWidget* parent = 0);


    //Calcula a precisao para um conjunto de valores
    template<typename SetIt> 
    bool           getPrecision(SetIt itBegin, SetIt itEnd) {
      
      
      SetIt it = itBegin;
      
      double max = -TeMAXFLOAT;
      double min = TeMAXFLOAT;
      
      double kValue;
      while (it != itEnd) {
        
        if ((*it).getDoubleProperty(0, kValue)) {
          if (kValue > max)
            max = kValue;
          if ((kValue > 0) && (kValue < min))
            min = kValue;
        }
        ++it;
      }
      
      if (min == TeMAXFLOAT)
        return false;
      if (max == -TeMAXFLOAT)
        return false;

      double diff = max - min;

      double prec = log10(diff);
      //Diferenca esta antes da virgula
      if (prec > 0) {
        precision_ = 2;
      }
      else {
        precision_ = (int)(ceil(fabs(prec)) + 3);
      }
      return true;
    }
    

public:

    TeKernelParams params_;

    /** Geracao de conjuntos **/
    bool           isPoint1_;
    bool           isPoint2_;

    bool			isGridEv_;
    bool			isGridReg_;
    int				gridCols_;
	TeDatabase*		db_;
    string			resName_; //Nome do layer ou da coluna gerada

    TeKernelMethod():
		evTheme1_(0),
		evTheme2_(0),
		regTheme_(0),
		rasterTheme_(0),
		evSet1_(0),
		evSet2_(0),
		regSet_(0),
		precision_(-1),
		isPoint1_(true),
		isPoint2_(true),
		isGridEv_(false),
		isGridReg_(false),
		gridCols_(0),
		db_(0)
		{}

	~TeKernelMethod();

    //Recupera tema e inicializa os parametros necessarios
    bool setTheme(string name, int id, TeView* view); 

    //Define nome de layer para criacao de grid
    bool setGridLayer(string name);

    //Verifica se layer eh nova
    bool isNewLayer() {
      return gridLayer_ == NULL;
    }

    //Cria layer de grid para criar uma nova
    bool createLayer(TeRaster* raster, TerraViewBase* mWindow);

    bool createColumn(TeAppTheme* currTheme, TeQtGrid* grid);

    //Aplica o metodo de kernel
    bool apply(void*& regSet, QWidget* parent);

    //Recupera precisao dos dados
    //Definida como prec(max - min) + 3
    bool getPrecision(int& prec);

    TeTheme* getTheme(int id) {
      if (id == 0)
        return (TeTheme *)regTheme_;
      if (id == 1)
        return evTheme1_;
      if (id == 2)
        return evTheme2_;
      return NULL;
    }

    bool createKernelLegends(int nColors, bool color, TeAppTheme* appTheme);

	TeTheme* regTheme()
	{	return regTheme_; }

	TeTheme* rasterTheme()
	{	return rasterTheme_; }

};

#endif
