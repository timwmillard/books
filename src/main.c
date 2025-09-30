#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "sokol_imgui.h"

#include "sqlite3.h"
#include "schema.h"

/*** Dynamic Array ***/
// typedef void *(Allocator)(void *ptr, size_t size);

// stretchy buffer
// init: NULL 
// free: array_free() 
// push_back: array_push() 
// size: array_count() 
#define array_free(a)         ((a) ? free(array__raw(a)),0 : 0)
#define array_push(a,v)       (array__maybegrow(a,1), (a)[array__n(a)++] = (v))
#define array_count(a)        ((a) ? array__n(a) : 0)
#define array_capacity(a)     ((a) ? array__m(a) : 0)
#define array_add(a,n)        (array__maybegrow(a,n), array__n(a)+=(n), &(a)[array__n(a)-(n)])
#define array_last(a)         ((a)[array__n(a)-1])

#include <stdlib.h>
#define array__raw(a) ((int *) (a) - 2)
#define array__m(a)   array__raw(a)[0]
#define array__n(a)   array__raw(a)[1]

#define array__needgrow(a,n)  ((a)==0 || array__n(a)+n >= array__m(a))
#define array__maybegrow(a,n) (array__needgrow(a,(n)) ? array__grow(a,n) : 0)
#define array__grow(a,n)  array__growf((void **) &(a), (n), sizeof(*(a)))

static void array__growf(void **arr, int increment, int itemsize)
{
	int m = *arr ? 2*array__m(*arr)+increment : increment+1;
	void *p = reallocf(*arr ? array__raw(*arr) : 0, itemsize * m + sizeof(int)*2);
	assert(p);
	if (p) {
		if (!*arr) ((int *) p)[1] = 0;
		*arr = (void *) ((int *) p + 2);
		array__m(*arr) = m;
	}
}

/*** Areana's ***/
static Arena arena = {0};

/*** Error Handling ***/

typedef struct {
    int code;
    const char *msg;
} Error;

static Error Ok = {0};

Error error(int code, const char *msg) {return (Error){code, msg};}

/*** Models ***/

#define MAX_TEXT_LEN 256

typedef struct {
    int id;
    char name[MAX_TEXT_LEN];
} Business;

typedef enum {
    ASSET,
    LIABILITY,
    EQUITY,
    REVENUE,
    EXPENSE
} AccountType;

typedef struct Account {
    int id;
    AccountType type;
    char name[MAX_TEXT_LEN];
    char description[MAX_TEXT_LEN];
    int parent_id;
    struct Account *parent;
} Account;

typedef struct {
    Account *items;
    size_t count;
    size_t capacity;
} ChartOfAccounts;

typedef struct AccountNode {
    int id;
    AccountType type;
    char name[MAX_TEXT_LEN];
    char description[MAX_TEXT_LEN];

    struct AccountNode *children;
} AccountNode;

typedef struct {
    AccountNode *asset;
    AccountNode *liability;
    AccountNode *equity;
    AccountNode *revenue;
    AccountNode *expense;

} AccountsTree;

typedef struct {
    int id;
    int account_id;
    char account_name[MAX_TEXT_LEN];
    int debit;
    int credit;
} JournalLine;

typedef struct {
    int id;
    char description[MAX_TEXT_LEN];
    JournalLine *items;
    size_t count;
    size_t capacity;
} JournalEntry;

typedef struct {
    JournalEntry *items;
    size_t count;
    size_t capacity;
} Ledger;

typedef struct {
    Business business;
    Ledger ledger;
    ChartOfAccounts accounts;
    AccountsTree accounts_tree;

    Arena ledger_arena;
    Arena accounts_arena;
} Data;

/*** Global State ***/
static struct {
    sg_pass_action pass_action;
    bool show_accounts;
    bool show_business;
    bool show_general_ledger;

    bool show_demo;
    bool dock_setup_done;

    sqlite3 *db;

