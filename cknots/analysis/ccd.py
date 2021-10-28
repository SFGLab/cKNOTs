from dataclasses import dataclass
from typing import List

from cknots.analysis.link import Link, parse_links_from_string


@dataclass
class CCD:
    start: int
    end: int
    links: List[Link]

    def load_from_file(self, path):
        with open(path) as f:
            minor_text = f.read()

        self.links = parse_links_from_string(minor_text)
