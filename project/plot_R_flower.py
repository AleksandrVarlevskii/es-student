import matplotlib
matplotlib.use("TkAgg")

import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import numpy as np
import sys
import time

# ---------- настройки ----------
PORT = "COM7"
BAUD = 115200
WINDOW_SECONDS = 5
SLEEP_MS = 1
SAMPLE_RATE_HZ = int(1000 / SLEEP_MS)
MAX_POINTS = int(WINDOW_SECONDS * SAMPLE_RATE_HZ)   # 5000

G = 3.2          # коэффициент усиления
V_ref = 3.3      # напряжение МК
R0 = 5e6        # сопротивление до цветка [Ом]

# --- параметры авто-масштаба по Y (в Омах) ---
MIN_SPAN_OHM = 50e3      # минимальный вертикальный диапазон: 50 кОм
Y_PAD_FRAC   = 0.05      # 5% отступ сверху/снизу от видимых данных

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
xs = deque(maxlen=MAX_POINTS)   # время, с
ys = deque(maxlen=MAX_POINTS)   # сопротивление, Ом
t0 = time.time()

# ---------- график ----------
fig, ax = plt.subplots(figsize=(10, 5))
line, = ax.plot([], [], lw=1.0, color="#00a0ff")
ax.set_xlabel("Время, с")
ax.set_ylabel("Сопротивление, МОм")
ax.set_title("Сопротивление цветка (Raspberry Pi Pico, live)")
ax.set_xlim(0, WINDOW_SECONDS)
ax.grid(True, alpha=0.3)

# ---------- парсер напряжения ----------
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

    # --- читаем всё, что накопилось в порту ---
    while ser.in_waiting:
        try:
            line_bytes = ser.readline().decode("ascii", errors="ignore")
        except Exception:
            continue

        v = parse_voltage(line_bytes)
        if v is None:
            continue

        # защита от деления на ноль / отрицательных значений
        denom = G * V_ref - v
        if denom <= 1e-3:
            continue

        R_flower = v * R0 / denom        # Ом

        xs.append(time.time() - t0)
        ys.append(R_flower)
        count += 1

    # --- отчёт раз в секунду ---
    now = time.time()
    if now - last_report >= 1.0:
        last_r = f"{ys[-1]/1e6:.3f} МОм" if ys else "—"
        print(f"{count} точек/с, последнее R = {last_r}")
        count = 0
        last_report = now

    # --- обновляем график и авто-масштаб по видимому окну ---
    if xs:
        t_end = xs[-1]
        t_start = max(0.0, t_end - WINDOW_SECONDS)

        x_arr = np.fromiter(xs, dtype=float)
        y_arr = np.fromiter(ys, dtype=float)

        mask = x_arr >= t_start
        x_vis = x_arr[mask]
        y_vis = y_arr[mask]

        line.set_data(x_arr, y_arr / 1e6)   # переводим в МОм для отображения
        ax.set_xlim(t_start, max(WINDOW_SECONDS, t_end))

        if y_vis.size:
            y_min = y_vis.min()
            y_max = y_vis.max()

            # минимальный span, чтобы шум не «раздувал» график
            if y_max - y_min < MIN_SPAN_OHM:
                c = 0.5 * (y_max + y_min)
                y_min = c - MIN_SPAN_OHM / 2
                y_max = c + MIN_SPAN_OHM / 2

            pad = Y_PAD_FRAC * (y_max - y_min)
            ax.set_ylim((y_min - pad) / 1e6, (y_max + pad) / 1e6)

    return (line,)

ani = animation.FuncAnimation(fig, update, interval=30, blit=False, cache_frame_data=False)

plt.tight_layout()
plt.show()
ser.close()