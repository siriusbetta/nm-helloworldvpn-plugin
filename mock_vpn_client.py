#!/usr/bin/env python3
import sys
import time
import signal

def on_terminate(signum, frame):
    # Демон вызовет process.terminate() -> приходит SIGTERM
    print("disconnected", flush=True)
    sys.exit(0)

# Регистрируем обработчик graceful shutdown
signal.signal(signal.SIGTERM, on_terminate)

# 1. Эмуляция попытки подключения
print("try to connect", flush=True)
time.sleep(5)

# 2. Успешное подключение
print("connected", flush=True)

# 3. Держим процесс живым (демон читает этот пайп в фоне)
while True:
    time.sleep(1)
