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
        splitting_algorithm='splitter'):

    scheduler = ComputationScheduler(
        in_bedpe=in_bedpe,
        in_ccd=in_ccd,
        out_dir=out_dir,
        chromosome=chromosome,
        ccd_timeout=ccd_timeout,
        minor_finding_algorithm=minor_finding_algorithm,
        splitting_algorithm=splitting_algorithm
    )

    scheduler.run()


if __name__ == "__main__":
    parsed_args = docopt(__doc__)

