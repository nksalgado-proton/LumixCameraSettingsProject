"""
Session management for NKS Focus Culler v3.
Sessions stored in %LOCALAPPDATA%/NKSFocusCuller/sessions/.
All decisions are deferred — files copied only on explicit Commit.
"""

import hashlib
import json
import os
import platform
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Optional


def _sessions_dir() -> Path:
    """Get the sessions directory (platform-aware)."""
    if platform.system() == 'Windows':
        base = os.environ.get('LOCALAPPDATA',
                              os.path.expanduser('~'))
        d = Path(base) / 'NKSFocusCuller' / 'sessions'
    else:
        d = Path.home() / '.local' / 'share' / 'nks_focus_culler' / 'sessions'
    d.mkdir(parents=True, exist_ok=True)
    return d


def _session_id(source: str, destination: str) -> str:
    """Stable ID from source+dest paths."""
    key = f'{source}|{destination}'.lower().replace('\\', '/')
    return hashlib.md5(key.encode()).hexdigest()[:12]


@dataclass
class PhotoRecord:
    path: str
    companions: list[str] = field(default_factory=list)
    timestamp: str = ''
    burst_id: int = -1
    classification: str = 'general-scene'
    confidence: str = 'LOW'
    decision: str = 'pending'  # pending | keep | discard
    scenario_override: Optional[str] = None

    @property
    def effective_scenario(self) -> str:
        return self.scenario_override or self.classification

    @property
    def all_files(self) -> list[str]:
        return [self.path] + self.companions


@dataclass
class BurstRecord:
    id: int
    photo_indices: list[int]
    mode: str = 'solo'  # solo | grid | tournament
    status: str = 'pending'  # pending | in_progress | completed
    stack_group: int = -1  # for stacks: which stack number


@dataclass
class Session:
    source: str = ''
    destination: str = ''
    created: str = ''
    gap: float = 0.5
    photos: list[PhotoRecord] = field(default_factory=list)
    bursts: list[BurstRecord] = field(default_factory=list)
    current_burst_idx: int = 0
    committed: bool = False
    _stack_counter: int = 0

    @property
    def total_photos(self) -> int:
        return len(self.photos)

    @property
    def total_bursts(self) -> int:
        return len(self.bursts)

    @property
    def keep_count(self) -> int:
        return sum(1 for p in self.photos if p.decision == 'keep')

    @property
    def discard_count(self) -> int:
        return sum(1 for p in self.photos if p.decision == 'discard')

    @property
    def pending_count(self) -> int:
        return sum(1 for p in self.photos if p.decision == 'pending')

    @property
    def completed_bursts(self) -> int:
        return sum(1 for b in self.bursts if b.status == 'completed')

    @property
    def burst_count(self) -> int:
        return sum(1 for b in self.bursts
                   if len(b.photo_indices) > 1
                   and b.photo_indices[0] not in
                   [i for b2 in self.bursts
                    if self.photos[b2.photo_indices[0]].classification == 'stacks'
                    for i in b2.photo_indices])

    @property
    def single_count(self) -> int:
        return sum(1 for b in self.bursts if len(b.photo_indices) == 1)

    @property
    def stack_count(self) -> int:
        return sum(1 for b in self.bursts
                   if self.photos[b.photo_indices[0]].classification == 'stacks')

    def next_stack_id(self) -> int:
        self._stack_counter += 1
        return self._stack_counter

    def files_to_copy(self) -> list[tuple[str, str]]:
        """(source_path, scenario_subfolder) for all kept photos."""
        result = []
        for i, p in enumerate(self.photos):
            if p.decision == 'keep':
                scenario = p.effective_scenario
                # Stacks get numbered subdirectory
                if scenario == 'stacks':
                    burst = next((b for b in self.bursts
                                  if i in b.photo_indices), None)
                    if burst and burst.stack_group >= 0:
                        scenario = f'stacks/stack_{burst.stack_group:03d}'
                for filepath in p.all_files:
                    result.append((filepath, scenario))
        return result

    def mark_burst_photos(self, burst_idx: int, selected: set[int],
                          scenario: str) -> None:
        burst = self.bursts[burst_idx]
        for local_idx, photo_idx in enumerate(burst.photo_indices):
            photo = self.photos[photo_idx]
            photo.decision = 'keep' if local_idx in selected else 'discard'
            if scenario != photo.classification:
                photo.scenario_override = scenario
        burst.status = 'completed'

    def reset_burst(self, burst_idx: int) -> None:
        burst = self.bursts[burst_idx]
        for photo_idx in burst.photo_indices:
            self.photos[photo_idx].decision = 'pending'
            self.photos[photo_idx].scenario_override = None
        burst.status = 'pending'

    def reset_all(self) -> None:
        for p in self.photos:
            p.decision = 'pending'
            p.scenario_override = None
        for b in self.bursts:
            b.status = 'pending'
        self.current_burst_idx = 0
        self.committed = False

    def accept_all_remaining(self) -> None:
        for p in self.photos:
            if p.decision == 'pending':
                p.decision = 'keep'
        for b in self.bursts:
            if b.status != 'completed':
                b.status = 'completed'

    def reject_all_remaining(self) -> None:
        for p in self.photos:
            if p.decision == 'pending':
                p.decision = 'discard'
        for b in self.bursts:
            if b.status != 'completed':
                b.status = 'completed'


def session_file(source: str, destination: str) -> Path:
    sid = _session_id(source, destination)
    return _sessions_dir() / f'session_{sid}.json'


def save_session(session: Session, path: Path | None = None) -> None:
    if path is None:
        path = session_file(session.source, session.destination)
    path.parent.mkdir(parents=True, exist_ok=True)
    data = {
        'source': session.source,
        'destination': session.destination,
        'created': session.created,
        'gap': session.gap,
        'current_burst_idx': session.current_burst_idx,
        'committed': session.committed,
        '_stack_counter': session._stack_counter,
        'photos': [asdict(p) for p in session.photos],
        'bursts': [asdict(b) for b in session.bursts],
    }
    tmp = path.with_suffix('.tmp')
    with open(tmp, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
    tmp.replace(path)


def load_session(source: str, destination: str) -> Optional[Session]:
    path = session_file(source, destination)
    if not path.exists():
        return None
    try:
        with open(path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        s = Session(
            source=data['source'],
            destination=data['destination'],
            created=data['created'],
            gap=data.get('gap', 0.5),
            current_burst_idx=data.get('current_burst_idx', 0),
            committed=data.get('committed', False),
            _stack_counter=data.get('_stack_counter', 0),
        )
        for pd in data.get('photos', []):
            s.photos.append(PhotoRecord(**pd))
        for bd in data.get('bursts', []):
            s.bursts.append(BurstRecord(**bd))
        return s
    except (json.JSONDecodeError, KeyError, TypeError) as e:
        print(f'Warning: could not load session: {e}')
        return None


def delete_session(source: str, destination: str) -> None:
    path = session_file(source, destination)
    if path.exists():
        path.unlink()
