"""Open-Meteo forecast client for thunderstorm prediction."""

import requests
import logging
from datetime import datetime

log = logging.getLogger(__name__)


class ForecastClient:
    def __init__(self, config):
        self.lat = config["observer"]["latitude"]
        self.lon = config["observer"]["longitude"]
        self.cfg = config["open_meteo"]
        self.forecast_hours = self.cfg["forecast_hours"]
        self.cape_high = self.cfg["cape_threshold_high"]
        self.cape_mod = self.cfg["cape_threshold_moderate"]
        self.li_high = self.cfg["lifted_index_threshold_high"]
        self.li_mod = self.cfg["lifted_index_threshold_moderate"]

    def check_forecast(self):
        """Query Open-Meteo and return thunderstorm risk windows."""
        url = self.cfg["base_url"]
        params = {
            "latitude": self.lat,
            "longitude": self.lon,
            "hourly": "weather_code,cape,lifted_index",
            "forecast_days": 2,
            "timezone": "America/Sao_Paulo",
        }
        try:
            resp = requests.get(url, params=params, timeout=15)
            resp.raise_for_status()
            data = resp.json()
            return self._parse_response(data)
        except Exception as e:
            log.error("Open-Meteo request failed: %s", e)
            return []

    def _parse_response(self, data):
        """Extract thunderstorm risk windows from forecast data."""
        hourly = data.get("hourly", {})
        times = hourly.get("time", [])
        weather_codes = hourly.get("weather_code", [])
        capes = hourly.get("cape", [])
        lis = hourly.get("lifted_index", [])

        now = datetime.now()
        windows = []

        for i, time_str in enumerate(times):
            t = datetime.fromisoformat(time_str)

            # Only look at future hours within forecast window
            hours_ahead = (t - now).total_seconds() / 3600
            if hours_ahead < 0 or hours_ahead > self.forecast_hours:
                continue

            wcode = weather_codes[i] if i < len(weather_codes) else 0
            cape = capes[i] if i < len(capes) else 0
            li = lis[i] if i < len(lis) else 0

            # Handle None values
            cape = cape or 0
            li = li or 0

            risk = self._assess_risk(cape, li, wcode)
            if risk:
                windows.append({
                    "time": t.strftime("%H:%M"),
                    "risk": risk,
                    "cape": int(cape),
                    "li": round(li, 1),
                    "weather_code": wcode,
                })

        # Merge consecutive same-risk windows
        return self._merge_windows(windows)

    def _assess_risk(self, cape, li, wcode):
        """Assess thunderstorm risk level."""
        # WMO codes 95, 96, 99 = thunderstorm
        if wcode >= 95:
            return "ALTO"
        if cape >= self.cape_high and li <= self.li_high:
            return "ALTO"
        if cape >= self.cape_mod and li <= self.li_mod:
            return "MODERADO"
        return None

    def _merge_windows(self, windows):
        """Merge consecutive windows with the same risk level."""
        if not windows:
            return []

        merged = [windows[0].copy()]
        for w in windows[1:]:
            prev = merged[-1]
            if w["risk"] == prev["risk"]:
                prev["time"] = f"{prev['time'].split('-')[0]}-{w['time']}"
                prev["cape"] = max(prev["cape"], w["cape"])
                prev["li"] = min(prev["li"], w["li"])
            else:
                merged.append(w.copy())

        return merged
