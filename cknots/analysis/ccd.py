from dataclasses import dataclass, field
from typing import List

from cknots.analysis.link import Link, parse_links_from_string


@dataclass
class CCD:
    start: int = field(default=None)
    end: int = field(default=None)
    number: int = field(default=None)
    links: List[Link] = field(default_factory=list)

    def count_links(self):
        return len(self.links)

    def load_from_file(self, path):
        with open(path) as f:
            text = f.read()
        self.links = parse_links_from_string(text)

    def __str__(self):
        out_str = f'CCD {self.number}: [{self.start}, {self.end}]: {self.count_links()} links'
        return out_str

    def __repr__(self):
        return str(self)
