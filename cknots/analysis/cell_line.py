import os
import shutil
import warnings
from dataclasses import dataclass, field
from typing import List

from cknots.analysis.chromosome import Chromosome


@dataclass
class CellLine:
    name: str = field(default='')
    chromosomes: List[Chromosome] = field(default_factory=list)

    def load_from_path(self, path, name=None):

        if name is not None:
            self.name = name
        else:
            self.name = os.path.split(path)[-1]

        for chr_dir in os.listdir(path):
            if chr_dir.startswith('chr_'):
                chromosome_to_add = Chromosome()
                chromosome_to_add.load_from_path(
                    os.path.join(path, chr_dir)
                )
                self.chromosomes.append(chromosome_to_add)

    def remove_duplicate_links(self):
        for chromosome in self.chromosomes:
            chromosome.remove_duplicate_links()

    def remove_similar_links(self, min_differences):
        for chromosome in self.chromosomes:
            chromosome.remove_similar_links(min_differences)

    def save_to_path(self, path):

        name = os.path.split(path)[-1]

        try:
            os.makedirs(path)
        except FileExistsError:
            shutil.rmtree(path)
            os.makedirs(path)
            warnings.warn(f'Overwriting {path}')

        for chromosome in self.chromosomes:
            chr_path = os.path.join(path, f'chr_{chromosome.name}')
            chromosome.save_to_path(name, chr_path)

    def __str__(self):
        out_str = f'Cell line {self.name} of {len(self.chromosomes)} chromosomes:\n'
        for chromosome in self.chromosomes:
            out_str += str(chromosome) + '\n'
        return out_str

    def __repr__(self):
        return str(self)
