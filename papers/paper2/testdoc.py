import os
import subprocess
import glob
import shlex

tarfile = "CS444_weeklysummary3_118.tar.bz2"

def remove_pdfs():
    subprocess.Popen(shlex.split("make clean"))
    subprocess.Popen(shlex.split("rm -rf *.pdf"))

def cleanup_mess(): # I don't want to FileZilla all the garbage from latex to my workstation
    subprocess.Popen(shlex.split("rm -f *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *.toc"))

def untar_file(tar_bz2_file):
    subprocess.Popen(['tar', '-jxf', tar_bz2_file]) # see 11 and 12 in syllabus
    remove_pdfs()
    subprocess.Popen(['make']) # see 7 and 8 in syllabus
    cleanup_mess()

untar_file(tarfile)
