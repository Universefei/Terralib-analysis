/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TePGInterface.h
    \brief This file contains a database recordset and connection class to PostgreSQL.
	\author Gilberto Ribeiro de Queiroz - gribeiro@dpi.inpe.br
 */


#ifndef  __TERRALIB_INTERNAL_PGINTERFACE_H
#define  __TERRALIB_INTERNAL_PGINTERFACE_H
 
/**
  * TODO: 1. O caminhamento para tras esta na ordem errada.
  *       2. A funcao getData devera ser completada para tipos diferentes da TerraLib
  *
  */
//extern "C"
//{
#include <libpq-fe.h>
//}

#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif


#include <stdlib.h>
#include <string>
#include <cstring>

#include "../../kernel/TeDatabase.h"
#include "../../kernel/TeMultiGeometry.h"
#include "../../kernel/TeMutex.h"

#include "TePostgreSQLDefines.h"

using namespace std;

/***************************************************************************
 * Constantes numericas dos tipos de dados do PostgreSQL                   * 
 ***************************************************************************/
const Oid PG_BOOL_TYPE = 16;
const Oid PG_BYTEA_TYPE = 17;
const Oid PG_CHAR_TYPE = 18;
const Oid PG_NAME_TYPE = 19;
const Oid PG_INT8_TYPE = 20;
const Oid PG_INT2_TYPE = 21;
const Oid PG_INT4_TYPE = 23;
const Oid PG_TEXT_TYPE = 25;
const Oid PG_OID_TYPE = 26;
const Oid PG_POINT_TYPE = 600;
const Oid PG_LSEG_TYPE = 601;
const Oid PG_PATH_TYPE = 602;
const Oid PG_BOX_TYPE = 603;
const Oid PG_POLYGON_TYPE = 604;
const Oid PG_LINE_TYPE = 628;
const Oid PG_PG_FLOAT4_TYPE = 700;
const Oid PG_FLOAT8_TYPE = 701;
const Oid PG_CIRCLE_TYPE = 718;
const Oid PG_MONEY_TYPE = 790;
const Oid PG_BPCHAR_TYPE = 1042;
const Oid PG_VARCHAR_TYPE = 1043;
const Oid PG_DATE_TYPE = 1082;
const Oid PG_TIME_TYPE = 1083;
const Oid PG_TIMESTAMP_TYPE = 1114;
const Oid PG_TIMESTAMPTZ_TYPE = 1184;
const Oid PG_INTERVAL_TYPE = 1186;
const Oid PG_TIMETZ_TYPE = 1266;
const Oid PG_BIT_TYPE = 1560;
const Oid PG_VARBIT_TYPE = 1562;
const Oid PG_NUMERIC_TYPE = 1700;


//method modified from Terralib5 used for EWKB compatibility
#define TE_EWKB_ZM_OFFSET    0xC0000000
#define TE_EWKB_Z_OFFSET     0x80000000
#define TE_EWKB_M_OFFSET     0x40000000
#define TE_EWKB_SRID_FLAG    0x20000000
#define TE_EWKB_BBOX_FLAG    0x10000000


/***************************************************************************
 * Classe responsavel pela abertura e fechamento de uma conexao com o      *
 * banco de dados. Alem de abrir e fechar uma conexao, permite que uma     *
 * consulta seja executada, retornando um conjunto de registros, ou que um *
 * comando - insert, update, delete - seja executado diretamente,          *
 * retornando o numero de registros afetados pelo comando.                 *
 ***************************************************************************/
class TLPOSTGRESQL_DLL TePGConnection
{
    protected:

        PGconn *pg_connection_;	// A pointer to a connection
        int int_connection_;	// A flag that indicates the state of a connection
        Oid last_oid_;			// Always stores the last OID resulted of a command
		bool inTransaction_;
		bool isDateInt_;		// Tells if server date format is int64 or double (IEEE754)

    public:

        // Constructor / Destructor
        TePGConnection(const string& str_connection = "")
			: pg_connection_(0), int_connection_(0), last_oid_(0), inTransaction_(false), isDateInt_(true)
		{
			if(!str_connection.empty())
				open(str_connection);
		}

        virtual ~TePGConnection() { close(); }

		// Basic database methods
        bool open(const string& str_connection);

		bool open(PGconn* conn);

        bool state(void) const { return (PQstatus(pg_connection_) == CONNECTION_OK); }
        void close(void);
        const char* dbname(void) const { return PQdb(pg_connection_); }
        const char* err_msg(void) const { return PQerrorMessage(pg_connection_); }
        PGresult* exec(const string& str_qry) { return PQexec(pg_connection_, str_qry.c_str()); }
        int exec_cmd(const string& str_qry);
        Oid getOID(void) const { return last_oid_; }
		PGconn* c_ptr(void) { return pg_connection_; }

