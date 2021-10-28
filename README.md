# cKNOTs: Chromatin Knots

## About

This software allows user to look for links in chromatin. It takes `.bedpe` files as an input and outputs 
files containing information about localizations of links. 


## How to set it up?

To do

# cKNOTs: Chromatin Knots

## About

This software allows user to look for links in chromatin. It takes `.bedpe` files as an input and outputs 
files containing information about localizations of links. 


## How to set it up?

To do

## How to use it?
### Looking for minors 
```
Usage:
    cknots.py run (local|slurm) <in_bedpe> <in_ccd> <out_dir>
    cknots.py preprocess orientation <in_bedpe> <in_motif> <in_ref> <out_bedpe>
    cknots.py preprocess pet_filter <in_bedpe> <out_bedpe> <min_pet_count>
    cknots.py (-h | --help)

Options:
    -h --help     Show this help message.
```

- `cknots.py run`: Run the minor finding algorithm.
    - `(local|slurm)` Select if the algorithm should be run locally or distribute it by chromosome on Slurm cluster. 
    - `<in_bedpe>` Path to the `.bedpe` file containing information about contacts.
    - `<in_ccd>` Path to the `.bed` file containing information about CCDs.
    - `<out_dir>` Path to the output directory. The directory should not exist, it will be created by the program.

    
- `cknots.py preprocess orientation`: Create a new `.bedpe` file with two new columns containing
  motif orientation for first and second locus of contact. 
    - `<in_bedpe>` Path to the `.bedpe` file containing information about contacts.
    - `<in_motif>` Path to the `.jaspar` file containing motif.
    - `<in_ref>` Path to the `.fa` file containing reference genome.
    - `<out_bedpe>` Path to the output `.bedpe` file.
    

- `cknots.py preprocess pet_filter`: Create a new `.bedpe` file in which rows with PET count smaller
  than `<min_pet_count>` are filtered out. 
    - `<in_bedpe>` Path to the `.bedpe` file containing information about contacts.
    - `<out_bedpe>` Path to the output `.bedpe` file.
    - `<min_pet_count>` Minimal number of contacts that should be left in the output file.
    

### Analyzing the results
The `cnots.analysis` module contains classes and functions that can be used to analyze the output 
of the algorithm. This includes visualization of the minors loci, comparing similarity of the 
results for different cell lines and generating tables of number of minors found in different chromosomes.


## How to use it?
### Looking for minors 
```
Usage:
    cknots.py run (local|slurm) <in_bedpe> <in_ccd> <out_dir>
    cknots.py preprocess orientation <in_bedpe> <in_motif> <in_ref> <out_bedpe>
    cknots.py preprocess pet_filter <in_bedpe> <out_bedpe> <min_pet_count>
    cknots.py (-h | --help)

Options:
    -h --help     Show this help message.
```

- `cknots.py run`: Run the minor finding algorithm.
    - `(local|slurm)` Select if the algorithm should be run locally or distribute it by chromosome on Slurm cluster. 
    - `<in_bedpe>` Path to the `.bedpe` file containing information about contacts.
    - `<in_ccd>` Path to the `.bed` file containing information about CCDs.
    - `<out_dir>` Path to the output directory. The directory should not exist, it will be created by the program.

    
- `cknots.py preprocess orientation`: Create a new `.bedpe` file with two new columns containing
  motif orientation for first and second locus of contact. 
    - `<in_bedpe>` Path to the `.bedpe` file containing information about contacts.
    - `<in_motif>` Path to the `.jaspar` file containing motif.
    - `<in_ref>` Path to the `.fa` file containing reference genome.
    - `<out_bedpe>` Path to the output `.bedpe` file.
    

- `cknots.py preprocess pet_filter`: Create a new `.bedpe` file in which rows with PET count smaller
  than `<min_pet_count>` are filtered out. 
    - `<in_bedpe>` Path to the `.bedpe` file containing information about contacts.
    - `<out_bedpe>` Path to the output `.bedpe` file.
    - `<min_pet_count>` Minimal number of contacts that should be left in the output file.
    

### Analyzing the results
The `cnots.analysis` module contains classes and functions that can be used to analyze the output 
of the algorithm. This includes visualization of the minors loci, comparing similarity of the 
results for different cell lines and generating tables of number of minors found in different chromosomes.
