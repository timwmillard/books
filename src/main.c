#include <stdio.h>
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "sokol_imgui.h"

static struct {
    sg_pass_action pass_action;
    bool show_window;
    bool show_demo;
} state = {0};

void frame(void)
{
    simgui_new_frame(&(simgui_frame_desc_t){
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = sapp_frame_duration(),
        .dpi_scale = sapp_dpi_scale(),
    });

    /*=== UI CODE STARTS HERE ===*/
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

    ImGuiID dockspace_id = igGetID_Str("MyDockSpace");
    ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_None;
    igDockSpaceOverViewport(dockspace_id, igGetMainViewport(), dock_flags, NULL);

    if (state.show_demo) {
        igShowDemoWindow(&state.show_window);
    }

    if (state.show_window) {
        if (state.show_window && igBegin("Window", &state.show_window, ImGuiWindowFlags_None)) {

        }
        igEnd();
    }


    /*=== UI CODE ENDS HERE ===*/

    // the sokol_gfx draw pass
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
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 1200,
        .height = 800,
        .window_title = "Books",
    };
}
