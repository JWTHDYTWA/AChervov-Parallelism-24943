import numpy as np
import os
import csv
import matplotlib.pyplot as plt

# Файл ищется в текущей рабочей директории (откуда запущен скрипт)
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

    plt.figure(figsize=(10, 6))

    # Сначала найдем все уникальные значения потоков, чтобы построить линию идеального (линейного) ускорения
    all_threads = set()
    for val in data.values():
        all_threads.update(val.keys())
    
    if all_threads:
        sorted_all_threads = sorted(list(all_threads))
        plt.plot(sorted_all_threads, sorted_all_threads, label='Linear (Ideal)', color='gray', linestyle='--', linewidth=2)

    # Строим график для каждого размера (Size)
    # Сортируем по ключам size, чтобы линии в легенде шли по возрастанию размера
    for size, val in sorted(data.items()):
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
            print(f"Warning: 1 thread data not found for size {size}. Skipping.")
            continue
            
        one_thread_d = one_thread_data[0]
        speedup_arr = one_thread_d / duration_arr

        # Строим линию для текущего размера. Цвет выбирается автоматически.
        plt.plot(
            thread_arr, 
            speedup_arr, 
            label=f'Sp [M=N={size}]', 
            marker='o', 
            markersize=6, 
            markerfacecolor='white', 
            markeredgewidth=2
        )

    plt.legend(loc='upper left', 
               fontsize=10,
               facecolor='#f0f0f0',
               edgecolor='gray',
               framealpha=1,
               borderpad=1)

    plt.grid(True, alpha=0.3)
    plt.xlabel('Num threads (P)')
    plt.ylabel('Speedup')
    plt.title('Speed Up on P threads')

    # Сохраняем объединенный график в текущую рабочую директорию
    plt.savefig(os.path.join(os.getcwd(), 'plot_combined.jpg'), dpi=300)
    plt.show()


if __name__ == '__main__':
    main()