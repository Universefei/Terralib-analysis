#include "shapefil.h"
#include "TeImportExport.h"
#include "TeTable.h"
#include "TeException.h"
#include "TeUtils.h"
#include "TeLayer.h"
#include "TeDatabase.h"

bool
TeExportShapefile(TeLayer& layer)
{
    DBFHandle	hDBF;
    SHPHandle	hSHP;

	string filename = layer.name();
	string indexName = layer.indexName();
	int nrepres = 0;
	int rep = 0;
    int	nShapeType;
	if (layer.hasGeometry(TePOINTS))
	{
		nShapeType = SHPT_POINT;
		rep = TePOINTS;
		nrepres++;
	}
	if (layer.hasGeometry(TePOLYGONS))
	{
		nShapeType = SHPT_POLYGON;
		rep = TePOLYGONS;
		nrepres++;
	}
	if (layer.hasGeometry(TeLINES))
	{
		nShapeType = SHPT_ARC;
		rep = TeLINES;
		nrepres++;
	}
	if (nrepres == 0 || nrepres>1)
	{
		return false;
	}

	string dbfFilename = filename + ".dbf";
    hDBF = DBFCreate( dbfFilename.c_str() );
    if( hDBF == 0 )
		return false;

	string shpFilename = filename + ".shp";
    hSHP = SHPCreate( shpFilename.c_str(), nShapeType );
    if( hSHP == 0 )
		return false;

	TeDatabasePortal* portalObjs = layer.database()->getPortal();
	TeDatabasePortal* portalGeoms = layer.database()->getPortal();

	string sql1 = "SELECT * FROM " + layer.attributeTable();
	if (!portalObjs->query(sql1))
	{
		delete portalObjs;
		delete portalGeoms;
		return false;
	}

	int i = portalObjs->numFields();
	vector<int> fieldsTypes;
	fieldsTypes.resize(i);
	i=0;
	TeAttributeList attList=portalObjs->AttributeList();
	TeAttributeList::iterator it=attList.begin();

	while ( it != attList.end() )
    {
		TeAttribute at = (*it);
		fieldsTypes[i] = at.rep_.type_;

		if (at.rep_.type_ == TeSTRING )
		{
			if (DBFAddField( hDBF, at.rep_.name_.c_str(), FTString, at.rep_.numChar_, 0 ) == -1 )
			{
				delete portalObjs;
				delete portalGeoms;
				return false;
			}
		}
		else if (at.rep_.type_ == TeDATETIME)
		{
			if (DBFAddField( hDBF, at.rep_.name_.c_str(), FTDate, 8, 0 ) == -1 )
			{
				delete portalObjs;
				delete portalGeoms;
				return false;
			}
		}
		else if (at.rep_.type_ == TeINT)
		{
			if (DBFAddField( hDBF, at.rep_.name_.c_str(), FTInteger, 10, 0 ) == -1 )
			{
				delete portalObjs;
				delete portalGeoms;
				return false;
			}
		}
		else if (at.rep_.type_ == TeREAL)
			if (DBFAddField( hDBF, at.rep_.name_.c_str(), FTDouble, 10, 5 ) == -1 )
			{
				delete portalObjs;
				delete portalGeoms;
				return false;
			}
		i++;
		it++;
	}
    int iRecord = DBFGetRecordCount( hDBF );
	string geoid, sql2;
	bool ok;

    int	 nVertices, nParts, *panParts;
    double	*padfX, *padfY;
    SHPObject	*psObject;

	TePointSet pos;
	TeLineSet lis;
	TePolygonSet pols;
	while (portalObjs->fetchRow())
	{
		geoid = portalObjs->getData(indexName);
		nVertices = 0;
		switch (rep)
		{
		case TePOINTS:
			sql2 = "SELECT * FROM " + layer.tableName(TePOINTS) + " WHERE id='" + geoid + "'";
			ok = false;
			if (portalGeoms->query(sql2))
			{
				while (portalGeoms->fetchRow())
				{
					TePoint point = portalGeoms->getPoint();
					pos.add(point);
					ok = true;
				}
				portalGeoms->freeResult();
				nVertices = nParts = pos.size();
				panParts = (int *) malloc(sizeof(int) * nParts);
				padfX = (double *) malloc(sizeof(double) * nParts);
				padfY = (double *) malloc(sizeof(double) * nParts);
				for (int k=0;k<nParts;k++)
				{
					panParts[k] = k;
					padfX[k] = pos[k].location().x();
					padfY[k] = pos[k].location().y();
				}
				psObject = SHPCreateObject( nShapeType, -1, nParts, panParts, NULL,
											nVertices, padfX, padfY, NULL, NULL );
				SHPWriteObject( hSHP, -1, psObject );

				SHPDestroyObject( psObject );
				free( panParts );
				free( padfX );
				free( padfY );
				pos.clear();
			}
			break;
		case TeLINES:
			sql2 = "SELECT * FROM " + layer.tableName(TeLINES) + " WHERE id='" + geoid + "'";
			ok = false;
			int npoints;
			if (portalGeoms->query(sql2))
			{
				npoints = 0;
				while (portalGeoms->fetchRow())
				{
					TeLine2D line = portalGeoms->getLine();
					lis.add(line);
					ok = true;
					npoints += line.size();
				}
				portalGeoms->freeResult();
				nVertices = lis.size();
				panParts = (int *) malloc(sizeof(int) * nVertices);
				padfX = (double *) malloc(sizeof(double) * npoints);
				padfY = (double *) malloc(sizeof(double) * npoints);
				
				int posXY = 0;
				for (int k=0; k<nVertices; k++)
				{
					int lineSize = lis[k].size();
					panParts[k]=posXY;
					for (int l=0; l<lineSize; l++ )
					{
						padfX[posXY] = lis[k][l].x();
						padfY[posXY] = lis[k][l].y();
						posXY++;
					}
				}
				lis.clear();
				psObject = SHPCreateObject( nShapeType, -1, nVertices, panParts, NULL,
											posXY, padfX, padfY, NULL, NULL );
				SHPWriteObject( hSHP, -1, psObject );

				SHPDestroyObject( psObject );
				free( panParts );
				free( padfX );
				free( padfY );
			}
			break;
		case TePOLYGONS:
			sql2 = "SELECT * FROM " + layer.tableName(TePOLYGONS) + " WHERE id='" + geoid + "'";
			ok = false;
			int totpoints;
			nVertices = 0;
			if (portalGeoms->query(sql2))
			{
				totpoints = 0;
				while (portalGeoms->fetchRow())
				{
					TePolygon poly = portalGeoms->getPolygon();
					pols.add(poly);
					ok = true;
					nVertices += poly.size();
					for (int n=0; n<poly.size();n++)
						totpoints += poly[n].size();
				}
				portalGeoms->freeResult();
				panParts = (int *) malloc(sizeof(int) * nVertices);
				padfX = (double *) malloc(sizeof(double) * totpoints);
				padfY = (double *) malloc(sizeof(double) * totpoints);
				
				int posXY = 0;
				int npolygons, nrings, npoints, nelem = 0;
				npolygons = pols.size();
				for (int k=0; k<npolygons; k++)
				{
					nrings = pols[k].size();
					for (int l=0; l<nrings; l++)
					{
						npoints = pols[k][l].size();
						panParts[nelem]=posXY;
						for (int m=0; m<npoints; m++ )
						{
							padfX[posXY] = pols[k][l][m].x();
							padfY[posXY] = pols[k][l][m].y();
							posXY++;
						}
						nelem++;
					}
				}
				pols.clear();
				psObject = SHPCreateObject( nShapeType, -1, nelem, panParts, NULL,
											posXY, padfX, padfY, NULL, NULL );
				SHPWriteObject( hSHP, -1, psObject );

				SHPDestroyObject( psObject );
				free( panParts );
				free( padfX );
				free( padfY );
			}
			break;
		}
		
		if (!ok)
			continue;

		for (int j = 0; j < portalObjs->numFields(); j++)
		{
			int c = fieldsTypes[j];
			if ( c == TeSTRING )
			{
				DBFWriteStringAttribute(hDBF, iRecord, j, portalObjs->getData(j) );
			}
			else if (c == TeINT)
			{
				DBFWriteIntegerAttribute(hDBF, iRecord, j, atoi(portalObjs->getData(j)) );
			}
			else if (c == TeREAL)
			{
				DBFWriteDoubleAttribute(hDBF, iRecord, j, atof(portalObjs->getData(j)) );
			}
		}
		iRecord++;
	}
	DBFClose( hDBF );
    SHPClose( hSHP );
	delete portalObjs;
	delete portalGeoms;
    return true ;
}