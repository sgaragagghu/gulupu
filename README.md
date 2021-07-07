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

#### Windows

List the dll to be bundled together with the executable.

```ldd mygtkapp.exe | sed -n 's/\([^ ]*\) => \/mingw.*/\1/p' | sort```

Copy all the dll necessary to ditribute the executable.

```ldd mygtkapp.exe | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}"```