    Data data;
} state = {0};

Error db_open(char *name)
{
    int rc = sqlite3_open(name, &state.db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(state.db));
        sqlite3_close(state.db);
        return error(rc,  "open database error");
    }

    // Execute embedded schema
    char *err_msg = NULL;
    rc = sqlite3_exec(state.db, (char*)sql_schema_sql_data, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Schema execution failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        return error(rc, "schema execution failed");
    }
    return Ok;
}

static int load_business_cb(void *NotUsed, int argc, char **argv, char **azColName)
{
    for (int i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "name") == 0) {
            strncpy(state.data.business.name, argv[i], MAX_TEXT_LEN);
        }
    }
    return 0;
}

static int load_ledger_cb(void *NotUsed, int argc, char **argv, char **azColName)
{
    JournalEntry entry = {0};
    JournalLine line = {0};

    for (int i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "line_id") == 0 && argv[i]) {
            line.id = atoi(argv[i]);
        }
        if (strcmp(azColName[i], "journal_id") == 0 && argv[i]) {
            entry.id = atoi(argv[i]);
        }
        if (strcmp(azColName[i], "account_id") == 0 && argv[i]) {
            line.account_id = atoi(argv[i]);
        }
        if (strcmp(azColName[i], "account_name") == 0) {
            strncpy(line.account_name, argv[i], MAX_TEXT_LEN);
        }
        if (strcmp(azColName[i], "description") == 0) {
            strncpy(entry.description, argv[i], MAX_TEXT_LEN);
        }
        if (strcmp(azColName[i], "debit") == 0 && argv[i]) {
            line.debit = atoi(argv[i]);
        }
        if (strcmp(azColName[i], "credit") == 0 && argv[i]) {
            line.credit = atoi(argv[i]);
        }
    }

    JournalEntry *entry_ptr = NULL;
    for (int i = 0; i < state.data.ledger.count; i++) {
        if (state.data.ledger.items[i].id == entry.id) {
            entry_ptr = &state.data.ledger.items[i];
            break;
        }
    }
    if (entry_ptr == NULL) {
        arena_da_append(&state.data.ledger_arena, &state.data.ledger, entry);
        entry_ptr = &state.data.ledger.items[state.data.ledger.count - 1];
    }
    arena_da_append(&state.data.ledger_arena, entry_ptr, line);

    return 0;
}

void db_list_ledger()
{
    arena_reset(&state.data.ledger_arena);
    memset(&state.data.ledger, 0, sizeof(state.data.ledger));

    int rc;
    char *zErrMsg = 0;
    char *sql = "select * from general_ledger";
    rc = sqlite3_exec(state.db, sql, load_ledger_cb, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

static int load_account_cb(void *NotUsed, int argc, char **argv, char **azColName)
{
    Account account = {0};
    for (int i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "id") == 0 && argv[i]) {
            account.id = atoi(argv[i]);
        }
        if (strcmp(azColName[i], "type") == 0 && argv[i]) {
            if (strcmp(argv[i], "asset") == 0)
                account.type = ASSET;
            else if (strcmp(argv[i], "liability") == 0)
                account.type = LIABILITY;
            else if (strcmp(argv[i], "equity") == 0)
                account.type = EQUITY;
            else if (strcmp(argv[i], "revenue") == 0)
                account.type = REVENUE;
            else if (strcmp(argv[i], "expence") == 0)
                account.type = EXPENSE;
        }
        if (strcmp(azColName[i], "parent_id") == 0 && argv[i]) {
            account.parent_id = atoi(argv[i]);
        }
        if (strcmp(azColName[i], "name") == 0) {
            strncpy(account.name, argv[i], MAX_TEXT_LEN);
        }
        if (strcmp(azColName[i], "description") == 0) {
            strncpy(account.description, argv[i], MAX_TEXT_LEN);
        }
    }

    for (int i = 0; i < state.data.accounts.count; i++) {
        if (state.data.accounts.items[i].id == account.parent_id) {
            account.parent = &state.data.accounts.items[i];
            break;
        }
    }
    arena_da_append(&state.data.accounts_arena, &state.data.accounts, account);
    return 0;
}

