import re
from dataclasses import dataclass
from typing import List

import networkx as nx
from matplotlib import pyplot as plt


@dataclass(frozen=True)
class Edge:
    start_segment: int
    end_segment: int
    edge_id: int
    left: int
    right: int


@dataclass(frozen=True)
class Endpoint:
    segment_number: int
    start: int
    end: int

    def __str__(self):
        return f"[{self.start},{self.end}]"


@dataclass
class Link:
    endpoints: List[Endpoint]
    edges: List[Edge]

    def plot(self):
        g = nx.DiGraph()

        for endpoint in self.endpoints:
            g.add_node(endpoint)

        solid_edges = []
        for i in range(5):
            solid_edges.append([self.endpoints[i], self.endpoints[i + 1]])

        jump_edges = []
        for edge in self.edges:
            jump_edges.append([self.endpoints[edge.start_segment], self.endpoints[edge.end_segment]])

        plt.figure(figsize=(14, 8))
        plt.margins(x=0.2)

        nx.draw_networkx_nodes(g, pos=nx.circular_layout(g))
        nx.draw_networkx_labels(g, pos=nx.circular_layout(g), font_size=13)

        nx.draw_networkx_edges(g,
                               pos=nx.circular_layout(g),
                               edgelist=solid_edges,
                               edge_color='red',
                               arrows=True,
                               width=2,
                               arrowsize=24)

        nx.draw_networkx_edges(g,
                               pos=nx.circular_layout(g),
                               edgelist=jump_edges,
                               edge_color='blue',
                               style='dashed',
                               arrows=False)

        plt.show()

    def min(self):
        return min([x.start for x in self.endpoints])

    def max(self):
        return max([x.end for x in self.endpoints])

    def __repr__(self):
        return str(self)

    def __str__(self):
        out_str = 'Endpoints = [\n'
        for endpoint in self.endpoints:
            out_str += f'\t{endpoint}\n'
        out_str += ']\nEdges = [\n'
        for edge in self.edges:
            out_str += f'\t{edge}\n'
        out_str += ']\n'
        return out_str


def parse_links_from_string(raw_minor_text: str) -> List[Link]:
    links = []

    for minor_text in raw_minor_text.split('MINOR'):
        endpoints = __get_endpoints(minor_text[minor_text.find('endpoints'):minor_text.find('edges')])
        edges = __get_edges(minor_text[minor_text.find('edges'):])

        links.append(
            Link(endpoints, edges)
        )

    return links


def __get_endpoints(endpoints_text: str) -> List[Endpoint]:
    endpoints = []

    endpoints_segment_numbers = re.findall('segment=(\d+)', endpoints_text)
    endpoints_starts = re.findall('start=\(\d+=chr(?:\d+|X)_(\d+)', endpoints_text)
    endpoints_ends = re.findall('end=\(\d+=chr(?:\d+|X)_(\d+)', endpoints_text)

    for seg_num, start, end in zip(endpoints_segment_numbers, endpoints_starts, endpoints_ends):
        endpoints.append(
            Endpoint(int(seg_num), int(start), int(end))
        )

    return endpoints


def __get_edges(edges_text) -> List[Edge]:
    edges = []

    edges_starts = re.findall('from (\d+)', edges_text)
    edges_ends = re.findall(' to (\d+)', edges_text)
    edges_ids = re.findall('eid=(\d+)', edges_text)
    edges_rights = re.findall('right=\(\d+=chr(?:\d+|X)_(\d+)', edges_text)
    edges_lefts = re.findall('left=\(\d+=chr(?:\d+|X)_(\d+)', edges_text)

    for start, end, id, right, left in zip(edges_starts, edges_ends, edges_ids, edges_rights, edges_lefts):
        edges.append(
            Edge(int(start), int(end), int(id), int(right), int(left))
        )

    return edges
