"""
#####################
#       cKNOTs      #
#  Chromatin Knots  #
#####################

Script for running cKNOTs on a Slurm cluster with Enroot.

It runs cKNOTs in 23 separate tasks for each chromosome

Usage:
    slurm_cknots.py <data_path> <in_bedpe> <in_ccd> <out_dir> [timeout]

"""
import os
import subprocess
from shutil import which

from docopt import docopt
from jinja2 import Template

SBATCH_TEMPLATE = """#!/bin/bash
#SBATCH --ntasks=2
#SBATCH --mem 60G
#SBATCH --job-name="{{ job_name }}"
#SBATCH --partition=medium

srun  --container-mounts={{ data_path }}:/data \
--container-image=krzysztofspalinski/cknots:latest \
/cknots-app/docker_cknots.sh \
{{ bedpe_path }} \
{{ ccd_path }} \
{{ results_path }} \
{{ chromosome_number }} \
{{ timeout }}
"""


def run_slurm(arguments):
    for chromosome_number in range(1, 24):
        timeout = arguments['timeout'] if arguments['timeout'] else ''

        script_to_run = create_sbatch_script(
            data_path=os.path.abspath(arguments['<data_path>']),
            in_bedpe=arguments['<in_bedpe>'],
            in_ccd=arguments['<in_ccd>'],
            out_dir=arguments['<out_dir>'],
            chromosome=chromosome_number,
            timeout=timeout
        )

        input_cmd = ['sbatch',
                     script_to_run]

        subprocess.run(
            input_cmd
        )

        os.remove(script_to_run)


def create_sbatch_script(data_path: str,
                         in_bedpe: str,
                         in_ccd: str,
                         out_dir: str,
                         chromosome: int,
                         timeout='') -> str:
    template = Template(SBATCH_TEMPLATE)
    script_text = template.render(
        job_name=f'chr{chromosome}',
        data_path=data_path,
        bedpe_path=in_bedpe,
        ccd_path=in_ccd,
        results_path=out_dir,
        chromosome_number=chromosome,
        timeout=timeout
    )

    script_file_path = f'tmp_sbatch_script_chr{chromosome}.sh'

    with open(script_file_path, 'w') as f:
        f.write(script_text)

    return script_file_path


def check_program_exists(name):
    app_version = which(name)
    if app_version is not None:
        print(f'Program {name} was found in version {app_version}.')
    else:
        raise ModuleNotFoundError(f'Program {name} not found on this machine, aborting.')


if __name__ == '__main__':
    parsed_args = docopt(__doc__)
    check_program_exists('srun')
    check_program_exists('enroot')
    run_slurm(parsed_args)
