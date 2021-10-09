"""
#####################
#       cKNOTs      #
#  Chromatin Knots  #
#####################

cKNOTs is a program that allows user to find links in chromatin. It takes .bedpe files
as an input and outputs files containing information about localizations of links.

Usage:
    cknots.py run docker <in_bedpe> <in_ccd> <out_dir> <chromosome> [timeout]
    cknots.py preprocess orientation <in_bedpe> <in_motif> <in_ref> <out_bedpe>
    cknots.py preprocess pet_filter <in_bedpe> <out_bedpe> <min_pet_count>
    cknots.py (-h | --help)

Options:
    -h --help     Show this help message.
"""
import datetime
import logging
import os
from docopt import docopt


def main(arguments):
    if arguments['run']:
        run(arguments)
    if arguments['preprocess']:
        preprocess(arguments)


def run(arguments):
    from cknots.cknots import run_docker
    if arguments['docker']:
        if arguments['[timeout]']:
            run_docker.run(
                in_bedpe=arguments['<in_bedpe>'],
                in_ccd=arguments['<in_ccd>'],
                out_dir=arguments['<out_dir>'],
                chromosome=arguments['<chromosome>'],
                ccd_timeout=arguments['[timeout]']
            )
        else:
            run_docker.run(
                in_bedpe=arguments['<in_bedpe>'],
                in_ccd=arguments['<in_ccd>'],
                out_dir=arguments['<out_dir>'],
                chromosome=arguments['<chromosome>']
            )


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
    log_filename = os.path.join(f'cknots_{time_now_str}.log')

    if parsed_args['run']:
        log_filename = os.path.join(
            os.path.split(parsed_args['<out_dir>'])[0],
            f'cknots_{time_now_str}.log')

    logging.basicConfig(filename=log_filename,
                        level=logging.INFO,
                        format='%(asctime)s [%(levelname)s]: %(message)s')

    logging.info(f'cKNOTs started.')

    main(parsed_args)
