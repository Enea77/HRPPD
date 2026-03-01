# Author: Enea Prifti
# This program automates the PDE scan for the HRPPD

import os
import subprocess

Xs = [str(40 - i * 0.5) for i in range(0, 6)]
Ys = ['70.35']
z = '37.5'

label = "_July14_test"

folder = os.getcwd()
zab_dir = os.path.join(folder, 'zaber')
run_dir = os.path.join(folder, 'm113_cern', 'run')
exe_dir = os.path.join(folder, 'm113_cern', 'exe')

def run(command, logfile):
    """Runs the command and logs output."""
    subprocess.run(command, shell=True)
    print(f'\n{command}\n', file=logfile)
    print(f'\n{command}\n')

errors = []

with open('PDE_scan_test_log.txt', mode='w') as logfile:

    # -- Begin Scan -- #
    for x in Xs:
        for y in Ys:
            # File name extensions
            extension = label + '_X' + x.replace('.', 'p') + '_Y' + y.replace('.', 'p')
            dataname = '_autoscan' + extension
            logname = f'out_scan{extension}.log'
            rootname = f'adc_histos_scan{extension}.root'

            # Move the Zaber
            os.chdir(zab_dir)
            run(f'./zaber -- {y} {z} {x}', logfile)

            # Run acquisition
            os.chdir(run_dir)
            run(f'./run_long.sh {dataname}', logfile)

            # Run analysis
            os.chdir(exe_dir)
            run('rm -rf data', logfile)
            run(f'ln -s ../run/long_run{dataname}/ data', logfile)
            run(f"bash -c './wavedump_read >& {logname}'", logfile)
            run('root -q polya_exp_fit_mcp875_bzero.C', logfile)

            if os.path.isfile('adc_histos.root'):
                run(f'mv adc_histos.root {rootname}', logfile)
            else:
                # Re-run acquisition
                os.chdir(run_dir)
                run(f'rm -rf long_run{dataname}', logfile)
                run(f'./run_long.sh {dataname}', logfile)

                # Re-run analysis
                os.chdir(exe_dir)
                run('rm -rf data', logfile)
                run(f'ln -s ../run/long_run{dataname} data', logfile)
                run(f"bash -c './wavedump_read >& {logname}'", logfile)
                run('root -q polya_exp_fit_mcp875_bzero.C', logfile)

                if os.path.isfile('adc_histos.root'):
                    run(f'mv adc_histos.root {rootname}', logfile)
                else:
                    errors.append([y, x, z])
                    continue

print('Skipped points:\n', errors)
