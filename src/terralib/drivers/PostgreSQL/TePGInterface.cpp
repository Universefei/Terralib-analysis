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

/*
 * TODO: 1. NAO PEGAR O DOBRO DE REGISTROS NA VOLTA (MOVE PREVIOUS) NO FUTURO!!!!
 *       2. set datestyle = 'ISO, YMD'
 *
 */

#include "TePGInterface.h"
#include "TePGUtils.h"
#include <stdexcept>
#include <string>
#include <climits>

#include "../../kernel/TeUtils.h"

/*
 *	Read/write mode flags for inversion (large object) calls
 */

#define INV_WRITE		0x00020000
#define INV_READ		0x00040000

/*
 * Read buffer size
 */
#define BUFFSIZE        1024

/*
 * PostgreSQL Geometric Types
 */
typedef struct
{
	double		x, y;
} BIN_PG_POINT;

using namespace std;

/*******************************
 * FUN��ES AUXILIARES          *
 *******************************/
// swapp bytes
unsigned int TePGSwapUInt(const unsigned int& uintVal)
{
	char uintIn[4], uintOut[4];

	unsigned int outVal;

	memcpy(uintIn,&uintVal,4);

	uintOut[0] = uintIn[3];
	uintOut[1] = uintIn[2];
	uintOut[2] = uintIn[1];
	uintOut[3] = uintIn[0];

	memcpy (&outVal,uintOut,4);

	return outVal;
}


// This function is the same parse_hex function in PostGIS rc3 source code.
inline unsigned char parse_hex(char *str)
{
	//do this a little brute force to make it faster

	unsigned char		result_high = 0;
	unsigned char		result_low = 0;

	switch (str[0])
	{
		case '0' :
			result_high = 0;
			break;
		case '1' :
			result_high = 1;
			break;
		case '2' :
			result_high = 2;
			break;
		case '3' :
			result_high = 3;
			break;
		case '4' :
			result_high = 4;
			break;
		case '5' :
			result_high = 5;
			break;
		case '6' :
			result_high = 6;
			break;
		case '7' :
			result_high = 7;
			break;
		case '8' :
			result_high = 8;
			break;
		case '9' :
			result_high = 9;
			break;
		case 'A' :
			result_high = 10;
			break;
		case 'B' :
			result_high = 11;
			break;
		case 'C' :
			result_high = 12;
			break;
		case 'D' :
			result_high = 13;
			break;
		case 'E' :
			result_high = 14;
			break;
		case 'F' :
			result_high = 15;
			break;
	}
	switch (str[1])
	{
		case '0' :
			result_low = 0;
			break;
		case '1' :
			result_low = 1;
			break;
		case '2' :
			result_low = 2;
			break;
		case '3' :
			result_low = 3;
			break;
		case '4' :
			result_low = 4;
			break;
		case '5' :
			result_low = 5;
			break;
		case '6' :
			result_low = 6;
			break;
		case '7' :
			result_low = 7;
			break;
		case '8' :
			result_low = 8;
			break;
		case '9' :
			result_low = 9;
			break;
		case 'A' :
			result_low = 10;
			break;
		case 'B' :
			result_low = 11;
			break;
		case 'C' :
			result_low = 12;
			break;
		case 'D' :
			result_low = 13;
			break;
		case 'E' :
			result_low = 14;
			break;
		case 'F' :
			result_low = 15;
			break;
	}
	return (unsigned char) ((result_high<<4) + result_low);
}

//Os trechos abaixo fram retirados do codigo fonte do Servidor PostgreSQL, pois aqui utilizamos cursores binarios
// timestamp values are stored as seconds before or after midnight 2000-01-01
#define UNIX_EPOCH_JDATE		2440588 /* == date2j(1970, 1, 1) */

#define POSTGRES_EPOCH_JDATE	2451545 /* == date2j(2000, 1, 1) */
#define MAXDATELEN		51		/* maximum possible length of an input date string (not counting tr. null) */
#define MAXDATEFIELDS	25		/* maximum possible number of fields in a date string */

typedef int TeDateADT;
typedef long long int te_int64;
typedef te_int64 te_pg_time_t;

typedef int te_fsec_t;

/* Decide if we need to decorate 64-bit constants */
//#ifdef HAVE_LL_CONSTANTS
#define INT64CONST(x)  ((te_int64) x##LL)
//#else
//#define INT64CONST(x)  ((te_int64) x)
//#endif


#define DT_NOBEGIN		(-INT64CONST(0x7fffffffffffffff) - 1)
#define DT_NOEND		(INT64CONST(0x7fffffffffffffff))

#define TIMESTAMP_NOBEGIN(j)	do {j = DT_NOBEGIN;} while (0)
#define TIMESTAMP_IS_NOBEGIN(j) ((j) == DT_NOBEGIN)

#define TIMESTAMP_NOEND(j)		do {j = DT_NOEND;} while (0)
#define TIMESTAMP_IS_NOEND(j)	((j) == DT_NOEND)

#define TIMESTAMP_NOT_FINITE(j) (TIMESTAMP_IS_NOBEGIN(j) || TIMESTAMP_IS_NOEND(j))

#define TMODULO_INT64(t,q,u) \
do { \
	q = (t / u); \
	if (q != 0) t -= (q * u); \
} while(0)



struct te_pg_tm
{
	int			tm_sec;
	int			tm_min;
	int			tm_hour;
	int			tm_mday;
	int			tm_mon;			/* origin 0, not 1 */
	int			tm_year;		/* relative to 1900 */
	int			tm_wday;
	int			tm_yday;
	int			tm_isdst;
	long int	tm_gmtoff;
	const char *tm_zone;
};

inline void j2date(int jd, int *year, int *month, int *day)
{
	unsigned int julian;
	unsigned int quad;
	unsigned int extra;
	int			y;

	julian = jd;
	julian += 32044;
	quad = julian / 146097;
	extra = (julian - quad * 146097) * 4 + 3;
	julian += 60 + quad * 3 + extra / 146097;
	quad = julian / 1461;
	julian -= quad * 1461;
	y = julian * 4 / 1461;
	julian = ((y != 0) ? ((julian + 305) % 365) : ((julian + 306) % 366))
		+ 123;
	y += quad * 4;
	*year = y - 4800;
	quad = julian * 2141 / 65536;
	*day = julian - 7834 * quad / 256;
	*month = (quad + 10) % 12 + 1;

	return;
}	/* j2date() */

/* EncodeDateOnly()
 * Encode date as local time.
 */
inline bool EncodeDateOnly(struct te_pg_tm* tm, char *str)
{
	if ((tm->tm_mon < 1) || (tm->tm_mon > 12))
		return false;

	/* compatible with ISO date formats */
	if (tm->tm_year > 0)
		sprintf(str, "%04d-%02d-%02d",
				tm->tm_year, tm->tm_mon, tm->tm_mday);
	else
		sprintf(str, "%04d-%02d-%02d %s", -(tm->tm_year - 1), tm->tm_mon, tm->tm_mday, "BC");

	
	return true;
}	/* EncodeDateOnly() */

inline  int time2tm(te_int64 time, struct te_pg_tm * tm, te_fsec_t *fsec)
{
	tm->tm_hour = (int)((time / INT64CONST(3600000000)));
	time -= (tm->tm_hour * INT64CONST(3600000000));
	tm->tm_min = (int)((time / INT64CONST(60000000)));
	time -= (tm->tm_min * INT64CONST(60000000));
	tm->tm_sec = (int)((time / INT64CONST(1000000)));
	time -= (tm->tm_sec * INT64CONST(1000000));
	*fsec = (te_fsec_t)time;

	return 0;
}

/* EncodeTimeOnly()
 * Encode time fields only.
 */
inline bool EncodeTimeOnly(struct te_pg_tm * tm, te_fsec_t fsec, int *tzp, char *str)
{
	if ((tm->tm_hour < 0) || (tm->tm_hour > 24))
		return false;

	sprintf(str, "%02d:%02d", tm->tm_hour, tm->tm_min);

	/*
	 * Print fractional seconds if any.  The field widths here should be
	 * at least equal to the larger of MAX_TIME_PRECISION and
	 * MAX_TIMESTAMP_PRECISION.
	 */
	if(fsec != 0)
	{
		sprintf((str + strlen(str)), ":%02d.%06d", tm->tm_sec, fsec);

		/* chop off trailing pairs of zeros... */
		while ((strcmp((str + strlen(str) - 2), "00") == 0)
			   && (*(str + strlen(str) - 3) != '.'))
			*(str + strlen(str) - 2) = '\0';
	}
	else
		sprintf((str + strlen(str)), ":%02d", tm->tm_sec);

	if(tzp != NULL)
	{
		int			hour,
					min;

		hour = -(*tzp / 3600);
		min = ((abs(*tzp) / 60) % 60);
		sprintf((str + strlen(str)), ((min != 0) ? "%+03d:%02d" : "%+03d"), hour, min);
	}

	return true;
}	/* EncodeTimeOnly() */

