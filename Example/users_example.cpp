#include "users.hpp"
#include "users_dao.hpp"

#include "libSqliteWrapped.h"

/**
 *	SQLiteWrapped Modern Examples as used in Oblivion/2 XRM BBS Software
 *  Michael Griffin (c) 2015-2018 Free for use and modification
 *  mrmisticismo@hotmail.com
 *
 *  Below are some basic examples for calling the user_dao (Data Access Object).
 *  You can also use this, along with the users_dao and base_dao as references for
 *  writting queries and basic ORM operations.
 *
 *  For more example you can always browse Oblivion/2 XRM BBS Software
 *  https://github.com/M-griffin/Oblivion2-XRM
 *
 *  NOTE: I just through this together as a quick sample, let me know if there are compile issues.
 */


int main()
{
	// Setup a logger and create the initial handle to the database
	// If it doesn't exist it will create one.  if no path is given it will use the current working directory.
    SQLW::StderrLog log;
    SQLW::Database db("test_database.db", &log);

    // Handle to the Database Access Object
    users_dao_ptr dao(new UsersDao(db));



    // Check if it Exists.
    bool exist = dao->isTableExists();
    if (exist)
        std::cout << "table exist" << std::endl;
    else
    {
        std::cout << "table doesn't exist" << std::endl;

         // Setup database Param, cache sies etc..
		dao->firstTimeSetupParams());
    }

    // Create Table using Transaction
    exist = dao->createTable();
    if (exist)
        std::cout << "table created" << std::endl;
    else
        std::cout << "table not created" << std::endl;

    // Check if Exists.
    exist = dao->isTableExists();
    if (exist)
        std::cout << "table exist" << std::endl;
    else
        std::cout << "table doesn't exist" << std::endl;

    // Drop Table using Transaction
    exist = dao->dropTable();
    if (exist)
        std::cout << "table was dropped" << std::endl;
    else
        std::cout << "table was not dropped" << std::endl;


    // Check if Exists.
    exist = dao->isTableExists();
    if (exist)
        std::cout << "table exist" << std::endl;
    else
        std::cout << "table doesn't exist" << std::endl;


    // Create Temp User Record.
    user_ptr usr(new Users());

    // Test update query string creation.
    dao->insertUserRecord(usr);

	// Get a Vector filled with all user records
	std::vector<user_ptr> listOfAllUsers dao->getAllRecords();

	// Record Counts in table
    long checkCounts = objdb->getRecordsCount();

	// Test update query string creation.
	usr->sHandle = "New Name";
    dao->updateUserRecord(usr);

	return 0;
}

