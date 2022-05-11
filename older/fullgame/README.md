
Na początek (tylko raz lub gdy dodajemy nowe pliki):

```bash
sudo apt install libsdl2-dev cmake g++
```

```bash
mkdir build
cd build
cmake ..
make
./sdldemoapp
```

Później w katalogu build wystarczy wykonać:

```bash
make
./sdldemoapp
```