inline void TrimTrailingZeros(char *str)
{
	size_t	len = strlen(str);

	/* chop off trailing zeros... but leave at least 2 fractional digits */
	while ((*(str + len - 1) == '0') && (*(str + len - 3) != '.'))
	{
		len--;
		*(str + len) = '\0';
	}
}


/* EncodeDateTime()
 * Encode date and time interpreted as local time.
 * Support several date styles:
 *	Postgres - day mon hh:mm:ss yyyy tz
 *	SQL - mm/dd/yyyy hh:mm:ss.ss tz
 *	ISO - yyyy-mm-dd hh:mm:ss+/-tz
 *	German - dd.mm.yyyy hh:mm:ss tz
 * Variants (affects order of month and day for Postgres and SQL styles):
 *	US - mm/dd/yyyy
 *	European - dd/mm/yyyy
 */
inline bool EncodeDateTime(struct te_pg_tm * tm, te_fsec_t fsec, int *tzp, char ** /* tzn */, char *str)
{
	int			hour,
				min;

	if ((tm->tm_mon < 1) || (tm->tm_mon > 12))
		return false;

	/* Compatible with ISO-8601 date formats */

	sprintf(str, "%04d-%02d-%02d %02d:%02d",
		  ((tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1)),
			tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min);

	/*
		* Print fractional seconds if any.  The field widths here
		* should be at least equal to MAX_TIMESTAMP_PRECISION.
		*
		* In float mode, don't print fractional seconds before 1 AD,
		* since it's unlikely there's any precision left ...
		*/

	if (fsec != 0)
	{
		sprintf((str + strlen(str)), ":%02d.%06d", tm->tm_sec, fsec);
		TrimTrailingZeros(str);
	}
	else
        sprintf((str + strlen(str)), ":%02d", tm->tm_sec);

	/*
		* tzp == NULL indicates that we don't want *any* time zone
		* info in the output string. *tzn != NULL indicates that we
		* have alpha time zone info available. tm_isdst != -1
		* indicates that we have a valid time zone translation.
		*/
	if ((tzp != NULL) && (tm->tm_isdst >= 0))
	{
		hour = -(*tzp / 3600);
		min = ((abs(*tzp) / 60) % 60);
		sprintf((str + strlen(str)), ((min != 0) ? "%+03d:%02d" : "%+03d"), hour, min);
	}

	if (tm->tm_year <= 0)
		sprintf((str + strlen(str)), " BC");

	return true;
}


/* EncodeSpecialTimestamp()
 * Convert reserved timestamp data type to string.
 */
inline bool EncodeSpecialTimestamp(te_int64 dt, char *str)
{
	if (TIMESTAMP_IS_NOBEGIN(dt))
		strcpy(str, "-infinity");
	else if (TIMESTAMP_IS_NOEND(dt))
		strcpy(str, "infinity");
	else
		return false;

	return true;
}	/* EncodeSpecialTimestamp() */

inline  void dt2time(te_int64 jd, int *hour, int *min, int *sec, te_fsec_t *fsec)
{

	te_int64		time;

	time = jd;

	*hour = (int)((time / INT64CONST(3600000000)));
	time -= ((*hour) * INT64CONST(3600000000));
	*min = (int)((time / INT64CONST(60000000)));
	time -= ((*min) * INT64CONST(60000000));
	*sec = (int)((time / INT64CONST(1000000)));
	*fsec = (te_fsec_t)(time - (*sec * INT64CONST(1000000)));

	return;
}	/* dt2time() */


/*
 * timestamp2tm() - Convert timestamp data type to POSIX time structure.
 *
 * Note that year is _not_ 1900-based, but is an explicit full value.
 * Also, month is one-based, _not_ zero-based.
 * Returns:
 *	 0 on success
 *	-1 on out of range
 */