		bool isDateInt() const
		{
			return isDateInt_;
		}

		// Utilities functions - You must destroy the return string
		static char* escapeString(const char* from, const size_t& fromLen, size_t& newLen)
		{
			char *sql = new char[2 * fromLen + 1];	

			newLen = PQescapeString(sql, from, fromLen);

			return sql;
		}

		static void freeMem(void* ptr) 	{ PQfreemem(ptr); }

		static unsigned char* escapeBytea(const unsigned char *from, size_t from_length, size_t* to_length) { return PQescapeBytea(from, from_length, to_length); }
		
		Oid insertBlob(char* buff, const int& blobSize);	// Returns 0 on error
		int getBlob(const Oid& blobId, char *&buff);		// return -1 on error otherwise returns bytes read
		bool removeBlob(const Oid& blobId) { return lo_unlink(pg_connection_, blobId) == 1; }

		bool beginTransaction(void);
		bool commitTransaction(void);
		bool rollBackTransaction(void);

		int exec_cmd_params(const char *command,
							 int nParams,
							 const Oid *paramTypes,
							 const char * const *paramValues,
							 const int *paramLengths,
							 const int *paramFormats,
							 int resultFormat);

		std::string getClientEncoding();

	protected:

        // don't support copying of TePGConnection objects,
        // so make copy constructor and assignment op private.
        TePGConnection(const TePGConnection& c);
        TePGConnection& operator=(const TePGConnection& c);
};

/***************************************************************************
 * Classe resposnsavel pela manipulacao de dados resultantes de uma        *
 * consulta.                                                               *
 ***************************************************************************/
class TLPOSTGRESQL_DLL TePGRecordset
{
	protected:

		static const int        int_bof_;
		static int              openRecordSets_;
		static set<int>         freeCursorNumbers_;
		static TeMutex          _mutexLock;
		
        PGresult               *pg_recordset_;
		TePGConnection*         conn_;
		bool                    cursorDeclared_;
        int                     int_index_current_;
        int                     int_eof_;
		TeCursorType            cursorType_;
		TeCursorLocation        cursorLocation_;
		TeCursorDataType        cursorDataType_;
		int                     numRowsToRetrieve_;

		int                     recordSetId_;
		bool                    isLittleEndian_;

		string                  data_;

		bool                    movingBackward_;	// Mario
		bool                    lastFetch;			// Mario
		bool                    firstFetch;		// Mario
		

    public:

        // Constructor / Destructor
        TePGRecordset(const string& str_qry = string(""), TePGConnection* con_x = 0,
			          const TeCursorType& cursorType = TeUNIDIRECTIONAL,
				      const TeCursorLocation& cursorLocation = TeSERVERSIDE,
				      const TeCursorDataType& cursorDataType = TeTEXTCURSOR,
					  const int& numRowsToRetrieve = 2000);
        virtual ~TePGRecordset();

        // Basic database methods
		void close(void);

        int recordCount(void) const
		{
			if(pg_recordset_)
				return PQntuples(pg_recordset_);
			
			return 0;
		}

		int getFieldCount(void) const	{ return PQnfields(pg_recordset_); }

		bool state(void) const
		{
			if(pg_recordset_) 
				return true;
			else
				return false;
		}

		bool eof(void) const { return int_index_current_ == int_eof_; }
        bool bof(void) const { return int_index_current_ == int_bof_; }

		bool moveFirst(void);
        bool movePrevious(void);	
        bool moveNext(void);		
        bool moveLast(void);		
        bool moveTo(const int& lin_number);	// first index == 0
		
		int index(void) const
		{
			if(!(this->bof() || this->eof()))
				return int_index_current_;
			
			return -1;
		}

	    bool open(const string& str_qry, TePGConnection* con_x,
			      const TeCursorType& cursorType = TeUNIDIRECTIONAL,
				  const TeCursorLocation& cursorLocation = TeSERVERSIDE,
				  const TeCursorDataType& cursorDataType = TeTEXTCURSOR,
				  const int& numRowsToRetrieve = 2000);      

		

		// Functions tha apllies to columns
		char *fieldName(const int& field_num) const { return PQfname(pg_recordset_, field_num); }