void db_list_accounts()
{
    arena_reset(&state.data.accounts_arena);
    memset(&state.data.accounts, 0, sizeof(state.data.accounts));

    int rc;
    char *zErrMsg = 0;
    char *sql = "select * from account";
    rc = sqlite3_exec(state.db, sql, load_account_cb, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    for (int i = 0; i < state.data.accounts.count; i++) {
        Account account = state.data.accounts.items[i];
        AccountNode node = {0};
        node.id = account.id,
        strncpy(node.name, account.name, MAX_TEXT_LEN);
        strncpy(node.description, account.description, MAX_TEXT_LEN);
        if (account.type == ASSET && account.parent_id == 0)
            array_push(state.data.accounts_tree.asset, node);
        else if (account.type == LIABILITY && account.parent_id == 0)
            array_push(state.data.accounts_tree.liability, node);
        else if (account.type == EQUITY && account.parent_id == 0)
            array_push(state.data.accounts_tree.equity, node);
        else if (account.type == REVENUE && account.parent_id == 0)
            array_push(state.data.accounts_tree.revenue, node);
        else if (account.type == EXPENSE && account.parent_id == 0)
            array_push(state.data.accounts_tree.expense, node);
    }
}

void db_get_business()
{
    int rc;
    char *zErrMsg = 0;
    char *sql = "select * from business order by id desc limit 1";
    rc = sqlite3_exec(state.db, sql, load_business_cb, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

void db_save_business()
{
    sqlite3_stmt *stmt;
    char *sql = "insert or replace into business (id, name) values (1, ?)";

    if (sqlite3_prepare_v2(state.db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, state.data.business.name, -1, SQLITE_STATIC);
        int result = sqlite3_step(stmt);
        if (result != SQLITE_DONE) {
            fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(state.db));
        } else {
            printf("Business '%s' updated successfully\n", state.data.business.name);
        }
        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Unable to prepare statement: %s\n", sqlite3_errmsg(state.db));
    }
}

void ui_reset_layout(ImGuiID dockspace_id)
{
    state.show_accounts = true;
    state.show_business = true;
    state.show_general_ledger = true;

    igDockBuilderRemoveNode(dockspace_id);
    igDockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    igDockBuilderSetNodeSize(dockspace_id, igGetMainViewport()->Size);

    igDockBuilderDockWindow("Business Details", dockspace_id);
    igDockBuilderDockWindow("Chart of Accounts", dockspace_id);
    igDockBuilderDockWindow("General Ledger", dockspace_id);
    igDockBuilderDockWindow("Dear ImGui Demo", dockspace_id);
    igDockBuilderFinish(dockspace_id);
}

void draw_ui(void)
{
    ImGuiID dockspace_id = igGetID_Str("MainDockSpace");

    // Main Menu
    if (igBeginMainMenuBar()) {
        if (igBeginMenu("File", true)) {
            if (igMenuItem_Bool("New", "Ctrl+N", false, true)) {
                // Handle new file
            }
            if (igMenuItem_Bool("Open", "Ctrl+O", false, true)) {
                // Handle open file
            }
            if (igMenuItem_Bool("Save", "Ctrl+S", false, true)) {
                // Handle save file
            }
            igSeparator();
            if (igMenuItem_Bool("Import", "", false, true)) {
            }
            if (igMenuItem_Bool("Export", "", false, true)) {
            }
            igSeparator();
            if (igMenuItem_Bool("Quit", "Ctrl+Q", false, true)) {
                sapp_quit();
            }
            igEndMenu();
        }
        if (igBeginMenu("Accounting", true)) {
            if (igMenuItem_Bool("Journal Entry", "Ctrl+J", false, true)) {
            }
            if (igMenuItem_Bool("Chart of Accounts", "", false, true)) {
                state.show_accounts = true;
                igSetWindowFocus_Str("Chart of Accounts");
            }
            if (igMenuItem_Bool("General Ledger", "", false, true)) {
                state.show_general_ledger = true;
                db_list_ledger();
                igSetWindowFocus_Str("General Ledger");
            }
            if (igMenuItem_Bool("Bank Reconciliation", "", false, true)) {
            }
            igEndMenu();
        }
        if (igBeginMenu("Reports", true)) {
            if (igMenuItem_Bool("Profit & Loss", "", false, true)) {
            }
            if (igMenuItem_Bool("Balance Sheet", "", false, true)) {
            }
            if (igMenuItem_Bool("Cash Flow", "", false, true)) {
            }
            igEndMenu();
        }
        if (igBeginMenu("Business", true)) {
            if (igMenuItem_Bool("Details", "", false, true)) {
                state.show_business = true;
                igSetWindowFocus_Str("Business Details");
            }
            igEndMenu();
        }
        if (igBeginMenu("View", true)) {
            if (igMenuItem_Bool("Reset Layout", "", false, true)) {
                ui_reset_layout(dockspace_id);
            }
            igSeparator();
            if (igMenuItem_Bool("Zoom In", "", false, true)) {
            }
            if (igMenuItem_Bool("Zoom Out", "", false, true)) {
            }
            if (igMenuItem_Bool("Reset Zoom", "", false, true)) {
            }
            igEndMenu();
        }
        if (igBeginMenu("Debug", true)) {
            if (igMenuItem_Bool("Demo", "", false, true)) {
                state.show_demo = true;
            }
            igEndMenu();
        }
        igEndMainMenuBar();
    }

    // Dockspace
    igDockSpaceOverViewport(dockspace_id, igGetMainViewport(), ImGuiDockNodeFlags_None, NULL);

    // Setup docking layout on first frame
    if (!state.dock_setup_done) {
        ui_reset_layout(dockspace_id);

        state.dock_setup_done = true;
    }

    // Chart of Accounts Window
    if (state.show_accounts) {
        if (igBegin("Chart of Accounts", &state.show_accounts, ImGuiWindowFlags_None)) {
            if (igTreeNode_Str("Assets")) {
                igSeparatorText("Current Assets");
                igText("Cash");
                igText("Accounts Receivable");
                igTreePop();
            }
            if (igTreeNode_Str("Liabilities")) {
                igSeparatorText("Current Liabilities");
                igText("Accounts Payable");
                igTreePop();
            }
            if (igTreeNode_Str("Revenue")) {
                igTreePop();
            }
            if (igTreeNode_Str("Expenses")) {
                igTreePop();
            }
            if (igTreeNode_Str("Owner's Equity")) {
                igTreePop();
            }
        }
        igEnd();
    }
    // Business Details Window
    if (state.show_business) {
        if (igBegin("Business Details", &state.show_business, ImGuiWindowFlags_None)) {
            igText("Please enter your business name:");
            igInputText("##business_name", state.data.business.name, sizeof(state.data.business.name), ImGuiInputTextFlags_None, NULL, NULL);

            igSeparator();

            if (igButton("Save", (ImVec2){80, 0})) {
                db_save_business();
            }
            igSameLine(0, -1);
            if (igButton("Reset", (ImVec2){80, 0})) {
                db_get_business();
                // state.show_business = false;
            }
        }
        igEnd();
    }

    if (state.show_general_ledger) {
        if (igBegin("General Ledger", &state.show_general_ledger, ImGuiWindowFlags_None)) {
            if (igBeginTable("ledger", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable, (ImVec2){0}, 0)) {
                igTableSetupColumn("Description", 0, 0, 0);
                igTableSetupColumn("Account ID", 0, 0, 0);
                igTableSetupColumn("Account", 0, 0, 0);
                igTableSetupColumn("Debit", 0, 0, 0);
                igTableSetupColumn("Credit", 0, 0, 0);

                igTableHeadersRow();
                for (int e = 0; e < state.data.ledger.count; e++) {
                    int col = -1;
                    JournalEntry *entry = &state.data.ledger.items[e];
                    igTableNextRow(0, 30.0f);
                    igTableSetColumnIndex(++col);
                    igText("%s", entry->description);
                    for (int l = 0; l < entry->count; l++) {
                        int col = -1;
                        JournalLine *line = &entry->items[l];
                        igPushID_Int(e+l);

                        igTableNextRow(0, 0.0f);

                        igTableSetColumnIndex(++col);
                        // if (l==0) {
                        //     igText("%s", entry->description);
                        //     // igSmallButton("Edit");
                        // }

                        igTableSetColumnIndex(++col);
                        igText("%d", line->account_id);

                        igTableSetColumnIndex(++col);
                        igText("%s", line->account_name);

                        igTableSetColumnIndex(++col);
                        if (line->debit > 0) {
                            float column_width = igGetColumnWidth(col);
                            char amount_str[32];
                            float amount =  line->debit/100.0f;
                            snprintf(amount_str, sizeof(amount_str), "$%.2f", amount);
                            ImVec2 text_size;
                            igCalcTextSize(&text_size, amount_str, NULL, false, -1.0f);
                            float cursor_x = igGetCursorPosX();
                            igSetCursorPosX(cursor_x + column_width - text_size.x);
                            igText("%s", amount_str);
                        }

                        igTableSetColumnIndex(++col);
                        if (line->credit > 0) {
                            float column_width = igGetColumnWidth(col);
                            char amount_str[32];
                            float amount =  line->credit/100.0f;
                            snprintf(amount_str, sizeof(amount_str), "$%.2f", amount);
                            ImVec2 text_size;
                            igCalcTextSize(&text_size, amount_str, NULL, false, -1.0f);
                            float cursor_x = igGetCursorPosX();
                            igSetCursorPosX(cursor_x + column_width - text_size.x);
                            igText("%s", amount_str);
                        }
                        igPopID();
                    }
                }
                igEndTable();
            }
        }
        igEnd();
    }

    // Demo Window
    if (state.show_demo) {
        igShowDemoWindow(&state.show_demo);
    }
}

void frame(void)
{
    simgui_new_frame(&(simgui_frame_desc_t){
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = sapp_frame_duration(),
        .dpi_scale = sapp_dpi_scale(),
    });

    draw_ui();

    sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });
    simgui_render();
    sg_end_pass();
    sg_commit();
}

void event(const sapp_event *ev)
{
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN && ev->key_code == SAPP_KEYCODE_ESCAPE) {
        sapp_quit();
    }
    simgui_handle_event(ev);
}

void init(void)
{
    state.show_accounts = true;
    state.show_business = true;
    state.show_general_ledger = true;

    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });
    simgui_setup(&(simgui_desc_t){ 0 });

    // Enable docking
    ImGuiIO* io = igGetIO_Nil();
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // initial clear color
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.0f, 0.5f, 1.0f, 1.0 } }
    };
}


void cleanup(void) {
    simgui_shutdown();
    sg_shutdown();

    arena_free(&arena);
}

sapp_desc sokol_main(int argc, char* argv[]) {
    char *db_name = "accounting.book";
    if (argc > 1) {
        db_name = argv[1];
    }

    Error err;

    err = db_open(db_name);
    if (err.code) {
        fprintf(stderr, "Open database error: %s\n", err.msg);
        sapp_quit();
    }
    db_get_business();
    db_list_ledger();
    db_list_accounts();

    char *window_title = arena_sprintf(&arena,  "Books - %s", db_name);
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 1200,
        .height = 800,
        .window_title = window_title,
    };
}
