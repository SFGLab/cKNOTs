FROM teeks99/boost-cpp-docker:gcc-11
ENV DEBIAN_FRONTEND noninteractive
ENV SPLITTER_PATH '/cknots-app/cknots/cpp/src/splitter/'
ENV MINOR_FINDER_PATH '/cknots-app/cknots/cpp/src/minor-finder/'
ENV BIN_PATH '/cknots-app/cknots/cpp/bin/'

COPY requirements_docker.txt /cknots-app/requirements.txt
COPY cknots /cknots-app/cknots
COPY cknots.py /cknots-app/cknots.py
COPY docker_cknots.sh /cknots-app/docker_cknots.sh

WORKDIR /cknots-app
SHELL ["/bin/bash", "-c"]

# Downloading required software
RUN \
    apt-get update \
    && apt -y install python3.9-venv python3-pip cmake libboost-program-options-dev\
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Setting up Python
RUN \
    python3.9 -m venv /venv \
    && source /venv/bin/activate \
    && pip3 install --upgrade pip \
    && pip3 install --upgrade wheel \
    && pip3 install -r requirements.txt \
    && python3.9 --version

# Creating splitter binary
RUN cmake $SPLITTER_PATH \
    && make \
    && mv splitter $BIN_PATH/splitter \
    && rm CMakeCache.txt \
	&& rm cmake_install.cmake \
	&& rm Makefile \
	&& rm -r CMakeFiles

# Creating find-k6-linear binary
RUN cmake $MINOR_FINDER_PATH \
    && make \
    && mv find-k6-linear $BIN_PATH/find-k6-linear \
    && mv find-knots $BIN_PATH/find-knots \
    && mv parse_raw $BIN_PATH/parse_raw \
    && mv path-decomposition $BIN_PATH/path-decomposition \
    && rm CMakeCache.txt \
	&& rm cmake_install.cmake \
	&& rm Makefile \
	&& rm -r CMakeFiles

RUN chmod +x docker_cknots.sh
