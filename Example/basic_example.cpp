#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string>

#include "Database.h"
#include "Query.h"

int main()
{
    Database db( "database_file.db" );
    Query q(db);

    q.execute("delete from user");
    q.execute("insert into user values(1,'First Person')");
    q.execute("insert into user values(2,'Another Person')");

    q.get_result("select num,name from user");
    while (q.fetch_row())
    {
        long num = q.getval();
        std::string name = q.getstr();

        printf("User#%ld: %s\n", num, name.c_str() );
    }
    q.free_result();
}
