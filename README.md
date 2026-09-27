# Focus Clock

Bigass clock that sits on top of all windows to help you focus.

![Screenshot](./.github/screenshot.png)

A fork of [KorigamiK/focusclock](https://github.com/KorigamiK/focusclock) that
adds an optional date line under the time. See [Date line](#date-line).

## Install

This fork is not published to any package repository. The upstream routes below
give you a clock **without** the date line:

| Route | Includes the date line? |
| --- | --- |
| AUR `focusclock-git` | No — builds upstream |
| Homebrew `korigamik/tap/focusclock` | No — upstream |
| Upstream [releases](https://github.com/KorigamiK/focusclock/releases) | No — upstream binaries |

Build it from source instead.

## Building

Requires gtkmm4, gtk4-layer-shell, CMake and pkg-config. If gtk4-layer-shell is
not found the build still succeeds, but the clock cannot float above other
windows and `--layer` is unavailable.

On Arch / CachyOS:

```sh
sudo pacman -S cmake pkgconf gtkmm-4.0 gtk4-layer-shell
```

On Debian / Ubuntu the equivalents are `cmake`, `pkg-config`,
`libgtkmm-4.0-dev` and `libgtk4-layer-shell-dev`.

Then build and run:

```sh
git clone https://github.com/Golden-Icon/focusclock.git
cd focusclock
cmake -B build
cmake --build build
./build/focusclock --anchor-right --anchor-bottom
```

## Keybinds

To toggle the clock from your window manager, bind a key to kill and restart
it. In a Hyprland config, for example:

```
bind = $mainMod ALT, M, exec, killall -SIGTERM focusclock || focusclock -br -B 70
```

## Usage

You can configure the position of the clock using the following command-line
options:

```sh
Usage:
  focusclock [OPTION…]

Help Options:
  -h, --help              Show help options

Application Options:
  -v, --version           Show version information
  -t, --anchor-top        Anchor to the top edge
  -b, --anchor-bottom     Anchor to the bottom edge
  -l, --anchor-left       Anchor to the left edge
  -r, --anchor-right      Anchor to the right edge
  -T, --margin-top        Margin from the top edge
  -B, --margin-bottom     Margin from the bottom edge
  -L, --margin-left       Margin from the left edge
  -R, --margin-right      Margin from the right edge
  -f, --font-size         Base font size
  -c, --color             Text color (hex format: RGB, RGBA, RRGGBB, or RRGGBBAA)
  -D, --show-date         Show the date below the time (default: true)
  --no-date               Hide the date line, leaving a time-only clock
  -d, --date-format       strftime format for the date line (default: "%x")
  -F, --font-family       Font family name
  -a, --alpha             Text opacity (0.0-1.0, overridden by RGBA color)
  -y, --layer             GTK shell layer (0=background, 1=bottom, 2=top, 3=overlay, 4=no_layer)
```

### Date line

By default the date is shown underneath the time:

```
02:47
09/27/2026
```

The two lines are centred as a single block, and the time keeps exactly the
size `--font-size` and the window geometry produce — adding the date does not
resize it.

The date is **not** given a font size of its own. It is scaled automatically so
that its rendered width matches the time's, which keeps the clock the same
overall width whichever date you use. Because a 10-character date has to be
roughly half the size of a 5-character time to fit that width, longer formats
produce smaller text.

Scaling is clamped so the date can never be larger than the time, nor smaller
than 35% of it. A format long enough to hit that floor (for example
`--date-format "%A %d %B"`) will render narrower than the time rather than
become illegible.

`--date-format` takes any [strftime](https://man7.org/linux/man-pages/man3/strftime.3.html)
format, and defaults to `%x`, your locale's short date — `09/27/2026` under
`en_US`, `27/09/2026` under `en_GB`, and so on. Set an explicit format to pin it
regardless of locale:

```sh
# Saturday, 27 September
focusclock --date-format "%A %d %B"

# 2026-09-27
focusclock --date-format "%Y-%m-%d"
```

To go back to a time-only clock, hide the date line:

```sh
focusclock --no-date
```


## Changelog

See [CHANGELOG.md](CHANGELOG.md) for a list of changes.

# References

- https://github.com/nwg-piotr/nwg-wrapper/
- https://github.com/wmww/gtk4-layer-shell/
- https://www.gtk.org/docs/language-bindings/cpp

## License

GNU GPL3+
