import argparse
import numpy as np
import os
import csv
import matplotlib.pyplot as plt

# Файл ищется в текущей рабочей директории (откуда запущен скрипт)
results_dir = os.path.join(os.getcwd(), 'aggregated_results.csv')


def extract_durations(file, group_column):
    data = {}
    reader = csv.DictReader(file, delimiter=';')
    
    # Проверяем наличие целевого столбца в заголовке CSV
    if reader.fieldnames and group_column not in reader.fieldnames:
        raise ValueError(f"Столбец '{group_column}' не найден в CSV файле. "
                         f"Доступные столбцы: {', '.join(reader.fieldnames)}")

    for row in reader:
        threads = int(row['Threads'])
        
        # Пытаемся привести значение группирующего столбца к числу для корректной сортировки
        raw_val = row[group_column]
        try:
            if '.' in raw_val:
                group_val = float(raw_val)
            else:
                group_val = int(raw_val)
        except ValueError:
            group_val = raw_val

        if group_val not in data:
            data[group_val] = {}
        # Используем Filtered_Mean в качестве времени выполнения
        data[group_val][threads] = float(row['Filtered_Mean'])
    return data


def main():
    # Настройка парсера аргументов командной строки
    parser = argparse.ArgumentParser(description='Построение графиков ускорения по данным из CSV.')
    parser.add_argument(
        '--group-by', '-g',
        type=str,
        default='Size',
        help='Имя столбца в CSV, по уникальным значениям которого будут строиться отдельные линии (по умолчанию: Size)'
    )
    args = parser.parse_args()
    group_column = args.group_by

    if os.path.exists(results_dir):
        try:
            with open(results_dir, 'r') as f:
                data = extract_durations(f, group_column)
        except ValueError as e:
            print(f"Ошибка чтения данных: {e}")
            return 1
    else:
        print(f'Файл aggregated_results.csv не найден в директории {os.getcwd()}.')
        return 1

    plt.figure(figsize=(10, 6))

    # Сначала найдем все уникальные значения потоков для построения линии идеального ускорения
    all_threads = set()
    for val in data.values():
        all_threads.update(val.keys())
    
    if all_threads:
        sorted_all_threads = sorted(list(all_threads))
        plt.plot(sorted_all_threads, sorted_all_threads, label='Linear (Ideal)', color='gray', linestyle='--', linewidth=2)

    # Строим график для каждой уникальной группы
    for group_val, val in sorted(data.items()):
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
            print(f"Предупреждение: данные для 1 потока не найдены для значения {group_val}. Пропуск группы.")
            continue
            
        one_thread_d = one_thread_data[0]
        speedup_arr = one_thread_d / duration_arr

        # Название линии в легенде теперь адаптируется под выбранный столбец
        plt.plot(
            thread_arr, 
            speedup_arr, 
            label=f'Sp [{group_column}={group_val}]', 
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
    plt.title(f'Speed Up on P threads (grouped by {group_column})')

    # Имя сохраняемого файла теперь содержит название столбца группировки
    filename = f'plot_combined_by_{group_column}.jpg'
    plt.savefig(os.path.join(os.getcwd(), filename), dpi=300)
    print(f"График успешно сохранен как {filename}")


if __name__ == '__main__':
    main()