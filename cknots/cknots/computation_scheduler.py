import os
import subprocess
import logging


class ComputationScheduler:

    def __init__(self, in_bedpe, in_ccd, out_dir, chromosome,
                 ccd_timeout=6 * 60 * 60,
                 minor_finding_algorithm='find-k6-linear',
                 splitting_algorithm='splitter'):
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

        self.in_bedpe = in_bedpe
        self.in_ccd = in_ccd
        self.out_dir = out_dir
        self.chromosome = chromosome
        self.ccd_timeout = ccd_timeout
        self.minor_finding_algorithm = self._get_bin_path(minor_finding_algorithm)
        self.splitting_algorithm = self._get_bin_path(splitting_algorithm)

        self.ccd_dirs = []  # filled in in self._run_splitter()

        logging.info('Computation scheduler created.')

    def run(self):
        """
        Run computations. Creates directory with results, splits input files
        and running minor finder on each CCD (for each chromosome, if running
        on more than one).
        :return: None
        """
        logging.info(f'Looking for minors in {self.in_bedpe} with CCDs defined in {self.in_ccd}')

        self._create_results_dir()
        self._run_splitter()

        for ccd_dir in self.ccd_dirs:
            self._run_minor_finder(ccd_dir)

    def _create_results_dir(self):
        if os.path.exists(self.out_dir):
            raise FileExistsError(f'Directory {self.out_dir} already exists. Remove it before proceeding.')
        else:
            os.makedirs(self.out_dir)
            results_absolute_path = os.path.abspath(self.out_dir)
            logging.info(f'Directory with results created: {results_absolute_path}')

    def _run_splitter(self):
        logging.info('Running splitter')

        if self.chromosome in range(1, 24):
            chromosmes_to_process = [self.chromosome]
        elif self.chromosome == -1:
            chromosmes_to_process = list(range(1, 24))
        else:
            error_message = f'Invalid chromosome value: {self.chromosome}.'
            logging.error(error_message)
            raise ValueError(error_message)

        for chromosome in chromosmes_to_process:

            chromosome_name = f"{chromosome:02d}" if chromosome != 23 else 'X'
            logging.info(f'Running splitter on chromosome {chromosome_name}')

            input_cmd = [self.splitting_algorithm,
                         '-c', f'{chromosome}',
                         '-s',
                         '-f', f'{self.in_bedpe}',
                         '-d', f'{self.in_ccd}']

            subprocess.run(
                input_cmd
            )

            ccd_files_current_path = os.path.split(self.in_bedpe)[0]
            ccd_files_destination_path = os.path.join(self.out_dir, f'chr_{chromosome_name}')

            files_to_move = [x for x in os.listdir(ccd_files_current_path) if x.endswith('.mp')]
            os.makedirs(
                ccd_files_destination_path
            )

            for file in files_to_move:
                os.rename(
                    os.path.join(ccd_files_current_path, file),
                    os.path.join(ccd_files_destination_path, file)
                )

            self.ccd_dirs.append(ccd_files_destination_path)

        logging.info('Bedpe file split into CCDs and divided into folders in results directory.')

    def _run_minor_finder(self, ccd_dir_path):
        ccds_to_analyze = [x for x in os.listdir(ccd_dir_path) if x.endswith('.mp')]

        for ccd in ccds_to_analyze:
            file_path = os.path.join(self.out_dir, ccd)

            logging.info(f'Running minor finder on {file_path}')

            input_cmd = [self.minor_finding_algorithm, '-c', '-f', f'{file_path}', '-o', f'{file_path}.raw_minors']

            try:
                subprocess.run(
                    input_cmd,
                    timeout=self.ccd_timeout
                )
                computation_finished = True
                logging.info(f'Finished processing {file_path}')

            except subprocess.TimeoutExpired:
                computation_finished = False
                logging.warning(f'Timeout expired on {file_path}')

        # todo: json file with results description

    @staticmethod
    def _get_bin_path(algorithm_name):
        return f'cknots/cpp/bin/{algorithm_name}'
