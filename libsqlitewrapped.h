/**
 *	IError.h
 *
 * Rewritten / author: 2016-02-19 / mrmisticismo@hotmail.com
 * Published / author: 2005-08-12 / grymse@alhem.net
 * Copyright (C) 2015-2016  Michael Griffin
 * Copyright (C) 2001-2006  Anders Hedstrom
 * This program is made available under the terms of the GNU GPL.
 */

#ifndef _IERROR_H_SQLITE
#define _IERROR_H_SQLITE

#include <string>

namespace SQLW
{
    class Database;
    class Query;

    /** Log class interface. */
    class IError
    {
    public:
        virtual void databaseError(Database&, const std::string&) = 0;
        virtual void databaseError(Database&, Query&, const std::string&) = 0;
    };

} // namespace SQLW {

#endif // _IERROR_H
/**
 *	StderrLog.h
 *
 * Rewritten / author: 2016-02-19 / mrmisticismo@hotmail.com
 * Published / author: 2005-08-12 / grymse@alhem.net
 * Copyright (C) 2015-2016  Michael Griffin
 * Copyright (C) 2001-2006  Anders Hedstrom
 * This program is made available under the terms of the GNU GPL.
 */

#ifndef _STDERRLOG_H_SQLITE
#define _STDERRLOG_H_SQLITE

#include "IError.h"
#include <string>

namespace SQLW
{
    class Database;

    /** Log class writing to standard error. */
    class StderrLog : public IError
    {
    public:
        void databaseError(Database&, const std::string&);
        void databaseError(Database&, Query&, const std::string&);
    };

} // namespace SQLW {


#endif // _STDERRLOG_H
/**
 *	SysLog.h
 *
 * Rewritten / author: 2016-02-19 / mrmisticismo@hotmail.com
 * Published / author: 2005-08-12 / grymse@alhem.net
 * Copyright (C) 2015-2016  Michael Griffin
 * Copyright (C) 2001-2006  Anders Hedstrom
 * This program is made available under the terms of the GNU GPL.
 */

#ifndef _SYSLOG_H_SQLITE
#define _SYSLOG_H_SQLITE
#ifndef _WIN32

#include <syslog.h>


namespace SQLW
{
    /** Log class writing to syslog. */
    class SysLog : public IError
    {
    public:
        SysLog(const std::string& = "database", int = LOG_PID, int = LOG_USER);
        virtual ~SysLog();

        void databaseError(Database&, const std::string&);
        void databaseError(Database&, Query&, const std::string&);

    };

} // namespace SQLW {

#endif // WIN32
#endif // _SYSLOG_H
/**
 *	Database.h
 *
 * Rewritten / author: 2016-02-19 / mrmisticismo@hotmail.com
 * Published / author: 2005-08-12 / grymse@alhem.net
 * Copyright (C) 2015-2016  Michael Griffin
 * Copyright (C) 2001-2006  Anders Hedstrom
 * This program is made available under the terms of the GNU GPL.
 */

#ifndef _DATABASE_H_SQLITE
#define _DATABASE_H_SQLITE

#include <sqlite3.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <string>
#include <list>
#include <stdint.h>

namespace SQLW
{
    // handle unused paramters for template funcations.
    #define UNUSED(x) [&x]{}()

    // Forward Declarations
    class IError;
    class Query;
    class Mutex;

    /** Connection information and pool. */
    class Database
    {
    public:
        /** Mutex container class, used by Lock. ingroup threading */

        class Mutex
        {
        public:

            Mutex();
            ~Mutex();
            void MutexLock();
            void Unlock();

        private:

// Replace this with std::mutex!
#ifdef _WIN32
            HANDLE          m_mutex;
#else
            pthread_mutex_t m_mutex;
#endif
        };
    private:

        // Mutex helper class.
        class MutexLock
        {
        public:
            MutexLock(Mutex& mutex, bool use);
            ~MutexLock();
        private:
            Mutex& m_mutex;
            bool m_is_inuse;
        };

    public:

        // Connection pool struct.
        struct DatabasePool
        {
            DatabasePool() : busy(false) {}
            sqlite3 *db;
            bool busy;
        };
        typedef std::list<DatabasePool *> m_database_pool;

    public:

        // use file
        Database(const std::string& database, IError *databaseError = nullptr);

        // Use file + thread safe
        Database(Mutex&, const std::string& database, IError *databaseError = nullptr);

        virtual ~Database();

        /** try to establish connection with given host */
        bool isConnected();

        void errorHandler(IError *);
        void databaseError(Query&,const char *format, ...);
        void databaseError(Query&,const std::string&);

