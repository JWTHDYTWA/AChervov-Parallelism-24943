import numpy as np
import os
import csv
import matplotlib.pyplot as plt

Pl = [1,2,4,7,8,16,20,40]

root_dir = os.path.dirname(__file__)
results_dir = os.path.join(root_dir, 'results.csv')


def extract_durations(file):
    data = {}
    reader = csv.DictReader(file, delimiter=';')
    for row in reader:
        size = int(row['Size'])
        threads = int(row['Threads'])
        if size not in data:
            data[size] = {}
        data[size][threads] = float(row['Duration'])
    return data


def main():

    data = None
    if os.path.exists(results_dir):
        with open(results_dir, 'r') as f:
            data = extract_durations(f)
    else:
        print('No results.csv found.')
        return 1

    for size, val in data.items():
        thread_list = []
        duration_list = []

        for threads, dur in val.items():
            thread_list.append(int(threads))
            duration_list.append(float(dur))

        thread_arr = np.array(thread_list)
        duration_arr = np.array(duration_list)

        one_thread_d = duration_arr[thread_arr==1][0]
        speedup_arr = one_thread_d / duration_arr

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

        plt.savefig(os.path.join(root_dir, f'plot_{size}.jpg'), dpi=300)
        plt.show()

if __name__ == '__main__':
    main()