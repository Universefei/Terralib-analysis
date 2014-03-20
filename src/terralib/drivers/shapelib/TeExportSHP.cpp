/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/

#include "TeDriverSHPDBF.h"
#include "TeTable.h"
#include "TeException.h"
#include "TeUtils.h"
#include "TeLayer.h"
#include "TeDatabase.h"
#include "TeQuerier.h"

#include <assert.h>
#include <vector>

DBFHandle TeCreateDBFFile (const string& dbfFilename, TeAttributeList& attList)
{
	DBFHandle hDBF = DBFCreate( dbfFilename.c_str() );
	if( hDBF == 0 )
		return 0;
	
	int i =0;
	TeAttributeList::iterator it=attList.begin();

	while ( it != attList.end() )
	{
		TeAttribute at = (*it);
		string atName = at.rep_.name_;

		if (atName.size() > 10)
		{
			int extra = (int)(atName.size() - 10)/2;
			int middle = (int)(atName.size()/2);
			string str = atName.substr(0,middle-extra-1);
			str += atName.substr(middle+extra);
			atName = str;
		}
		if (at.rep_.type_ == TeSTRING )
		{
			if (DBFAddField( hDBF, atName.c_str(), FTString, at.rep_.numChar_, 0 ) == -1 )
				return 0;
		}
		else if (at.rep_.type_ == TeINT)
		{
			if (DBFAddField( hDBF, atName.c_str(), FTInteger, 32, 0 ) == -1 )
				return 0;
		}
		else if (at.rep_.type_ == TeREAL)
		{
			if (DBFAddField( hDBF, atName.c_str(), FTDouble, 40, 15 ) == -1 )
				return 0;
		}
		// OBS: shapelib doesn´t deal with xBase field type for Date 
		// we are transforming it to string
		else if (at.rep_.type_ == TeDATETIME)
		{
			if (DBFAddField( hDBF, atName.c_str(), FTDate, 8, 0 ) == -1 )
				return 0;
		}
		++i;
		++it;
	}
	return hDBF;
}

bool TeExportPolygonSet2SHP( const TePolygonSet& ps,const std::string& base_file_name  )
{
    // creating files names
    std::string dbfFilename = base_file_name + ".dbf";
    std::string shpFilename = base_file_name + ".shp";

    // creating polygons attribute list ( max attribute size == 25 )
    TeAttributeList attList;
    TeAttribute at;
    at.rep_.type_ = TeSTRING;               //the id of the cell
    at.rep_.numChar_ = 25;
    at.rep_.name_ = "object_id_";
    at.rep_.isPrimaryKey_ = true;
	attList.push_back(at);
    
    /* DBF output file handle creation */
	DBFHandle hDBF = TeCreateDBFFile (dbfFilename, attList);
	if ( hDBF == 0 )
		return false;
    
    /* SHP output file handle creation */
    SHPHandle hSHP = SHPCreate( shpFilename.c_str(), SHPT_POLYGON );
    if( hSHP == 0 ) 
	{
      DBFClose( hDBF );
      return false;
    }
    
    /* Writing polygons */
    int iRecord = 0;
    int totpoints = 0;
    double  *padfX, *padfY;
    SHPObject       *psObject;
    int posXY, npoints, nelem;
    int nVertices;
    int* panParts;

    TePolygonSet::iterator itps;
    TePolygon poly;

    for (itps = ps.begin() ; itps != ps.end() ; itps++ ) {
      poly=(*itps);
      totpoints = 0;
      nVertices = poly.size();
      for (unsigned int n=0; n<poly.size();n++) {
        totpoints += poly[n].size();
      }

      panParts = (int *) malloc(sizeof(int) * nVertices);
      padfX = (double *) malloc(sizeof(double) * totpoints);
      padfY = (double *) malloc(sizeof(double) * totpoints);
      posXY = 0;
      nelem = 0;
      
      for (unsigned int l=0; l<poly.size(); ++l) {
        if (l==0) {
          if (TeOrientation(poly[l]) == TeCOUNTERCLOCKWISE) {
            TeReverseLine(poly[l]);
          }
        } else {
          if (TeOrientation(poly[l]) == TeCLOCKWISE) {
            TeReverseLine(poly[l]);
          }
        }
        
        npoints = poly[l].size();
        panParts[nelem]=posXY;
        
        for (int m=0; m<npoints; m++ ) {
          padfX[posXY] = poly[l][m].x_;
          padfY[posXY] = poly[l][m].y_;
          posXY++;
        }
        nelem++;
      }
                
      psObject = SHPCreateObject( SHPT_POLYGON, -1, nelem, panParts, NULL,
        posXY, padfX, padfY, NULL, NULL );
        
      int shpRes = SHPWriteObject( hSHP, -1, psObject );
      if (shpRes == -1 )
	  {
	      DBFClose( hDBF );
		  SHPClose( hSHP );
		  return false;
	  }
        
      SHPDestroyObject( psObject );
      free( panParts );
      free( padfX );
      free( padfY );

      // writing attributes - same creation order
      DBFWriteStringAttribute(hDBF, iRecord, 0, poly.objectId().c_str() );
            
      iRecord++;
    }
    
    DBFClose( hDBF );
    SHPClose( hSHP );
    return true;  
}


