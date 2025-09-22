#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "sokol_imgui.h"

#include "sqlite3.h"
#include "schema.h"

/* Models */
typedef struct {
    char name[256];
} Business;

typedef struct {
    Business *business;
} Data;

static struct {
    sg_pass_action pass_action;
    bool show_window;
    bool show_demo;

    sqlite3 *db;

    Data data;
} state = {0};

void draw_ui(void)
{
    if (igBeginMainMenuBar()) {
        if (igBeginMenu("File", true)) {
            if (igMenuItem_Bool("New", "Ctrl+N", false, true)) {
                state.show_window = true;
                // Handle new file
            }
            if (igMenuItem_Bool("Open", "Ctrl+O", false, true)) {
                // Handle open file
            }
            if (igMenuItem_Bool("Save", "Ctrl+S", false, true)) {
                // Handle save file
            }
            igSeparator();
            if (igMenuItem_Bool("Exit", "Alt+F4", false, true)) {
                sapp_quit();
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

    ImGuiID dockspace_id = igGetID_Str("MainDockSpace");
    igDockSpaceOverViewport(dockspace_id, igGetMainViewport(), ImGuiDockNodeFlags_None, NULL);

    if (state.show_demo) {
        igShowDemoWindow(&state.show_window);
    }

    if (state.show_window) {
        if (state.show_window && igBegin("Window", &state.show_window, ImGuiWindowFlags_None)) {

        }
        igEnd();
    }

    if (state.data.business == NULL) {
        // Allocate business object so we can use its name buffer
        state.data.business = malloc(sizeof(Business));
        memset(state.data.business->name, 0, sizeof(state.data.business->name));
        igOpenPopup_Str("Business Setup", ImGuiPopupFlags_None);
    }

    // Business setup modal
    if (igBeginPopupModal("Business Setup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        igText("Please enter your business name:");
        igInputText("##business_name", state.data.business->name, sizeof(state.data.business->name), ImGuiInputTextFlags_None, NULL, NULL);

        igSeparator();

        if (igButton("Create", (ImVec2){80, 0})) {
            if (strlen(state.data.business->name) > 0) {
                sqlite3_stmt *stmt;
                char *sql = "INSERT INTO business (name) VALUES (?)";

                if (sqlite3_prepare_v2(state.db, sql, -1, &stmt, NULL) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, state.data.business->name, -1, SQLITE_STATIC);
                    int result = sqlite3_step(stmt);
                    if (result != SQLITE_DONE) {
                        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(state.db));
                    } else {
                        printf("Business '%s' inserted successfully\n", state.data.business->name);
                    }
                    sqlite3_finalize(stmt);
                } else {
                    fprintf(stderr, "Unable to prepare statement: %s\n", sqlite3_errmsg(state.db));
                }
                igCloseCurrentPopup();
            }
        }
        igSameLine(0, -1);
        if (igButton("Cancel", (ImVec2){80, 0})) {
            free(state.data.business);
            state.data.business = NULL;
            igCloseCurrentPopup();
        }

        igEndPopup();
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
    state.show_window = true;

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
}

sapp_desc sokol_main(int argc, char* argv[]) {
    char *db_name = "accouting.book";
    if (argc > 1) {
        db_name = argv[1];
    }
    int rc = sqlite3_open(db_name, &state.db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(state.db));
        sqlite3_close(state.db);
        sapp_quit();
    }

    // Execute embedded schema
    char *err_msg = NULL;
    rc = sqlite3_exec(state.db, (char*)sql_schema_sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Schema execution failed: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    char *window_title = malloc(64);
    snprintf(window_title, 64, "Books - %s", db_name);
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