        /** Request a database connection.
        The "grabdb" method is used by the Query class, so that each object instance of Query gets a unique
        database connection. I will re-implement your connection check logic in the Query class, as that's where
        the database connection is really used.
        It should be used something like this.
        {
        	Query q(db);
        	if (!q.isConnected())
            {
        		 return false;
            }
        	q.execute("delete * from user"); // well maybe not
        }

        When the Query object is deleted, then "freeDatabasePool" is called - the database connection stays open in the
        m_opendbs vector. New Query objects can then reuse old connections.
        */

        DatabasePool *addDatabasePool();

        void freeDatabasePool(DatabasePool *odb);

    	// Escape string - change all ' to ''.
    	std::string safestr(const std::string& );
		
        // Make string xml safe.
        std::string xmlsafestr(const std::string&);

        // Convert string to 64-bit integer.
        int64_t a2bigint(const std::string&);

        // Convert string to unsigned 64-bit integer.
        uint64_t a2ubigint(const std::string&);

    private:

        Database(const Database&) : m_mutex(m_mutex) {}

        Database& operator=(const Database&)
        {
            return *this;
        }

        void databaseError(const char *format, ...);

        std::string         m_database;
        m_database_pool     m_opendbs;
        IError             *m_errhandler;
        bool                m_embedded;
        Mutex&              m_mutex;
        bool                m_is_mutex;
    };



} // namespace SQLW


#endif // _DATABASE_H
/*
 **	Query.h
 **
 **	Published / author: 2005-08-12 / grymse@alhem.net
 **/

