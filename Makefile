
all: cmake-build

run: cmake-build
	@./build/books

cmake-build: build
	@cmake --build build

# build directory
build:
	mkdir -p build
	cmake -B build

clean:
	rm -rf build


update-deps: update-cimgui
	wget -O deps/sokol_app.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_app.h
	wget -O deps/sokol_audio.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_audio.h
	wget -O deps/sokol_gfx.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_gfx.h
	wget -O deps/sokol_glue.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_glue.h
	wget -O deps/sokol_time.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_time.h
	wget -O deps/sokol_log.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_log.h
	wget -O deps/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
	wget -O deps/sokol_imgui.h https://github.com/floooh/sokol/raw/refs/heads/master/util/sokol_imgui.h

update-cimgui: update-imgui
	wget -O deps/cimgui/cimconfig.h https://raw.githubusercontent.com/cimgui/cimgui/refs/heads/docking_inter/cimconfig.h
	wget -O deps/cimgui/cimgui.cpp https://raw.githubusercontent.com/cimgui/cimgui/refs/heads/docking_inter/cimgui.cpp
	wget -O deps/cimgui/cimgui.h https://raw.githubusercontent.com/cimgui/cimgui/refs/heads/docking_inter/cimgui.h

update-imgui:
	wget -O deps/cimgui/imgui/imconfig.h https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imconfig.h
	wget -O deps/cimgui/imgui/imgui.h https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imgui.h
	wget -O deps/cimgui/imgui/imgui.cpp https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imgui.cpp
	wget -O deps/cimgui/imgui/imgui_demo.cpp https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imgui_demo.cpp
	wget -O deps/cimgui/imgui/imgui_draw.cpp https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imgui_draw.cpp
	wget -O deps/cimgui/imgui/imgui_internal.h https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imgui_internal.h
	wget -O deps/cimgui/imgui/imgui_tables.cpp https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imgui_tables.cpp
	wget -O deps/cimgui/imgui/imgui_widgets.cpp https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imgui_widgets.cpp
	wget -O deps/cimgui/imgui/imstb_rectpack.h https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imstb_rectpack.h
	wget -O deps/cimgui/imgui/imstb_textedit.h https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imstb_textedit.h
	wget -O deps/cimgui/imgui/imstb_truetype.h https://raw.githubusercontent.com/ocornut/imgui/refs/heads/docking/imstb_truetype.h

