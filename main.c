#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

enum ERR {
    OK,
    ERROR
};

void menu(sqlite3 *db);
enum ERR add_TODO(sqlite3 *db);
enum ERR mark_complete(sqlite3 *db);
enum ERR delete_TODO(sqlite3 *db);
enum ERR update_TODO(sqlite3 *db);
enum ERR list_all(sqlite3 *db);
int get_num_TODOS(sqlite3 *db);
int get_num_completed_TODOS(sqlite3 *db);

sqlite3 *setup_DB();

int main()
{
    sqlite3 *db = setup_DB();
    if (!db) {
        return 1;
    }

    menu(db);

    sqlite3_close(db);

    return 0;
}

sqlite3 *setup_DB()
{
    sqlite3 *db;
    int rc = sqlite3_open("todo.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s:%d: [ERROR] cannot open database %s\n", __FILE__, __LINE__, sqlite3_errmsg(db));
        return NULL;
    }

    return db;
}

void menu(sqlite3 *db)
{
    printf("-------------\n");
    printf("TERMINAL TODO\n\n");

    char input;
    enum ERR err;
    int count, done;
    while (1) {
        if ((count = get_num_TODOS(db)) == -1) {
            return;
        }
        if ((done = get_num_completed_TODOS(db)) == -1) {
            return;
        }
        printf("You have completed %d/%d TODOs. What do you want to do?\n", done, count);
        printf("[A]dd, [L]ist, [C]omplete, [U]pdate, [R]emove, [Q]uit\n");
        printf("> ");
        input = getchar();
        while (getchar() != '\n') {
            ;
        }
        printf("\n");
        switch (input) {
            case 'A':
            case 'a':
                if ((err = add_TODO(db)) == ERROR) {
                    return;
                }
                printf("Adding a TODO\n\n");
                break;
            case 'L':
            case 'l':
                if ((err = list_all(db)) == ERROR) {
                    return;
                }
                printf("Listing all TODO\n\n");
                break;
            case 'C':
            case 'c':
                if ((err = mark_complete(db)) == ERROR) {
                    return;
                }
                printf("Completing a TODO\n\n");
                break;
            case 'R':
            case 'r':
                if ((err = delete_TODO(db)) == ERROR) {
                    return;
                }
                printf("Removing a TODO\n\n");
                break;
            case 'U':
            case 'u':
                if ((err = update_TODO(db)) == ERROR) {
                    return;
                }
                printf("Updating a TODO\n\n");
                break;
            case 'Q':
            case 'q':
                printf("Quitting. Thank you\n");
                return;
                break;
        }
    }
}

enum ERR add_TODO(sqlite3 *db)
{
    char todo[128];
    char sql[256];

    printf("enter TODO: ");
    fgets(todo, sizeof(todo), stdin);
    todo[strcspn(todo, "\n")] = '\0';
    sprintf(sql, "insert into Todos(ID, Title, Completed) values(NULL, '%s', 0);", todo);

    char *err_msg = 0;
    int rc = sqlite3_exec(db ,sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s:%d: [ERROR] cannot write to DB %s\n", __FILE__, __LINE__, err_msg);
        return ERROR;
    }
    return OK;
}

enum ERR mark_complete(sqlite3 *db)
{
    char sql[256], mark[4];

    printf("enter ID to mark complete: ");
    fgets(mark, sizeof(mark), stdin);
    int ID = atoi(mark);
    sprintf(sql, "update Todos set Completed=1 where id=%d;", ID);

    char *err_msg = 0;
    int rc = sqlite3_exec(db ,sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s:%d: [ERROR] cannot complete TODO: %s\n", __FILE__, __LINE__, err_msg);
        return ERROR;
    }
    return OK;
}

enum ERR delete_TODO(sqlite3 *db)
{
    char sql[256], mark[4];
    char *err_msg;

    printf("enter id to delete: ");
    fgets(mark, sizeof(mark), stdin);
    int ID = atoi(mark);
    sprintf(sql, "delete from Todos where ID=%d;", ID);

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s:%d: [ERROR] cannot delete TODO: %s\n", __FILE__, __LINE__, err_msg);
        return ERROR;
    }
    return OK;
}

enum ERR update_TODO(sqlite3 *db)
{
    char sql[256], mark[4], TODO[128];
    char *err_msg;
    printf("enter ID to edit: ");
    fgets(mark, sizeof(mark), stdin);
    int ID = atoi(mark);

    printf("enter new title for TODO: ");
    fgets(TODO, sizeof(TODO), stdin);
    TODO[strcspn(TODO, "\n")] = '\0';

    sprintf(sql, "update Todos set Title='%s' where ID=%d;", TODO, ID);

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s:%d: [ERROR] cannot delete TODO: %s\n", __FILE__, __LINE__, err_msg);
        return ERROR;
    }
    return OK;
}

enum ERR list_all(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    const char *sql = "select * from Todos;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s:%d: [ERROR] cannot read DB: %s\n", __FILE__, __LINE__, sqlite3_errmsg(db));
        return ERROR;
    }

    printf("%-5s%-35s%s\n", "ID", "Title", "Completed");
    printf("-----------------------------------------------\n");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) 
    {
        int ID = sqlite3_column_int(stmt, 0);
        char *TODO = (char *)sqlite3_column_text(stmt, 1);
        if (!TODO) {
            TODO = "NULL";
        }
        int completed = sqlite3_column_int(stmt, 2);
        char *done = "done";
        if (!completed) {
            done = "not";
        }
        printf("%-5d %-35s %s\n", ID, TODO, done);
        printf("-----------------------------------------------\n");
    }
    return OK;
}

int get_num_TODOS(sqlite3 *db)
{
    int count = 0;
    sqlite3_stmt *stmt;
    const char *sql = "select count(*) from Todos;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s:%d: [ERROR] cannot get count form DB: %s\n", __FILE__, __LINE__, sqlite3_errmsg(db));
        return -1;
    }

    while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
        count = sqlite3_column_int(stmt, 0);
    }

    return count;
}

int get_num_completed_TODOS(sqlite3 *db)
{
    int count = 0;
    sqlite3_stmt *stmt;
    const char *sql = "select * from Todos where Completed=1;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s:%d: [ERROR] cannot get count form DB: %s\n", __FILE__, __LINE__, sqlite3_errmsg(db));
        return -1;
    }

    while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
        count = sqlite3_column_int(stmt, 0);
    }

    return count;
}

