import numpy as np
import matplotlib.pyplot as plt

IQ_FILE = "out_iq.bin"
SYMBOLS_FILE = "out_symbols.tsv"

ZC_END = 3989 * 40
NUM_SYMBOLS = 25

iq = np.fromfile(IQ_FILE, dtype="int16")
iq = iq.reshape(-1, 2)

symbols = np.loadtxt(SYMBOLS_FILE)

fig, axs = plt.subplots(2, 1, figsize=(8, 4))

axs[0].plot(iq[:ZC_END:40])
axs[0].set_title("Sync sequence")

axs[1].plot(iq[ZC_END:ZC_END + 20 * NUM_SYMBOLS])
axs[1].stem(np.arange(NUM_SYMBOLS) * 20, symbols[:NUM_SYMBOLS, 0], 'C0+')
axs[1].stem(np.arange(NUM_SYMBOLS) * 20, symbols[:NUM_SYMBOLS, 1], 'C1+')
axs[1].set_title("First symbols")

plt.tight_layout()
plt.savefig("output.png")
