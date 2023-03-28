import os
import shutil
import warnings
from dataclasses import dataclass, field
from typing import List

import numpy as np
import pandas as pd

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

    def get_chromosome(self, chromosome_num):
        if chromosome_num == 23:
            chromosome_num = 'X'
        chromosome_num = str(chromosome_num)

        for chromosome in self.chromosomes:
            if chromosome.name == chromosome_num:
                return chromosome

        if chromosome_num in [str(x) for x in range(1, 23)] + ['X']:
            chromosome_to_append = Chromosome(ccds=[], name=chromosome_num)
            self.chromosomes.append(
                chromosome_to_append
            )
            return chromosome_to_append

        raise RuntimeError(f'Chromosome {chromosome_num} not found.')

    def remove_duplicate_links(self):
        for chromosome in self.chromosomes:
            chromosome.remove_duplicate_links()

    def remove_similar_links(self, min_differences):
        for chromosome in self.chromosomes:
            chromosome.remove_similar_links(min_differences)

    def similarity_score(self, other: 'CellLine', tolerance=0, weight_by_size=False):
        scores = []
        weights = []
        for chr_num in range(1, 23):
            chr_this = self.get_chromosome(chr_num)
            chr_other = other.get_chromosome(chr_num)
            weights.append(max(chr_this.size(), chr_other.size()))
            scores.append(
                chr_this.similarity_score(chr_other, tolerance=tolerance)
            )
        
        if weight_by_size:
            return np.average(scores, weights=weights)
        else:
            return np.average(scores)

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

    def load_from_bed_file(self, path, name=None):
        self.name = name

        df = pd.read_csv(path,
                         sep='\t',
                         names=['chromosome', 'start', 'end'],
                         header=None)
        for chromosome in df['chromosome'].unique():
            df_chromosome = df[df['chromosome'] == chromosome]
            chromosome_obj = Chromosome(name=chromosome)
            chromosome_obj.load_from_dataframe(df_chromosome)
            self.chromosomes.append(chromosome_obj)

    def __str__(self):
        out_str = f'Cell line {self.name} of {len(self.chromosomes)} chromosomes:\n'
        for chromosome in self.chromosomes:
            out_str += str(chromosome) + '\n'
        return out_str

    def __repr__(self):
        return str(self)
