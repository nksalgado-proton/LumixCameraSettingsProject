"""Geometry utilities for lightning strike filtering."""

import math


def haversine(lat1, lon1, lat2, lon2):
    """Distance in km between two points on Earth."""
    R = 6371.0
    dlat = math.radians(lat2 - lat1)
    dlon = math.radians(lon2 - lon1)
    a = (math.sin(dlat / 2) ** 2 +
         math.cos(math.radians(lat1)) * math.cos(math.radians(lat2)) *
         math.sin(dlon / 2) ** 2)
    return R * 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))


def bearing(lat1, lon1, lat2, lon2):
    """Azimuth (0-360°) from point 1 to point 2."""
    lat1, lat2 = math.radians(lat1), math.radians(lat2)
    dlon = math.radians(lon2 - lon1)
    x = math.sin(dlon) * math.cos(lat2)
    y = (math.cos(lat1) * math.sin(lat2) -
         math.sin(lat1) * math.cos(lat2) * math.cos(dlon))
    return (math.degrees(math.atan2(x, y)) + 360) % 360


def is_in_fov(azimuth, center, width):
    """Check if azimuth falls within field of view."""
    half = width / 2
    low = (center - half) % 360
    high = (center + half) % 360
    if low < high:
        return low <= azimuth <= high
    else:  # wraps around 0°
        return azimuth >= low or azimuth <= high


def compass_direction(azimuth):
    """Convert azimuth to cardinal direction."""
    dirs = ["N", "NE", "E", "SE", "S", "SW", "W", "NW"]
    idx = round(azimuth / 45) % 8
    return dirs[idx]


def compass_sector(azimuth):
    """Return sector index (0-7) for compass display: N=0, NE=1, ..., NW=7."""
    return round(azimuth / 45) % 8


def encode_geohash(lat, lon, precision=4):
    """Encode lat/lon to geohash string (pure Python)."""
    chars = "0123456789bcdefghjkmnpqrstuvwxyz"
    lat_range, lon_range = [-90.0, 90.0], [-180.0, 180.0]
    bits = [16, 8, 4, 2, 1]
    geohash = []
    is_lon = True
    bit = 0
    ch = 0
    while len(geohash) < precision:
        if is_lon:
            mid = (lon_range[0] + lon_range[1]) / 2
            if lon > mid:
                ch |= bits[bit]
                lon_range[0] = mid
            else:
                lon_range[1] = mid
        else:
            mid = (lat_range[0] + lat_range[1]) / 2
            if lat > mid:
                ch |= bits[bit]
                lat_range[0] = mid
            else:
                lat_range[1] = mid
        is_lon = not is_lon
        bit += 1
        if bit == 5:
            geohash.append(chars[ch])
            bit = 0
            ch = 0
    return "".join(geohash)


def filter_strike(lat, lon, observer_lat, observer_lon, radius_km, fov_center, fov_width):
    """Filter a single strike. Returns dict with info if it passes, None otherwise."""
    dist = haversine(observer_lat, observer_lon, lat, lon)
    if dist > radius_km:
        return None
    az = bearing(observer_lat, observer_lon, lat, lon)
    if not is_in_fov(az, fov_center, fov_width):
        return None
    return {
        "lat": lat,
        "lon": lon,
        "distance_km": round(dist, 1),
        "azimuth": round(az, 1),
        "direction": compass_direction(az),
        "sector": compass_sector(az),
    }
