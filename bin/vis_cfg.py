import json
import networkx as nx
import matplotlib.pyplot as plt

def draw_cfg(json_file):
    with open(json_file, 'r') as f:
        data = json.load(f)

    G = nx.DiGraph()

    # 加节点
    for node in data["nodes"]:
        label = node["label"]
        patchloc = tuple(node["patchloc"])
        G.add_node(label, patchloc=patchloc)

    # 加边
    for edge in data["edges"]:
        src = edge["from"]
        dst = edge["to"]
        G.add_edge(src, dst)

    # 使用Graphviz布局（dot）
    pos = nx.nx_agraph.graphviz_layout(G, prog='dot')  # 竖直方向，从上到下

    nx.draw(
        G,
        pos,
        with_labels=True,
        arrows=True,
        node_size=2000,
        node_color='white',
        font_size=12,
        node_shape='s',
        edgecolors='black',
    )

    plt.show()

if __name__ == "__main__":
    draw_cfg("CFG.json")
