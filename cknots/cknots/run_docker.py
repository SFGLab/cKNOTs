"""
Usage:
    run_docker.py run docker <in_bedpe> <in_ccd> <out_dir> <chromosome> [timeout]

Options:
    -h --help     Show this help message.
"""

from docopt import docopt

from cknots.cknots.computation_scheduler import ComputationScheduler


def run(in_bedpe, in_ccd, out_dir, chromosome,
        ccd_timeout=6 * 60 * 60,  # 6 hours
        minor_finding_algorithm='find-k6-linear',
        splitting_algorithm='splitter',
        compute_chromosome=False
        ):

    arguments = None

    if minor_finding_algorithm == 'find-k6-linear':
        arguments = ['-c']
    if minor_finding_algorithm == 'find-knots':
        arguments = ['-n', '6', '-N', '6', '-d', '5', '-e', '0']

    scheduler = ComputationScheduler(
        in_bedpe=in_bedpe,
        in_ccd=in_ccd,
        out_dir=out_dir,
        chromosome=chromosome,
        ccd_timeout=ccd_timeout,
        minor_finding_algorithm=minor_finding_algorithm,
        splitting_algorithm=splitting_algorithm,
        arguments=arguments,
        compute_chromosome=compute_chromosome
    )

    scheduler.run()


if __name__ == "__main__":
    parsed_args = docopt(__doc__)

