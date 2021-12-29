import re
from dataclasses import dataclass, field
from typing import List, Tuple

import networkx as nx
from matplotlib import pyplot as plt


@dataclass(frozen=True)
class Locus:
    id: int
    chromosome: str
    locus: int

    def __str__(self):
        out_str = f"({self.id}=chr{self.chromosome}_{self.locus:010d})"
        return out_str


@dataclass(frozen=True)
class Edge:
    start: int
    end: int
    edge_id: int
    left: Locus
    right: Locus

    def __str__(self):
        # from 0 to 4, eid=9, left=(0=chr1_0006779133), right=(86=chr1_0007251702)
        return f"from {self.start} to {self.end}, eid={self.edge_id}, left={self.left}, right={self.right}"


@dataclass(frozen=True)
class Endpoint:
    segment_number: int
    start: Locus
    end: Locus

    def __str__(self):
        # segment=0 start=(0=chr1_0006779133) end=(49=chr1_0006948069)
        return f"segment={self.segment_number} start={self.start} end={self.end}"


@dataclass(frozen=True, eq=True)
class Link:
    endpoints: Tuple[Endpoint]
    edges: Tuple[Edge] = field(default_factory=Tuple, hash=False)

    def plot(self):
        g = nx.DiGraph()

        for endpoint in self.endpoints:
            g.add_node(endpoint)

        solid_edges = []
        for i in range(5):
            solid_edges.append([self.endpoints[i], self.endpoints[i + 1]])

        jump_edges = []
        for edge in self.edges:
            jump_edges.append([self.endpoints[edge.left.locus], self.endpoints[edge.right.locus]])

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
                               arrowsize=30)

        nx.draw_networkx_edges(g,
                               pos=nx.circular_layout(g),
                               edgelist=jump_edges,
                               edge_color='blue',
                               style='dashed',
                               arrows=False)

        plt.show()

    def min(self):
        return min([x.start.locus for x in self.endpoints])

    def max(self):
        return max([x.end.locus for x in self.endpoints])

    def __repr__(self):
        return str(self)

    def __str__(self):
        out_str = 'MINOR { \n'
        out_str += '  endpoints=[\n'
        for endpoint in self.endpoints:
            out_str += f'    {endpoint}\n'
        out_str += ']\n  edges=[\n'
        for edge in self.edges:
            out_str += f'  {edge}\n'
        out_str += '  ]\n'
        out_str += '}\n'
        return out_str


def parse_links_from_string(raw_minor_text: str) -> List[Link]:
    links = []

    for minor_text in raw_minor_text.split('MINOR'):
        endpoints = __get_endpoints(minor_text[minor_text.find('endpoints'):minor_text.find('edges')])
        edges = __get_edges(minor_text[minor_text.find('edges'):])

        if len(endpoints) > 0 and len(edges) > 0:
            links.append(
                Link(tuple(endpoints), tuple(edges))
            )

    return links


def __get_endpoints(endpoints_text: str) -> List[Endpoint]:
    endpoints = []

    endpoints_segment_numbers = re.findall('segment=(\d+)', endpoints_text)

    endpoints_starts_id = re.findall('start=\((\d+)=chr', endpoints_text)
    endpoints_starts_chr = re.findall('start=\(\d+=chr(\d+|X)', endpoints_text)
    endpoints_starts_pos = re.findall('start=\(\d+=chr(?:\d+|X)_(\d+)', endpoints_text)
    endpoints_ends_id = re.findall('end=\((\d+)=chr', endpoints_text)
    endpoints_ends_chr = re.findall('end=\(\d+=chr(\d+|X)', endpoints_text)
    endpoints_ends_pos = re.findall('end=\(\d+=chr(?:\d+|X)_(\d+)', endpoints_text)

    for seg_num, start_id, start_chr, start_pos, end_id, end_chr, end_pos \
            in zip(endpoints_segment_numbers, endpoints_starts_id, endpoints_starts_chr, endpoints_starts_pos,
                   endpoints_ends_id, endpoints_ends_chr, endpoints_ends_pos):
        endpoints.append(
            Endpoint(
                int(seg_num),
                Locus(int(start_id), start_chr, int(start_pos)),
                Locus(int(end_id), end_chr, int(end_pos))
            )
        )

    return endpoints


def __get_edges(edges_text) -> List[Edge]:
    edges = []

    edges_starts = re.findall('from (\d+)', edges_text)
    edges_ends = re.findall(' to (\d+)', edges_text)
    edges_ids = re.findall('eid=(\d+)', edges_text)

    edges_lefts_id = re.findall('left=\((\d+)=chr', edges_text)
    edges_lefts_chr = re.findall('left=\(\d+=chr(\d+|X)', edges_text)
    edges_lefts_pos = re.findall('left=\(\d+=chr(?:\d+|X)_(\d+)', edges_text)
    edges_rights_id = re.findall('right=\((\d+)=chr', edges_text)
    edges_rights_chr = re.findall('right=\(\d+=chr(\d+|X)', edges_text)
    edges_rights_pos = re.findall('right=\(\d+=chr(?:\d+|X)_(\d+)', edges_text)

    for start, end, edge_id, left_id, left_chr, left_pos, right_id, right_chr, right_pos \
            in zip(edges_starts, edges_ends, edges_ids,
                   edges_lefts_id, edges_lefts_chr, edges_lefts_pos,
                   edges_rights_id, edges_rights_chr, edges_rights_pos):
        edges.append(
            Edge(
                int(start), int(end), int(edge_id),
                Locus(int(left_id), left_chr, int(left_pos)),
                Locus(int(right_id), right_chr, int(right_pos))
            )
        )

    return edges
