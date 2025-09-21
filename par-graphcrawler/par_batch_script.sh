#!/bin/bash
#SBATCH --job-name=par-graphcrawler
#SBATCH --partition=Centaurus
#SBATCH --time=10:00:00
#SBATCH --mem=10G
$HOME/parallelProgramming/par-graphcrawler/par-graphcrawler "$1" $2 $3
