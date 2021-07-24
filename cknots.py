"""
CKNOTS
"""

from cknots.cknots import run_single_file, run_slurm
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    args = parser.parse_args()
    run_single_file.run(args)
