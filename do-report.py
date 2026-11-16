import argparse
from pathlib import Path
import subprocess
import re
import shutil

parser = argparse.ArgumentParser()
parser.add_argument('dir', type=Path)
args = parser.parse_args()

if not args.dir.is_dir():
    exit(1)

# ---------------------------------
# Process raw data into merged data
# ---------------------------------
# TODO: Consider a set?
data_to_process = list(args.dir.glob('*.profraw'))

# Assume clang set in PATH
command_to_exec = [shutil.which('llvm-profdata'), 'merge', '--sparse', '-o', 'merged-data.profcooked']
res = subprocess.run([*command_to_exec, *data_to_process], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

if res.returncode != 0:
    warning_match = re.compile(r'warning: (.+\.profraw): .+')
    files_to_ignore = set()

    for line in res.stderr.decode('utf-8').splitlines():
        if line_match := re.search(warning_match, line):
            files_to_ignore.add(Path(line_match.group(1)))
    
    for file in files_to_ignore:
        data_to_process.remove(file)

    res = subprocess.run([*command_to_exec, *data_to_process], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if res.returncode != 0:
        exit(res.returncode)

# ----------------------------------------
# Get all executables and shared libraries
# ----------------------------------------
BUILD_DIR = Path('/home/dev/irods-build')
# TODO: Consider using a set
debs_to_query = list(BUILD_DIR.glob('*.deb'))

execs_and_libs_to_genny = set()

query_deb = [shutil.which('dpkg-deb'), '--contents']
for deb in debs_to_query:
    res = subprocess.run([*query_deb, deb], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    if res.returncode != 0:
        print(res.stderr.decode('utf-8'))
        exit(res.returncode)

    exec_match = re.compile(r'^-..x..x..x .* \.(/.+)$')
    lib_match = re.compile(r'''
                            \.              # All packages start with dot

                            (               # We only want the path after the dot
                            /.+\.so         # Caputure anything that ends with .so

                            |               # Or...

                            /.+\.so
                            \.\d+\.\d+\.\d+ # The .so has the version number at the end
                            )

                            $               # Ensure it's the end of line!
                            ''', re.X)

    for line in res.stdout.decode('utf-8').splitlines():
        if (line_match := re.search(exec_match, line)) or (line_match := re.search(lib_match, line)):
            exec_or_lib = Path(line_match.group(1))
            if exec_or_lib.suffix in ['.py', '.sh'] or not exec_or_lib.exists():
                continue
            execs_and_libs_to_genny.add(exec_or_lib)

# ---------------------------------------------------------
# Generate reports for all executables and shared libraries
# ---------------------------------------------------------
report_genny = [shutil.which('llvm-cov'), 'export', '--format=text', '--ignore-filename-regex=irods-externals', '--instr-profile', 'merged-data.profcooked']

cov_report_genny = [*report_genny, *[f'--object={x}' for x in execs_and_libs_to_genny]]

temp_gen_file = Path('/tmp/genny-file.json')
with open(temp_gen_file, 'w') as out_file:
    res = subprocess.run(cov_report_genny, stdout=out_file, stderr=subprocess.PIPE)

if res.returncode != 0:
    print(res.stderr.decode('utf-8'))
    exit(res.returncode)

# -----------------------------
# Convert JSON coverage to lcov
# -----------------------------
# NOTE: We can directly produce lcov using llvm-cov.
#       However, it generates much more errors!
final_lcov_file = Path('cov-thang.info')
lcov_genny = ['llvm2lcov', '--branch-coverage', '--ignore-errors', 'inconsistent', '--exclude', '/opt/irods-externals/*' , '-o', final_lcov_file, temp_gen_file]
res = subprocess.run(lcov_genny, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

print(res.stdout.decode('utf-8'))
if res.returncode != 0:
    print(res.stderr.decode('utf-8'))
    exit(res.returncode)


# ------------------------
# Create website from lcov
# ------------------------
http_dir = Path('guh')
res = subprocess.run(['genhtml', '-o', http_dir, final_lcov_file], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

print(res.stdout.decode('utf-8'))
if res.returncode != 0:
    print(res.stderr.decode('utf-8'))
    exit(res.returncode)
# llvm2lcov --branch-coverage --ignore-errors inconsistent --exclude '/opt/irods-externals/*' -o cov-vov.info cov-gain.json