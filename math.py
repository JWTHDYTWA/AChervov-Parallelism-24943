import math

measures = {
    't1': 2.4911384536082473,
    't2': 1.4787463917525776,
    't4': 0.7716678383838386,
    't7': 0.4433078888888888,
    't8': 0.3758523541666668,
    't16': 0.20196302020202017,
    't20': 0.17977410526315787,
    't40': 0.1162065515789473
}

def main():

    for key, val in measures.items():
        incr = measures['t1'] / val
        print(f'{key}: avg {val}, speedup {incr:.6f}, efficiency {incr / int(key[1:]):.2f}')

if __name__ == '__main__':
    main()