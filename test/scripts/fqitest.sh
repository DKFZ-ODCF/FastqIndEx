#!/bin/bash

set -uvx

declare scratch="/local/$(whoami)/${LSB_JOBID}"
declare extractedFastq=$scratch/$(basename $FASTQ .gz)
[[ ! -f $extractedFastq ]] && gzip -dc $FASTQ | tee $extractedFastq | wc -l > $extractedFastq.lineCount

[[ ! -f $INDEX ]] && fastqindex index -f=$FASTQ -i=$INDEX

declare -i count=1
declare -i maxlines=$(cat $extractedFastq.lineCount)
declare -i diffs=0

echo $maxlines
while [[ $count -lt $maxlines ]]; do
	original=$(tail -n +$count $extractedFastq | head -n 1)
	extracted=$(fastqindex extract -f=$FASTQ -i=$INDEX -n=1 -e=1 -s=$(( $count - 1)) )
	[[ "$extracted" != "$original" ]] && diffs=$(( $diffs + 1 ))

	count=$(( $count + 1000000 ))
	echo $count
done
echo $diffs > $INDEX.errors