        int fieldNum(const string& field_name) const
		{
			int num = PQfnumber(pg_recordset_, field_name.c_str());

			if(num < 0)
			{
				std::string::size_type ip = field_name.rfind('.');
                
				if(ip == std::string::npos)
					return num;

				std::string::size_type len = field_name.size();

				std::string fieldName = field_name.substr(ip + 1, len - ip - 1);

				num = PQfnumber(pg_recordset_, fieldName.c_str());
			}

			return num;
		}

		Oid fieldType(const int& field_num) const { return PQftype(pg_recordset_, field_num); }
        Oid fieldType(const string& field_name) const { return fieldType(fieldNum(field_name)); }

		bool isNull(const int& field_num) const { return PQgetisnull(pg_recordset_, index(), field_num) == 1; }
		bool isNull(const string& field_name) const { return isNull(fieldNum(field_name)); }

		int fieldSize(const int& field_num);
        int fieldSize(const string& field_name) { return fieldSize(fieldNum(field_name)); }

        int fieldSizeFractionaryPart(const int& field_num);
        int fieldSizeFractionaryPart(const string& field_name) {  return fieldSizeFractionaryPart(fieldNum(field_name)); }


		// Datatype functions
		
		// when you use value, make sure to copy the value, otherwise, a sucessive call may destroy the pointer
		char* value(const int& field_num) const { return PQgetvalue(pg_recordset_, int_index_current_, field_num); }
		char* value(const string& field_name) const { return value(fieldNum(field_name)); }

		int getBytea(const int& field_num, char*& buff) const;	
		int getBytea(const string& field_name, char*& buff) const { return getBytea(fieldNum(field_name), buff); }
		
		int getInt(const int& field_num) const
		{
			if(cursorDataType_ == TeTEXTCURSOR) 
				return atol(value(field_num));
			else
			{
				if(fieldType(field_num) == PG_INT8_TYPE)
				{
					return getInt8(field_num);
				}
				else
				{
					unsigned int number;

					memcpy(&number, value(field_num), sizeof(int));

					if(isLittleEndian_)
						number = ntohl(number);

					return number;
				}
			}
		}

		int getInt(const string& field_name) const { return getInt(fieldNum(field_name)); }

		int getInt8(const int& field_num) const ;
		int getInt8(const string& field_name) const { return getInt8(fieldNum(field_name)); }

		bool getBool(const int& field_num) const
		{
			if(cursorDataType_ == TeTEXTCURSOR) 
			{
				string t = "t";

				return (t == value(field_num));
			}
			else
			{
				unsigned char v = value(field_num)[0];
				int v1 = v;
				return v1 != 0;		
			}
		}

		bool getBool(const string& field_name) const { return getBool(fieldNum(field_name)); }

		double getDouble(const int& field_num) const
		{
			if(cursorDataType_ == TeTEXTCURSOR) 
				return atof(value(field_num));
			else
			{
				union
				{
					double dWord_;
					unsigned int aux_[2];
				} swapDouble1, swapDouble2;


				// endianness test
				if(isLittleEndian_)
				{
					// little-endian!
					memcpy(&swapDouble1.dWord_, value(field_num), sizeof(double));

					swapDouble2.aux_[1] = ntohl(swapDouble1.aux_[0]);
					swapDouble2.aux_[0] = ntohl(swapDouble1.aux_[1]);

					return swapDouble2.dWord_;
				}
				else
				{
					// big-endian!
					memcpy(&swapDouble1.dWord_, value(field_num), sizeof(double));

					swapDouble2.aux_[0] = ntohl(swapDouble1.aux_[0]);
					swapDouble2.aux_[1] = ntohl(swapDouble1.aux_[1]);

					return swapDouble2.dWord_;
				}
					
			}
		}

		double getDouble(const string& field_name) const { return getDouble(fieldNum(field_name)); }

		string getDate(const int& field_num) const;
		string getDate(const string& field_name) const { return getDate(fieldNum(field_name)); }

		string getTime(const int& field_num);
		string getTime(const string& field_name) { return getTime(fieldNum(field_name)); }

		string getTimeStamp(const int& field_num);
		string getTimeStamp(const string& field_name) { return getTimeStamp(fieldNum(field_name)); }



		char* getData(const int& field_num);
		char* getData(const string& field_name) { return getData(fieldNum(field_name)); }

		// TerraLib Geometric Types
		void getByteALine2D(const int& field_num, TeLine2D& l) const;
		void getByteALine2D(const string& field_name, TeLine2D& l) const { getByteALine2D(fieldNum(field_name), l); }

		// PostgreSQL Geometric types
		void getPGBox(const int& field_num, TeBox& b) const;
		void getPGBox(const string& field_name, TeBox& b) const { getPGBox(fieldNum(field_name), b); }

