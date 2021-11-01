import os
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

    def __str__(self):
        out_str = f'Cell line {self.name} of {len(self.chromosomes)} chromosomes:\n'
        for chromosome in self.chromosomes:
            out_str += str(chromosome) + '\n'
        return out_str

    def __repr__(self):
        return str(self)
