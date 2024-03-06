import numpy as np
import matplotlib.pyplot as plt

x1 = np.linspace(0, np.pi, 6)
x2 = np.linspace(np.pi, 2*np.pi, 6)
y1 = 3*(1+1/(1-np.exp(x1)))
y2 = 3*(1+1/(1-np.exp(2*np.pi-x2)))

x=np.concatenate((x1, x2))
y=np.concatenate((y1, y2))

print("X : ", x)
print("\nY : ", y)

# red color and dot data point
plt.plot(x, y, color = 'red', marker = "o")
plt.title("Waveform")
plt.xlabel("X")
plt.ylabel("Y")
plt.show()