bool
TeExportQuerierToShapefile(TeQuerier* querier, const std::string& base)
{
	// check initial conditions
	if (!querier)
		return false;

	if (!querier->loadInstances())
		return false;

	// Get the list of attributes defined by the input querier
	bool onlyObjId = false;
	TeAttributeList qAttList = querier->getAttrList();
	if (qAttList.empty())
	{
		TeAttributeList qAttList;
		TeAttribute at;
		at.rep_.type_ = TeSTRING;               
		at.rep_.numChar_ = 100;
		at.rep_.name_ = "ID";
		at.rep_.isPrimaryKey_ = true;
		qAttList.push_back(at);
		onlyObjId = true;
	}

	// Handles to each type of geometries that will be created if necessary
	DBFHandle hDBFPol = 0;
	SHPHandle hSHPPol = 0;
	DBFHandle hDBFLin = 0;
	SHPHandle hSHPLin = 0;
	DBFHandle hDBFPt = 0;
	SHPHandle hSHPPt = 0;

	// Some auxiliary variables
    int totpoints;
    double*	padfX;
	double*	padfY;
    unsigned int nVertices;
    int* panPart;
    SHPObject *psObject;
    unsigned int posXY, npoints, nelem;
	int shpRes;

	// progress information
	if (TeProgress::instance())
		TeProgress::instance()->setTotalSteps(querier->numElemInstances());
	clock_t	t0, t1, t2;
	t2 = clock();
	t0 = t1 = t2;
	int dt = CLOCKS_PER_SEC/2;
	int dt2 = CLOCKS_PER_SEC; //* .000001;

	// Loop through the instances writting their geometries and attributes
	unsigned int iRecPol=0, iRecLin=0, iRecPt=0;
	unsigned int n, l, m;
	unsigned int nIProcessed = 0;
	TeSTInstance st;
	while(querier->fetchInstance(st))
	{
		totpoints = 0;
		nVertices = 0;
		if (st.hasPolygons())
		{
			TePolygonSet& polSet = st.getPolygons();
			TePolygonSet::iterator itps;
			int nVerticesCount = 0;
			for (itps = polSet.begin(); itps != polSet.end(); ++itps) 
			{
				nVertices = (*itps).size();
				nVerticesCount += nVertices;
				for (n=0; n<nVertices;++n) 
					totpoints += (*itps)[n].size();
			}

			panPart = (int *) malloc(sizeof(int) * nVerticesCount);
			padfX = (double *) malloc(sizeof(double) * totpoints);
			padfY = (double *) malloc(sizeof(double) * totpoints);

			posXY = 0;
			nelem = 0;
			for (itps = polSet.begin(); itps != polSet.end(); ++itps) 
			{
				TePolygon poly = *itps;
				for (l=0; l<poly.size(); ++l) 
				{
					if (l==0) 
					{
						if (TeOrientation(poly[l]) == TeCOUNTERCLOCKWISE) 
							TeReverseLine(poly[l]);
					}
					else 
					{
						if (TeOrientation(poly[l]) == TeCLOCKWISE)
							TeReverseLine(poly[l]);
					}
					npoints = poly[l].size();
					panPart[nelem]=posXY;
        
					for (m=0; m<npoints; ++m) 
					{
						padfX[posXY] = poly[l][m].x_;
						padfY[posXY] = poly[l][m].y_;
						posXY++;
					}
					nelem++;
				}
			}
			psObject = SHPCreateObject(SHPT_POLYGON, -1, nelem, panPart, NULL, posXY, padfX, padfY, NULL, NULL);
			if (hSHPPol == 0)
			{
				string fname = base + "_pol.shp";
				hSHPPol = SHPCreate(fname.c_str(), SHPT_POLYGON);
   				assert (hSHPPol != 0);
			}	
			shpRes = SHPWriteObject(hSHPPol, -1, psObject);
			SHPDestroyObject(psObject);
			free(panPart);
			free(padfX);
			free(padfY);
			assert(shpRes != -1);   
			if (hDBFPol == 0)
			{
				hDBFPol = TeCreateDBFFile(base + "_pol.dbf", qAttList);
				assert (hDBFPol != 0);
			}
			if (onlyObjId)
			{
				DBFWriteStringAttribute(hDBFPol, iRecPol, 0, st.objectId().c_str());
			}
			else
			{
				string val;
				for (n=0; n<qAttList.size();++n) 
				{
					st.getPropertyValue(val,n);
					if (qAttList[n].rep_.type_ == TeSTRING || qAttList[n].rep_.type_ == TeDATETIME)
					{
						DBFWriteStringAttribute(hDBFPol, iRecPol, n, val.c_str());
					}
					else if (qAttList[n].rep_.type_ == TeINT)
					{
						DBFWriteIntegerAttribute(hDBFPol, iRecPol, n, atoi(val.c_str()));
					}
					else if (qAttList[n].rep_.type_ == TeREAL)
					{
						DBFWriteDoubleAttribute(hDBFPol, iRecPol, n, atof(val.c_str()));
					}
				}
			}
			++iRecPol;
		}
		if (st.hasCells())
		{
			TeCellSet& cellSet = st.getCells();
			nVertices = cellSet.size();
			totpoints = nVertices*5;
			panPart = (int *) malloc(sizeof(int) * nVertices);
			padfX = (double *) malloc(sizeof(double) * totpoints);
			padfY = (double *) malloc(sizeof(double) * totpoints);
			posXY = 0;
			nelem = 0;
			TeCellSet::iterator itcs;
			for (itcs=cellSet.begin(); itcs!=cellSet.end(); ++itcs)
			{
				panPart[nelem]=posXY;
				padfX[posXY] = (*itcs).box().lowerLeft().x();
				padfY[posXY] = (*itcs).box().lowerLeft().y();
				posXY++;		
				padfX[posXY] = (*itcs).box().upperRight().x();
				padfY[posXY] = (*itcs).box().lowerLeft().y();
				posXY++;		
				padfX[posXY] = (*itcs).box().upperRight().x();
				padfY[posXY] = (*itcs).box().upperRight().y();
				posXY++;		
				padfX[posXY] = (*itcs).box().lowerLeft().x();
				padfY[posXY] = (*itcs).box().upperRight().y();
				posXY++;		
				padfX[posXY] = (*itcs).box().lowerLeft().x();
				padfY[posXY] = (*itcs).box().lowerLeft().y();
				++posXY;	
				++nelem;
			} 
			psObject = SHPCreateObject(SHPT_POLYGON, -1, nelem, panPart, NULL,posXY, padfX, padfY, NULL, NULL);
			if (hSHPPol == 0)
			{
				string fname = base + "_pol.shp";
				hSHPPol = SHPCreate(fname.c_str(), SHPT_POLYGON);
   				assert (hSHPPol != 0);
			}	
			shpRes = SHPWriteObject(hSHPPol, -1, psObject);
			SHPDestroyObject(psObject);
			free(panPart);
			free(padfX);
			free(padfY);
			assert(shpRes != -1);
			if (hDBFPol == 0)
			{
				hDBFPol = TeCreateDBFFile(base + "_pol.dbf", qAttList);
				assert (hDBFPol != 0);
			}

			if (onlyObjId)
			{
				DBFWriteStringAttribute(hDBFPol, iRecPol, 0, st.objectId().c_str());
			}
			else
			{
				string val;
				for (n=0; n<qAttList.size();++n) 
				{
					st.getPropertyValue(val,n);
					if (qAttList[n].rep_.type_ == TeSTRING || qAttList[n].rep_.type_ == TeDATETIME)
					{
						DBFWriteStringAttribute(hDBFPol, iRecPol, n, val.c_str());
					}
					else if (qAttList[n].rep_.type_ == TeINT)
					{
						DBFWriteIntegerAttribute(hDBFPol, iRecPol, n, atoi(val.c_str()));
					}
					else if (qAttList[n].rep_.type_ == TeREAL)
					{
						DBFWriteDoubleAttribute(hDBFPol, iRecPol, n, atof(val.c_str()));
					}
				}
			}
			++iRecPol;
		}
		if (st.hasLines())
		{
			TeLineSet& lineSet = st.getLines();
			nVertices = lineSet.size();
			TeLineSet::iterator itls;
			for (itls=lineSet.begin(); itls!=lineSet.end(); ++itls)
				totpoints += (*itls).size();
			panPart = (int *) malloc(sizeof(int) * nVertices);
			padfX = (double *) malloc(sizeof(double) * totpoints);
			padfY = (double *) malloc(sizeof(double) * totpoints);
			posXY = 0;
			nelem = 0;
			for (itls=lineSet.begin(); itls!=lineSet.end(); ++itls)
			{
				panPart[nelem]=posXY;
				for (l=0; l<(*itls).size(); ++l)
				{
					padfX[posXY] = (*itls)[l].x();
					padfY[posXY] = (*itls)[l].y();
					++posXY;
				}
				++nelem;
			}
			psObject = SHPCreateObject(SHPT_ARC, -1, nVertices, panPart, NULL,	posXY, padfX, padfY, NULL, NULL);
			if (hSHPLin == 0)
			{
				string fname = base + "_lin.shp"; 
				hSHPLin = SHPCreate(fname.c_str(), SHPT_ARC);
   				assert (hSHPLin != 0);
			}		
			shpRes = SHPWriteObject(hSHPLin, -1, psObject);
			SHPDestroyObject(psObject);
			free(panPart);
			free(padfX);
			free(padfY);
			assert(shpRes != -1);
			if (hDBFLin == 0)
			{
				hDBFLin = TeCreateDBFFile(base + "_lin.dbf", qAttList);
				assert (hDBFLin != 0);
			}
			if (onlyObjId)
			{
				DBFWriteStringAttribute(hDBFLin, iRecLin, 0, st.objectId().c_str());
			}
			else
			{
				string val;
				for (n=0; n<qAttList.size();++n) 
				{
					st.getPropertyValue(val,n);
					if (qAttList[n].rep_.type_ == TeSTRING || qAttList[n].rep_.type_ == TeDATETIME)
					{
						DBFWriteStringAttribute(hDBFLin, iRecLin, n, val.c_str());
					}
					else if (qAttList[n].rep_.type_ == TeINT)
					{
						DBFWriteIntegerAttribute(hDBFLin, iRecLin, n, atoi(val.c_str()));
					}
					else if (qAttList[n].rep_.type_ == TeREAL)
					{
						DBFWriteDoubleAttribute(hDBFLin, iRecLin, n, atof(val.c_str()));
					}
				}
			}
			++iRecLin;
		}
		if (st.hasPoints())
		{
			TePointSet& pointSet = st.getPoints();
			nVertices = pointSet.size();
			panPart = (int *) malloc(sizeof(int) * nVertices);
			padfX = (double *) malloc(sizeof(double) * nVertices);
			padfY = (double *) malloc(sizeof(double) * nVertices);
			nelem = 0;
			TePointSet::iterator itpts;
			for (itpts=pointSet.begin(); itpts!=pointSet.end(); ++itpts)
			{
				panPart[nelem] = nelem;
				padfX[nelem] = (*itpts).location().x();
				padfY[nelem] = (*itpts).location().y();
				++nelem;
			}
			psObject = SHPCreateObject(SHPT_POINT, -1, nVertices, panPart, NULL, nVertices, padfX, padfY, NULL, NULL );
			if (hSHPPt == 0)
			{
				string fname = base + "_pt.shp";
				hSHPPt = SHPCreate(fname.c_str(), SHPT_POINT);
   				assert (hSHPPt != 0);
			}		
			shpRes = SHPWriteObject(hSHPPt, -1, psObject);
			SHPDestroyObject(psObject);
			free(panPart);
			free(padfX);
			free(padfY);
			assert(shpRes != -1);
			if (hDBFPt == 0)
			{
				hDBFPt = TeCreateDBFFile(base + "_pt.dbf", qAttList);
				assert (hDBFPt != 0);
			}
			if (onlyObjId)
			{
				DBFWriteStringAttribute(hDBFPt, iRecPt, 0, st.objectId().c_str());
			}
			else
			{
				string val;
				for (n=0; n<qAttList.size();++n) 
				{
					st.getPropertyValue(val,n);
						if (qAttList[n].rep_.type_ == TeSTRING || qAttList[n].rep_.type_ == TeDATETIME)
					{
						DBFWriteStringAttribute(hDBFPt, iRecPt, n, val.c_str());
					}
					else if (qAttList[n].rep_.type_ == TeINT)
					{
						DBFWriteIntegerAttribute(hDBFPt, iRecPt, n, atoi(val.c_str()));
					}
					else if (qAttList[n].rep_.type_ == TeREAL)
					{
						DBFWriteDoubleAttribute(hDBFPt, iRecPt, n, atof(val.c_str()));
					}
				}
			}
			++iRecPt;
		}
		++nIProcessed;
		if (TeProgress::instance() && int(t2-t1) > dt)
		{
			t1 = t2;
			if(((int)(t2-t0) > dt2))
			{
				if (TeProgress::instance()->wasCancelled())
					break;
				else
					TeProgress::instance()->setProgress(nIProcessed);
			}
		}

	}
	if (hDBFPol != 0)
		DBFClose(hDBFPol);
	if (hSHPPol != 0)
        SHPClose(hSHPPol);
	if (hDBFLin != 0)
		DBFClose(hDBFLin);
	if (hSHPLin != 0)
        SHPClose(hSHPLin);
	if (hDBFPt != 0)
		DBFClose(hDBFPt);
	if (hSHPPt != 0)
        SHPClose(hSHPPt);

	if (TeProgress::instance())
		TeProgress::instance()->reset();
	return true;
}


