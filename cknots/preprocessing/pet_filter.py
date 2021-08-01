"""
Filters .bedpe file by selecting only interactions
with number of PET count greater or equal than
probided value.

Usage:
    pet_filter.py <in_bedpe> <out_bedpe> <min_pet_count>
    pet_filter.py (-h | --help)

Options:
    -h --help     Show this help message.
"""

from tqdm import tqdm
from docopt import docopt


def filter_by_pet_count(input_bedpe: str, output: str, min_pet_count: int) -> None:
    in_file_length = count_lines(input_bedpe)
    print(f'[INFO] Processing interactions file of {in_file_length} lines.')
    out_file_length = 0
    with open(input_bedpe, 'r') as f_in:
        with open(output, 'w+') as f_out:
            for line in tqdm(f_in, total=in_file_length):
                pet_count = int(line[:-1].split('\t')[-1])
                if pet_count >= min_pet_count:
                    f_out.write(line)
                    out_file_length += 1
    print(f'[INFO] Saved interactions file of {out_file_length} lines.')
    print('[INFO] Finished.')
    return None


def count_lines(file_path):
    with open(file_path, 'r') as f:
        line_count = 0
        for _ in f:
            line_count += 1
    return line_count


if __name__ == '__main__':
    parsed_args = docopt(__doc__)
    filter_by_pet_count(
        input_bedpe=parsed_args['<in_bedpe>'],
        output=parsed_args['<out_bedpe>'],
        min_pet_count=parsed_args['<min_pet_count>']
    )
