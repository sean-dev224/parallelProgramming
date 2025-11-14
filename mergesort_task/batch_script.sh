#!/bin/bash
#SBATCH --job-name=mergesort_par
#SBATCH --partition=Centaurus
#SBATCH --time=10:00:00
#SBATCH --mem=10G
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
$HOME/parallelProgramming/mergesort_task/mergesort_par $1 $2 
