#!/bin/bash
#SBATCH --job-name=par-nbody
#SBATCH --partition=Centaurus
#SBATCH --time=10:00:00
#SBATCH --mem=10G
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
$HOME/parallelProgramming/nbody-par/nbody-par $1 $2 $3 $4 $5
