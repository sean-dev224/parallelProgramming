import sys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def main():
    if len(sys.argv) < 2:
        print("Please include path to .csv file as command line argument")
        exit(0)

    f = plt.figure()

    df = pd.read_csv(sys.argv[1])

    n_labels = []
    n_magnitudes = df['n_magnitude'].tolist()
    for num in n_magnitudes:
        n_labels.append(f"10^{num}")
    df.insert(len(df.columns), "n_label", n_labels)


    plt.plot(df['n_label'], df['execution_time_ms'])
    plt.yscale('log')
    plt.xlabel('N Elements')
    plt.ylabel('Time (ms)')
    plt.title('Execution Times')

    f.savefig("plot.pdf")


if __name__ == "__main__":
    main()