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

### `recept.c` (C port of `recept.py`)
```
./recept_test_build.sh # add `-lm` as an argument if you get undefined reference errors to math functions
./recept_test.sh
```

Look for `BEGIN CONFIG` in `recept.c`.

### `recept.py`
```
./recept_mic.py
```

## Read mic and send samples to the app

Must install `sox` via `brew` or `apt`. This script simply uses sox to read samples from the mic device, and sends them across the fifo in the expected format.

In another terminal, where access to mic device is easiest. That is, on OS X, use a regular Terminal app, and grant permissions when this starts:
```
./rec.sh
```

The app in xterm will now start. When the app stops, this process will also stop, because the pipe will be broken across the fifo. So start this again each time you start the app in the other terminal.