/*
Copyright (C) 2001-2006  Anders Hedstrom

This program is made available under the terms of the GNU GPL.

If you would like to use this program in a closed-source application,
a separate license agreement is available. For information about
the closed-source license agreement for this program, please
visit http://www.alhem.net/sqlwrapped/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _QUERY_H_SQLITE
#define _QUERY_H_SQLITE

#include <sqlite3.h>

#include <iostream>
#include <ctime>
#include <string>
#include <map>
#include <vector>

#include <stdint.h>


namespace SQLW
{
    /** SQL Statement execute / result. */
    class Query
    {
    public:
        /** Constructor accepting reference to database object. */
        Query(Database& dbin);
        /** Constructor accepting reference to database object
        	and query string to execute. */
        Query(Database& dbin,const std::string& sql);
        ~Query();

        /** Check if database object is connectable. */
        bool isConnected();
        /** Return reference to database object. */
        Database& getDatabase() const;

        /** Return string containing last query executed. */
        const std::string& getLastQuery();

        /** execute() returns true if query is successful,
        	does not store result. */
        bool execute(const std::string& sql);

        /** Execute query and store result. */
        sqlite3_stmt *getResult(const std::string& sql);
        /** Free stored result, must be called after get_result() before calling
        	execute()/get_result() again. */

        void freeResult();
        /** Fetch next result row.
        	\return false if there was no row to fetch (end of rows) */

        // Fill the Resource with the current row, also incriments to next row.
        bool fetchRow();

        /** Get id of last insert. */
        sqlite_int64 getInsertId();

        /** Returns 0 if there are no rows to fetch. */
        long getNumRows();

        /** Number of columns in current result. */
        int getNumCols();

        /** Last error string. */
        std::string GetError();

        /** Last error code. */
        int GetErrno();

        /** Check if column x in current row is null. */
        bool isNull(int x);

        /**
         * @brief Execute Query and Get first Result as the following.
         */

        /** Execute query and return first result as a string. */
        const char *exeGetCharString(const std::string& sql);

        /** Execute query and return first result as a long integer. */
        long exeGetResultLong(const std::string& sql);

        /** Execute query and return first result as a double. */
        double exeGetResultDouble(const std::string& sql);

        /**
         * @brief On FetchRow, get the current column values as.
         */

        /** Return column named x as a string value. */
        const char *getstr(const std::string& x);

        /** Return column x as a string value. */
        const char *getstr(int x);

        /** Return next column as a string value - see rowcount. */
        const char *getstr();

        /**
         * @brief On FetchRow, get the current column values as.
         */

        /** Return column named x as a long integer. */
        long getval(const std::string& x);

        /** Return column x as a long integer. */
        long getval(int x);

        /** Return next column as a long integer - see rowcount. */
        long getval();

        /**
         * @brief On FetchRow, get the current column values as.
         */

        /** Return column named x as an unsigned long integer. */
        unsigned long getuval(const std::string& x);

        /** Return column x as an unsigned long integer. */
        unsigned long getuval(int x);

        /** Return next column as an unsigned long integer. */
        unsigned long getuval();

        /**
         * @brief On FetchRow, get the current column values as.
         */

        /** Return column named x as a 64-bit integer value. */
        int64_t getbigint(const std::string& x);

        /** Return column x as a 64-bit integer value. */
        int64_t getbigint(int x);

        /** Return next column as a 64-bit integer value. */
        int64_t getbigint();

        /** Return column named x as an unsigned 64-bit integer value. */
        uint64_t getubigint(const std::string& x);

        /** Return column x as an unsigned 64-bit integer value. */
        uint64_t getubigint(int x);

        /** Return next column as an unsigned 64-bit integer value. */
        uint64_t getubigint();

        /**
         * @brief On FetchRow, get the current column values as.
         */

        /** Return column named x as a double. */
        double getnum(const std::string& x);

        /** Return column x as a double. */
        double getnum(int x);

        /** Return next column as a double. */
        double getnum();

        /**
         * @brief On FetchRow, Get Field Value by Column Name and Type
         */

        // Int
        int getFieldValue(int type, int index)
        {
            UNUSED(type);
            int result = static_cast<int>(sqlite3_column_int(res, index));
            return result;
        }

        // Long or Time_T for Date/Time.
        long getFieldValue(long type, int index)
        {
            UNUSED(type);
            long result = static_cast<long>(sqlite3_column_int64(res, index));
            return result;
        }

        // Long Long
        long long getFieldValue(long long type, int index)
        {
            UNUSED(type);
            long long result = static_cast<long long>(sqlite3_column_int64(res, index));
            return result;
        }

        // Double
        double getFieldValue(double type, int index)
        {
            UNUSED(type);
            double result = static_cast<double>(sqlite3_column_double(res, index));
            return result;
        }

        // Float
        float getFieldValue(float type, int index)
        {
            UNUSED(type);
            float result = static_cast<float>(sqlite3_column_double(res, index));
            return result;
        }

        // Long Double
        long double getFieldValue(long double type, int index)
        {
            UNUSED(type);
            long double result = static_cast<long double>(sqlite3_column_double(res, index));
            return result;
        }

        // Std::String
        std::string getFieldValue(std::string type, int index)
        {
            UNUSED(type);

            std::string newString = "";
            try
            {
                const char *text_result = reinterpret_cast<const char *>(sqlite3_column_text(res , index));
                newString = text_result;
                return newString;
            }
            catch (std::exception ex)
            {
                std::string message = "string getFieldValue: ";
                message.append(ex.what());
                queryError(message.c_str());
            }
            return newString;
        }

        // Single Character
        char getFieldValue(char type, int index)
        {
            UNUSED(type);
            const char *result = reinterpret_cast<const char *>(sqlite3_column_text(res , index));
            return result[0];
        }

        // Char *, or BLOB. sqlite3_column_blob ?  test this lateron!
        const char *getFieldValue(char *type, int index)
        {
            UNUSED(type);
            /*
            if (strcmp(type, "BLOB") == 0)
            {
                // Test if we need to get len in bytes then cutoff!
                const char *result = reinterpret_cast<const char *>(sqlite3_column_blob(res , index));
                return result;
            }
            else
            {*/
            const char *result = reinterpret_cast<const char *>(sqlite3_column_text(res , index));
            return result;
            //}
            //return "\0";
        }

        // Handle Boolean Values.
        bool getFieldValue(bool type, int index)
        {
            UNUSED(type);
            int result = static_cast<int>(sqlite3_column_int(res , index));
            return (result == 1) ? true : false;
        }

        /**
        * @brief Template to Get Field Value by Column Name and Type.
        */
        template <typename TT, typename T>
        T getFieldByName(TT tt, T t)
        {
            // Grab the index of the Matching Field Name
            int index = m_nmap[tt] - 1;

            T result;
            switch(sqlite3_column_type(res, index))
            {
                case SQLITE_NULL:
                    return t;
                    break;

                default :
                    // Template to pull field value dynamically from overloads.
                    result = getFieldValue(t, index);
                    break;
            }
            return result;
        }

    private:
        /** Hide the copy constructor. */
        Query(const Query& q)
            : m_db(q.getDatabase())
        {
            std::cout << "Query Constructor: q.getDatabase()" << std::endl;
        }

        /** Hide the assignment operator. */
        Query& operator=(const Query&)
        {
            std::cout << "Query Constructor: return *this" << std::endl;
            return *this;
        }

        /** Print current result to stdout. */
        void printResults();

        /** Print error to debug class. */
        void queryError(const std::string&);


        Database&                  m_db;           ///< Reference to database object
        Database::DatabasePool    *odb;            ///< Connection pool handle

        sqlite3_stmt              *res;            ///< Stored result
        bool                       row;            ///< true if fetch_row succeeded
        short                      rowcount;       ///< Current column pointer in result
        std::string                m_tmpstr;       ///< Used to store result in get_string() call
        std::string                m_last_query;   ///< Last query executed
        int                        cache_rc;       ///< Cached result after call to get_result()
        bool                       cache_rc_valid; ///< Indicates cache_rc is valid
        int                        m_row_count;    ///< 0 if get_result() returned no rows

        std::map<std::string, int> m_nmap;         ///< map translating column names to index
        int                        m_num_cols;     ///< number of columns in result

    };

} // namespace SQLW


#endif // _QUERY_H
