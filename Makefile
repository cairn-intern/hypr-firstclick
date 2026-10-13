# Else exist specifically for clang
ifeq ($(CXX),g++)
    EXTRA_FLAGS = --no-gnu-unique
else
    EXTRA_FLAGS =
endif

# Locally extracted Hyprland headers (Ubuntu hyprland-dev package) are used
# when system headers are absent. Set PKG_CONFIG_PATH to their pkgconfig dir.
all:
	$(CXX) -shared -fPIC $(EXTRA_FLAGS) main.cpp -o firstclick.so -g `pkg-config --cflags pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon` -std=c++23 -Wall -Wextra -Wno-unused-parameter -Wno-narrowing

clean:
	rm -f ./firstclick.so

install: all
	install -Dm755 firstclick.so $(HOME)/.local/lib/hyprland-plugins/firstclick.so

.PHONY: all clean install
