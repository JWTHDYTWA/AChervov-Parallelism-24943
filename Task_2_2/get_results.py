import csv
import math
import re
from pathlib import Path


def get_mean(data: list[float], k: float = 3.0) -> float:
    """Вычисляет среднее значение после фильтрации выбросов (аналог C++ функции)."""
    if not data:
        return 0.0

    n = len(data)
    mean = sum(data) / n

    # Вычисление дисперсии по формуле генеральной совокупности (деление на N, как в C++)
    variance = sum((x - mean) ** 2 for x in data) / n
    stddev = math.sqrt(variance)

    # Фильтрация данных по правилу k сигм
    filtered_data = [x for x in data if abs(x - mean) <= k * stddev]

    if not filtered_data:
        return 0.0

    return sum(filtered_data) / len(filtered_data)


def process_files():
    # Регулярное выражение для поиска файлов вида: results_{threads}T_{size}S.csv
    file_pattern = re.compile(r"results_(\d+)T_(\d+)S\.csv")
    current_dir = Path(".")

    results = []

    # Поиск подходящих файлов в текущей директории
    for path in current_dir.glob("results_*T_*S.csv"):
        match = file_pattern.match(path.name)
        if not match:
            continue

        threads = int(match.group(1))
        size = int(match.group(2))

        durations = []

        try:
            with open(path, mode="r", encoding="utf-8", newline="") as f:
                # Чтение CSV с разделителем ';'
                reader = csv.DictReader(f, delimiter=";")

                for row in reader:
                    # Извлечение значения из колонки Duration
                    if "Duration" in row and row["Duration"]:
                        try:
                            durations.append(float(row["Duration"]))
                        except ValueError:
                            # Пропускаем некорректные строки, если они есть
                            continue
        except Exception as e:
            print(f"Ошибка при чтении файла {path.name}: {e}")
            continue

        if durations:
            clean_mean = get_mean(durations, k=3.0)
            original_mean = sum(durations) / len(durations)

            results.append(
                {
                    "Threads": threads,
                    "Size": size,
                    "Original_Mean": original_mean,
                    "Filtered_Mean": clean_mean,
                    "Source_File": path.name,
                }
            )

    # Запись результатов в новый файл, если данные были найдены
    if results:
        # Сортировка по размеру задачи и количеству потоков для удобства представления
        results.sort(key=lambda x: (x["Size"], x["Threads"]))

        output_path = current_dir / "aggregated_results.csv"
        try:
            with open(
                output_path, mode="w", encoding="utf-8", newline=""
            ) as f:
                fieldnames = [
                    "Threads",
                    "Size",
                    "Original_Mean",
                    "Filtered_Mean",
                    "Source_File",
                ]
                writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter=";")

                writer.writeheader()
                writer.writerows(results)

            print(f"Обработка завершена. Результаты сохранены в {output_path}")
        except Exception as e:
            print(f"Не удалось записать файл результатов: {e}")
    else:
        print("Файлы для обработки не найдены или они не содержат данных.")


if __name__ == "__main__":
    process_files()