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
MAX_POINTS = 5000          

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
ys_angle = deque(maxlen=MAX_POINTS)
t0 = time.time()

# ---------- график (две области) ----------
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(11, 7), gridspec_kw={'height_ratios': [3, 1]})
fig.subplots_adjust(hspace=0.3)

# Верхний график: Сопротивление
line_raw, = ax1.plot([], [], lw=0.8, color="#00a0ff", alpha=0.6, label="сырое (Pico ADC)")
line_f,   = ax1.plot([], [], lw=2.0, color="#ff4040",            label="EMA q15 на Pico")
ax1.set_ylabel("Сопротивление, МОм")
ax1.set_title("R цветка и Угол сервы")
ax1.set_xlim(0, WINDOW_SECONDS)
ax1.grid(True, alpha=0.3)
ax1.legend(loc="upper right")

# Нижний график: Угол
line_angle, = ax2.plot([], [], lw=2.0, color="#00cc66", label="Угол сервы")
ax2.set_xlabel("Время, с")
ax2.set_ylabel("Угол, град")
ax2.set_xlim(0, WINDOW_SECONDS)
ax2.set_ylim(0, 180)
ax2.grid(True, alpha=0.3)
ax2.legend(loc="upper right")

# ---------- парсер ----------
def parse_line(s: str):
    # ожидаем "R_raw: 123 Ohm | R_EMA: 123 Ohm | Angle: 45.5 deg"
    s = s.strip()
    if not s.startswith("R_raw:"):
        return None
    try:
        # Быстрый парсинг по разделителям
        parts = s.replace('|', '').split()
        # parts = ['R_raw:', '123', 'Ohm', 'R_EMA:', '123', 'Ohm', 'Angle:', '45.5', 'deg']
        R_raw = int(parts[1])
        R_f = int(parts[4])
        angle = float(parts[7])
        return R_raw, R_f, angle
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
        R_raw, R_f, angle = parsed

        xs.append(time.time() - t0)
        ys_raw.append(R_raw)
        ys_f.append(R_f)
        ys_angle.append(angle)
        count += 1

    now = time.time()
    if now - last_report >= 1.0:
        if ys_raw:
            print(f"{count:4d} т/с | R_raw: {ys_raw[-1]/1e6:6.4f} МОм | R_EMA: {ys_f[-1]/1e6:6.4f} МОм | Угол: {ys_angle[-1]:5.1f}°")
        count = 0
        last_report = now

    if xs:
        t_end = xs[-1]
        t_start = max(0.0, t_end - WINDOW_SECONDS)

        x_arr = np.fromiter(xs,     dtype=float)
        y_raw = np.fromiter(ys_raw, dtype=float) / 1e6
        y_f   = np.fromiter(ys_f,   dtype=float) / 1e6
        y_ang = np.fromiter(ys_angle, dtype=float)

        line_raw.set_data(x_arr, y_raw)
        line_f.set_data(x_arr, y_f)
        line_angle.set_data(x_arr, y_ang)
        
        ax1.set_xlim(t_start, max(WINDOW_SECONDS, t_end))
        ax2.set_xlim(t_start, max(WINDOW_SECONDS, t_end))

        # Автомасштаб для верхнего графика
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
            ax1.set_ylim(y_min - pad, y_max + pad)

    return (line_raw, line_f, line_angle)

ani = animation.FuncAnimation(fig, update, interval=30, blit=False, cache_frame_data=False)

plt.tight_layout()
plt.show()
ser.close()