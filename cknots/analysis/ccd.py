from dataclasses import dataclass, field
from typing import List

import numpy as np
import networkx as nx
from networkx.algorithms.approximation import treewidth_min_degree

from cknots.analysis.link import Link, parse_links_from_string


def parse_graph_from_mp(graph_text: str) -> nx.DiGraph:
    nodes_in = [x.replace('NODE ', '') for x in graph_text.split('\n') if x.startswith('NODE')]
    edges_in = [x.replace('EDGE ', '') for x in graph_text.split('\n') if x.startswith('EDGE')]

    node_map = dict()

    for i, node in enumerate(nodes_in):
        node_map[node] = i + 1

    if len(node_map.values()) > 0:
        number_of_vertices = max(node_map.values())
    else:
        number_of_vertices = 0

    edges = [[i + 1, i + 2] for i in range(number_of_vertices - 1)]
    edges = edges + [[node_map[x.split(' ')[0]], node_map[x.split(' ')[1]]] for x in edges_in]

    output = nx.DiGraph()

    for e in edges:
        output.add_edge(e[0], e[1])

    return output


@dataclass
class CCD:
    start: int = field(default=None)
    end: int = field(default=None)
    number: int = field(default=None)
    links: List[Link] = field(default_factory=list)
    graph: nx.DiGraph = field(default_factory=nx.DiGraph)

    def count_links(self):
        return len(self.links)

    def load_links_from_file(self, path_minors):
        with open(path_minors) as f:
            links_text = f.read()
        self.links = parse_links_from_string(links_text)

    def load_graph_from_file(self, path_mp):
        with open(path_mp) as f:
            graph_text = f.read()
        self.graph = parse_graph_from_mp(graph_text)

    def remove_duplicate_links(self):
        new_link_list = []
        link_set = set()

        for link in self.links:
            hashed_link_edges = hash(tuple([edge.edge_id for edge in link.edges]))
            if hashed_link_edges not in link_set:
                link_set.add(hashed_link_edges)
                new_link_list.append(link)

        self.links = new_link_list

    def remove_similar_links(self, min_differences=1):
        new_link_list = []

        for new_link in self.links:

            should_be_added = True

            for old_link in new_link_list:
                endpoints_new = np.array(new_link.endpoints)
                endpoints_old = np.array(old_link.endpoints)

                diff_count = np.sum(endpoints_new != endpoints_old)

                if diff_count < min_differences:
                    should_be_added = False
                    break

            if should_be_added:
                new_link_list.append(new_link)

    def save_to_file(self, path):
        out_file_contents = ''

        for link in self.links:
            out_file_contents += str(link) + '\n'

        with open(path, 'w') as f:
            f.write(out_file_contents)

    def treewidth_approximation(self):
        return treewidth_min_degree(self.graph.to_undirected())[0]

    def cutwidth_heuristic(self):
        if len(self.graph.nodes) == 0:
            return 0

        adj_matrix = nx.linalg.adjacency_matrix(self.graph)
        cutwidth = 0
        for i in range(1, adj_matrix.shape[0] + 1):
            potential_cutwidth = adj_matrix[:i, i:].sum()
            if potential_cutwidth > cutwidth:
                cutwidth = potential_cutwidth
        return cutwidth

    def __str__(self):
        out_str = f'CCD {self.number}: [{self.start}, {self.end}]: {self.count_links()} links'
        return out_str

    def __repr__(self):
        return str(self)
