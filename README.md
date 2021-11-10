# cKNOTs: Chromatin Knots

## About

This software allows user to look for links in chromatin. It takes `.bedpe` files as an input and outputs 
files containing information about localizations of links. 

This repository consist of three main components:
- `cknots.py` script: running the link finding algorithm. The best way to run it is by using Docker. 
- `preprocessing_cknots.py` script: preprocessing files before running the link finding algorithm on them. 
- `cknots` Python package: analyzing the results.

## How to set it up?

To run the link finding algorithm, the only requirement is having Docker installed on your machine. 

To run the `preprocessing_cknots.py` script and use `cknots` package, you need to have Python 3.7+ installed 
on your machine with packages specified in `requirements.txt` file. 

## How to use it?

First, create folder for data, and put the relevant files there (i.e. `GM12878.bedpe` file with contacts and
`GM12878.bed` file with CCDs). The files in the `cknots_data` folder will be available in the `/data` folder 
inside the container. 
```
mkdir /home/$USER/cknots_data/
```
Then, clone this repository in a location you prefer (for example `/home/$USER/`):
```
git clone https://github.com/SFGLab/cKNOTs
```

### Looking for minors 

Go to the repository:
```
cd cKNOTs
```
Build the Docker image and tag it as `cknots`:
```
docker build -t cknots .
```
Run the software in the Docker container:
```
docker run -v "/home/$USER/cknots_data":"/data" \
cknots \
/cknots-app/docker_cknots.sh \
/data/GM12878.bedpe \
/data/GM12878.bed \
/data/results/GM12878 \
23
```

The results will be available in the `/home/$USER/cknots_data/results/GM12878` folder. You will be able to check the progress 
by looking into log file (named based on chromosome number, so for 23'd chromosome 
the file name will be `cknots_x.log`):
```
cat /home/$USER/cknots_data/results/GM12878/cknots_x.log
```

### Preprocessing data  
    
- `preprocessing_cknots.py orientation`: Create a new `.bedpe` file with two new columns containing
  motif orientation for first and second locus of contact. 
    - `<in_bedpe>` Path to the `.bedpe` file containing information about contacts.
    - `<in_motif>` Path to the `.jaspar` file containing motif.
    - `<in_ref>` Path to the `.fa` file containing reference genome.
    - `<out_bedpe>` Path to the output `.bedpe` file.
    

- `preprocessing_cknots.py pet_filter`: Create a new `.bedpe` file in which rows with PET count smaller
  than `<min_pet_count>` are filtered out. 
    - `<in_bedpe>` Path to the `.bedpe` file containing information about contacts.
    - `<out_bedpe>` Path to the output `.bedpe` file.
    - `<min_pet_count>` Minimal number of contacts that should be left in the output file.
    

### Analyzing the results
The `cnots.analysis` module contains classes and functions that can be used to analyze the output 
of the algorithm. This includes visualization of the minors loci, comparing similarity of the 
results for different cell lines and generating tables of number of minors found in different chromosomes.
