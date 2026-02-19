# AChervov-Parallelism-24943
Решения заданий по предмету теория параллелизма.

# Задача 1
## Сборка и запуск
1. Меняем директорию на Task1: `cd ./Task1`
2. Создаём конфигурацию CMake: `cmake -S. -Bbuild`
    1. Если нужно скомпилировать с типом DOUBLE: `cmake -S. -Bbuild -DUSE_DOUBLE=1`
    2. По умолчанию создаётся конфигурация для FLOAT, но можно задать явно: `cmake -S. -Bbuild -DUSE_DOUBLE=0`
3. Собираем проект: `cmake --build build`
4. Запускаем программу: `./build/Debug/circlesin.exe`
    * _Note: последний шаг для компиляции на Windows. На Linux через `g++` скорее всего будет просто `./build/circlesin`._
## Значения суммы
* float: 0.291951
* double: 4.89582e-11
