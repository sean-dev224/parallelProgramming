#!/bin/bash
#SBATCH --job-name=seq-mergesort
#SBATCH --partition=Centaurus
#SBATCH --time=00:40:00
#SBATCH --mem=10G
$HOME/parallelProgramming/seq-mergesort/seq-mergesort.out
