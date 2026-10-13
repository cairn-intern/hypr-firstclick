# hypr-firstclick

A Hyprland plugin that makes the first click on an unfocused window only focus it, the way macOS behaves. The click is not delivered to the window, so it cannot press a button, follow a link, or place a text cursor by accident. The matching mouse release is swallowed as well, so applications never see a release without a press.

Built for Hyprland 0.53.3 (Ubuntu 26.04 package). Hyprland has no setting for this behaviour, so it is implemented with the cancellable `mouseButton` plugin hook.

## Behaviour

- Left, right, and middle clicks on an unfocused window focus it and go no further. The next click works normally.
- Clicks with any modifier held pass through untouched, so Super+drag window moves, Ctrl+click, and mouse keybinds still work on the first press.
- Clicks on the bar, wallpaper, and other layer surfaces pass through. Only clicks that land on an unfocused window are affected.
- Set `enabled = false` in the config below to turn the behaviour off without unloading the plugin.

## Config

Add to `hyprland.conf`:

```ini
exec-once = hyprctl plugin load $HOME/.local/lib/hyprland-plugins/firstclick.so

plugin {
    firstclick {
        enabled = true
    }
}
```

## Build

The plugin must be built against headers that exactly match the running Hyprland, or it refuses to load with a version mismatch. On this machine the Ubuntu `hyprland-dev` package is downloaded and extracted locally instead of installed:

```bash
cd /tmp
apt download hyprland-dev libpango1.0-dev libharfbuzz-dev libgraphite2-dev \
  libhyprcursor-dev libxcb-xfixes0-dev libxcb-icccm4-dev libxcb-composite0-dev \
  libxcb-res0-dev libxcb-errors-dev libxcb-shape0-dev libfribidi-dev \
  libthai-dev libdatrie-dev libxft-dev
```

Extract the packages, point the `.pc` prefixes at the extraction root, then:

```bash
export PKG_CONFIG_PATH=$HOME/source/hyprland-headers/pkgconfig:$HOME/source/pango-headers/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig
make install
hyprctl plugin load $HOME/.local/lib/hyprland-plugins/firstclick.so
hyprctl plugin list
```

A Hyprland upgrade breaks the plugin until it is rebuilt against the new headers. That is normal for Hyprland plugins.

## License

Copyright © 2026 euxaristia. Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE) for the full text.
