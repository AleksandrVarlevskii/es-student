import matplotlib
matplotlib.use("TkAgg")   # надёжный бэкенд, окно точно откроется

import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import numpy as np
import sys
import time

# ---------- настройки ----------
PORT = "COM5"        # твой порт; можно None для автопоиска
BAUD = 115200
WINDOW_SECONDS = 5
SLEEP_MS = 1
SAMPLE_RATE_HZ = int(1000 / SLEEP_MS)              # 1000
MAX_POINTS = int(WINDOW_SECONDS * SAMPLE_RATE_HZ)  # 5000

# --- авто-масштаб по Y (в вольтах) ---
Y_MIN_SPAN = 0.3       # минимальная высота окна, В (шум не раздувается)
Y_PAD_FRAC = 0.05      # 5% отступ сверху/снизу от видимых данных

# ---------- порт ----------
if PORT is None:
    ports = serial.tools.list_ports.comports()
    for p in ports:
        if "USB" in p.description or "Pico" in p.description or "CDC" in p.description:
            PORT = p.device
            break
    if PORT is None and ports:
        PORT = ports[0].device

if PORT is None:
    print("Не найден COM-порт.")
    sys.exit(1)

print(f"Открываю {PORT} @ {BAUD}...")
try:
    ser = serial.Serial(PORT, BAUD, timeout=1)
except serial.SerialException as e:
    print(f"Не удалось открыть порт: {e}")
    sys.exit(1)

time.sleep(2)
ser.reset_input_buffer()

# ---------- буферы ----------
xs = deque(maxlen=MAX_POINTS)
ys = deque(maxlen=MAX_POINTS)
t0 = time.time()

# ---------- график ----------
fig, ax = plt.subplots(figsize=(10, 5))
line, = ax.plot([], [], lw=1.0, color="#00a0ff")
ax.set_xlabel("Время, с")
ax.set_ylabel("Напряжение, В")
ax.set_title("Напряжение с Raspberry Pi Pico (live)")
ax.set_xlim(0, WINDOW_SECONDS)
ax.grid(True, alpha=0.3)

# ---------- парсер ----------
def parse_voltage(s):
    s = s.strip()
    if not s:
        return None
    if "Voltage:" in s:
        try:
            return float(s.split("Voltage:")[1].replace("V", "").strip())
        except (IndexError, ValueError):
            return None
    try:
        return float(s)
    except ValueError:
        return None

# ---------- счётчик ----------
count = 0
last_report = time.time()

def update(frame):
    global count, last_report

    while ser.in_waiting:
        try:
            line_bytes = ser.readline().decode("ascii", errors="ignore")
        except Exception:
            continue
        v = parse_voltage(line_bytes)
        if v is None:
            continue
        xs.append(time.time() - t0)
        ys.append(v)
        count += 1

    now = time.time()
    if now - last_report >= 1.0:
        last_v = f"{ys[-1]:.3f}" if ys else "—"
        print(f"{count} точек/с, последнее V = {last_v}")
        count = 0
        last_report = now

    # ---------- обновление графика + авто-масштаб Y по видимому окну ----------
    if xs:
        t_end = xs[-1]
        t_start = max(0.0, t_end - WINDOW_SECONDS)

        x_arr = np.fromiter(xs, dtype=float)
        y_arr = np.fromiter(ys, dtype=float)

        mask = x_arr >= t_start
        x_vis = x_arr[mask]
        y_vis = y_arr[mask]

        line.set_data(x_arr, y_arr)
        ax.set_xlim(t_start, max(WINDOW_SECONDS, t_end))

        if y_vis.size:
            y_min = float(y_vis.min())
            y_max = float(y_vis.max())

            # если реальный размах меньше Y_MIN_SPAN — растягиваем окно
            # симметрично вокруг центра, чтобы шум был виден, но не «раздут»
            if (y_max - y_min) < Y_MIN_SPAN:
                c = 0.5 * (y_max + y_min)
                y_min = c - Y_MIN_SPAN / 2
                y_max = c + Y_MIN_SPAN / 2
            else:
                # реальный размах больше — берём его с небольшим отступом
                pad = Y_PAD_FRAC * (y_max - y_min)
                y_min -= pad
                y_max += pad

            ax.set_ylim(y_min, y_max)

    return (line,)

ani = animation.FuncAnimation(fig, update, interval=30, blit=False, cache_frame_data=False)

plt.tight_layout()
plt.show()
ser.close()