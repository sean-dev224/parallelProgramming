#!/bin/bash
#SBATCH --job-name=seq-graphcrawler
#SBATCH --partition=Centaurus
#SBATCH --time=00:10:00
#SBATCH --mem=10G
$HOME/parallelProgramming/seq-nbody/nbody.out $1 $2 $3 
