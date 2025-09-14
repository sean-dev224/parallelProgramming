#!/bin/bash
#SBATCH --job-name=seq-nbody
#SBATCH --partition=Centaurus
#SBATCH --time=10:00:00
#SBATCH --mem=10G
$HOME/parallelProgramming/seq-nbody/nbody.out $1 $2 $3 $4 $5
