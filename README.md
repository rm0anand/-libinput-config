# Maintainer needed

Back when I made this hack, I didn't expect so many people to actually
use it. Unfortunately, I'm no longer able to maintain it on my own.
Your help is always welcome.

---

libinput-config allows you to configure your inputs in case your
Wayland compositor doesn't have a certain config or has none.

## How to build and install

Before installing, make sure the following prerequisites are met:

* You're running an X11 server or a Wayland compositor that uses
  libinput for input handling. If it uses another library such as
  Synaptics, libinput-config won't work.
* You have the libinput header files installed. They usually come in a
  package called something like `libinput-dev` or `libinput-devel`.
  You may also need to do the same for libudev.
* You have the Meson build system installed.
* You have written a working config file (see [How to use] for more
  details).

[How to use]: #how-to-use

```
meson build
cd build
# meson configure -Dnon_glibc=true
# meson configure -Dshitty_sandboxing=true
ninja
sudo ninja install
```

If you're using a C library that's not glibc, uncomment the third
line. ([#12])

If you're using Snap and seeing error messages when launching apps,
uncomment the fourth line. ([#13])

After you compiled and installed the library, you need to relogin or
reboot in order for the effect to take place.

[#12]: https://gitlab.com/warningnonpotablewater/libinput-config/-/issues/12
[#13]: https://gitlab.com/warningnonpotablewater/libinput-config/-/issues/13

## How to use

In order for the library to work, you need to create a config file
under `/etc/libinput.conf`. You only need to put the options you're
going to use. For all left out options, the system defaults will be
used. If you already configured an option in your compositor, the
compositor will take the priority unless you enable
`override-compositor`.

The config file must consist of `key=value` entries. The following
settings are implemented:

```
override-compositor={disabled,enabled}
tap={disabled,enabled}
tap-button-map={lrm,lmr}
drag={disabled,enabled}
drag-lock={disabled,enabled}
accel-speed=[number]
accel-profile={none,flat,adaptive}
natural-scroll={disabled,enabled}
left-handed={disabled,enabled}
click-method={none,button-areas,clickfinger}
middle-emulation={disabled,enabled}
scroll-method={none,two-fingers,edge,on-button-down}
scroll-button=[number]
scroll-button-lock={disabled,enabled}
dwt={disabled,enabled}
scroll-factor=[number]
scroll-factor-x=[number]
scroll-factor-y=[number]
discrete-scroll-factor=[number]
discrete-scroll-factor-x=[number]
discrete-scroll-factor-y=[number]
speed=[number]
speed-x=[number]
speed-y=[number]
gesture-speed=[number]
gesture-speed-x=[number]
gesture-speed-y=[number]
remap-key=[number]:[number]
```

Note:

* `override-compositor` has no effect on the following settings. If
  you configure them in both your compositor and libinput-config,
  they'll be applied at the same time:
    * `scroll-factor`
    * `discrete-scroll-factor`
    * `speed`
    * `gesture-speed`
    * `remap-key`
* In contrast to the touchpad `scroll-factor` that always works, the
  mouse `discrete-scroll-factor` only works if your compositor
  supports high-resolution scroll wheels.
* `scroll-button` accepts a key code from the [Linux input event code
  list] which is incompatible with the one used by X11.
* For obvious reasons, `remap-key` can be used multiple times to remap
  different keys.

[Linux input event code list]: https://github.com/torvalds/linux/blob/bb7c241fae6228e89c0286ffd6f249b3b0dea225/include/uapi/linux/input-event-codes.h#L355

## How to uninstall

```
cd build
sudo ninja pre-uninstall uninstall
```

## How it works

The library wraps around libinput and hacks into the event loop to
read the config and configure devices. Additional hackery is used to
configure scrolling sensitivity and pointer speed. To do all of this,
it uses the `/etc/ld.so.preload` file, which is modified by the
install script. If non-GNU libc support is enabled, an `LD_PRELOAD`
fallback is used instead.

## Inspiration

* [libinput-touchpad-scroll-fix](https://gitlab.com/warningnonpotablewater/libinput-touchpad-scroll-fix)

## License

[ISC License](LICENSE).
