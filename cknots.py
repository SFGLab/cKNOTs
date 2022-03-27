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
    cknots.py <in_bedpe> <in_ccd> <out_dir> <chromosome> [--full] [timeout]

Options:
    -h --help     Show this help message.
"""
import logging
import os
from docopt import docopt
from cknots.cknots import run_docker


def run(arguments):
    if arguments['--full']:
        splitting_algorithm = 'find-knots'
    else:
        splitting_algorithm = 'find-k6-linear'

    if arguments['timeout']:
        run_docker.run(
            in_bedpe=arguments['<in_bedpe>'],
            in_ccd=arguments['<in_ccd>'],
            out_dir=arguments['<out_dir>'],
            chromosome=arguments['<chromosome>'],
            minor_finding_algorithm=splitting_algorithm,
            ccd_timeout=arguments['timeout']
        )
    else:
        run_docker.run(
            in_bedpe=arguments['<in_bedpe>'],
            in_ccd=arguments['<in_ccd>'],
            out_dir=arguments['<out_dir>'],
            minor_finding_algorithm=splitting_algorithm,
            chromosome=arguments['<chromosome>']
        )


def create_results_dir(out_dir):
    out_dir_abs_path = os.path.abspath(out_dir)

    try:
        os.makedirs(out_dir)
        msg = {
            'level': 'info',
            'content': f'Directory with results created at {out_dir_abs_path}'
        }

    except FileExistsError:
        msg = {
            'level': 'warning',
            'content': f'Directory with results already exists {out_dir_abs_path}'
        }

    return out_dir_abs_path, msg


def set_up_logger(chromosome):
    log_filename = os.path.join(
        parsed_args['<out_dir>'],
        f'cknots_{chromosome}.log'
    )

    logging.basicConfig(filename=log_filename,
                        level=logging.INFO,
                        format='%(asctime)s [%(levelname)s] %(message)s')


def check_arg(args, arg_name):
    file_path = args[arg_name]

    if os.path.exists(file_path):
        logging.info(f"File {arg_name} exists at {file_path}")
    else:
        msg = f"File {arg_name} does not exist at {file_path}"
        logging.critical(msg)
        raise FileNotFoundError(msg)


if __name__ == "__main__":
    parsed_args = docopt(__doc__)

    results_absolute_path, result_dir_message = create_results_dir(parsed_args['<out_dir>'])

    in_chromosome = int(parsed_args['<chromosome>'])

    if in_chromosome == 0:
        chromosome_name = 'all'
    elif in_chromosome == 23:
        chromosome_name = 'x'
    elif in_chromosome in range(1, 23):
        chromosome_name = f"{int(parsed_args['<chromosome>']):02d}"
    else:
        message = f'Invalid chromosome number: {in_chromosome}'
        raise Exception(message)

    set_up_logger(chromosome_name)

    logging.info(f'cKNOTs started.')
    logging.info(f'Computing on {chromosome_name} chromosome.')

    if result_dir_message['level'] == 'info':
        logging.info(result_dir_message['content'])
    elif result_dir_message['level'] == 'warning':
        logging.warning(result_dir_message['content'])

    check_arg(parsed_args, '<in_ccd>')
    check_arg(parsed_args, '<in_bedpe>')

    run(parsed_args)
