from cknots.cknots.computation_scheduler import ComputationScheduler


def run(in_bedpe, in_ccd, out_dir,
        ccd_timeout=6 * 60 * 60,
        minor_finding_algorithm='find-k6-linear',
        splitting_algorithm='splitter'):

    scheduler = ComputationScheduler(
        in_bedpe=in_bedpe,
        in_ccd=in_ccd,
        out_dir=out_dir,
        ccd_timeout=ccd_timeout,
        minor_finding_algorithm=minor_finding_algorithm,
        splitting_algorithm=splitting_algorithm
    )

    scheduler.run()
