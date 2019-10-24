# recept
Receptor Field Playground

Detect the transition from guassian downward to gamma sideways.

In one terminal:
```
mkfifo input.sock
./recept_test_build.sh
./recept_test.sh
```

In another terminal:
```
./rec.sh
```
