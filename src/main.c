#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

bool file_exists(char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

static int callback(void *not_used, int argc, char **argv, char **az_col_name) {
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", az_col_name[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}

int create_database_table(sqlite3 *db) {
    int rc;
    char *error_message;
    char *sql = "CREATE TABLE PRS("
                "ID INT PRIMARY KEY     NOT NULL,"
                "NAME           TEXT    NOT NULL,"
                "EXERCISE       TEXT	NOT NULL,"
                "WEIGHT         REAL    NOT NULL,"
                "REPS        	INT		NOT NULL);";

    rc = sqlite3_exec(db, sql, callback, 0, &error_message);
    return !rc;
}

int add(char *name, char *exercise, double weight, int reps, sqlite3 *db) {
    int rc;
    char *error_message;
    char sql[300];

    sprintf(sql,
            "INSERT INTO PRS (ID,NAME,EXERCISE,WEIGHT,REPS) VALUES"
            "(0, '%s', '%s', %.2lf, %d);",
            name, exercise, weight, reps);

    rc = sqlite3_exec(db, sql, callback, 0, &error_message);
    return !rc;
}

int main() {
    sqlite3 *db;
    int rc;
    char database_name[] = "test.db";

    bool exist_before_connect = file_exists(database_name);
    rc = sqlite3_open(database_name, &db);

    if (rc) {
        printf("Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    } else {
        printf("Opened database successfully.\n");
    }

    if (!exist_before_connect) {
        if (!create_database_table(db)) {
            printf("Could not create table.\n");
            return 1;
        }
    }

    if (!add("Jake", "Bench", 135, 10, db)) {
        printf("Error adding to database.\n");
    }

    sqlite3_close(db);

    printf("We Go Jim!\n");
    return 0;
}
