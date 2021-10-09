"""
#####################
#       cKNOTs      #
#  Chromatin Knots  #
#####################

cKNOTs preprocessing script.

orientation
    Takes <in_bedpe> .bedpe contacts file, <in_motif> .jaspar motif file
    and <in_ref> .fa genome reference file to output <out_bedpe> .bedpe file
    containing additional column with contact orientation.

pet_filter
    Takes <in_bedpe> file, outputs <out_bedpe> file containing
    only contacts that have minimum <min_pet_count> PET count.

Usage:
    cknots_preprocessing.py orientation <in_bedpe> <in_motif> <in_ref> <out_bedpe>
    cknots_preprocessing.py pet_filter <in_bedpe> <out_bedpe> <min_pet_count>
    cknots_preprocessing.py (-h | --help)

Options:
    -h --help     Show this help message.
"""

import datetime
import logging
import os
from docopt import docopt


def preprocess(arguments):
    from cknots.preprocessing import motif_orientation, pet_filter

    if arguments['orientation']:
        motif_orientation.check_motif_orientation(
            input_bedpe=arguments['<in_bedpe>'],
            motif=arguments['<in_motif>'],
            reference=arguments['<in_ref>'],
            output=arguments['<out_bedpe>']
        )
    if arguments['pet_filter']:
        pet_filter.filter_by_pet_count(
            input_bedpe=arguments['<in_bedpe>'],
            output=arguments['<out_bedpe>'],
            min_pet_count=int(arguments['<min_pet_count>'])
        )


if __name__ == "__main__":
    parsed_args = docopt(__doc__)

    time_now_str = datetime.datetime.now().strftime("%Y-%m-%d_%H:%M:%S")
    log_filename = os.path.join(f'cknots_preprocessing_{time_now_str}.log')

    logging.basicConfig(filename=log_filename,
                        level=logging.INFO,
                        format='%[%(levelname)s] (asctime)s %(message)s')

    logging.info(f'cKNOTs preprocessing started.')

    preprocess(parsed_args)
