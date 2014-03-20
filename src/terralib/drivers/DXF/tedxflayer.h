/*********************************
 * FUNCATE - GIS development team
 *
 * Tdk Framework
 *********************************/
/** @file tedxflayer.h
 *  @brief 
 *  @version 1.0
 *  @author Frederico Augusto Bedê
 *  @date 11/2009
 */
#ifndef _TEDXFLAYER_H
#define _TEDXFLAYER_H

#include "tedxfdefines.h"
#include <string>
using namespace std;
#include "TeGeometry.h"

class TePoint;
class TeLine2D;
class TePolygon;

/** @class TeDXFLayer
 *  @brief 
 */
class TEDXF_DLL TeDXFLayer 
{
  public:
    /** @brief 
         */
        TeDXFLayer(const string & layerName);

    /** @brief 
         */
        ~TeDXFLayer();

    /** @brief 
         */
        const string getName() const;

    /** @brief 
         */
        void addPoint(const TePoint & pt);

    /** @brief 
         */
        void addLine(const TeLine2D & line);

    /** @brief 
         */
        void addPolygon(const TePolygon & poly);

    //Return the point collection.
    const TePointSet getPoints() const;

    //Returns the line collection.
    const TeLineSet getLines() const;

    //Returns the polygon collection.
    const TePolygonSet getPolygons() const;

    void setVisible(const bool & value);

    const bool getVisible() const;


  protected:
    string name_; //!< 

    bool _visible; //!< Defines if the layer is visible or not.    

    TePointSet _points; //!< point collection.    

    TeLineSet _lines; //!< line collection.    

    TePolygonSet _polygons; //!< poligons collection.    


  private:
    /** @brief No copy allowed.    
         */
        TeDXFLayer(const TeDXFLayer & source);

    /** @brief No copy allowed.    
         */
        TeDXFLayer & operator=(const TeDXFLayer & source);

};
#endif
