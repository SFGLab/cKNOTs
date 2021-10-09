"""
#####################
#       cKNOTs      #
#  Chromatin Knots  #
#####################

cKNOTs is a program that allows user to find links in chromatin. It takes .bedpe files
as an input and outputs files containing information about localizations of links.

Chromosome should be an integer between 1-23 (23 is X chromosome) or -1 to run
all chromosomes.

Usage:
    cknots.py <in_bedpe> <in_ccd> <out_dir> <chromosome> [timeout]

Options:
    -h --help     Show this help message.
"""
import datetime
import logging
import os
from docopt import docopt


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


if __name__ == "__main__":
    parsed_args = docopt(__doc__)

    time_now_str = datetime.datetime.now().strftime("%Y-%m-%d_%H:%M:%S")

    log_filename = os.path.join(
        os.path.split(parsed_args['<out_dir>'])[0],
        f'cknots_{time_now_str}.log'
    )

    logging.basicConfig(filename=log_filename,
                        level=logging.INFO,
                        format='%[%(levelname)s] (asctime)s %(message)s')

    logging.info(f'cKNOTs started.')

    run(parsed_args)
