# recept
Receptor Field Playground


## Setup
Do once:
```
mkfifo input.sock
```

That will allow one terminal to read the mic, and another to display the app. On OS X, a different kind of terminal app program is needed for each of these cases.

## Apps

Need to use bitmap fonts, so use xterm. That is, on OS X, install xquarts, and use the xterm it provides.

To get the UTF-8 bitmap font with the special bar graph characters now supported by the C version of the program, you need to install the code7 font.

```
git clone https://github.com/legitparty/code7.git
cd code7
./setup.sh
xterm
```
The `setup.sh` script installs the bitmap UTF-8 fonts to the X11 session (not permanently), and adds special xresources for xterm to provide a solarized dark color theme with the UTF-8 bitmap fonts. Just run xterm and it will use this configuration.


### `recept.c` (C port of `recept.py`)
```
./recept_test_build.sh # add `-lm` as an argument if you get undefined reference errors to math functions
./recept_test.sh
```

Look for `BEGIN CONFIG` in `recept.c`. You can make canges and rebuilt and run again. Make sure not too add too many receptors for it to process. CPU usage should be less than 100%, and the time report should be keeping up with actual time.

### `recept.py`

Need to install pypy via `apt` or `brew`. That is a Python JIT interpreter that is reasonably good at optimizing math computations.

```
./recept_mic.py
```

This is currently configured to do a heavily optimized set of sensors for reading as many notes at once, where each octave is fed as low a sample rate as sampling theory allows. This allows it to do a reasonable number of receptors despite being python.

The C version is probably 20 times faster, at least, and can do many more receptors without any kind of subsampling. Planning to port this to Typescript and/or CUDA.

## Read mic and send samples to the app

Must install `sox` via `brew` or `apt`. This script simply uses sox to read samples from the mic device, and sends them across the fifo in the expected format.

In another terminal, where access to mic device is easiest. That is, on OS X, use a regular Terminal app, and grant permissions when this starts:
```
./rec.sh
```

The app in xterm will now start. When the app stops, this process will also stop, because the pipe will be broken across the fifo. So start this again each time you start the app in the other terminal.
