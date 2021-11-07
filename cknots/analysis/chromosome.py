import json
import os
import re
from dataclasses import dataclass, field
from typing import List

from matplotlib import pyplot as plt
from matplotlib.collections import PolyCollection

from cknots.analysis.ccd import CCD
from cknots.analysis.link import Link


@dataclass
class Chromosome:
    ccds: List[CCD] = field(default=None)
    name: str = field(default=None)

    def get_ccd(self, number: int):
        for ccd in self.ccds:
            if ccd.number == number:
                return ccd
        return None

    def count_links(self):
        link_count = 0
        for ccd in self.ccds:
            link_count += len(ccd.links)
        return link_count

    def load_from_path(self, path, name=None):
        """
        path: path to directory with cKNOTs results for given chromosome.
        chromosome_number: 1 to 22 or 'X' or 'Y'
        """

        if name is None:
            regex_groups = re.findall('chr_(\d+|X|Y)', path)
            if len(regex_groups) > 0:
                if regex_groups[-1] in ('X', 'Y'):
                    self.name = regex_groups[-1]
                elif regex_groups[-1] in [str(x) for x in range(1, 23)]:
                    self.name = str(int(regex_groups[-1]))
                else:
                    raise ValueError(
                        'Cannot extract chromosome name from path. Please specify chromosome_number argument.')
            else:
                raise ValueError('Cannot extract chromosome name from path. Please specify chromosome_number argument.')
        else:
            self.name = str(name)

        with open(os.path.join(path, 'results.json')) as f:
            results_data = json.load(f)

        self.ccds = []
        for ccd_results in results_data:

            ccd = CCD(ccd_results['ccd_start'],
                      ccd_results['ccd_end'],
                      int(ccd_results['input_filename'].replace('.mp', '')[-4:]))

            ccd.load_from_file(os.path.join(path, ccd_results['results_filename']))
            self.ccds.append(ccd)

    def plot(self):

        rectangles = []
        for link in self.get_links():
            r = [
                (link.min(), 0),
                (link.min(), 1),
                (link.max(), 1),
                (link.max(), 0),
                (link.min(), 0)
            ]
            rectangles.append(r)

        links = PolyCollection(rectangles, facecolors='red', zorder=2)

        rectangles = []
        for ccd in self.ccds:
            r = [
                (ccd.start, 0),
                (ccd.start, 1),
                (ccd.end, 1),
                (ccd.end, 0),
                (ccd.start, 0)
            ]
            rectangles.append(r)

        ccds = PolyCollection(rectangles, facecolors='cyan', zorder=0)

        plt.figure(figsize=(16, 3))
        ax = plt.gca()
        ax.add_collection(links)
        ax.add_collection(ccds)

        ax.autoscale()
        ax.get_yaxis().set_ticks([])

        plt.title(f"Chromosome {self.name}")
        plt.xlabel("Location (in relation to HG38 reference genome)")

        plt.show()

    def get_links(self) -> List[Link]:
        links = []
        for ccd in self.ccds:
            links = links + ccd.links

        return links

    def __get_links_locations(self):
        return [{'min': x.min(), 'max': x.max()} for x in self.get_links()]

    def __str__(self):
        return f'Chromosome {self.name} with {len(self.ccds)} CCDs containing {self.count_links()} links.'

    def __repr__(self):
        return str(self)