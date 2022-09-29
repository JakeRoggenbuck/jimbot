#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

struct PR {
    char name[100];
    char exercise[100];
    double weight;
    int reps;
};

struct PRS {
    struct PR prs[10];
    int index;
};

void print_pr(struct PR *pr) {
    printf("%s %s %lf %d\n", pr->name, pr->exercise, pr->weight, pr->reps);
}

void print_prs(struct PRS *prs) {
    for (int i = 0; i < prs->index; i++) {
        print_pr(&prs->prs[i]);
    }
}

bool file_exists(char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

static int get_callback(void *data, int argc, char **argv, char **az_col_name) {
    struct PRS *prs = (struct PRS *)data;

    struct PR new;
    strcpy(new.name, argv[1]);
    strcpy(new.exercise, argv[2]);

    sscanf(argv[3], "%lf", &new.weight);
    sscanf(argv[4], "%d", &new.reps);

    prs->prs[prs->index] = new;
    prs->index++;

    return 0;
}

static int callback(void *data, int argc, char **argv, char **az_col_name) {
    int i;
    fprintf(stderr, "%s: ", (const char *)data);

    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", az_col_name[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}

int create_database_table(sqlite3 *db) {
    int rc;
    char *error_message;
    char *sql = "CREATE TABLE PRS("
                "ID INT PRIMARY KEY,"
                "NAME           TEXT    NOT NULL,"
                "EXERCISE       TEXT	NOT NULL,"
                "WEIGHT         REAL    NOT NULL,"
                "REPS        	INT		NOT NULL);";

    rc = sqlite3_exec(db, sql, callback, 0, &error_message);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", error_message);
        sqlite3_free(error_message);
    } else {
        fprintf(stdout, "Operation done successfully.\n");
    }
    return !rc;
}

int add(sqlite3 *db, char *name, char *exercise, double weight, int reps) {
    int rc;
    char *error_message;
    char sql[300];

    sprintf(sql,
            "INSERT INTO PRS (ID,NAME,EXERCISE,WEIGHT,REPS) VALUES"
            "(NULL, '%s', '%s', %.2lf, %d);",
            name, exercise, weight, reps);

    rc = sqlite3_exec(db, sql, callback, 0, &error_message);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", error_message);
        sqlite3_free(error_message);
    } else {
        fprintf(stdout, "Operation done successfully.\n");
    }
    return !rc;
}

int get(sqlite3 *db, struct PRS *prs, char *name) {
    int rc;
    char *error_message;
    char sql[300];

    sprintf(sql, "SELECT * FROM PRS WHERE NAME='%s'", name);

    rc = sqlite3_exec(db, sql, get_callback, (void *)prs, &error_message);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", error_message);
        sqlite3_free(error_message);
    } else {
        fprintf(stdout, "Operation done successfully.\n");
    }
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

        add(db, "Jake", "Bench", 135, 10);
    }

    struct PRS prs;
    prs.index = 0;

    add(db, "Jake", "Bench", 145, 34);
    get(db, &prs, "Jake");
    print_prs(&prs);

    sqlite3_close(db);

    printf("We Go Jim!\n");
    return 0;
}
