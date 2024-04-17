
# Instrukcja na pracowni

Biblioteka jest w [SDL2-devel-2.30.1-VC.zip](https://github.com/libsdl-org/SDL/releases/download/release-2.30.1/SDL2-devel-2.30.1-VC.zip)

Dzięki naszemu adminowi, mamy taką instrukcję:


Zastosuj CMakeLists.txt do Swojego projektu. Instrukcja obrazkowa:


![image](01.png)

W Toolchains należy zmienić architekturę VisualStudio na 64 bitową (np. amd64)


![image](02.png)
W konfiguracji CMake należy wybrać Toolchain Visual Studio

Użyć CMakeLists 
![image](03.png)


![image](04.png)



UWAGA: Czasami trzeba będzie dodać:

```c++
#define SDL_MAIN_HANDLED
```
