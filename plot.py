import numpy as np
import os
import csv
import matplotlib.pyplot as plt

# Файл теперь ищется в текущей рабочей директории (откуда запущен скрипт)
results_dir = os.path.join(os.getcwd(), 'aggregated_results.csv')


def extract_durations(file):
    data = {}
    reader = csv.DictReader(file, delimiter=';')
    for row in reader:
        size = int(row['Size'])
        threads = int(row['Threads'])
        if size not in data:
            data[size] = {}
        # Используем Filtered_Mean в качестве времени выполнения
        data[size][threads] = float(row['Filtered_Mean'])
    return data


def main():
    if os.path.exists(results_dir):
        with open(results_dir, 'r') as f:
            data = extract_durations(f)
    else:
        print(f'No aggregated_results.csv found in {os.getcwd()}.')
        return 1

    for size, val in data.items():
        # Сортируем потоки по возрастанию для корректного построения линий на графике
        sorted_threads = sorted(val.keys())
        
        thread_list = []
        duration_list = []

        for threads in sorted_threads:
            thread_list.append(threads)
            duration_list.append(val[threads])

        thread_arr = np.array(thread_list)
        duration_arr = np.array(duration_list)

        # Находим время для 1 потока
        one_thread_data = duration_arr[thread_arr == 1]
        if len(one_thread_data) == 0:
            print(f"Warning: 1 thread data not found for size {size}. Skipping plot.")
            continue
            
        one_thread_d = one_thread_data[0]
        speedup_arr = one_thread_d / duration_arr

        # Очищаем фигуру перед каждым новым графиком
        plt.figure()

        plt.plot(thread_arr, thread_arr, label='Linear', color='gray', linestyle='--', linewidth=2)
        plt.plot(thread_arr, speedup_arr, label='Sp', color='red', marker='o', markersize=8, markerfacecolor='white', markeredgewidth=2)
        
        plt.legend(loc='lower right', 
                   fontsize=10,
                   facecolor='#f0f0f0',
                   edgecolor='gray',
                   framealpha=1,
                   borderpad=1)

        plt.grid(True, alpha=0.3)
        plt.xlabel('Num threads (P)')
        plt.ylabel('Speedup')
        plt.title(f'Speed Up on P threads [M=N={size}]')

        # Сохраняем график в текущую рабочую директорию
        plt.savefig(os.path.join(os.getcwd(), f'plot_{size}.jpg'), dpi=300)
        plt.show()


if __name__ == '__main__':
    main()