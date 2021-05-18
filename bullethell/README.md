# PJ Bullet Hell Demo app

## Dependencies

```bash
sudo apt install libsdl2-dev libsdl2-image-dev cmake g++
```

## Compilation

### First time

Whenever you add or remove source file, or asset, you should regenerate build directory.

```bash
mkdir build
cd build
cmake ..
make
./pjbullethellapp
```

### Next time

If some source file was modified, you only have to run make

```bash
make
./pjbullethellapp
```
