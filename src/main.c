#include <sqlite3.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdbool.h>

bool file_exists(char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int main() {
    sqlite3 *db;
    int rc;
    char *zErrMsg = 0;
    char database_name[] = "test.db";

    bool exist_before_connect = file_exists(database_name);
    rc = sqlite3_open(database_name, &db);

    if (rc) {
        printf("Can't open database: %s\n", sqlite3_errmsg(db));
        return (0);
    } else {
        printf("Opened database successfully\n");
    }

    sqlite3_close(db);

    printf("We Go Jim!\n");
    return 0;
}