bool timestamp2tm(te_int64 dt, int *tzp, struct te_pg_tm * tm, te_fsec_t *fsec, char **tzn)
{
	te_int64 date;
	te_int64	time;

	/*
	 * If HasCTZSet is true then we have a brute force time zone
	 * specified. Go ahead and rotate to the local time zone since we will
	 * later bypass any calls which adjust the tm fields.
	 */
	//if (HasCTZSet && (tzp != NULL))
	//{
	//	dt -= CTimeZone * INT64CONST(1000000);
	//
	//}

	time = dt;

	TMODULO_INT64(time, date, INT64CONST(86400000000));

	if (time < INT64CONST(0))
	{
		time += INT64CONST(86400000000);
		date	  -=1;
	}

	/* add offset to go from J2000 back to standard Julian date */
	date	  +=POSTGRES_EPOCH_JDATE;

	/* Julian day routine does not work for negative Julian days */
	if (date <0 || date > (te_int64) INT_MAX)
		return false;

	j2date((int) date, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
	dt2time(time, &tm->tm_hour, &tm->tm_min, &tm->tm_sec, fsec);

	/* Done if no TZ conversion wanted */
	if (tzp == NULL)
	{
		tm->tm_isdst = -1;
		tm->tm_gmtoff = 0;
		tm->tm_zone = NULL;
		if (tzn != NULL)
			*tzn = NULL;
		return true;
	}

	return false;
//	return true;
}

/*******************************
 * Definicao de TePGConnection *
 *******************************/
bool TePGConnection::open(const string& str_connection)
{
	close();

        std::string test = str_connection;

        pg_connection_ = PQconnectdb(str_connection.c_str());

	if(this->state())
	{
		int_connection_ = 1;

		string off = "off";
		string answer = PQparameterStatus(pg_connection_, "integer_datetimes");

		if(answer == off)
			isDateInt_ = false;

		return true;
	}
	else
		return false;
}

bool TePGConnection::open(PGconn* conn)
{
	pg_connection_ = conn;

	if(this->state())
	{
		int_connection_ = 1;

		string off = "off";
		string answer = PQparameterStatus(pg_connection_, "integer_datetimes");

		if(answer == off)
			isDateInt_ = false;

		return true;
	}
	return false;
}

void TePGConnection::close(void)
{
 if(int_connection_)
   {
    if(pg_connection_)
      {
       PQfinish(pg_connection_);
      }
    pg_connection_ = 0;
    int_connection_ = 0;
   }
 return;
}


int TePGConnection::exec_cmd(const string& str_qry)
{
 PGresult *result_temp;
 int resp = -1;
 result_temp = PQexec(pg_connection_, str_qry.c_str());

    switch (PQresultStatus(result_temp))
	{
		case PGRES_EMPTY_QUERY: // The string sent to the backend was empty.
				break;
		case PGRES_COMMAND_OK: // Successful completion of a command returning no data
			    resp = atoi(PQcmdTuples(result_temp));
				last_oid_ = PQoidValue(result_temp);
				break;
		case PGRES_TUPLES_OK: // The query successfully executed
				break;

		case PGRES_COPY_OUT: // Copy Out (from server) data transfer started
		case PGRES_COPY_IN: // Copy In (to server) data transfer started
				break;

		case PGRES_BAD_RESPONSE: // The server's response was not understood
		case PGRES_NONFATAL_ERROR: // TODO: Is this one really an error?
		case PGRES_FATAL_ERROR:
			throw runtime_error("The server's response was not understood");

		default:
			throw logic_error("Internal lib TePGInterface error: ");
	}

 PQclear(result_temp);
 result_temp = 0;
 return resp;
}

Oid TePGConnection::insertBlob(char* buff, const int& blobSize)
{
	if(!buff)
		return 0;

	Oid obj_oid = lo_creat(pg_connection_, INV_READ | INV_WRITE);

	int fd  = lo_open(pg_connection_, obj_oid, INV_READ);

	if(fd < 0)
		return 0;

	int nbwrite = 0;

	int size_tmp = blobSize;

	while(size_tmp > 0)
	{
		if((size_tmp - BUFFSIZE) >= 0)
			nbwrite = lo_write(pg_connection_, fd, buff, BUFFSIZE);				
		else
			nbwrite = lo_write(pg_connection_, fd, buff, size_tmp);

		if(nbwrite <= 0)
			return 0;

		buff += nbwrite;
		size_tmp -= nbwrite;
	}

	if(lo_close(pg_connection_, fd) == 0)
		return obj_oid;

	return 0;
}

int TePGConnection::getBlob(const Oid& blobId, char *&buff)
{
	int fd = lo_open(pg_connection_, blobId, INV_READ);

	if(fd < 0)
		return -1;

	int len = lo_lseek(pg_connection_, fd, 0, SEEK_END);

	lo_lseek(pg_connection_, fd, 0, SEEK_SET);

	if(len <= 0)
		return len;

	if(!buff)
		buff = new char[len];

	char* buffAux = buff;

	int nbread = 0;

	while((nbread = lo_read(pg_connection_, fd, buffAux, BUFFSIZE)) > 0)
		buffAux += nbread;

	return len;
}

bool TePGConnection::beginTransaction(void)
{
	if(inTransaction_)
		return true;

	if(exec_cmd("BEGIN TRANSACTION") == 0)
	{
		inTransaction_ = true;

		return true;
	}

	return false;
}


bool TePGConnection::commitTransaction(void)
{
	if(exec_cmd("COMMIT TRANSACTION") == 0)
	{
		inTransaction_ = false;

		return true;
	}

	return false;
}

bool TePGConnection::rollBackTransaction(void)
{
	if(exec_cmd("ROLLBACK TRANSACTION") == 0)
	{
		inTransaction_ = false;

		return true;
	}

	return false;
}

int TePGConnection::exec_cmd_params(const char *command,
								int nParams,
								const Oid *paramTypes,
								const char * const *paramValues,
								const int *paramLengths,
								const int *paramFormats,
								int resultFormat)
{
	PGresult *result_temp;
	int resp = -1;
	result_temp = PQexecParams(pg_connection_,
                       command, 
                       nParams,
                       paramTypes,
                       paramValues,
                       paramLengths,
                       paramFormats,
                       resultFormat);

	//errorMessage_= PQresultErrorMessage(result_temp);
    switch (PQresultStatus(result_temp))
	{
		case PGRES_EMPTY_QUERY: // The string sent to the backend was empty.
				break;
		case PGRES_COMMAND_OK: // Successful completion of a command returning no data
			    resp = atoi(PQcmdTuples(result_temp));
				last_oid_ = PQoidValue(result_temp);
				break;
		case PGRES_TUPLES_OK: // The query successfully executed
				break;

		case PGRES_COPY_OUT: // Copy Out (from server) data transfer started
		case PGRES_COPY_IN: // Copy In (to server) data transfer started
				break;

		case PGRES_BAD_RESPONSE: // The server's response was not understood
		case PGRES_NONFATAL_ERROR: // TODO: Is this one really an error?
		case PGRES_FATAL_ERROR:
			throw runtime_error("The server's response was not understood");

		default:
			throw logic_error("Internal lib TePGInterface error: ");
	}

	PQclear(result_temp);
	result_temp = 0;
	return resp;
}

std::string TePGConnection::getClientEncoding()
{
	std::string clientEncoding = "";

	PGresult* result = PQexec(pg_connection_, "SHOW client_encoding");
	if(PQresultStatus(result) == PGRES_TUPLES_OK)
	{
		clientEncoding = PQgetvalue(result, 0, 0);  //Don't free lin because the return of PQgetvalue is in the struct PGResult
	}
	PQclear(result);
	return clientEncoding;
}


/******************************
 * Definicao de TePGRecordset *
 ******************************/
const int TePGRecordset::int_bof_ = -1;
int TePGRecordset::openRecordSets_ = 0;
std::set<int> TePGRecordset::freeCursorNumbers_;
TeMutex TePGRecordset::_mutexLock;

//const int TePGRecordset::numRowsToRetrieve_ = 2000;

TePGRecordset::TePGRecordset(const string& str_qry, TePGConnection* con_x,
			                 const TeCursorType& cursorType,
				             const TeCursorLocation& cursorLocation,
				             const TeCursorDataType& cursorDataType,
					         const int& numRowsToRetrieve)
 : pg_recordset_(0), conn_(con_x), cursorDeclared_(false), int_index_current_(-1),
   int_eof_(-1), cursorType_(cursorType), cursorLocation_(cursorLocation), 
   cursorDataType_(cursorDataType), numRowsToRetrieve_(numRowsToRetrieve)
{
	// endianness test
	if(((unsigned int) 1) == htonl((unsigned int) 1))
		isLittleEndian_ = false;	
	else
		isLittleEndian_ = true;

	recordSetId_ = TePGRecordset::getFreeCursor();

	/*if(freeCursorNumbers_.empty())
	{	// Se n�o houver cursores livres aberto => criar novo id
		++openRecordSets_;
		recordSetId_ = openRecordSets_;
	}
	else
	{
		set<int>::iterator it = freeCursorNumbers_.begin();
		recordSetId_ = *it;
		freeCursorNumbers_.erase(it);
	}*/

	if(!str_qry.empty() && conn_)
		open(str_qry, conn_, cursorType_, cursorLocation_, cursorDataType_);
}

TePGRecordset::~TePGRecordset()
{	
	// Deixa o id livre
	//freeCursorNumbers_.insert(recordSetId_);
	TePGRecordset::releaseCursor(recordSetId_);

	close();
}

void TePGRecordset::close(void)
{
	if(cursorDeclared_ && conn_)
	{
		string sqlCloseCursor  = "CLOSE CURSOR_";
	           sqlCloseCursor += Te2String(recordSetId_);

		conn_->exec_cmd(sqlCloseCursor.c_str());
	}	

	freeRecordSet();

	cursorDeclared_ = false;

	conn_ = 0;

	return;
}

bool TePGRecordset::moveFirst(void)
{
	if(!cursorDeclared_)
		return false;

	if(cursorLocation_ == TeCLIENTESIDE)
	{
		if(recordCount() > 0)
		{
			int_index_current_ = 0;
			return true;
		}
		else
			return false;
	}

	firstFetch = false;
	lastFetch = false;

	string moveFirstSQL  = "MOVE ABSOLUTE 0 IN CURSOR_";
		   moveFirstSQL += Te2String(recordSetId_);

	int res = conn_->exec_cmd(moveFirstSQL.c_str());

	if(res == 0)
	{
		freeRecordSet();

		string fetchSQL  = "FETCH FORWARD " + Te2String(numRowsToRetrieve_);
	           fetchSQL += " FROM CURSOR_";
		       fetchSQL += Te2String(recordSetId_);

		pg_recordset_ = conn_->exec(fetchSQL.c_str());

		if(PQresultStatus(pg_recordset_) == PGRES_TUPLES_OK)
		{
			if(recordCount() > 0)
			{
				int_eof_ = recordCount();
				int_index_current_ = 0;
				return true;
			}
		}
		
		freeRecordSet();
		return false;
	}
 
	return false;
}

bool TePGRecordset::movePrevious(void)
{	
	//if(!cursorDeclared_ || (cursorType_ == TeUNIDIRECTIONAL))
	if(!cursorDeclared_)
		return false;

	if(cursorLocation_ == TeCLIENTESIDE)
	{
		if(bof())
			return false;
		else
		{
			--int_index_current_;
			return true;
		}
	}

	if(recordCount() > 0)
	{
		string move = "";
		string fetchSQL = "";

		if(!movingBackward_)
		{
			 if(bof())
				 return false;
		
			--int_index_current_;

			if(bof())
			{
				movingBackward_ = true;

				int val = recordCount() - 1;

				if(lastFetch)
				{
					++val;
				}

				move = " MOVE BACKWARD "+Te2String(val)+" FROM CURSOR_"+Te2String(recordSetId_)+";";

				fetchSQL = move;
				fetchSQL += " FETCH BACKWARD ";
				fetchSQL += Te2String(numRowsToRetrieve_);
				fetchSQL += " FROM CURSOR_";
				fetchSQL += Te2String(recordSetId_);
			}
			else
			{
				return true;
			}
		}
		else
		{
			if(eof())
				 return false;
		
			++int_index_current_;

			if(eof())
			{	
				fetchSQL  = "FETCH BACKWARD ";
				fetchSQL += Te2String(numRowsToRetrieve_);
				fetchSQL += " FROM CURSOR_";
				fetchSQL += Te2String(recordSetId_);
			}
			else
			{
				return true;
			}
		}
		pg_recordset_ = conn_->exec(fetchSQL.c_str());

		if(PQresultStatus(pg_recordset_) == PGRES_TUPLES_OK)
		{			
			firstFetch = false;
			lastFetch = false;
			
			if(this->recordCount() > 0)
			{
				int_eof_ = recordCount();
				int_index_current_ = 0;

				if(this->recordCount() != numRowsToRetrieve_)
				{
					firstFetch = true;
				}


				return true;
			}
			
		}

		freeRecordSet();
 	}

	return false;
}

bool TePGRecordset::moveNext(void)
{
	if(!cursorDeclared_)
		return false;

	if(cursorLocation_ == TeCLIENTESIDE)
	{
		if(eof())
			return false;

		++int_index_current_;

		return true;
	}

	if((recordCount() > 0))
	{
		string move = "";
		string fetchSQL = "";

		if(!movingBackward_)
		{	
			if(!eof())
			{
				++int_index_current_;

				if(eof())
				{
					fetchSQL  = "FETCH FORWARD ";
					fetchSQL += Te2String(numRowsToRetrieve_);
					fetchSQL += " FROM CURSOR_";
					fetchSQL += Te2String(recordSetId_);
				}
				else
				{
					return true;
				}
			}
		}
		else
		{
			if(!bof())
			{
				--int_index_current_;

				if(bof())
				{
					movingBackward_ = false;	
					int val = recordCount() -1;
					if(firstFetch)
					{
						++val;
					}
					move = " MOVE FORWARD "+Te2String(val)+" FROM CURSOR_"+Te2String(recordSetId_)+";";

					fetchSQL = move;

					fetchSQL += "FETCH FORWARD ";
					fetchSQL += Te2String(numRowsToRetrieve_);
					fetchSQL += " FROM CURSOR_";
					fetchSQL += Te2String(recordSetId_);
				}
				else
				{
					return true;
				}
			}
		}
		freeRecordSet();

		pg_recordset_ = conn_->exec(fetchSQL.c_str());

		if(PQresultStatus(pg_recordset_) == PGRES_TUPLES_OK)
		{
			firstFetch = false;
			lastFetch = false;

			if(recordCount() > 0)
			{
				int_eof_ = recordCount();
				int_index_current_ = 0;

				if(this->recordCount() != numRowsToRetrieve_)
				{
					lastFetch = true;
				}
				
				return true;
			}
		}
		
		freeRecordSet();
	}

	return false;
}

bool TePGRecordset::moveLast(void)
{
	//if(!cursorDeclared_ || (cursorType_ == TeUNIDIRECTIONAL))
	if(!cursorDeclared_)
		return false;

	if(cursorLocation_ == TeCLIENTESIDE)
	{
		if(recordCount() > 0)
		{
			int_index_current_ = recordCount() - 1;
			return true;
		}
		
		return false;
	}

	firstFetch = false;
	lastFetch = false;

	freeRecordSet();

	string fetchSQL  = "FETCH LAST FROM CURSOR_";
		   fetchSQL += Te2String(recordSetId_);

	pg_recordset_ = conn_->exec(fetchSQL.c_str());

	if(PQresultStatus(pg_recordset_) == PGRES_TUPLES_OK)
	{
		if(this->recordCount() > 0)
		{
			int_eof_ = recordCount();
			int_index_current_ = 0;
			return true;
		}		
	}
	
	freeRecordSet();
 
	return false;
	
}

bool TePGRecordset::moveTo(const int& lin_number)
{
	//if(!cursorDeclared_ || (cursorType_ == TeUNIDIRECTIONAL))
	if(!cursorDeclared_)
		return false;

	if(cursorLocation_ == TeCLIENTESIDE)
	{
		if((lin_number > int_bof_) && (lin_number < int_eof_))
		{
			int_index_current_ = lin_number;
			return true;
		}
		
		return false;		
	}

	firstFetch = false;
	lastFetch = false;

	freeRecordSet();

	string fetchSQL  = "FETCH ABSOLUTE ";
		   fetchSQL += Te2String(lin_number);
	       fetchSQL += " FROM CURSOR_";
		   fetchSQL += Te2String(recordSetId_);

	pg_recordset_ = conn_->exec(fetchSQL.c_str());

	if(PQresultStatus(pg_recordset_) == PGRES_TUPLES_OK)
	{
		if(this->recordCount() > 0)
		{
			int_eof_ = recordCount();
			int_index_current_ = 0;
			return true;
		}
	}

	freeRecordSet();
 
	return false;	
}

bool TePGRecordset::open(const string& str_qry, TePGConnection* con_x,
			      const TeCursorType& cursorType,
				  const TeCursorLocation& cursorLocation,
				  const TeCursorDataType& cursorDataType,
				  const int& numRowsToRetrieve)
{
	close();

	if(str_qry.empty() || !con_x)
		return false;

	cursorType_ = cursorType;
	cursorLocation_ = cursorLocation;
	cursorDataType_ = cursorDataType;
	numRowsToRetrieve_ = numRowsToRetrieve;
	cursorDeclared_ = false;
	conn_ = con_x;
	movingBackward_ = false;
	lastFetch = false;
	firstFetch = false;

	string sqlCursor  = "DECLARE CURSOR_";
		   sqlCursor += Te2String(recordSetId_);
		   sqlCursor += (cursorDataType_  == TeBINARYCURSOR) ? " BINARY " : " ";
		   //sqlCursor += (cursorType_ == TeUNIDIRECTIONAL) ? " NO " : " ";
		   sqlCursor += "SCROLL CURSOR WITH HOLD FOR ";
		   sqlCursor += str_qry;	

        

	pg_recordset_ = conn_->exec(sqlCursor.c_str());

	if(PQresultStatus(pg_recordset_) == PGRES_COMMAND_OK)
	{
		cursorDeclared_ = true;

		string numRows = (cursorLocation_ == TeSERVERSIDE) ? Te2String(numRowsToRetrieve_) : string("ALL");
		
		string fetchSQL  = "FETCH FORWARD " + numRows;
	           fetchSQL += " FROM CURSOR_";
		       fetchSQL += Te2String(recordSetId_);

		PQclear(pg_recordset_);

		pg_recordset_ = conn_->exec(fetchSQL.c_str());

		if(PQresultStatus(pg_recordset_) == PGRES_TUPLES_OK)
		{
			if(recordCount() > 0)
			{
				int_eof_ = recordCount();
				int_index_current_ = 0;				
			}

			return true;
		}
	}
	
	freeRecordSet();

	return false;
}

int TePGRecordset::getBytea(const int& field_num, char*& buff) const
{
	if(cursorDataType_ == TeTEXTCURSOR)
	{
		size_t newLen;

		unsigned char* ptData = PQunescapeBytea((unsigned char*)(value(field_num)), &newLen);

		if(newLen <= 0)
			return (int)newLen;
		
		if(!buff)
			buff = new char[newLen];

		memcpy(buff, ptData, newLen);

		TePGConnection::freeMem(ptData);

		return (int)newLen;
	}
	else	// TeBINARYCURSOR
	{
		unsigned int numBytes = PQgetlength(pg_recordset_, int_index_current_, field_num);

		char* ptData = value(field_num);		

		if(!buff)
			buff = new char[numBytes];

		memcpy(buff, ptData, numBytes);

		return numBytes;
	}
}

void TePGRecordset::getByteALine2D(const int& field_num, TeLine2D& l) const
{
	char* ptDataAux = 0;

	unsigned long newLen = getBytea(field_num, ptDataAux);

	double* data = (double*) ptDataAux;

	unsigned int npts = newLen / (2 * sizeof(double));

	double x, y;

	for(unsigned int k = 0; k < npts; ++k)
	{
		x = (*data++);
		y = (*data++);
		
		TeCoord2D pt(x,y);

		l.add(pt);
	}

	delete [] ptDataAux;
}

void TePGRecordset::getPGBox(const int& field_num, TeBox& b) const
{
	char* pgbox = value(field_num);

	if(cursorDataType_ == TeBINARYCURSOR)
	{
		// POINT ARRAY IS SUPPOSED ALWAYS IN BIG ENDIAN
		BIN_PG_POINT* pts = (BIN_PG_POINT*)(pgbox);

		// endianness test
		if(isLittleEndian_)
		{
			union
			{
				double dWord_;
				unsigned int aux_[2];
			} swapx1, swapy1, swapx2, swapy2;

			// little-endian
			swapx1.dWord_ = pts[0].x;
			swapy1.dWord_ = pts[0].y;

			swapx2.aux_[1] = ntohl(swapx1.aux_[0]);
			swapx2.aux_[0] = ntohl(swapx1.aux_[1]);

			swapy2.aux_[1] = ntohl(swapy1.aux_[0]);
			swapy2.aux_[0] = ntohl(swapy1.aux_[1]);

			double X2 = swapx2.dWord_;
			double Y2 = swapy2.dWord_;

			swapx1.dWord_ = pts[1].x;
			swapy1.dWord_ = pts[1].y;

			swapx2.aux_[1] = ntohl(swapx1.aux_[0]);
			swapx2.aux_[0] = ntohl(swapx1.aux_[1]);

			swapy2.aux_[1] = ntohl(swapy1.aux_[0]);
			swapy2.aux_[0] = ntohl(swapy1.aux_[1]);

			double X1 = swapx2.dWord_;
			double Y1 = swapy2.dWord_;

			TeBox baux(X1, Y1, X2, Y2);

			b = baux;						
		}
		else
		{
			// big-endian!
			double X2 = pts[0].x;
			double Y2 = pts[0].y;
			double X1 = pts[1].x;
			double Y1 = pts[1].y;
			TeBox baux(X1, Y1, X2, Y2);

			b = baux;
		}
	}
	else	//TeTEXTCURSOR
		b = PGBoxRtree_decode(pgbox);
}

void TePGRecordset::getPGLine2D(const int& field_num, TeLine2D& l) const
{
	char* polygon = value(field_num);

	if(cursorDataType_ == TeBINARYCURSOR)
	{
		unsigned int numPts;
		memcpy(&numPts, polygon, sizeof(int));
		numPts = ntohl(numPts);

		l.reserve(numPts);

		// POINT ARRAY IS SUPPOSED ALWAYS IN BIG ENDIAN
		BIN_PG_POINT* pts = (BIN_PG_POINT*)(polygon + sizeof(int));		

		// endianness test
		if(isLittleEndian_)
		{
			union
			{
				double dWord_;
				unsigned int aux_[2];
			} swapx1, swapy1, swapx2, swapy2;

			// little-endian
			for(unsigned int i = 0; i < numPts ; ++i)
			{
				swapx1.dWord_ = pts[i].x;
				swapy1.dWord_ = pts[i].y;

				swapx2.aux_[1] = ntohl(swapx1.aux_[0]);
				swapx2.aux_[0] = ntohl(swapx1.aux_[1]);

				swapy2.aux_[1] = ntohl(swapy1.aux_[0]);
				swapy2.aux_[0] = ntohl(swapy1.aux_[1]);

				l.add(TeCoord2D(swapx2.dWord_, swapy2.dWord_));
			}
			
		}
		else
		{
			// big-endian!
			for(unsigned int i = 0; i < numPts ; ++i)
				l.add(TeCoord2D(pts[i].x, pts[i].y));
			
		}
	}
	else	//TeTEXTCURSOR
		l = PgGeomPolygon2Te(polygon);
}

char* TePGRecordset::getWKBHeader(char* v, unsigned char &byteOrder, unsigned int &wkbType, unsigned int &numGeometries, int& srid) const
{
	srid = -1;

	const int byteOrderPlusGeomType = sizeof(unsigned char) + sizeof(unsigned int);	

	if(cursorDataType_ == TeTEXTCURSOR)
	{
		unsigned char header [byteOrderPlusGeomType];

		for (int t = 0; t < byteOrderPlusGeomType; ++t)
		{
			header[t] =(unsigned char) parse_hex(v) ;
			v += 2;
		}

		memcpy(&byteOrder, header, sizeof(unsigned char));		
		memcpy(&wkbType, header + sizeof(unsigned char), sizeof(unsigned int));
	}
	else
	{
		memcpy(&byteOrder, v, sizeof(unsigned char));		
		memcpy(&wkbType, v + sizeof(unsigned char), sizeof(unsigned int));

		v += byteOrderPlusGeomType;
	}	
	

	// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
	if((byteOrder == 0) && isLittleEndian_)
	{
		wkbType = ntohl(wkbType);
	}
	else if((byteOrder == 1) && !isLittleEndian_)
	{
		wkbType = TePGSwapUInt(wkbType);
	}
	
	if(wkbType & TE_EWKB_SRID_FLAG)
	{
		if(cursorDataType_ == TeTEXTCURSOR)
		{
			unsigned char sridArray[sizeof(unsigned int)];
			for(unsigned int t = 0; t < sizeof(unsigned int); ++t)
			{
				sridArray[t] = (unsigned char)parse_hex(v);
				v += 2;
			}

			memcpy(&srid, sridArray, sizeof(unsigned int));
		}
		else
		{
			memcpy(&srid, v, 4);
			v += 4;
		}
	}

//added for EWKB compatibility
	bool isPostGIS = false;
	TeConvert2OGCWKBType(wkbType, isPostGIS);

	numGeometries = 0;

	if(wkbType > 1 && wkbType <= 7)
	{
		if(cursorDataType_ == TeTEXTCURSOR)
		{
			unsigned char qtd[sizeof(unsigned int)];

			for(unsigned int t = 0; t < sizeof(unsigned int); ++t)
			{
				qtd[t] = (unsigned char)parse_hex(v);
				v += 2;
			}

			memcpy(&numGeometries, qtd, sizeof(unsigned int));
		}
		else
		{
			memcpy(&numGeometries, v, sizeof(unsigned int));
			v += sizeof(unsigned int);
		}

		// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
		if((byteOrder == 0) && isLittleEndian_)
		{
			numGeometries = ntohl(numGeometries);	
		}
		else if((byteOrder == 1) && !isLittleEndian_)
		{
			numGeometries = TePGSwapUInt(numGeometries);
		}
	}

	return v;	
}

char* TePGRecordset::getWKBPoint(char* v, TePoint& p) const
{
	unsigned char byteOrder;
	unsigned int wkbType;
	unsigned int numGeometries;
	int srid = -1;
	TeCoord2D c;

	v = getWKBHeader(v, byteOrder, wkbType, numGeometries, srid);

	p.srid(srid);

	if(wkbType != 1)
		throw logic_error("Binary data doesn't supported!");
		

	union
	{
		double dWord_;
		unsigned int aux_[2];
	} swapx1, swapy1;

	if(cursorDataType_ == TeTEXTCURSOR)
	{

		const int double2Size = 2 * sizeof(double);
		unsigned char data[double2Size];
		
		for(int t = 0; t < double2Size; ++t) // len/2
		{
			data[t] = (unsigned char)parse_hex(v);
			v += 2;
		}
		

		memcpy(&swapx1.dWord_, data , sizeof(double));
		memcpy(&swapy1.dWord_, data + sizeof(double), sizeof(double));
	}
	else
	{
		memcpy(&swapx1.dWord_, v , sizeof(double));
		memcpy(&swapy1.dWord_, v + sizeof(double), sizeof(double));

		v += (sizeof(double) + sizeof(double)); // x + y
	}

	// 0 = Big Endian (wkbXDR)
	if((byteOrder == 0) && isLittleEndian_)
	{
		union
		{
			double dWord_;
			unsigned int aux_[2];
		} swapx2, swapy2;

		swapx2.aux_[1] = ntohl(swapx1.aux_[0]);
		swapx2.aux_[0] = ntohl(swapx1.aux_[1]);

		swapy2.aux_[1] = ntohl(swapy1.aux_[0]);
		swapy2.aux_[0] = ntohl(swapy1.aux_[1]);	
		
		c.x(swapx2.dWord_);
		c.y(swapy2.dWord_);

		p.add(c);

		return v;
	}
	else if((byteOrder == 1) && !isLittleEndian_)
	{
		union
		{
			double dWord_;
			unsigned int aux_[2];
		} swapx2, swapy2;

		swapx2.aux_[1] = TePGSwapUInt(swapx1.aux_[0]);
		swapx2.aux_[0] = TePGSwapUInt(swapx1.aux_[1]);

		swapy2.aux_[1] = TePGSwapUInt(swapy1.aux_[0]);
		swapy2.aux_[0] = TePGSwapUInt(swapy1.aux_[1]);
		
		c.x(swapx2.dWord_);
		c.y(swapy2.dWord_);

		p.add(c);

		return v;

	}

	c.x(swapx1.dWord_);
	c.y(swapy1.dWord_);

	p.add(c);

	return v;
}

char* TePGRecordset::getWKBLinearRing(char* v, int byteOrder, TeLine2D &line) const
{
	unsigned int numPoints;
	
	const int size2Double = sizeof(double) + sizeof(double); // x + y

	if(cursorDataType_ == TeTEXTCURSOR)
	{
		unsigned char data[size2Double];

		for(unsigned int n = 0; n < sizeof(unsigned int); ++n)
		{
			data[n] = (unsigned char)parse_hex(v);
			v += 2;
		}

		memcpy(&numPoints, data , sizeof(unsigned int));
	}
	else
	{
		memcpy(&numPoints, v , sizeof(unsigned int));
		v += sizeof(unsigned int);
	}

	// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
	if((byteOrder == 0) && isLittleEndian_)
	{
		numPoints = ntohl(numPoints);	
	}
	else if((byteOrder == 1) && !isLittleEndian_)
	{
		numPoints = TePGSwapUInt(numPoints);
	}

	for(unsigned int i = 0; i < numPoints; ++i)
	{
		union
		{
			double dWord_;
			unsigned int aux_[2];
		} swapx1, swapy1;

		if(cursorDataType_ == TeTEXTCURSOR)
		{
			unsigned char data[size2Double];

			for(int t = 0; t < size2Double; ++t) // len/2
			{
				data[t] = (unsigned char)parse_hex(v);
				v += 2;
			}	

			memcpy(&swapx1.dWord_, data , sizeof(double));
			memcpy(&swapy1.dWord_, data + sizeof(double), sizeof(double));
		}
		else
		{
			memcpy(&swapx1.dWord_, v , sizeof(double));
			memcpy(&swapy1.dWord_, v + sizeof(double), sizeof(double));
			v += size2Double;
		}

		// 0 = Big Endian (wkbXDR)
		if((byteOrder == 0) && isLittleEndian_)
		{
			union
			{
				double dWord_;
				unsigned int aux_[2];
			} swapx2, swapy2;

			swapx2.aux_[1] = ntohl(swapx1.aux_[0]);
			swapx2.aux_[0] = ntohl(swapx1.aux_[1]);

			swapy2.aux_[1] = ntohl(swapy1.aux_[0]);
			swapy2.aux_[0] = ntohl(swapy1.aux_[1]);	
			
			line.add(TeCoord2D(swapx2.dWord_, swapy2.dWord_));			
		}
		else if((byteOrder == 1) && !isLittleEndian_)	//1 = Little Endian (wkbNDR)
		{
			union
			{
				double dWord_;
				unsigned int aux_[2];
			} swapx2, swapy2;

			swapx2.aux_[1] = TePGSwapUInt(swapx1.aux_[0]);
			swapx2.aux_[0] = TePGSwapUInt(swapx1.aux_[1]);

			swapy2.aux_[1] = TePGSwapUInt(swapy1.aux_[0]);
			swapy2.aux_[0] = TePGSwapUInt(swapy1.aux_[1]);
			
			line.add(TeCoord2D(swapx2.dWord_, swapy2.dWord_));
		}
		else
		{
			line.add(TeCoord2D(swapx1.dWord_, swapy1.dWord_));
		}
	}

	return v;

}

char* TePGRecordset::getWKBLine(char* v, TeLine2D& l) const
{
	unsigned char byteOrder;
	unsigned int wkbType;
	unsigned int numPoints;
	int srid = -1;

	v = getWKBHeader(v, byteOrder, wkbType, numPoints, srid);
	
	if(wkbType != 2)
		throw logic_error("Binary data doesn't supported!");

	if(cursorDataType_ == TeTEXTCURSOR)
	{
		// Volta duas vezes o n�mero de bytes do n�mero de geometrias lidos ao chamar getWKBHeader
		v = getWKBLinearRing(v - (sizeof(unsigned int) + sizeof(unsigned int)), byteOrder, l);
	}
	else
	{
		// Volta o n�mero de bytes do n�mero de geometrias lidos ao chamar getWKBHeader
		v = getWKBLinearRing(v - sizeof(unsigned int), byteOrder, l);
	}

	l.srid(srid);

	return v;

}

char* TePGRecordset::getWKBPolygon(char* v, TePolygon& p) const
{
	unsigned char byteOrder;
	unsigned int wkbType;
	unsigned int numRings;
	int srid = -1;

	v = getWKBHeader(v, byteOrder, wkbType, numRings, srid);
	//TeWKBGeometryDecoder::getWKBHeader(v, byteOrder, wkbType, numRings);
	
	if(wkbType != 3)
		throw logic_error("Binary data doesn't supported!");

	for(unsigned int i = 0; i < numRings; ++i)
	{
		TeLine2D line;
		v = getWKBLinearRing(v, byteOrder, line);
		line.srid(srid);
		p.add(line);
	}

	p.srid(srid);

	return v;

}

char* TePGRecordset::getWKBMultiPoint(char* v, TePointSet &ps) const
{
	unsigned char byteOrder; 
	unsigned int wkbType; 
	unsigned int num_wkbPoints;
	int srid = -1;

	char* vCopy = getWKBHeader(v, byteOrder, wkbType, num_wkbPoints, srid);
	if(wkbType == 1)
	{
		TePoint p;
		v = getWKBPoint(v, p);
		p.srid(srid);
		ps.add(p);
		return v;
	}

	v = vCopy;

	if(wkbType != 4)
		throw logic_error("Binary data doesn't supported!");

	for(unsigned int i = 0; i < num_wkbPoints; ++i)
	{
		TePoint p;
		v = getWKBPoint(v, p);
		p.srid(srid);
		ps.add(p);
	}

	return v;
}

char* TePGRecordset::getWKBMultiLine(char* v, TeLineSet &ls) const
{
	unsigned char byteOrder; 
	unsigned int wkbType; 
	unsigned int num_wkbLineStrings;
	int srid = -1;

	char* vCopy = getWKBHeader(v, byteOrder, wkbType, num_wkbLineStrings, srid);
	if(wkbType == 2)
	{
		TeLine2D l;
		v = getWKBLine(v, l);
		l.srid(srid);
		ls.add(l);
		return v;
	}

	v = vCopy;

	if(wkbType != 5)
		throw logic_error("Binary data doesn't supported!");

	for(unsigned int i = 0; i < num_wkbLineStrings; ++i)
	{
		TeLine2D line;
		v = getWKBLine(v, line);
		line.srid(srid);
		ls.add(line);
	}

	return v;
}

char* TePGRecordset::getWKBMultiPolygon(char* v, TePolygonSet &ps) const
{
	unsigned char byteOrder;
	unsigned int wkbType;
	unsigned int num_wkbPolygons;
	int srid = -1;

	char* vCopy = getWKBHeader(v, byteOrder, wkbType, num_wkbPolygons, srid);

	if(wkbType == 3)
	{
		TePolygon p;
		v = getWKBPolygon(v, p);
		p.srid(srid);
		ps.add(p);
		return v;
	}

	v = vCopy;

	if(wkbType != 6)
		throw logic_error("Binary data doesn't supported!");

	for(unsigned int i = 0; i < num_wkbPolygons; ++i)
	{
		TePolygon poly;
		v = getWKBPolygon(v, poly);
		poly.srid(srid);
		ps.add(poly);
	}

	return v;
}

void TePGRecordset::getWKBGeomColl(char* v, TeMultiGeometry &mg) const
{
	unsigned char byteOrder; 
	unsigned int wkbType; 
	unsigned int num_wkbGeometries;
	int srid = -1;

	v = getWKBHeader(v, byteOrder, wkbType, num_wkbGeometries, srid);

	if(wkbType != 7)
		throw logic_error("Binary data doesn't supported!");

	for(unsigned int i = 0; i < num_wkbGeometries; ++i)
	{
		unsigned int geomType;
		unsigned char geomByteOrder;
		unsigned int num_geometries;
		int srid = -1;

		getWKBHeader(v, geomByteOrder, geomType, num_geometries, srid);

		if(geomType == 1)
		{
			TePoint p;

			v = getWKBPoint(v, p);

			mg.addGeometry(p);
		}
		else if(geomType == 2)
		{
			TeLine2D line;

			v = getWKBLine(v, line);

			mg.addGeometry(line);
		}
		else if(geomType == 3)
		{
			TePolygon poly;

			v = getWKBPolygon(v, poly);

			mg.addGeometry(poly);
		}
		else if(geomType == 4)
		{
			TePointSet pointSet;

			v = getWKBMultiPoint(v, pointSet);

			for(unsigned int a = 0; a < pointSet.size(); ++a)
			{
				TePoint point = pointSet[a];
				mg.addGeometry(point);
			}
		}
		else if(geomType == 5)
		{
			TeLineSet lineSet;

			v = getWKBMultiLine(v, lineSet);

			for(unsigned int a = 0; a < lineSet.size(); ++a)
			{
				TeLine2D coord = lineSet[a];

				mg.addGeometry(coord);
			}
		}
		else if(geomType == 6)
		{
			TePolygonSet polygonSet;
			
			v = getWKBMultiPolygon(v, polygonSet);
			
			for(unsigned int a = 0; a < polygonSet.size(); ++a)
			{
				TePolygon poly = polygonSet[a];

				mg.addGeometry(poly);
			}
		}
		else
		{
			throw logic_error("The data couldn't be decoded as a valid WKB geometry!");
		}
	}

}


void TePGRecordset::getPGISPoint(const int& field_num, TePoint& p) const
{
	getWKBPoint(value(field_num), p);
}


void TePGRecordset::getPGISLine(const int& field_num, TeLine2D& l) const
{
	getWKBLine(value(field_num), l);		
}


void TePGRecordset::getPGISPolygon(const int& field_num, TePolygon& p) const
{
	getWKBPolygon(value(field_num), p);		
}


void TePGRecordset::getPGISMultiPoint(const int& field_num, TePointSet& ps) const
{
	getWKBMultiPoint(value(field_num), ps);		
}


void TePGRecordset::getPGISMultiLine(const int& field_num, TeLineSet& ls) const
{
	getWKBMultiLine(value(field_num), ls);		
}


void TePGRecordset::getPGISMultiPolygon(const int& field_num, TePolygonSet& ps) const
{
	getWKBMultiPolygon(value(field_num), ps);	
}


void TePGRecordset::getPGISGeomColl(const int& field_num, TeMultiGeometry& m) const
{
	getWKBGeomColl(value(field_num), m);
}

string TePGRecordset::getDate(const int& field_num) const
{
	if(cursorDataType_ == TeTEXTCURSOR)
		return value(field_num);

// for binary cursors only
	struct te_pg_tm tt;
	struct te_pg_tm *tm = &tt;
    
	TeDateADT date = getInt(field_num);

	j2date(date + POSTGRES_EPOCH_JDATE,
		   &(tm->tm_year), &(tm->tm_mon), &(tm->tm_mday));

	char buf[MAXDATELEN + 1];
	
	string datestr = "";

	if(EncodeDateOnly(tm, buf))
        datestr = buf;
	
	return datestr;
}

string TePGRecordset::getTime(const int& field_num)
{
	if(cursorDataType_ == TeTEXTCURSOR)
	{
		string res = value(field_num);

		return res;
	}

	te_int64 time;

	if(this->conn_->isDateInt())
	{
        unsigned int h32;
		unsigned int l32;

		memcpy(&h32, value(field_num), sizeof(unsigned));
		memcpy(&l32, value(field_num) + sizeof(unsigned), sizeof(unsigned));

		h32 = ntohl(h32);
		l32 = ntohl(l32);

		time = h32;
		time <<= 32;
		time |= l32;
	}
	else
	{
		double t_aux = getDouble(field_num);

		t_aux*=1000000;
		time=(te_int64)t_aux;
	}

	struct te_pg_tm tt;
	struct te_pg_tm *tm = &tt;
	te_fsec_t fsec;

	char buf[MAXDATELEN + 1];

	string datestr = "";

	time2tm(time, tm, &fsec);
	
	if(EncodeTimeOnly(tm, fsec, NULL, buf))
        datestr = buf;
	
	return datestr;
}

string TePGRecordset::getTimeStamp(const int& field_num)
{
	if(cursorDataType_ == TeTEXTCURSOR)
	{
		string res = value(field_num);

		return res;
	}

	te_int64 timestamp;

	if(this->conn_->isDateInt())
	{
        unsigned int h32;
		unsigned int l32;

		memcpy(&h32, value(field_num), sizeof(unsigned));
		memcpy(&l32, value(field_num) + sizeof(unsigned), sizeof(unsigned));

		h32 = ntohl(h32);
		l32 = ntohl(l32);

		timestamp = h32;
		timestamp <<= 32;
		timestamp |= l32;
	}
	else
	{
		double t_aux = getDouble(field_num);

		t_aux*=1000000;
		timestamp=(te_int64)t_aux;
	}

	struct te_pg_tm tt;
	struct te_pg_tm *tm = &tt;
	te_fsec_t fsec;
	char *tzn = NULL;

	char buf[MAXDATELEN + 1];

	string datestr = "";

	if(TIMESTAMP_NOT_FINITE(timestamp))
	{
		if(EncodeSpecialTimestamp(timestamp, buf))
			datestr = buf;
	}
	else if (timestamp2tm(timestamp, NULL, tm, &fsec, NULL))
	{
		if(EncodeDateTime(tm, fsec, NULL, &tzn, buf))
			datestr = buf;
	}
        
	
	return datestr;
}

int TePGRecordset::getInt8(const int& field_num) const
{
	unsigned int h32;
	unsigned int l32;

	memcpy(&h32, value(field_num), sizeof(unsigned));
	memcpy(&l32, value(field_num) + sizeof(unsigned), sizeof(unsigned));

	h32 = ntohl(h32);
	l32 = ntohl(l32);

	te_int64 i64value = h32;
	i64value <<= 32;
	i64value |= l32;

	return (int)i64value;
}


char* TePGRecordset::getData(const int& field_num)
{
	data_ = "";

	if(cursorDataType_ == TeTEXTCURSOR)
		return value(field_num);
	else
	{
		// chamar o conversor de tipo de dado de binario p/ string p/ cada tipo!
		switch(fieldType(field_num))
		{
			case PG_NAME_TYPE			:
			case PG_TEXT_TYPE			:
			case PG_VARCHAR_TYPE		:		if(isNull(field_num))  
												{
													data_ = "";
												}
												else
												{
													return value(field_num);
												}

												break;

			case PG_INT8_TYPE			:		if(isNull(field_num))  
												{
													data_ = "";
												}
												else
												{
													data_ = Te2String(getInt8(field_num));
												}
												break;

			case PG_BOOL_TYPE			:
			case PG_BYTEA_TYPE			:
			case PG_CHAR_TYPE			:			
			case PG_INT2_TYPE			: 
											break;

			case PG_OID_TYPE			:
			case PG_INT4_TYPE			: if(isNull(field_num))  
										  {
											  data_ = "";
										  }
										  else
										  {
                                              data_ = Te2String(getInt(field_num));
										  }
										  break;			
			
			case PG_PG_FLOAT4_TYPE      :
			case PG_POINT_TYPE			:
			case PG_LSEG_TYPE			:
			case PG_PATH_TYPE			:
			case PG_BOX_TYPE			:
			case PG_POLYGON_TYPE		:
			case PG_LINE_TYPE			: 
											break;


			case PG_FLOAT8_TYPE			: if(isNull(field_num))  
										  {
											  data_ = "";
										  }
										  else
										  {
											  data_ = Te2String(getDouble(field_num), 15);
										  }
										  break;

			case PG_TIMESTAMP_TYPE		: if(isNull(field_num))  
										  {
											  data_ = "";
										  }
										  else
										  {
                                              data_ = getTimeStamp(field_num);
										  }
				                          break;

		    case PG_DATE_TYPE			: if(isNull(field_num))  
										  {
											  data_ = "";
										  }
										  else
										  {
											  data_ = getDate(field_num);
										  }
				                          break;

		    case PG_TIME_TYPE			: if(isNull(field_num))  
										  {
											  data_ = "";
										  }
										  else
										  {
											  data_ = getTime(field_num);	
										  }
										  break;


			case PG_TIMESTAMPTZ_TYPE	:			
			case PG_TIMETZ_TYPE			:
										
										  break;

			case PG_INTERVAL_TYPE		: 
										  break;

			case PG_CIRCLE_TYPE			:
			case PG_MONEY_TYPE			:
			case PG_BPCHAR_TYPE			:			
			case PG_BIT_TYPE			:
			case PG_VARBIT_TYPE			:
			case PG_NUMERIC_TYPE		:
			default						: 
											
											break;
		}
		
		return (char*)data_.c_str();
	}
}

int TePGRecordset::fieldSize(const int& field_num)
{
 Oid field_t = this->fieldType(field_num);
 Oid field_m = PQfmod(pg_recordset_, field_num);
 PGresult *result_temp;
 char str_int[5];   //integer part if DECIMAL(p,s) or NUMERIC(p,s)
 //int size = -1;
 int size = 0;
 char str_field_t[50];
 char str_field_m[50];
 sprintf(str_field_t,"%d", field_t);
 sprintf(str_field_m,"%d", field_m);
 string str_qry = "SELECT format_type(";
        str_qry += str_field_t;
        str_qry += ",";
        str_qry += str_field_m;
        str_qry += ")";
 switch(field_t)
       {
		case 1042:	// CHAR(n)
        case 1043:   //VARCHAR(s) or CHARACTER VARYING(s)
        case 1700:   //NUMERIC(p,s) or DECIMAL(p,s)
                     result_temp = PQexec(conn_->c_ptr(), str_qry.c_str());
                     if(PQresultStatus(result_temp) == PGRES_TUPLES_OK)
                       {
                        char *lin = PQgetvalue(result_temp, 0, 0);  //Don't free lin because the return of PQgetvalue is in the struct PGResult
                        int i = 0;
                        while(*lin != '\0' && *lin != ',')
                             {
                              if(*lin >= '0' && *lin <= '9' && i < 4)
                                {
                                 str_int[i] = *lin;
                                 i++;
                                }
                              lin++;
                             }
                        str_int[i]='\0';
                        size = atoi(str_int);
                       }
                     PQclear(result_temp);
                     break;
        default:     //The size of fixed size or for other variable size -1
                     return PQfsize(pg_recordset_, field_num);
       }
 result_temp = 0;
 return size;
}

int TePGRecordset::fieldSizeFractionaryPart(const int& field_num)
{
 Oid field_t = this->fieldType(field_num);
 Oid field_m = PQfmod(pg_recordset_, field_num);
 PGresult *result_temp;
 char str_frac[5];   //fractionary part if DECIMAL(p,s) or NUMERIC(p,s)
 //int size = -1;
 int size = 0;
 char str_field_t[50];
 char str_field_m[50];
 sprintf(str_field_t,"%d", field_t);
 sprintf(str_field_m,"%d", field_m);
 string str_qry = "SELECT format_type(";
        str_qry += str_field_t;
        str_qry += ",";
        str_qry += str_field_m;
        str_qry += ")";
 switch(field_t)
       {
        case 1700:   //NUMERIC(p,s) or DECIMAL(p,s)
                     result_temp = PQexec(conn_->c_ptr(), str_qry.c_str());
                     if(PQresultStatus(result_temp) == PGRES_TUPLES_OK)
                       {
                        char *lin = PQgetvalue(result_temp, 0, 0);  //Don't free lin because the return of PQgetvalue is in the struct PGResult
                        int i = 0;
                        while(*lin != '\0' && *lin != ',')
                             {
                              lin++;
                             }
                        if(*lin == ',')
                          {
                           lin++;
                          }
                        while(*lin != '\0')
                             {
                              if(*lin >= '0' && *lin <= '9' && i < 4)
                                {
                                 str_frac[i] = *lin;
                                 i++;
                                }
                              lin++;
                             }
                        str_frac[i]='\0';
                        size = atoi(str_frac);
                       }
                     PQclear(result_temp);
                     break;
        default:     //Others doesn't have a fractionary part
                     break;

       }
 result_temp = 0;
 return size;
}

void TePGRecordset::freeRecordSet()
{
	if(pg_recordset_)
		PQclear(pg_recordset_);


	pg_recordset_ = 0;
	int_index_current_ = -1;
	int_eof_ = -1;
	return;
}

char* TeLine2DToPGBinary(const TeLine2D &line, unsigned int &size)
{
	unsigned int numPts = line.size();
	size = sizeof(int) + (numPts * sizeof(double) * 2);

	char* p = new char[size];

	char* paux = p;

	//coloca o cabe�alho (Numero de pontos)
	unsigned int numPts_net = htonl(numPts);

	memcpy(p, &numPts_net, sizeof(int));
	p += sizeof(int);

	for(unsigned int i = 0; i < numPts ; ++i)
	{
		double swappedDoubleX = TeConvertToBigEndian(line[i].x());
		double swappedDoubleY = TeConvertToBigEndian(line[i].y());

		memcpy(p, &swappedDoubleX, sizeof(double));
		p += sizeof(double);
		
		memcpy(p, &swappedDoubleY, sizeof(double));
		p += sizeof(double);
	}

	return paux;
}

char* TeBoxToPGBinary(const TeBox &box, unsigned int &size)
{
	size = 4 * sizeof(double);

	char* b = new char[size];
	char* baux = b;

	double swappedDoubleURX = TeConvertToBigEndian(box.upperRight().x());
	double swappedDoubleURY = TeConvertToBigEndian(box.upperRight().y());

	double swappedDoubleLFX = TeConvertToBigEndian(box.lowerLeft().x());
	double swappedDoubleLFY = TeConvertToBigEndian(box.lowerLeft().y());

	memcpy(b, &swappedDoubleURX, sizeof(double));
	b += sizeof(double);
	
	memcpy(b, &swappedDoubleURY, sizeof(double));
	b += sizeof(double);

	memcpy(b, &swappedDoubleLFX, sizeof(double));
	b += sizeof(double);
	
	memcpy(b, &swappedDoubleLFY, sizeof(double));
	b += sizeof(double);

	return baux;
}

double TeConvertToBigEndian(const double &value)
{
	double swappedValue;

	union
	{
		double dWord_;
		unsigned int aux_[2];
	} swapIN, swapOUT;


	// endianness test
	if(((unsigned int) 1) == htonl((unsigned int) 1))
	{
		//isLittleEndian_ = false => doesn't need to swap bytes;	
		swapIN.dWord_ = value;

		memcpy(&swappedValue, &swapIN.dWord_, sizeof(double));
	}		
	else
	{
		//isLittleEndian_ = true  => swap bytes;
		swapIN.dWord_ = value;

		swapOUT.aux_[1] = htonl(swapIN.aux_[0]);
		swapOUT.aux_[0] = htonl(swapIN.aux_[1]);
	
		memcpy(&swappedValue, &swapOUT.dWord_, sizeof(double));
	}
	return swappedValue;
}

void TeConvert2OGCWKBType(unsigned int& gType, bool& isPostGIS)
{
	isPostGIS = false;
    if(gType & 0xF0000000)  // Is PostGIS?
    {
		isPostGIS = true;

		if((gType & TE_EWKB_ZM_OFFSET) == 0xC0000000) // z and m is on
		{
			gType = (gType & 0x0FFFFFFF) | 0xBB8;
		}
		else if(gType & TE_EWKB_Z_OFFSET)
		{
			gType = (gType & 0x0FFFFFFF) | 0x3E8;
		}
		else if(gType & TE_EWKB_M_OFFSET)
		{
			gType = (gType & 0x0FFFFFFF) | 0x7D0;
		}   
		else
		{
			gType = (gType & 0x0FFFFFFF);
		}
    }
}

void TeConvert2PostGISWKBType(unsigned int& gType)
{
	if((gType & 0xF00) == 0xB00)    // it is zm
	{
		gType -= 3000;
		gType |= TE_EWKB_ZM_OFFSET;
	}
	else if((gType & 0x0F00) == 0x300)
	{
		gType -= 1000;
		gType |= TE_EWKB_Z_OFFSET;
	}
	else if((gType & 0xF00) == 0x700)
	{
		gType -= 2000;
		gType |= TE_EWKB_M_OFFSET;
	}
}


char* TePointToWKBPoint(const TePoint &point, unsigned int &size)
{
	char byteOrder;
	unsigned int wkbType = 1;//WKBPoint
	int srid = point.srid();

	// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
	if(((unsigned int) 1) == htonl((unsigned int) 1))
	{
		byteOrder = 0;
	}
	else
	{
		byteOrder = 1;		
	}

	//cabecalho
	size = sizeof(char) + sizeof(int) + (2 * sizeof(double));
	if(srid != -1)
	{
		size += sizeof(int);
	}
	
	char* p = new char[size];
	char* paux = p;

	//byteOrder	
	memcpy(p, &byteOrder , sizeof(char));
	p += sizeof(char);

	//WKBPoint
	if(srid == -1)
	{
		memcpy(p, &wkbType , sizeof(unsigned int));
		p += sizeof(unsigned int);
	}
	else
	{
		TeConvert2PostGISWKBType(wkbType);

		wkbType |= TE_EWKB_SRID_FLAG;

		memcpy(p, &wkbType , sizeof(unsigned int));
		p += sizeof(unsigned int);

		memcpy(p, &srid, sizeof(int));
		p += sizeof(int);	
	}

	double x = point.location().x();
	double y = point.location().y();

	memcpy(p, &x, sizeof(double));
	p += sizeof(double);
	memcpy(p, &y, sizeof(double));
	p += sizeof(double);

	return paux;
}


char* TeLine2DToWKBLineString(const TeLine2D &line, unsigned int &size)
{
	char byteOrder;
	unsigned int wkbType = 2;//WKBLineString
	unsigned int nPoints = line.size();
	int srid = line.srid();

	// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
	if(((unsigned int) 1) == htonl((unsigned int) 1))
	{
		byteOrder = 0;
	}
	else
	{
		byteOrder = 1;		
	}

	size = sizeof(char) + sizeof(int) + sizeof(int) + (2 * sizeof(double) * nPoints);
	if(srid != -1)
	{
		size += sizeof(int);
	}
	
	char* l = new char[size];
	char* laux = l;

	//byteOrder	
	memcpy(l, &byteOrder , sizeof(char));
	l += sizeof(char);

	//WKBLineString
	if(srid == -1)
	{
		memcpy(l, &wkbType , sizeof(unsigned int));
		l += sizeof(unsigned int);
	}
	else
	{
		TeConvert2PostGISWKBType(wkbType);

		wkbType |= TE_EWKB_SRID_FLAG;

		memcpy(l, &wkbType , sizeof(unsigned int));
		l += sizeof(unsigned int);

		memcpy(l, &srid, sizeof(int));
		l += sizeof(int);	
	}

	//nPoints
	memcpy(l, &nPoints , sizeof(unsigned int));
	l += sizeof(unsigned int);

	for(unsigned int i = 0; i < nPoints ; ++i)
	{
		double x = line[i].x();
		double y = line[i].y();

		memcpy(l, &x, sizeof(double));
		l += sizeof(double);
		
		memcpy(l, &y, sizeof(double));
		l += sizeof(double);
	}

	return laux;
}


char* TePolygonToWKBPolygon(const TePolygon &polygon, unsigned int &size)
{
	char byteOrder;
	unsigned int wkbType = 3;//WKBPolygon
	unsigned int nRings = polygon.size();
	int srid = polygon.srid();

	// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
	if(((unsigned int) 1) == htonl((unsigned int) 1))
	{
		byteOrder = 0;
	}
	else
	{
		byteOrder = 1;		
	}

	//cabecalho
	size = sizeof(char) + sizeof(int) + sizeof(int);
	if(srid != -1)
	{
		size += sizeof(int);
	}
	//rings
	for(unsigned int i = 0; i < nRings ; ++i)
	{
		size += sizeof(int) + (2 * sizeof(double) * polygon[i].size());
	}
	
	char* p = new char[size];
	char* paux = p;

	//byteOrder	
	memcpy(p, &byteOrder , sizeof(char));
	p += sizeof(char);

	//WKBPolygon
	if(srid == -1)
	{
		memcpy(p, &wkbType , sizeof(unsigned int));
		p += sizeof(unsigned int);
	}
	else
	{
		TeConvert2PostGISWKBType(wkbType);

		wkbType |= TE_EWKB_SRID_FLAG;

		memcpy(p, &wkbType , sizeof(unsigned int));
		p += sizeof(unsigned int);

		memcpy(p, &srid, sizeof(int));
		p += sizeof(int);	
	}

	//nRings
	memcpy(p, &nRings , sizeof(unsigned int));
	p += sizeof(unsigned int);
	
	for(unsigned int i = 0; i < nRings ; ++i)
	{
		TeLinearRing ring = polygon[i];
		unsigned int nPoints = ring.size();

		memcpy(p, &nPoints, sizeof(int));
		p += sizeof(int);
		
		for(unsigned int j = 0; j < nPoints; ++j)
		{
			double x = ring[j].x();
			double y = ring[j].y();
		
			memcpy(p, &x, sizeof(double));
			p += sizeof(double);
			
			memcpy(p, &y, sizeof(double));
			p += sizeof(double);		
		}
	}

	return paux;
}




