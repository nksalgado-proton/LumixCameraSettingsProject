"""
Lightning Alert Monitor — Main entry point.

Usage:
    python lightning_monitor.py              # Run both real-time + forecast
    python lightning_monitor.py realtime     # Real-time MQTT only
    python lightning_monitor.py forecast     # Single forecast check, then exit
    python lightning_monitor.py test         # Send test message to Telegram
"""

import json
import sys
import time
import logging
import threading
from datetime import datetime, timedelta
from pathlib import Path

from blitzortung_client import BlitzortungClient
from forecast_client import ForecastClient
from telegram_notifier import TelegramNotifier
from geo_utils import compass_direction

# Logging setup
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler("lightning_alert.log", encoding="utf-8"),
    ],
)
log = logging.getLogger("monitor")


class LightningMonitor:
    def __init__(self, config_path="config.json"):
        with open(config_path, "r", encoding="utf-8") as f:
            self.config = json.load(f)

        tg = self.config["telegram"]
        self.notifier = TelegramNotifier(tg["bot_token"], tg["chat_id"])

        alerts = self.config["alerts"]
        self.batch_interval = alerts["batch_interval_seconds"]
        self.forecast_interval = alerts["forecast_check_interval_seconds"]
        self.quiet_start = alerts["quiet_hours_start"]
        self.quiet_end = alerts["quiet_hours_end"]
        self.min_strikes = alerts["min_strikes_to_alert"]
        self.cooldown = alerts["cooldown_seconds"]

        # Strike buffer
        self._strikes = []
        self._strikes_lock = threading.Lock()
        self._prev_avg_dist = None
        self._last_alert_time = None

    def _is_quiet_hours(self):
        """Check if current time is within quiet hours."""
        now = datetime.now().strftime("%H:%M")
        if self.quiet_start <= self.quiet_end:
            return self.quiet_start <= now <= self.quiet_end
        else:  # wraps midnight
            return now >= self.quiet_start or now <= self.quiet_end

    def _on_strike(self, strike):
        """Callback from MQTT client for each strike in FOV."""
        with self._strikes_lock:
            self._strikes.append(strike)
            log.info(
                "⚡ Raio detectado: %.1f km %s (%.4f, %.4f)",
                strike["distance_km"], strike["direction"],
                strike["lat"], strike["lon"],
            )

    def _process_batch(self):
        """Process accumulated strikes and send alert if needed."""
        with self._strikes_lock:
            batch = self._strikes.copy()
            self._strikes.clear()

        if len(batch) < self.min_strikes:
            return

        if self._is_quiet_hours():
            log.info("Horário silencioso — %d raios ignorados", len(batch))
            return

        # Cooldown check
        now = datetime.now()
        if self._last_alert_time:
            elapsed = (now - self._last_alert_time).total_seconds()
            if elapsed < self.cooldown:
                log.info("Cooldown ativo — %d raios acumulados para próximo alerta", len(batch))
                # Put them back for next batch
                with self._strikes_lock:
                    self._strikes.extend(batch)
                return

        self.notifier.send_strike_alert(batch, self._prev_avg_dist)
        self._last_alert_time = now

        # Track approach
        avg_dist = sum(s["distance_km"] for s in batch) / len(batch)
        self._prev_avg_dist = avg_dist

        log.info("Alerta enviado: %d raios, média %.1f km", len(batch), avg_dist)

    def run_realtime(self):
        """Run real-time MQTT monitoring with batched alerts."""
        log.info("Iniciando monitoramento em tempo real...")
        log.info(
            "Observador: %.4f, %.4f | Raio: %d km | FOV: %d° centrado em %s",
            self.config["observer"]["latitude"],
            self.config["observer"]["longitude"],
            self.config["filter"]["radius_km"],
            self.config["filter"]["fov_width_degrees"],
            compass_direction(self.config["filter"]["fov_center_azimuth"]),
        )

        bz_client = BlitzortungClient(self.config, self._on_strike)
        bz_client.run_in_thread()

        try:
            while True:
                time.sleep(self.batch_interval)
                self._process_batch()
        except KeyboardInterrupt:
            log.info("Parando monitoramento...")
            bz_client.stop()

    def run_forecast(self):
        """Run a single forecast check and send alert if needed."""
        log.info("Verificando previsão de tempestade...")
        fc = ForecastClient(self.config)
        windows = fc.check_forecast()

        if windows:
            log.info("Risco detectado: %d janela(s)", len(windows))
            if not self._is_quiet_hours():
                self.notifier.send_forecast_alert(windows)
        else:
            log.info("Sem risco de tempestade nas próximas %d horas",
                     self.config["open_meteo"]["forecast_hours"])

        return windows

    def run_forecast_loop(self):
        """Run forecast checks periodically in a loop."""
        while True:
            try:
                self.run_forecast()
            except Exception as e:
                log.error("Erro no forecast: %s", e)
            time.sleep(self.forecast_interval)

    def run(self):
        """Run both real-time and forecast monitoring."""
        log.info("=" * 50)
        log.info("Lightning Alert Monitor — Iniciando")
        log.info("=" * 50)

        # Start forecast loop in background thread
        forecast_thread = threading.Thread(
            target=self.run_forecast_loop, daemon=True
        )
        forecast_thread.start()
        log.info("Forecast checker iniciado (intervalo: %ds)", self.forecast_interval)

        # Run real-time in main thread (blocking)
        self.run_realtime()

    def test(self):
        """Send a test message to verify Telegram connection."""
        log.info("Enviando mensagem de teste...")
        ok = self.notifier.send_test()
        if ok:
            log.info("✅ Mensagem enviada! Verifica o Telegram.")
        else:
            log.error("❌ Falha ao enviar. Verifica token e chat_id.")
        return ok


def main():
    config_path = Path(__file__).parent / "config.json"
    monitor = LightningMonitor(str(config_path))

    mode = sys.argv[1] if len(sys.argv) > 1 else "all"

    if mode == "test":
        monitor.test()
    elif mode == "forecast":
        monitor.run_forecast()
    elif mode == "realtime":
        monitor.run_realtime()
    else:
        monitor.run()


if __name__ == "__main__":
    main()
