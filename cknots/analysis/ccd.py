from dataclasses import dataclass, field
from typing import List

import numpy as np
from tqdm import tqdm

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

    def remove_duplicate_links(self):
        new_link_list = []
        link_set = set()

        for link in self.links:
            hashed_link_edges = hash(tuple([edge.edge_id for edge in link.edges]))
            if hashed_link_edges not in link_set:
                link_set.add(hashed_link_edges)
                new_link_list.append(link)

        self.links = new_link_list

    def remove_similar_links(self, min_differences=1):
        new_link_list = []

        for new_link in self.links:

            should_be_added = True

            for old_link in new_link_list:
                endpoints_new = np.array(new_link.endpoints)
                endpoints_old = np.array(old_link.endpoints)

                diff_count = np.sum(endpoints_new != endpoints_old)

                if diff_count < min_differences:
                    should_be_added = False
                    break

            if should_be_added:
                new_link_list.append(new_link)

    def save_to_file(self, path):
        out_file_contents = ''

        for link in self.links:
            out_file_contents += str(link) + '\n'

        with open(path, 'w') as f:
            f.write(out_file_contents)

    def __str__(self):
        out_str = f'CCD {self.number}: [{self.start}, {self.end}]: {self.count_links()} links'
        return out_str

    def __repr__(self):
        return str(self)
