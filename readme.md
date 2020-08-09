# Dpishit
Dpishit is a simple portable library for accessing pixel-density information.
It can be used to build cross-platform DPI-aware apps for Linux (X11/Wayland),
Windows and OS X. DPIshit can find or compute the following values:
 - Physical display resolution (px)
 - Physical display size (mm)
 - Active display resolution (px)
 - Logic display density (dpi)
 - Scale



## Linking
For Linux/X11 you must link with XCB, RandR and xcb-util-xrm:
```
-lxcb -lxcb-randr -lxcb-xrm
```

For Linux/Wayland you must link with libwayland-client:
```
```

Under Windows you must link with `windows`, `GDI` and `Shcore`.
```
-mwindows -lgdi32 -lshcore
```
It is also necessary to set the `NTDDI_VERSION` to `NTDDI_WINBLUE`.
```
-DNTDDI_VERSION=NTDDI_WINBLUE
```

Under OS X you must link with:
```
```



## Documentation
### 
### Density information APIs feature matrix
```
                +---------------+---------------+------------------+---------------+
                | EDID dumped   | logic         | physical         | scaling       |
                | by system     | density       | res. and size    | settings      |
+---------------|---------------+---------------+------------------+---------------+
| Windows       | yes, registry | yes           | yes, approximate | yes           |
|---------------|---------------+---------------+------------------+---------------+
| Mac OS        | yes, registry | no            | yes, unreliable  | yes, font     |
|---------------|---------------+---------------+------------------+---------------+
| Linux/X11     |               | yes, font/env | yes, spot-on     | yes, font/env |
|---------------| yes, with KMS |---------------+------------------+---------------+
| Linux/Wayland |               | no            | yes, spot-on     | yes           |
+---------------+---------------+---------------+------------------+---------------+
```



## Implementation details and information sources
### Linux
#### EDID dump accessibility
Under Linux a system EDID dump is available under `/sys/class/drm` if KMS is on.
The user can talk directly to the display using DDC but with strange permissions.

#### Physical resolution and size
The X11 RandR API allows us to get the physical properties of a display using
[xcb_randr_get_screen_info_sizes](https://xcb.freedesktop.org/manual/group__XCB__RandR__API.html).
Under Wayland this same information is provided directly by the server in the
[geometry event](https://wayland-book.com/registry/server-side.html).

#### Logic density
X11 and Wayland do not support logic density, mainly because of its uselessness.
Unfortunately everyone expects DPI-aware apps to use Xft's font DPI setting as a
system-wide logic density information, so we are doomed to support it...

On top of that, Xft uses XResources to store its settings and the handy XRM
helpers were not officially ported to XCB. A good unofficial alternative is
[xcb-util-xrm](https://github.com/Airblader/xcb-util-xrm),
which is maintained by the good folks from i3-gaps.

If `Xft.dpi` is not available we can use other people's environment variables:
 - [GDK_DPI_SCALE](https://developer.gnome.org/gtk3/stable/gtk-x11.html)
 - [QT_FONT_DPI](https://bugreports.qt.io/browse/QTBUG-53022)

#### Scaling settings
X11 does not provide any scaling value, but we once again we can steal from
[Xft](https://www.keithp.com/~keithp/render/Xft.tutorial).
If `Xft.scale` is not availvable there are also environment variables for that:
 - [GDK_SCALE](https://developer.gnome.org/gtk3/stable/gtk-x11.html)
 - [QT_SCALE_FACTOR](https://doc.qt.io/qt-5/highdpi.html)
 - [ELM_SCALE](https://phab.enlightenment.org/w/elementary/)

When using Wayland however, it is a better idea to just catch the
[scale event](https://wayland-book.com/surfaces-in-depth/hidpi.html).



### Windows
#### EDID dump accessibility
A system EDID dump is stored in the registry and can be accessed with the
[setup API](https://docs.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdiopendevregkey).
This might require admin privileges, especially on school and office computers...

#### Physical resolution and size
Physical-ish resolution and size values are availabe through GDI's
[GetDeviceCaps](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getdevicecaps).
The physical pixel density can also be retrieved for a specific monitor using
[GetDpiForMonitor](https://docs.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-getdpiformonitor).

#### Logic density
Again, this value can be retrieved using GDI with
[GetDeviceCaps](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getdevicecaps).
It is also possible to use new APIs if the application executes on a
[DPI-aware thread](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setthreaddpiawarenesscontext).
In this case, a DPI value can be obtained for the window using
[GetDpiForWindow](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforwindow).

#### Scaling settings
The user-set scaling factor is available from
[GetScaleFactorForMonitor](https://docs.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-getscalefactorformonitor).
It is also possible to enable automatic window frame scaling with
[EnableNonClientDpiScaling](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enablenonclientdpiscaling).



### Mac OS
#### EDID dump accessibility
The EDID dump is stored as `IODisplayEDID` in the `AppleDisplay` class of the
IORegistry. It can be found using the command `ioreg -l | grep EDID`, but can
be retrieved programmatically using the functions
[documented by apple](https://developer.apple.com/documentation/kernel/iokit_fundamentals/registry_utilities?language=occ)

#### Physical density info
The display's physical density is computed directly from the EDID values.
It is returned by the `CGDisplayScreenSize` function, taking the display as parameter.

Beware, Apple fakes this value if the EDID it gets does not provide enough info.
This can happen when the display device is too old or when density doesn't make
sense, like in projectors. Sometimes the manufacturer simply didn't care.
Whatever the reason, in this case the returned density is 2.835 px/mm (72 dpi),
as explained in the [official documentation](https://developer.apple.com/documentation/coregraphics/1456599-cgdisplayscreensize)

#### System density info
Mac OS only provides one density value, primarily based on the active resolution
and the physical display size as advertised by the EDID. Because this information
is faked when not available, Mac OS does not need another density value.

#### Scaling settings
Mac OS supposedly provides every widget needed to build UIs, so the scaling
is done internally and its value is not easily available.
However, there is a [function](https://developer.apple.com/documentation/appkit/nsfont/1531931-systemfontsize?language=objc)
in AppKit which returns the system font size. It can be used as an alternative.