		void getPGLine2D(const int& field_num, TeLine2D& l) const;
		void getPGLine2D(const string& field_name, TeLine2D& l) const { getPGLine2D(fieldNum(field_name), l); }

		// PostGIS Methods
		void getPGISPoint(const int& field_num, TePoint& p) const;
		void getPGISPoint(const string& field_name, TePoint& p) const { getPGISPoint(fieldNum(field_name), p); }

		void getPGISLine(const int& field_num, TeLine2D& l) const;
		void getPGISLine(const string& field_name, TeLine2D& l) const { getPGISLine(fieldNum(field_name), l); }

		void getPGISPolygon(const int& field_num, TePolygon& p) const;
		void getPGISPolygon(const string& field_name, TePolygon& p) const { getPGISPolygon(fieldNum(field_name), p); }

		void getPGISMultiPoint(const int& field_num, TePointSet& ps) const;
		void getPGISMultiPoint(const string& field_name, TePointSet& ps) const { getPGISMultiPoint(fieldNum(field_name), ps); }

		void getPGISMultiLine(const int& field_num, TeLineSet& ls) const;
		void getPGISMultiLine(const string& field_name, TeLineSet& ls) const { getPGISMultiLine(fieldNum(field_name), ls); }

		void getPGISMultiPolygon(const int& field_num, TePolygonSet& ps) const;
		void getPGISMultiPolygon(const string& field_name, TePolygonSet& ps) const { getPGISMultiPolygon(fieldNum(field_name), ps); }

		void getPGISGeomColl(const int& field_num, TeMultiGeometry& m) const;
		void getPGISGeomColl(const string& field_name, TeMultiGeometry& m) const { getPGISGeomColl(fieldNum(field_name), m); }

	protected:

        // don't support copying of TePGRecordset objects,
        // so make copy constructor and assignment op private.
        TePGRecordset(const TePGRecordset& c);
        TePGRecordset& operator=(const TePGRecordset& c);

		static int getFreeCursor()
		{
			_mutexLock.lock();

			int freeId = 0;
			if(freeCursorNumbers_.empty())
			{	// Se n�o houver cursores livres aberto => criar novo id
				++openRecordSets_;
				freeId = openRecordSets_;
			}
			else
			{
				set<int>::iterator it = freeCursorNumbers_.begin();
				freeId = *it;
				freeCursorNumbers_.erase(it);
			}

			_mutexLock.unLock();

			return freeId;
		}
		static void releaseCursor(const int& idToRelease)
		{
			_mutexLock.lock();
			freeCursorNumbers_.insert(idToRelease);
			_mutexLock.unLock();
		}

		void freeRecordSet();

//method modified from Terralib5 used for EWKB compatibility
		char* getWKBHeader(char* v, unsigned char &byteOrder, unsigned int& wkbType, unsigned int &numGeometries, int& srid) const;

		char* getWKBPoint(char* v, TePoint& p) const;
		char* getWKBLine(char* v, TeLine2D& l) const;
		char* getWKBPolygon(char* v, TePolygon& p) const;

		char* getWKBMultiPoint(char* v, TePointSet &ps) const;
		char* getWKBMultiLine(char* v, TeLineSet &ls) const;
		char* getWKBMultiPolygon(char* v, TePolygonSet &ps) const;

		void getWKBGeomColl(char* v, TeMultiGeometry &mg) const;

        char* getWKBLinearRing(char* v, int byteOrder, TeLine2D &line) const;
};

TLPOSTGRESQL_DLL char* TeLine2DToPGBinary(const TeLine2D &line, unsigned int &size);
TLPOSTGRESQL_DLL char* TeBoxToPGBinary(const TeBox &box, unsigned int &size);

TLPOSTGRESQL_DLL char* TePointToWKBPoint(const TePoint &point, unsigned int &size);
TLPOSTGRESQL_DLL char* TeLine2DToWKBLineString(const TeLine2D &line, unsigned int &size);
TLPOSTGRESQL_DLL char* TePolygonToWKBPolygon(const TePolygon &polygon, unsigned int &size);

TLPOSTGRESQL_DLL double TeConvertToBigEndian(const double &value);

TLPOSTGRESQL_DLL void TeConvert2OGCWKBType(unsigned int& gType, bool& isPostGIS);
TLPOSTGRESQL_DLL void TeConvert2PostGISWKBType(unsigned int& gType);


#endif	//  __TERRALIB_INTERNAL_PGINTERFACE_H
