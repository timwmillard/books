
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

schema: src/schema.h

src/schema.h: sql/schema.sql
	xxd -i sql/schema.sql | sed 's/};/, 0x00\n};/' > src/schema.h

update-deps: update-sokol update-cimgui
	wget -O deps/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

update-sokol:
	wget -O deps/sokol_app.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_app.h
	wget -O deps/sokol_audio.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_audio.h
	wget -O deps/sokol_gfx.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_gfx.h
	wget -O deps/sokol_glue.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_glue.h
	wget -O deps/sokol_time.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_time.h
	wget -O deps/sokol_log.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_log.h
	wget -O deps/sokol_imgui.h https://github.com/floooh/sokol/raw/refs/heads/master/util/sokol_imgui.h

update-cimgui: update-imgui
	wget -O deps/cimgui/cimconfig.h https://raw.githubusercontent.com/cimgui/cimgui/refs/heads/docking_inter/cimconfig.h
	wget -O deps/cimgui/cimgui.cpp https://raw.githubusercontent.com/cimgui/cimgui/refs/heads/docking_inter/cimgui.cpp
	wget -O deps/cimgui/cimgui.h https://raw.githubusercontent.com/cimgui/cimgui/refs/heads/docking_inter/cimgui.h

update-imgui:
	wget -O deps/cimgui/imgui/imconfig.h https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imconfig.h
	wget -O deps/cimgui/imgui/imgui.h https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imgui.h
	wget -O deps/cimgui/imgui/imgui.cpp https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imgui.cpp
	wget -O deps/cimgui/imgui/imgui_demo.cpp https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imgui_demo.cpp
	wget -O deps/cimgui/imgui/imgui_draw.cpp https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imgui_draw.cpp
	wget -O deps/cimgui/imgui/imgui_internal.h https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imgui_internal.h
	wget -O deps/cimgui/imgui/imgui_tables.cpp https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imgui_tables.cpp
	wget -O deps/cimgui/imgui/imgui_widgets.cpp https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imgui_widgets.cpp
	wget -O deps/cimgui/imgui/imstb_rectpack.h https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imstb_rectpack.h
	wget -O deps/cimgui/imgui/imstb_textedit.h https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imstb_textedit.h
	wget -O deps/cimgui/imgui/imstb_truetype.h https://raw.githubusercontent.com/ocornut/imgui/44aa9a4b3a6f27d09a4eb5770d095cbd376dfc4b/imstb_truetype.h

update-sqlite3:
	wget -O deps/sqlite-amalgamation.zip https://sqlite.org/2025/sqlite-amalgamation-3500400.zip
	unzip -j deps/sqlite-amalgamation.zip -d deps/
	rm deps/sqlite-amalgamation.zip


# Helpful utilities
db: FORCE
	rlwrap --always-readline sqlite3 accounting.book

db-setup: FORCE
	sqlite3 accounting.book < sql/setup.sql

db-reset: FORCE
	rm accounting.book

FORCE:

