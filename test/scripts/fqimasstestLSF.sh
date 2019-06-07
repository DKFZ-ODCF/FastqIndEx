#!/bin/bash

set -uvex

declare listOfFiles="$1"
declare baseOutputFolder="$2"

[[ -z "$listOfFiles" ]] && echo "List of FASTQ files is empty. You need to call the masstest with [file with a list of fastq files] and [base output folder]." && exit 1

[[ -z "$baseOutputFolder" ]] && echo "There was no output folder present. You need to call the masstest with [file with a list of fastq files] and [base output folder]. " && exit 2

[[ ! -f "$listOfFiles" ]] && echo "File does not exist" && exit 3

[[ ! -d "$baseOutputFolder" ]] && echo "Base output folder does not exist, please create it." && exit 4

for f in `cat $listOfFiles`; do
	[[ -z "$f" ]] && continue
	[[ ! -f "$f" ]] && continue

	declare targetFile="$2"/"$f"
	declare targetDirectory=$(dirname $targetFile)
	echo $targetDirectory
	mkdir -p "$targetDirectory"
	FASTQ=$f INDEX=$targetFile bsub -W 1200 -n 1 -R "span[hosts=1]" -cwd $HOME -J FQIndExTest -R "rusage[mem=512]" -M 512 ~/fqitest.sh
done
