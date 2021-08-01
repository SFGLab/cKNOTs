"""
Finds protein motif orientation in both ends
of PET in bedpe file.

Usage:
    cknots.py preprocess orientation <in_bedpe> <in_motif> <in_ref> <out_bedpe>
    cknots.py (-h | --help)

Options:
    -h --help     Show this help message.
"""


import multiprocessing

import dask.dataframe
from docopt import docopt
import numpy as np
import pandas as pd
from Bio import SeqIO, motifs, SeqRecord


BEDPE_COLS = [
    'chrom1', 'start1', 'end1', 'chrom2', 'start2', 'end2', 'count'
]

MOTIF_COLS = [
    'pos', 'orientation', 'score', 'chromosome'
]

CHROMOSOMES = list(range(1, 23)) + ['X', 'Y']

MOTIF_ORIENTATION = None


def check_motif_orientation(input_bedpe: str, motif: str, reference: str, output: str):

    bedpe = pd.read_csv(input_bedpe, header=None, sep='\t', names=BEDPE_COLS)

    out_bedpe = pd.DataFrame()

    for chr_name in CHROMOSOMES:
        print(f'[INFO] Running chromosome {chr_name}...')
        file_fa = read_fasta(reference, chr_name)
        seq = file_fa.seq
        motif_orientation = get_motif_orientation(motif, chr_name, seq)
        transform_function = get_transform_function(motif_orientation)

        bedpe_chr = bedpe[bedpe['chrom1'] == f'chr{chr_name}']

        bedpe_dd = dask.dataframe.from_pandas(bedpe_chr,
                                              npartitions=2 * multiprocessing.cpu_count())

        orientation_columns = bedpe_dd \
            .map_partitions(lambda df: df.apply(transform_function, axis=1)) \
            .compute(scheduler='processes')\
            .str\
            .split(',', expand=True)\
            .rename(columns={0: 'orientation1', 1: 'orientation2'})

        bedpe_chr.insert(7, 'orientation1', orientation_columns['orientation1'],)
        bedpe_chr.insert(8, 'orientation2', orientation_columns['orientation2'])

        out_bedpe = pd.concat([out_bedpe, bedpe_chr], ignore_index=True)

    out_bedpe.to_csv(path_or_buf=output, sep='\t', index=False, header=False)
    return None


def read_fasta(fasta_path: str, chromosome: object) -> SeqRecord.SeqRecord:
    """
    Returns reference sequence for given chromosome.
    Parameters:
        fasta_path [str]: path to reference genome (fasta file)
        chromosome [object]: number of chromosome (1..22) or 'X' or 'Y'
    Output:
        [Bio.SeqRecord.SeqRecord]: reference sequence
    """
    with open(fasta_path) as f:
        return next(x for x in SeqIO.parse(f, 'fasta') if x.id == f'chr{chromosome}')


def get_motif_orientation(motif: str, chromosome: object, seq) -> pd.DataFrame:
    with open(motif) as f:
        for motif in motifs.parse(f, "jaspar"):
            pssm = motif.pssm
            search_res = pssm.search(seq, threshold=0, both=True)
            clean_res = [(abs(x[0]), orientation(x[0]), x[1], f'chr{chromosome}') for x in search_res]
            output_motif_orientation = pd.DataFrame(clean_res, columns=MOTIF_COLS)

    return output_motif_orientation


def orientation(x):
    if x > 0:
        return '+'
    else:
        return '-'


def get_transform_function(motif_orientation):
    def transform(row):
        indices_1 = np.logical_and(
            motif_orientation['pos'] > row['start1'],
            motif_orientation['pos'] < row['end1']
        )

        indices_2 = np.logical_and(
            motif_orientation['pos'] > row['start2'],
            motif_orientation['pos'] < row['end2']
        )

        if len(motif_orientation[indices_1]) > 0:
            orientation_1 = motif_orientation[indices_1] \
                .sort_values('score', ascending='False') \
                .iloc[0, 1]
        else:
            orientation_1 = '.'

        if len(motif_orientation[indices_2]) > 0:
            orientation_2 = motif_orientation[indices_2] \
                .sort_values('score', ascending='False') \
                .iloc[0, 1]
        else:
            orientation_2 = '.'

        return f'{orientation_1},{orientation_2}'
    return transform


if __name__ == '__main__':
    parsed_args = docopt(__doc__)
    check_motif_orientation(
        input_bedpe=parsed_args['<in_bedpe>'],
        motif=parsed_args['<in_motif>'],
        reference=parsed_args['<in_ref>'],
        output=parsed_args['<out_bedpe>']
    )
