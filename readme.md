# DPIshit
DPIshit is a portable screen information library for X11, Wayland, Windows
and macOS. It is lightweight and self-contained, making it suitable for use
with custom windowing code or low-level abstraction libraries like
[Globox](https://github.com/nullgemm/globox).

## Features
Lightweight does not always mean basic, and DPIshit supports advanced features:
 - The library reports physical screen information like size and resolution,
   but also the logic DPI value and scale factor in use on the target OS.
 - We only report the main screen the window is being displayed on in a similar
   fashion on all target platforms - except under Windows where we can only
   get information about the main screen without special privileges.
 - Hot plugging is entirely supported, as well as moving windows on new screens:
   the data returned by DPIshit reflects the current situation of the window.

## Building
The build system for DPIshit consists in bash scripts generating ninja scripts.
The first step is to generate the ninja script; then, this ninja script must
be ran to build the actual binary.

### General steps for the library
You will need 2 modules to be able to use one of DPIshit' backend:
 - The DPIshit core, providing code common across platforms.
   This is what implements the main interface of the library,
   to which you will bind the backend of your choice at run time.
 - The DPIshit backend module, holding all code specific to a platform.
   This is what is actually executed when the interface is used,
   after you bind all the backend's functions to the main library.

All these components are generated using the scripts found in `make/lib`.
They take arguments: execute them alone to get some help about that.

The scripts named after executable file formats generate ninja scripts
to compile the DPIshit core module. An example use for Linux would be:
```
./make/lib/elf.sh development
```

The scripts named after platforms will generate ninja files to compile backends:
```
./make/lib/x11.sh development
```

All ninja scripts are generated in `make/output`. To compile, simply execute
them using the original `ninja` or the faster `samurai` implementation.
```
ninja -f ./make/output/lib_elf.ninja
ninja -f ./make/output/lib_x11.ninja
```

All the binaries we build are automatically copied in a new `dpishit_bin` folder
suffixed with the latest tag on the repository, like this: `dpishit_bin_v0.0.0`.
To copy the library headers here and make the build ready to use and distribute,
we run the core and platform ninja scripts again, adding the `headers` argument:
```
ninja -f ./make/output/lib_elf.ninja headers
ninja -f ./make/output/lib_x11.ninja headers
```

### X11 support
DPIshit was built using the modern libxcb X11 library instead of libX11.
Make sure all its components are installed before you start compiling.

### Windows support
Our build system relies on the MinGW toolchain to build Windows binaries.

#### Compiling from Windows
To compile the Windows module under Windows, we recommend using the MinGW
toolchain provided by the [MSYS2](https://www.msys2.org) building platform.

For increased comfort we also recommend using Microsoft's "Windows Terminal":
it is available for download outside of the Microsoft Store on the project's
[GitHub](https://github.com/microsoft/terminal/latest).
It is possible to use MSYS2 from the new Windows Terminal with a custom profile:
to do this click the downwards arrow next to the tabs in the terminal window:
this will open the settings menu, from which new profiles can be created.
Create a new empty profile and paste the following command in the field for the
executable path - make sure the arguments are here otherwise it won't work.
```
C:/msys64/msys2_shell.cmd -defterm -here -no-start -ucrt64
```
You can also set the icon path to the following
```
C:/msys64/ucrt64.ico
```
Once your profile is configured, you can open new tabs using it from the main
window, or set it as the default profile from the settings menu.

Whether you decide to go for the Windows terminal or the included MinTTY,
make sure you are using MSYS2 with its UCRT64 environment, this is important.
When your setup is ready, install a basic MinGW toolchain in MSYS2, like this:
```
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain git ninja unzip
```

You can then proceed to the general build steps.

#### Cross-compiling from Linux
To cross-compile the Windows module under Linux, simply install MinGW and build.
Wine is fully supported, so you will be able to test the examples as expected,
but remember Wine is not Windows and does not support transparency or blur.

### macOS support
Our build system relies on the Xcode toolchain to build macOS binaries.

#### Compiling from macOS
To compile the macOS module under macOS, we recommend that you don't download
the entirety of Xcode, as it would be overkill. Instead, simply get the
command-line tools using your favorite installation method.

If you do not want to register an Apple account, it is possible to get the bare
macOS SDK from Apple's "Software Update" servers using some of the scripts in
[instant_macos_sdk](https://github.com/nullgemm/instant_macos_sdk).

If you did not get Xcode, you also have to install git independently
```
brew install git
```

You can then proceed to the general build steps.

#### Cross-compiling from Linux
To cross-compile the macOS module under Linux, we recommed using OSXcross.
The toolchain can be deployed easily without an Apple account by cloning
[instant_macos_sdk](https://github.com/nullgemm/instant_macos_sdk)
and following the instructions in the readme.

You can then proceed to the general build steps.

### All-in-one helper script
For convenience, a helper script can be found in `make/scripts/build.sh`.
It will automatically build the library and an example in a single command line.
To use it, you must supply all the following arguments:
 - a build type
 - a backend name
 - a build toolchain type

For instance:
```
./make/scripts/build.sh development x11 native
```

## Windowing setup
### X11
This backend's initialization data must include the following XCB structures:
 - The XCB connection pointer
 - The XCB root window
 - The XCB window

Forward XCB events to `dpishit_handle_event`

### Wayland
This backend's initialization data must include the following callbacks:
 - A registry handler callback DPIshit can call to register its own callback
   to be executed during the `wl_registry` globals enumeration
 - A capabilities handler callback DPIshit can call to register its own callback
   to be executed during the `wl_seat` capabilities enumeration
 - An event callback to be executed directly by DPIshit with the serial as data
   when it receives an event.

To insist on this last point, under Wayland the events are already reported
through callbacks, which are not tied to the windowing code.
DPIshit, when registering for events, has to supply its own event callbacks,
and for the sake of flexibility, we expose this behaviour to you.
It is your choice to pass a wrapper to your own event callback directly
or to have this callback post events to a secondary queue,
depending on how the rest of your code is organized.

Under Wayland, you also have to set the target window's surface,
in all likelihood after the globals registration has happened:
```
dpishit_set_wayland_surface(dpishit, wl_surface, &error);
```

### Windows and macOS
No initialization data is required under Windows and macOS, just configure the
library in a generic way and forward system events to `dpishit_handle_event`.

## Library usage
### Setting it up
Include the general DPIshit header and the backend header:
```
#include "dpishit.h"
#include "dpishit_x11.h"
```

Initialize configuration structures:
```
struct dpishit_error_info error = {0};
struct dpishit_config_backend config = {0};
```

Bind backend implementation:
```
dpishit_prepare_init_x11(&config);
```

Initialize DPIshit:
```
struct dpishit* dpishit = dpishit_init(&config, &error);
```

Start DPIshit with the appropriate backend data:
```
struct dpishit_x11_data backend_data =
{
    .conn = x11_conn,
    .window = x11_window,
    .root = x11_root_window,
};

dpishit_start(dpishit, &backend_data, &error);
```

Under Wayland, specify the window's surface after it's been created:
```
dpishit_set_wayland_surface(dpishit, wl_surface, &error);
```

### Cleaning it up
Stop DPIshit
```
dpishit_stop(dpishit, &error);
```

Perform cleanup
```
dpishit_clean(dpishit, &error);
```

### Handling events
Get event info passing the system's event:
```
struct dpishit_display_info* info = {0};
dpishit_handle_event(dpishit, event, &info, &error);
```

Get debug info:
```
const char* code_name = dpishit_get_event_code_name(dpishit, info.event_code, &error);
const char* state_name = dpishit_get_event_state_name(dpishit, info.event_state, &error);
dpishit_error_get_code(&error);
dpishit_error_log(dpishit, &error);
```

## Testing
### CI
The `ci` folder contains dockerfiles and scripts to generate testing images
and can be used locally, but a `concourse_pipeline.yml` file is also available
here and should be usable with a few modifications in case you want a more
user-friendly experience. Our own Concourse instance will not be made public
since it runs on a home server (mostly for economic reasons).
