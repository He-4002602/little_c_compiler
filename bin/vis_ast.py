import json
import matplotlib.pyplot as plt
import networkx as nx
from collections import defaultdict

def load_tree_from_json(filename):
    with open(filename) as f:
        return json.load(f)

def build_graph(tree, graph=None, parent=None, node_counter=None):
    if graph is None:
        graph = nx.DiGraph()
    if node_counter is None:
        node_counter = defaultdict(int)  # 用于生成唯一ID
    
    # 生成唯一节点ID
    node_name = tree['name']#+': '+str(tree['tokenIndex'])
    # node_name = tree['name']
    node_counter[node_name] += 1
    node_id = f"{node_name}_{node_counter[node_name]}"
    
    graph.add_node(node_id, label=node_name)  # 存储原始名称
    
    if parent is not None:
        graph.add_edge(parent, node_id)
    
    if 'children' in tree:
        for child in tree['children']:
            build_graph(child, graph, node_id, node_counter)
    
    return graph

def visualize_tree(filename):
    tree_data = load_tree_from_json(filename)
    G = build_graph(tree_data)
    pos = nx.nx_agraph.graphviz_layout(G, prog="dot") 
    
    labels = nx.get_node_attributes(G, 'label')
    
    # 绘制图形
    nx.draw(
        G,
        pos=pos,
        with_labels=True,
        labels=labels,
        node_shape='s',  # 方形节点
        font_size=10,    # 固定字体大小
        arrows=False,
        node_color='white',
        # edgecolors='black'
    )
    
    plt.show()

visualize_tree('AST.json')