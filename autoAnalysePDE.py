import os
import subprocess

# Scan parameters
Xs = [str(40 - i * 0.5) for i in range(0, 6)]
Ys = ['70.35']
z = '37.5'
label = "_H1"

# Base folders
folder = "Change/To/Absolute/Path"
run_dir = os.path.join(folder, 'm113_cern', 'run')
exe_dir = os.path.join(folder, 'm113_cern', 'exe')

# User-provided paths — CHANGE THESE
data_dir = "/absolute/path/to/data"
root_dir = "/absolute/path/to/root_files"

def run(command, logfile):
    """Run a shell command and log it."""
    print(f'\n{command}\n', file=logfile)
    print(f'\n{command}\n')
    subprocess.run(command, shell=True, check=False)

errors = []

with open('PDE_scan_test_log.txt', mode='w') as logfile:
    # Loop over scan positions
    for x in Xs:
        for y in Ys:
            extension = label + '_X' + x.replace('.', 'p') + '_Y' + y.replace('.', 'p')
            dataname = os.path.join(data_dir, 'long_run'+extension)
            logname = f'out_scan{extension}.log'
            rootname = os.path.join(root_dir, f'adc_histos_scan{extension}.root')

            # Check if data folder exists
            if not os.path.isdir(dataname):
                msg = f"Skipping: data folder {dataname} does not exist."
                print(msg)
                print(msg, file=logfile)
                errors.append(dataname)
                continue

            os.chdir(exe_dir)
            run('rm -rf data', logfile)
            run(f'ln -s {dataname}/ data', logfile)
            run(f"bash -c './wavedump_read >& {logname}'", logfile)

            # Check output ROOT file
            if os.path.isfile('adc_histos.root'):
                run(f'mv adc_histos.root {rootname}', logfile)
            else:
                msg = f"Analysis failed for {dataname}: no adc_histos.root found."
                print(msg)
                print(msg, file=logfile)
                errors.append(dataname)

    # Summary of failed scans
    if errors:
        print("\n=== Failed or missing scans ===", file=logfile)
        print("\n=== Failed or missing scans ===")
        for err in errors:
            print(err, file=logfile)
            print(err)
