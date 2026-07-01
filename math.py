import math

measures = {
    't1': 69.0201,
    't2': 34.5825,
    't4': 17.225659999999998,
    't8': 8.833486,
    't16': 5.440982,
    't24': 5.262472000000001,
    't40': 9.149136
}

def main():

    for key, val in measures.items():
        incr = measures['t1'] / val
        print(f'{key}: avg {val}, speedup {incr:.6f}, efficiency {incr / int(key[1:]):.2f}')

if __name__ == '__main__':
    main()