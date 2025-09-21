#!/bin/bash
#SBATCH --job-name=seq-graphcrawler
#SBATCH --partition=Centaurus
#SBATCH --time=10:00:00
#SBATCH --mem=10G
$HOME/parallelProgramming/par-graphcrawler/level_client "$1" $2
