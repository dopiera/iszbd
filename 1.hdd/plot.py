import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("sda")

plt.scatter(df["distance"], df["time_s"], s=10)
plt.xlabel("Seek distance (bytes)")
plt.ylabel("Access time (s)")
plt.title("Seek distance vs. access time on HDD")
plt.grid(True)
plt.show()

