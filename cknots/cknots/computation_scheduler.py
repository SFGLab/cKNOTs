import json
import os
import subprocess
import logging
import resource

import pandas as pd


def limit_max_memory(max_memory=None):
    if max_memory is None:
        max_memory = 64 * 1024 * 1024 * 1024  # 64 gigabytes
    resource.setrlimit(resource.RLIMIT_AS, (max_memory, max_memory))


class ComputationScheduler:

    def __init__(self, in_bedpe, in_ccd, out_dir, chromosome,
                 ccd_timeout=6 * 60 * 60,
                 minor_finding_algorithm='find-k6-linear',
                 splitting_algorithm='splitter',
                 arguments=None):
        """
        Class for scheaduling running of knot finding algorithm.

        :param in_bedpe: path to bedpe file
        :param in_ccd: path to ccd file
        :param out_dir: path to *non-existing* directory with results
        :param chromosome: chromosome (1-23 or -1 for all) to process
        :param ccd_timeout: time (seconds) for timeout of single ccd
        :param minor_finding_algorithm: name of minor finding algorithm
        :param splitting_algorithm: name of splitting algorithm
        """

        if minor_finding_algorithm == 'find-k6-linear':
            self.minor_finding_algorithm_type = 'linear'
        elif minor_finding_algorithm == 'find-knots':
            self.minor_finding_algorithm_type = 'full'
            self.path_decomposition_algorithm = self._get_bin_path('path-decomposition')
        else:
            raise NotImplementedError(f'{minor_finding_algorithm} not implemented.')

        self.in_bedpe = in_bedpe
        self.in_ccd = in_ccd
        self.out_dir = out_dir
        self.chromosome = int(chromosome)
        self.ccd_timeout = int(ccd_timeout)
        self.minor_finding_algorithm = self._get_bin_path(minor_finding_algorithm)
        self.splitting_algorithm = self._get_bin_path(splitting_algorithm)
        self.arguments = arguments

        self.ccd_dirs = []  # filled in in self._run_splitter()

        self.resuming_computation = False

        logging.info('Computation scheduler created.')

    def run(self):
        """
        Run computations. Splits input files and runs minor finder
        on each CCD (for each chromosome, if running on more than one).
        :return: None
        """
        logging.info(f'Looking for minors in {self.in_bedpe} with CCDs defined in {self.in_ccd}')

        self._run_splitter()

        for ccd_dir in self.ccd_dirs:
            if self.minor_finding_algorithm_type == 'full':
                self._run_full_minor_finder(ccd_dir)
            elif self.minor_finding_algorithm_type == 'linear':
                self._run_linear_minor_finder(ccd_dir)

    def _run_splitter(self):

        if self.chromosome in range(1, 24):
            chromosomes_to_process = [self.chromosome]
        elif self.chromosome == 0:
            chromosomes_to_process = list(range(1, 24))
        else:
            error_message = f'Invalid chromosome value: {self.chromosome}.'
            logging.error(error_message)
            raise ValueError(error_message)

        for chromosome in chromosomes_to_process:

            chromosome_name = f"{chromosome:02d}" if chromosome != 23 else 'X'
            logging.info(f'Running splitter on chromosome {chromosome_name}')

            input_cmd = [self.splitting_algorithm,
                         '-c', f'{chromosome}',
                         '-s',
                         '-f', f'{self.in_bedpe}',
                         '-d', f'{self.in_ccd}']

            # input_cmd = [self.splitting_algorithm,
            #              f'{self.in_bedpe}',
            #              f'{self.in_ccd}']

            subprocess.run(
                input_cmd
            )

            ccd_files_current_path = os.path.split(self.in_bedpe)[0]
            ccd_files_destination_path = os.path.join(self.out_dir, f'chr_{chromosome_name}')

            files_to_move = [x for x in os.listdir(ccd_files_current_path) if x.endswith(f'.mp') or x.endswith(f'.tr')]

            try:
                os.makedirs(
                    ccd_files_destination_path
                )
            except FileExistsError:
                message = f'Directory {ccd_files_destination_path} already exists, resuming computation.'
                logging.warning(message)
                self.resuming_computation = True

            for file in files_to_move:
                os.rename(
                    os.path.join(ccd_files_current_path, file),
                    os.path.join(ccd_files_destination_path, file)
                )

            self.ccd_dirs.append(ccd_files_destination_path)

        logging.info('Bedpe file split into CCDs and divided into folders in results directory.')

    def _run_linear_minor_finder(self, ccd_dir_path):
        ccds_to_analyze = sorted([x for x in os.listdir(ccd_dir_path) if x.endswith('.mp')])

        all_ccds = pd.read_csv(self.in_ccd,
                               sep='\t',
                               header=None,
                               names=['chromosome', 'start', 'end'])

        chromosome_results = []
        if self.resuming_computation:
            chromosome_results = [None] * len(all_ccds)

            with open(os.path.join(ccd_dir_path, 'results.json')) as f:
                previous_results = json.load(f)

            for results_for_ccd in previous_results:
                idx = int(results_for_ccd['input_filename'][-7:-3]) - 1
                chromosome_results[idx] = results_for_ccd

        csv_chr_name = str(os.path.split(ccd_dir_path)[-1]) \
            .replace('_0', '') \
            .replace('_', '')

        relevant_ccds_iterator = all_ccds[all_ccds['chromosome'] == csv_chr_name].iterrows()

        for i, ccd in enumerate(ccds_to_analyze):

            try:
                _, ccd_info = next(relevant_ccds_iterator)
                ccd_start = ccd_info['start']
                ccd_end = ccd_info['end']
            except StopIteration:
                continue

            ccd_results = {
                'input_filename': str(ccd),
                'results_exist': False,
                'results_not_empty': False,
                'results_filename': None,
                'return_code': None,
                'ccd_start': ccd_start,
                'ccd_end': ccd_end
            }

            file_path = os.path.join(ccd_dir_path, ccd)
            file_name = os.path.split(file_path)[-1]

            result_path = f'{file_path}.raw_minors'

            if self.resuming_computation and os.path.exists(result_path):
                logging.info(f'Results for {file_path} already exists, skipping')
                continue

            logging.info(f'Running minor finder on {file_path}')

            input_cmd = [self.minor_finding_algorithm,
                         '-f', f'{file_path}',
                         '-o', f'{file_path}.raw_minors']

            if self.arguments is not None:
                input_cmd = input_cmd + self.arguments

            try:
                result = subprocess.run(
                    input_cmd,
                    timeout=self.ccd_timeout
                )

                ccd_results['results_exist'] = True
                ccd_results['results_filename'] = os.path.split(result_path)[-1]
                ccd_results['return_code'] = result.returncode

                if result.returncode == 0:
                    logging.info(f'{file_name} processing finished')
                else:
                    if os.path.exists(result_path):
                        logging.warning(f'{file_name} processing ended with and error, but result file exists. '
                                        + f'Return code: {result.returncode}')
                    else:
                        logging.error(f'{file_name} processing ended with and error, and result file does not exist. '
                                      + f'Return code: {result.returncode}')
                        ccd_results['results_exist'] = False
                        ccd_results['results_filename'] = ''

            except subprocess.TimeoutExpired:
                logging.error(f'Timeout expired on {file_path}')
                ccd_results['results_exist'] = False
                ccd_results['results_filename'] = ''
                ccd_results['return_code'] = 124

            except Exception as other_exception:
                logging.error(f'Exception occurred {other_exception}')
                ccd_results['results_exist'] = False
                ccd_results['results_filename'] = ''
                ccd_results['return_code'] = 1

            if os.path.exists(result_path) and os.stat(result_path).st_size > 0:
                ccd_results['results_not_empty'] = True

            if self.resuming_computation:
                chromosome_results[i] = ccd_results
            else:
                chromosome_results.append(ccd_results)

            with open(os.path.join(ccd_dir_path, 'results.json'), 'w') as f:
                json.dump(chromosome_results, f, indent=4, sort_keys=True)

        with open(os.path.join(ccd_dir_path, 'results.json'), 'w') as f:
            json.dump(chromosome_results, f, indent=4, sort_keys=True)

    def _run_full_minor_finder(self, ccd_dir_path):
        ccds_to_analyze = sorted([x for x in os.listdir(ccd_dir_path) if x.endswith('.mp')])

        all_ccds = pd.read_csv(self.in_ccd,
                               sep='\t',
                               header=None,
                               names=['chromosome', 'start', 'end'])

        chromosome_results = []
        if self.resuming_computation:
            chromosome_results = [None] * len(all_ccds)

            with open(os.path.join(ccd_dir_path, 'results_full.json')) as f:
                previous_results = json.load(f)

            for results_for_ccd in previous_results:
                idx = int(results_for_ccd['input_filename'][-7:-3]) - 1
                chromosome_results[idx] = results_for_ccd

        csv_chr_name = str(os.path.split(ccd_dir_path)[-1]) \
            .replace('_0', '') \
            .replace('_', '')

        relevant_ccds_iterator = all_ccds[all_ccds['chromosome'] == csv_chr_name].iterrows()

        for i, ccd in enumerate(ccds_to_analyze):

            try:
                _, ccd_info = next(relevant_ccds_iterator)
                ccd_start = ccd_info['start']
                ccd_end = ccd_info['end']
            except StopIteration:
                continue

            ccd_results = {
                'input_filename': str(ccd),
                'results_exist': False,
                'results_not_empty': False,
                'results_filename': None,
                'return_code': None,
                'ccd_start': ccd_start,
                'ccd_end': ccd_end
            }

            file_path = os.path.join(ccd_dir_path, ccd)
            file_name = os.path.split(file_path)[-1]

            result_path = f'{file_path}.pd.raw_minors'

            if self.resuming_computation and os.path.exists(result_path):
                logging.info(f'Results for {file_path} already exists, skipping')
                continue

            logging.info(f'Running path decomposition on {file_path}')

            input_cmd = [self.path_decomposition_algorithm,
                         '-f', f'{file_path}']

            with open(f'{file_path}.pd', 'w') as f:
                subprocess.call(
                    input_cmd,
                    stdout=f
                )

            logging.info(f'Running minor finder on {file_path}')

            input_cmd = [self.minor_finding_algorithm,
                         '-f', f'{file_path}.pd',
                         '-o', f'{file_path}.pd.raw_minors']

            if self.arguments is not None:
                input_cmd = input_cmd + self.arguments

            try:
                result = subprocess.run(
                    input_cmd,
                    preexec_fn=limit_max_memory,
                    timeout=self.ccd_timeout
                )

                ccd_results['results_exist'] = True
                ccd_results['results_filename'] = os.path.split(result_path)[-1]
                ccd_results['return_code'] = result.returncode

                if result.returncode == 0:
                    logging.info(f'{file_name} processing finished')
                else:
                    if os.path.exists(result_path):
                        logging.warning(f'{file_name} processing ended with and error, but result file exists. '
                                        + f'Return code: {result.returncode}')
                    else:
                        logging.error(f'{file_name} processing ended with and error, and result file does not exist. '
                                      + f'Return code: {result.returncode}')
                        ccd_results['results_exist'] = False
                        ccd_results['results_filename'] = ''

            except subprocess.TimeoutExpired:
                logging.error(f'Timeout expired on {file_path}')
                ccd_results['results_exist'] = False
                ccd_results['results_filename'] = ''
                ccd_results['return_code'] = 124

            except Exception as other_exception:
                logging.error(f'Exception occurred {other_exception}')
                ccd_results['results_exist'] = False
                ccd_results['results_filename'] = ''
                ccd_results['return_code'] = 1

            if os.path.exists(result_path) and os.stat(result_path).st_size > 0:
                ccd_results['results_not_empty'] = True

            if self.resuming_computation:
                chromosome_results[i] = ccd_results
            else:
                chromosome_results.append(ccd_results)

            with open(os.path.join(ccd_dir_path, 'results_full.json'), 'w') as f:
                json.dump(chromosome_results, f, indent=4, sort_keys=True)

        with open(os.path.join(ccd_dir_path, 'results_full.json'), 'w') as f:
            json.dump(chromosome_results, f, indent=4, sort_keys=True)

    @staticmethod
    def _get_bin_path(algorithm_name):
        return f'/cknots-app/cknots/cpp/bin/{algorithm_name}'


