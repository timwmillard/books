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

    ImGuiID dockspace_id = igGetID_Str("MyDockSpace");
    ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_KeepAliveOnly;
    // ImGuiDockNodeFlags_PassthruCentralNode
    igDockSpaceOverViewport(dockspace_id, igGetMainViewport(), dock_flags, NULL);
    //

    if (state.show_window) {
        igShowDemoWindow(&state.show_window);
    }

    // bool my_tool_active;
    // // Create a window called "My First Tool", with a menu bar.
    // igBegin("My First Tool", &my_tool_active, ImGuiWindowFlags_MenuBar);
    // if (igBeginMenuBar())
    // {
    //     if (igBeginMenu("File", true))
    //     {
    //         if (igMenuItem_Bool("Open..", "Ctrl+O", false, true)) { /* Do stuff */ }
    //         if (igMenuItem_Bool("Save", "Ctrl+S", false, true))   { /* Do stuff */ }
    //         if (igMenuItem_Bool("Close", "Ctrl+W", false, true))  { my_tool_active = false; }
    //         igEndMenu();
    //     }
    //     igEndMenuBar();
    // }
    // igEnd();

    if (state.show_window) {
        if (igBegin("Window", &state.show_window, ImGuiWindowFlags_None)) {

            igEnd();
        }
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
