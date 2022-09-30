#include <assert.h>
#include <concord/discord.h>
#include <concord/log.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define GUILD_ID 709849079315955733

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

void on_ready(struct discord *client, const struct discord_ready *event) {
    struct discord_create_guild_application_command params_1 = {
        .name = "ping", .description = "Ping command!"};

    discord_create_guild_application_command(client, event->application->id,
                                             GUILD_ID, &params_1, NULL);

    struct discord_create_guild_application_command params_2 = {
        .name = "add", .description = "Add command!"};

    discord_create_guild_application_command(client, event->application->id,
                                             GUILD_ID, &params_2, NULL);

    (void)client;
    log_info("Cog-Bot succesfully connected to Discord as %s#%s!",
             event->user->username, event->user->discriminator);
}

int parse_add_args(struct PR *pr, char *content) {
    int cont_index = 0;
    int buf_jndex = 0;
    int write_kndex = 0;

    char buff[20];
    char arr[5][20];

    while (content[cont_index] != '\0') {
        if (content[cont_index] == ' ') {
            // Word ended, clean up and copy to array
            buff[buf_jndex] = '\0';
            strcpy(arr[write_kndex], buff);
            write_kndex++;
            buf_jndex = 0;
        } else {
            // Still in word, add content char to buff
            buff[buf_jndex] = content[cont_index];
            buf_jndex++;
        }
        cont_index++;
    }

    // Deal with the last copy of buff to arr
    buff[buf_jndex] = '\0';
    strcpy(arr[write_kndex], buff);

    strcpy(pr->name, arr[0]);
    strcpy(pr->exercise, arr[1]);
    sscanf(arr[2], "%lf", &pr->weight);
    sscanf(arr[3], "%d", &pr->reps);
}

void on_interaction(struct discord *client,
                    const struct discord_interaction *event) {
    if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND)
        return; /* return if interaction isn't a slash command */

    if (strcmp(event->data->name, "ping") == 0) {
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){.content = "pong"}};
        discord_create_interaction_response(client, event->id, event->token,
                                            &params, NULL);
    } else if (strcmp(event->data->name, "add") == 0) {
        struct PR pr;
        parse_add_args(&pr, "Jake Bench 135 10");
		print_pr(&pr);
    }
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

    get(db, &prs, "Jake");
    print_prs(&prs);

    struct discord *client = discord_config_init("config.json");
    discord_set_on_ready(client, &on_ready);
    discord_set_on_interaction_create(client, &on_interaction);
    discord_run(client);

    sqlite3_close(db);

    return 0;
}
