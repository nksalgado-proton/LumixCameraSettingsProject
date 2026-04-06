"""
Focus analysis tools for Burst Culler.

1. Focus peaking — highlight sharp edges with colored overlay
2. Sharpness scoring — numeric score for comparing burst frames
3. Stack animation — frame sequence for rapid playback
"""

import io
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter, ImageOps

import rawpy

JPEG_EXT = {'.jpg', '.jpeg'}


def load_preview(path: Path, size: int = 800) -> Image.Image | None:
    """Load an image preview, handling both RAW and JPEG."""
    try:
        if path.suffix.lower() in JPEG_EXT:
            img = Image.open(path)
        else:
            with rawpy.imread(str(path)) as raw:
                thumb = raw.extract_thumb()
                if thumb.format == rawpy.ThumbFormat.JPEG:
                    img = Image.open(io.BytesIO(thumb.data))
                else:
                    img = Image.fromarray(thumb.data)
        img = ImageOps.exif_transpose(img)
        img.thumbnail((size, size), Image.LANCZOS)
        return img
    except Exception:
        return None


def apply_peaking(img: Image.Image,
                  color: tuple = (255, 50, 50),
                  threshold: int = 30,
                  opacity: float = 0.7) -> Image.Image:
    """Apply focus peaking overlay to an image.

    Detects high-contrast edges (sharp areas) using Laplacian filter
    and overlays them in the specified color.

    Args:
        img: source PIL Image
        color: RGB tuple for peaking highlight (default red)
        threshold: edge detection sensitivity (lower = more edges shown)
        opacity: overlay opacity (0.0 to 1.0)

    Returns:
        New image with peaking overlay
    """
    # Convert to grayscale for edge detection
    gray = img.convert('L')

    # Apply Laplacian-like edge detection (FIND_EDGES is Laplacian based)
    edges = gray.filter(ImageFilter.FIND_EDGES)

    # Convert to numpy for thresholding
    edge_arr = np.array(edges)

    # Create mask: pixels above threshold are "in focus"
    mask = edge_arr > threshold

    # Create colored overlay
    result = img.copy().convert('RGBA')
    overlay = Image.new('RGBA', img.size, (0, 0, 0, 0))
    overlay_arr = np.array(overlay)

    # Apply color where edges detected
    overlay_arr[mask] = (*color, int(255 * opacity))

    overlay = Image.fromarray(overlay_arr, 'RGBA')
    result = Image.alpha_composite(result, overlay)

    return result.convert('RGB')


def sharpness_score(img: Image.Image) -> float:
    """Calculate a sharpness score for an image.

    Uses variance of Laplacian — higher = sharper.
    Useful for comparing frames within a burst to find the sharpest.

    Returns:
        Float score. Higher = sharper. Typical range 0-500+.
    """
    gray = img.convert('L')
    # Laplacian via FIND_EDGES
    edges = gray.filter(ImageFilter.FIND_EDGES)
    arr = np.array(edges, dtype=np.float64)
    # Variance of edge intensities = sharpness metric
    return float(arr.var())


def sharpness_map(img: Image.Image, block_size: int = 32) -> np.ndarray:
    """Create a sharpness heatmap — useful for seeing WHERE focus is.

    Returns a 2D array of sharpness scores per block.
    """
    gray = np.array(img.convert('L'), dtype=np.float64)
    h, w = gray.shape
    rows = h // block_size
    cols = w // block_size
    result = np.zeros((rows, cols))

    for r in range(rows):
        for c in range(cols):
            block = gray[r*block_size:(r+1)*block_size,
                         c*block_size:(c+1)*block_size]
            # Laplacian variance per block
            from scipy.ndimage import laplace
            lap = laplace(block)
            result[r, c] = lap.var()

    return result


def load_stack_frames(paths: list[Path],
                      size: int = 800) -> list[Image.Image]:
    """Load all frames of a stack at uniform size for animation."""
    frames = []
    for p in paths:
        img = load_preview(p, size)
        if img:
            frames.append(img)
    return frames
