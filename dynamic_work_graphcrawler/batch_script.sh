#!/bin/bash
#SBATCH --job-name=queue-graphcrawler
#SBATCH --partition=Centaurus
#SBATCH --time=10:00:00
#SBATCH --mem=10G
$HOME/parallelProgramming/dynamic_work_graphcrawler/queue-graphcrawler "$1" $2 $3
