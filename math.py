import math

measures = {
    't1': 0.3183311000000002,
    't2': 0.21902905050505056,
    't4': 0.11103538541666665,
    't7': 0.06425345773195877,
    't8': 0.05624082551020406,
    't16': 0.028715392929292934,
    't20': 0.023911970707070705,
    't40': 0.013649626262626257
}

def main():

    for key, val in measures.items():
        incr = measures['t1'] / val
        print(f'{key}: avg {val}, speedup {incr:.6f}, efficiency {incr / int(key[1:]):.2f}')

if __name__ == '__main__':
    main()