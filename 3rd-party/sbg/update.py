#!/usr/bin/python

import subprocess
import os
import shutil
import sys
    
def main():
    args = sys.argv[1:]
    branch_name = 'sb-graph-dev'
    if not args == []:
        branch_name = args[0]
    if os.path.isdir('./sb-graph'):
        shutil.rmtree('./sb-graph')
    subprocess.check_call(['git', 'clone', '-b' , branch_name, '--single-branch', 'git@github.com:CIFASIS/sb-graph.git'])
    shutil.rmtree('./sb-graph-dev', True)
    shutil.move('./sb-graph', './sb-graph-dev')
    subprocess.check_call(['tar', '-czvf', 'sb-graph-dev.tar.gz', './sb-graph-dev'])
    shutil.rmtree('./sb-graph-dev')

if (__name__ == '__main__'):
    main()

