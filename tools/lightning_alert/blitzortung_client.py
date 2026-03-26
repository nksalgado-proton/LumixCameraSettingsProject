"""Blitzortung MQTT client for real-time lightning data."""

import json
import time
import logging
import threading
import paho.mqtt.client as mqtt
from geo_utils import filter_strike

log = logging.getLogger(__name__)


class BlitzortungClient:
    def __init__(self, config, on_strike_callback):
        """
        config: full config dict
        on_strike_callback: function(strike_info) called for each strike in FOV
        """
        self.config = config
        self.callback = on_strike_callback
        self.obs_lat = config["observer"]["latitude"]
        self.obs_lon = config["observer"]["longitude"]
        self.radius = config["filter"]["radius_km"]
        self.fov_center = config["filter"]["fov_center_azimuth"]
        self.fov_width = config["filter"]["fov_width_degrees"]

        bz = config["blitzortung"]
        self.mqtt_host = bz["mqtt_host"]
        self.mqtt_port = bz["mqtt_port"]
        self.geohash = bz["geohash_prefix"]

        self.client = None
        self._running = False

    def _build_topic(self):
        """Build MQTT topic from geohash prefix.
        e.g. geohash '6gyf' -> 'blitzortung/1.1/6/g/y/f/#'
        """
        parts = "/".join(self.geohash)
        return f"blitzortung/1.1/{parts}/#"

    def _on_connect(self, client, userdata, flags, reason_code, properties=None):
        """Called when connected to MQTT broker."""
        topic = self._build_topic()
        client.subscribe(topic)
        log.info("Conectado ao Blitzortung MQTT. Tópico: %s", topic)

    def _on_disconnect(self, client, userdata, flags, reason_code, properties=None):
        """Called when disconnected — paho handles auto-reconnect."""
        log.warning("Desconectado do MQTT (rc=%s). Reconectando...", reason_code)

    def _on_message(self, client, userdata, msg):
        """Called for each lightning strike message."""
        try:
            data = json.loads(msg.payload)
            lat = data.get("lat")
            lon = data.get("lon")
            if lat is None or lon is None:
                return

            strike = filter_strike(
                lat, lon,
                self.obs_lat, self.obs_lon,
                self.radius, self.fov_center, self.fov_width,
            )
            if strike:
                strike["time"] = data.get("time", 0)
                self.callback(strike)

        except json.JSONDecodeError:
            pass
        except Exception as e:
            log.error("Erro ao processar strike: %s", e)

    def connect(self):
        """Connect to MQTT broker."""
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.client.on_message = self._on_message
        self.client.reconnect_delay_set(min_delay=1, max_delay=60)

        log.info("Conectando a %s:%s...", self.mqtt_host, self.mqtt_port)
        self.client.connect(self.mqtt_host, self.mqtt_port, keepalive=60)

    def run(self):
        """Blocking: start MQTT loop."""
        self._running = True
        self.connect()
        self.client.loop_forever()

    def stop(self):
        """Stop the MQTT loop."""
        self._running = False
        if self.client:
            self.client.disconnect()
            self.client.loop_stop()

    def run_in_thread(self):
        """Start MQTT loop in a background thread."""
        self.connect()
        self.client.loop_start()
        log.info("MQTT client rodando em background thread")
