
# Instrukcja na pracowni


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
