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
MAX_POINTS = 5000          # 5 c при 1000 Гц приёма; при 100 Гц хватит 500

MIN_SPAN_OHM = 50e3
Y_PAD_FRAC   = 0.05

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
xs     = deque(maxlen=MAX_POINTS)
ys_raw = deque(maxlen=MAX_POINTS)
ys_f   = deque(maxlen=MAX_POINTS)
t0 = time.time()

# ---------- график ----------
fig, ax = plt.subplots(figsize=(11, 5))
line_raw, = ax.plot([], [], lw=0.8, color="#00a0ff", alpha=0.6, label="сырое (Pico ADC)")
line_f,   = ax.plot([], [], lw=2.0, color="#ff4040",            label="EMA q15 на Pico")
ax.set_xlabel("Время, с")
ax.set_ylabel("Сопротивление, МОм")
ax.set_title("R цветка: сырое и EMA, посчитанные на Pico")
ax.set_xlim(0, WINDOW_SECONDS)
ax.grid(True, alpha=0.3)
ax.legend(loc="upper right")

# ---------- парсер ----------
def parse_line(s: str):
    # ожидаем "R: <raw> <ema>"
    s = s.strip()
    if not s.startswith("R:"):
        return None
    try:
        parts = s[2:].split()
        return int(parts[0]), int(parts[1])
    except (ValueError, IndexError):
        return None

count = 0
last_report = time.time()

def update(frame):
    global count, last_report

    while ser.in_waiting:
        try:
            line_bytes = ser.readline().decode("ascii", errors="ignore")
        except Exception:
            continue

        parsed = parse_line(line_bytes)
        if parsed is None:
            continue
        R_raw, R_f = parsed

        xs.append(time.time() - t0)
        ys_raw.append(R_raw)
        ys_f.append(R_f)
        count += 1

    now = time.time()
    if now - last_report >= 1.0:
        if ys_raw:
            print(f"{count:4d} т/с | сырое: {ys_raw[-1]/1e6:7.4f} МОм"
                  f" | EMA: {ys_f[-1]/1e6:7.4f} МОм")
        count = 0
        last_report = now

    if xs:
        t_end = xs[-1]
        t_start = max(0.0, t_end - WINDOW_SECONDS)

        x_arr = np.fromiter(xs,     dtype=float)
        y_raw = np.fromiter(ys_raw, dtype=float) / 1e6
        y_f   = np.fromiter(ys_f,   dtype=float) / 1e6

        line_raw.set_data(x_arr, y_raw)
        line_f.set_data(x_arr, y_f)
        ax.set_xlim(t_start, max(WINDOW_SECONDS, t_end))

        mask = x_arr >= t_start
        if mask.any():
            y_vis = np.concatenate([y_raw[mask], y_f[mask]])
            y_min = y_vis.min()
            y_max = y_vis.max()
            if y_max - y_min < MIN_SPAN_OHM / 1e6:
                c = 0.5 * (y_max + y_min)
                y_min = c - MIN_SPAN_OHM / 2e6
                y_max = c + MIN_SPAN_OHM / 2e6
            pad = Y_PAD_FRAC * (y_max - y_min)
            ax.set_ylim(y_min - pad, y_max + pad)

    return (line_raw, line_f)

ani = animation.FuncAnimation(fig, update, interval=30, blit=False, cache_frame_data=False)

plt.tight_layout()
plt.show()
ser.close()