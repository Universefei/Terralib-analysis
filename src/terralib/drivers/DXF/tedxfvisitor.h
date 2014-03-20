/*********************************
 * FUNCATE - GIS development team
 *
 * Tdk Framework
 *********************************/
/** @file tedxfvisitor.h
 *  @brief 
 *  @version 1.0
 *  @author Frederico Augusto Bedê
 *  @date 11/2009
 */
#ifndef _TEDXFVISITOR_H
#define _TEDXFVISITOR_H

#include "tedxfdefines.h"
#ifdef __APPLE__
	#include "../../../dxflib/dl_creationadapter.h"
#else
	#include "dl_creationadapter.h"
#endif 

#include <string>
#include <vector>
#include "TeGeometry.h"

struct DL_LayerData;
struct DL_PointData;
struct DL_LineData;
struct DL_PolylineData;
struct DL_VertexData;
class TeDXFLayer;
class DL_Dxf;

/** @class TeDXFVisitor
 *  @brief 
 */
class TeDXFVisitor : public DL_CreationAdapter 
{
  public:
    /** @brief 
         */
        TeDXFVisitor(const string & fileName);

    /** @brief 
         */
        ~TeDXFVisitor();

    /** @brief 
         */
        void addLayer(const DL_LayerData & data);

    /** @brief 
         */
        void addPoint(const DL_PointData & data);

    /** @brief 
         */
        void addLine(const DL_LineData & data);

    /** @brief 
         */
        void addPolyline(const DL_PolylineData & data);

    /** @brief 
         */
        void addVertex(const DL_VertexData & data);

    void endEntity();

    //Return the parsed layers.
    const vector<TeDXFLayer *> & getLayers() const;


  protected:
    std::vector<TeDXFLayer *> layers_; //!< 

    string name_; //!< 

    TeLinearRing line_; //!< 


  private:
    /** @brief Copy not allowed.    
         */
        TeDXFVisitor(const TeDXFVisitor & source);

    /** @brief Copy not allowed.    
         */
        TeDXFVisitor & operator=(const TeDXFVisitor & source);

};
#endif
