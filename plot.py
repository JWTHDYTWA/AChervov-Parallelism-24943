import numpy as np
import matplotlib.pyplot as plt

Pl = [1,2,4,7,8,16,20,40]
# Spl = [
#     1,
#     2.003290995382042,
#     3.627485420796958,
#     6.837785313845914,
#     7.997689249418238,
#     15.323844950154154,
#     19.063377750156892,
#     35.77265471030182
# ]
Spl = [
    1,
    1.2990385781856266,
    3.759113556178176,
    5.231759568824165,
    6.080733068847249,
    15.84428995513027,
    19.66449567147016,
    35.883691095857245
]

Pa = np.array(Pl)
Spa = np.array(Spl)

plt.plot(Pa, Pa, label='Linear', color='gray', linestyle='--', linewidth=2)
plt.plot(Pa, Spa, label='Sp', color='red', marker='o', markersize=8, markerfacecolor='white', markeredgewidth=2)
plt.legend(loc='lower right', 
           fontsize=10,
           facecolor='#f0f0f0',
           edgecolor='gray',
           framealpha=1,
           borderpad=1)

plt.grid(True, alpha=0.3)
plt.xlabel('Num threads (P)')
plt.ylabel('Speedup')
plt.title('Speed Up on P threads [M=N=40000]')

plt.savefig('plot.jpg', dpi=300)
plt.show()