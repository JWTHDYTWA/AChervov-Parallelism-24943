import math

measures = {
    't1': 0.9036781399999996,
    't2': 0.5253944199999999,
    't4': 0.48931089795918375,
    't8': 0.5325666,
    't16': 0.7003523199999999,
    't24': 0.76718218,
    't40': 0.7801111199999998
}

def main():

    for key, val in measures.items():
        incr = measures['t1'] / val
        print(f'{key}: avg {val}, speedup {incr:.6f}, efficiency {incr / int(key[1:]):.2f}')

if __name__ == '__main__':
    main()