#!/bin/bash

## GENERATES A DOPE GIT LOG FOR LATEX

# Find remote URL for hashes (designed for GitHub-hosted projects)
origin=`git config remote.origin.url`
base=`dirname "$origin"`/`basename "$origin" .git`

# Output LaTeX table in chronological order
echo "\\begin{tabular}{l l l}\\textbf{Detail} & \\textbf{Author} & \\textbf{Description}\\\\\\hline"
git log --pretty=format:"\\ %h & %an & %s\\\\\\hline" --reverse
echo "\end{tabular}"
