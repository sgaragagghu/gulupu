# Gulupu

Gulupu is a GUI wrapper for kulupu, built using GTK+3.

## Run

You can use the binary build at Gulupu's
[releases](https://github.com/sgaragagghu/gulupu/releases) page.

## Build

### Prerequisites

#### Windows

Install MSYS2 using [GTK installation guide](https://www.gtk.org/docs/installations/windows).

### Build

```bash
make
```

#### Windows distribution

* List the dll to be bundled together with the executable.

```ldd mygtkapp.exe | sed -n 's/\([^ ]*\) => \/mingw.*/\1/p' | sort```

1. Copy all the dll necessary to ditribute the executable.

```ldd mygtkapp.exe | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" ./bin```

1. Place gulupu.exe and gdbus.exe(from msys64\mingw64\bin) in ./bin

1. ```sh $ cp -r /mingw64/lib/gdk-pixbuf-2.0/* ./lib/gdk-pixbuf-2.0```

1. ```sh $ cp -r /mingw64/share/icons/* ./share/icons/```

1. ```sh $ cp /mingw64/share/glib-2.0/schemas/* share/glib-2.0/schemas/```

