## Алгоритм сборки

0. 
    * Удалить папку `build`, если она есть
    * Установить `boost` либо через `vcpkg` (Windows), либо через `apt` (Linux)
1. Сконфигурировать проект командой:
```sh
Task_2_1/> cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
```
2. Собрать проект:
```sh
Task_2_1/> cmake --build build
```
3. Запустить программу:
```sh
Task_2_1/> ./build/matrix_mult
```