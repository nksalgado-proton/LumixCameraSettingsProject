"""Telegram notification sender with compass display."""

import requests
import logging

log = logging.getLogger(__name__)

# Compass grid positions: sector -> (row, col) in a 5x5 grid
#   N=0, NE=1, E=2, SE=3, S=4, SW=5, W=6, NW=7
SECTOR_POS = {
    0: (0, 2),  # N  - top center
    1: (1, 3),  # NE - upper right
    2: (2, 4),  # E  - middle right
    3: (3, 3),  # SE - lower right
    4: (4, 2),  # S  - bottom center
    5: (3, 1),  # SW - lower left
    6: (2, 0),  # W  - middle left
    7: (1, 1),  # NW - upper left
}

SECTOR_LABELS = {0: "N", 1: "NE", 2: "E", 3: "SE", 4: "S", 5: "SW", 6: "W", 7: "NW"}


class TelegramNotifier:
    def __init__(self, bot_token, chat_id):
        self.bot_token = bot_token
        self.chat_id = chat_id
        self.api_url = f"https://api.telegram.org/bot{bot_token}/sendMessage"

    def send_message(self, text, parse_mode="HTML"):
        """Send a message via Telegram Bot API."""
        try:
            resp = requests.post(self.api_url, json={
                "chat_id": self.chat_id,
                "text": text,
                "parse_mode": parse_mode,
            }, timeout=10)
            if resp.status_code != 200:
                log.error("Telegram error %s: %s", resp.status_code, resp.text)
            return resp.status_code == 200
        except Exception as e:
            log.error("Telegram send failed: %s", e)
            return False

    def send_strike_alert(self, strikes, prev_avg_dist=None):
        """Send a batched lightning strike alert with compass."""
        if not strikes:
            return

        count = len(strikes)
        closest = min(strikes, key=lambda s: s["distance_km"])
        farthest = max(strikes, key=lambda s: s["distance_km"])
        avg_dist = sum(s["distance_km"] for s in strikes) / count

        # Compass
        compass = self._build_compass(strikes)

        # Approach detection
        approach = ""
        if prev_avg_dist is not None:
            diff = prev_avg_dist - avg_dist
            if diff > 1:
                approach = "\n⚠️ <b>SE APROXIMANDO</b> (média ficando mais perto)"
            elif diff < -1:
                approach = "\n↗️ Se afastando"

        msg = (
            f"⚡ <b>ALERTA DE RAIOS</b>\n\n"
            f"<b>{count}</b> raio{'s' if count > 1 else ''} nos últimos 5 min\n"
            f"Mais perto: <b>{closest['distance_km']} km</b> ({closest['direction']})\n"
            f"Mais longe: <b>{farthest['distance_km']} km</b> ({farthest['direction']})\n"
            f"{approach}\n\n"
            f"<pre>{compass}</pre>\n\n"
            f"🗺 <a href=\"https://map.blitzortung.org/#12/-23.53/-46.72\">Mapa ao vivo</a>\n"
            f"📷 Hora de preparar o equipamento!"
        )
        self.send_message(msg)

    def send_forecast_alert(self, windows):
        """Send a thunderstorm forecast alert."""
        if not windows:
            return

        lines = []
        for w in windows:
            icon = "🔴" if w["risk"] == "ALTO" else "🟡"
            lines.append(
                f"  {icon} <b>{w['time']}</b> — Risco {w['risk']}\n"
                f"      CAPE: {w['cape']} J/kg | LI: {w['li']}"
            )

        msg = (
            f"🌩 <b>PREVISÃO DE TEMPESTADE</b>\n\n"
            f"Janelas de risco nas próximas horas:\n\n"
            + "\n".join(lines)
            + "\n\n📷 Prepare o equipamento para fotos de raios!"
        )
        self.send_message(msg)

    def send_test(self):
        """Send a test message to verify the bot works."""
        msg = (
            "✅ <b>Lightning Alert — Teste OK</b>\n\n"
            "Bot configurado e funcionando.\n"
            "Você receberá alertas quando houver raios no seu campo de visão."
        )
        return self.send_message(msg)

    def _build_compass(self, strikes):
        """Build ASCII compass showing strike distribution."""
        # Count strikes per sector
        sector_counts = {}
        for s in strikes:
            sec = s["sector"]
            sector_counts[sec] = sector_counts.get(sec, 0) + 1

        # Build density indicators
        def density(n):
            if n == 0:
                return "·"
            elif n <= 3:
                return "⚡"
            elif n <= 10:
                return "⚡⚡"
            else:
                return "⚡⚡⚡"

        # 5x5 grid
        grid = [["     " for _ in range(5)] for _ in range(5)]
        grid[2][2] = " [📍] "  # observer

        for sec, (r, c) in SECTOR_POS.items():
            cnt = sector_counts.get(sec, 0)
            label = SECTOR_LABELS[sec]
            if cnt > 0:
                grid[r][c] = f"{density(cnt)}{label:>2}"
            else:
                grid[r][c] = f"  {label:>2} "

        return "\n".join("".join(row) for row in grid)
