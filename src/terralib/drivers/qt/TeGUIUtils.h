#ifndef __GUIUTILSH
#define __GUIUTILSH

#include "../../kernel/TeDataTypes.h"
#include <qcombobox.h>

#include "../../kernel/TeDefines.h"

class TeView;
class TeTheme;
class TeDatabase;


//! Fills the comboBox (cBox) with the themes that belong to the view and have a specific geometric representation   
TL_DLL bool fillThemeCombo(TeGeomRep tg,
                TeView* view, 
                QComboBox* cBox, 
                string selName, bool clear=true);

//! Fills the comboBox (cBox) with the themes that belong to the view and have the geometric representations   
TL_DLL bool fillThemeCombo(vector<TeGeomRep>& tg,
                TeView* view, 
                QComboBox* cBox, 
                string selName, bool clear=true);


//! Fills the comboBox (cBox) with all the themes that belong to the view    
TL_DLL bool fillThemeCombo( TeView* view,
                QComboBox* cBox, 
                string selName, bool clear=true);


//! Fills the comboBox (cTableCombo) with the tables of the comboBox (cThemeCombo) current theme  
TL_DLL bool fillTableCombo(TeView* view,
                     QComboBox *cTableCombo, 
                     QComboBox *cThemeCombo,
                     string selName="", bool clear=true, TeAttrTableType tableType=TeAllAttrTypes);


//! Fills the comboBox (cColumnCombo) with the specific type columns of the comboBox (cTableCombo) current table  
TL_DLL bool fillColumnCombo(TeAttrDataType aType,
                     TeTheme* theme,
                     QComboBox *cColumnCombo, 
                     QComboBox *cTableCombo,
                     string selName, bool clear=true);

//! Fills the comboBox (cColumnCombo) with columns of the comboBox (cTableCombo) current table  
TL_DLL bool fillColumnCombo(vector<TeAttrDataType> &aTypeVec,
                      TeTheme* theme,
                      QComboBox *cColumnCombo, 
                      QComboBox *cTableCombo,
                      string selName="", bool clear=true);

//! Fills the comboBox (cColumnCombo) with the all columns of specific types of the current theme  "tableName.columnName"
TL_DLL bool fillColumnCombo(vector<TeAttrDataType> &aTypeVec,
                     TeTheme* theme,
                     QComboBox *cColumnCombo, bool clear=true);

//! Fills the comboBox (lColumnCombo) with the all layers with specific representations
TL_DLL bool fillLayerCombo(TeDatabase* db, TeGeomRep rep, QComboBox *lColumnCombo, const string& selName="", bool clear=true);

#endif
