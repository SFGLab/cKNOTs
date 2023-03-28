from typing import List

from matplotlib import pyplot as plt
from matplotlib.collections import PolyCollection

from cknots.analysis.chromosome import Chromosome


def plot(list_of_chromosomes: List[Chromosome],
         y_labels: List[str],
         figsize=(15, 3),
         fontsize=12):
    rect_size = 0.45

    all_links = []
    all_ccds = []

    for i, chromosome in enumerate(list_of_chromosomes):

        rectangles = []
        for link in chromosome.get_links():
            r = [
                (link.min(), i - rect_size),
                (link.min(), i + rect_size),
                (link.max(), i + rect_size),
                (link.max(), i - rect_size),
                (link.min(), i - rect_size)
            ]
            rectangles.append(r)

        links = PolyCollection(rectangles, facecolors='red', zorder=2)

        rectangles = []
        for ccd in chromosome.ccds:
            r = [
                (ccd.start, i - rect_size),
                (ccd.start, i + rect_size),
                (ccd.end, i + rect_size),
                (ccd.end, i - rect_size),
                (ccd.start, i - rect_size)
            ]
            rectangles.append(r)

        ccds = PolyCollection(rectangles, facecolors='cyan', zorder=0)

        all_links.append(links)
        all_ccds.append(ccds)

    plt.figure(figsize=figsize)
    ax = plt.gca()

    for links, ccds in zip(all_links, all_ccds):
        ax.add_collection(links)
        ax.add_collection(ccds)

    ax.autoscale()
    ax.get_yaxis().set_ticks(range(len(y_labels)))
    ax.set_yticklabels(y_labels, fontsize=fontsize)

    plt.xlabel("Basepair location (in relation to HG38 reference genome)", fontsize=fontsize)
