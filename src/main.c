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

typedef struct {
    int id;
    AccountType type;
    char name[MAX_TEXT_LEN];
    char description[MAX_TEXT_LEN];
    int parent_id;
} Account;

typedef struct {
    char account_name[MAX_TEXT_LEN];
    char description[MAX_TEXT_LEN];
    int debit;
    int credit;
} LedgerRow;

typedef struct {
    LedgerRow *items;
    size_t count;
    size_t capacity;
} Ledger;

typedef struct {
    Business business;
    Ledger ledger;

    Arena arena;
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
    LedgerRow row;

    for (int i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "account_name") == 0) {
            strncpy(row.account_name, argv[i], MAX_TEXT_LEN);
        }
        if (strcmp(azColName[i], "description") == 0) {
            strncpy(row.description, argv[i], MAX_TEXT_LEN);
        }
        if (strcmp(azColName[i], "debit") == 0) {
            row.debit = atoi(argv[i]);
        }
        if (strcmp(azColName[i], "credit") == 0) {
            row.credit = atoi(argv[i]);
        }
    }

    arena_da_append(&state.data.arena, &state.data.ledger, row);
    return 0;
}

void db_list_ledger()
{
    int rc;
    char *zErrMsg = 0;
    char *sql = "select * from general_ledger";
    rc = sqlite3_exec(state.db, sql, load_ledger_cb, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
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
            if (igBeginTable("ledger", 4, 0, (ImVec2){0}, 0)) {
                igTableSetupColumn("Account", 0, 0, 0);
                igTableSetupColumn("Description", 0, 0, 0);
                igTableSetupColumn("Debit", 0, 0, 0);
                igTableSetupColumn("Credit", 0, 0, 0);

                igTableHeadersRow();
                for (int row = 0; row < state.data.ledger.count; row++) {
                    igTableNextRow(0, 0);

                    igTableSetColumnIndex(0);
                    igText("%s", state.data.ledger.items[row].account_name);

                    igTableSetColumnIndex(1);
                    igText("%s", state.data.ledger.items[row].description);

                    igTableSetColumnIndex(2); // Amount column
                    {
                        float column_width = igGetColumnWidth(2);
                        char amount_str[32];
                        float amount =  state.data.ledger.items[row].debit/100.0f;
                        snprintf(amount_str, sizeof(amount_str), "$%.2f", amount);
                        ImVec2 text_size;
                        igCalcTextSize(&text_size, amount_str, NULL, false, -1.0f);
                        float cursor_x = igGetCursorPosX();
                        igSetCursorPosX(cursor_x + column_width - text_size.x);
                        igText("%s", amount_str);
                    }

                    igTableSetColumnIndex(3); // Amount column
                    {
                        float column_width = igGetColumnWidth(3);
                        char amount_str[32];
                        float amount =  state.data.ledger.items[row].credit/100.0f;
                        snprintf(amount_str, sizeof(amount_str), "$%.2f", amount);
                        ImVec2 text_size;
                        igCalcTextSize(&text_size, amount_str, NULL, false, -1.0f);
                        float cursor_x = igGetCursorPosX();
                        igSetCursorPosX(cursor_x + column_width - text_size.x);
                        igText("%s", amount_str);
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