bool
TeExportShapefile(TeLayer* layer, const string& shpFileName, const string& tableName, const string& restriction)
{
  	if (!layer || shpFileName.empty())
		return false;

	// check if asked table exist
	TeAttrTableVector& vTables = layer->attrTables();
	TeAttrTableVector::iterator it = vTables.begin();
	while (it != vTables.end())
	{
		if (it->name() == tableName) 
			break;
		++it;
	}
	if (it == vTables.end())
		return false;
	TeAttrTableVector askedTable;
	askedTable.push_back(*it);

	TeTheme* tempTheme = new TeTheme();
	tempTheme->attributeRest(restriction);
	tempTheme->layer(layer);
	tempTheme->collectionTable("");
	tempTheme->collectionAuxTable("");
	tempTheme->setAttTables(askedTable);

	TeQuerierParams qPar(true, true);
	qPar.setParams(tempTheme);

	TeQuerier* tQuerier = new TeQuerier(qPar);
	bool res = TeExportQuerierToShapefile(tQuerier, shpFileName);
	delete tQuerier;
	delete tempTheme;
    return res ;
}

bool 
TeExportLayerToShapefile(TeLayer* layer, const string& baseName)
{
  	if (!layer)
		return false;

	string fbase = baseName;
	if (baseName.empty())
		fbase = layer->name();

	TeTheme* tempTheme = new TeTheme();
	tempTheme->layer(layer);
	tempTheme->collectionTable("");
	tempTheme->collectionAuxTable("");
	tempTheme->setAttTables(layer->attrTables());

	TeQuerierParams qPar(true, true);
	qPar.setParams(tempTheme);

	TeQuerier* tQuerier = new TeQuerier(qPar);
	bool res = TeExportQuerierToShapefile(tQuerier, baseName);
	delete tQuerier;
	delete tempTheme;
    return res ;
}


bool 
TeExportThemeToShapefile(TeTheme* theme, TeSelectedObjects selOb, const std::string& baseName)
{
	if (!theme)
		return false;

	string fbase = baseName;
	if (baseName.empty())
		fbase = theme->name();

	TeQuerierParams qPar(true, true);
	qPar.setParams(theme);
	qPar.setSelecetObjs(selOb);

	TeQuerier* tQuerier = new TeQuerier(qPar);
	bool res = TeExportQuerierToShapefile(tQuerier, fbase);
	delete tQuerier;
	return res;
}





