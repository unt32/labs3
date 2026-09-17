import matplotlib.pyplot as plt


# Формат каждой строки: <left>\t<node>\t<right>
def parse_lines(text):
    edges = {}
    childrens = set()
    nodes = set()

    for line in text.strip().splitlines():
        line = line.strip()
        if not line:
            continue
        parts = line.split("\t")
        if len(parts) != 3:
            parts = line.split()
        if len(parts) != 3:
            continue

        left_s, node_s, right_s = (p.strip() for p in parts)

        left = None if left_s == "null" else left_s
        right = None if right_s == "null" else right_s
        node = node_s

        edges[node] = (left, right)
        nodes.add(node)
        if left is not None:
            childrens.add(left)
        if right is not None:
            childrens.add(right)

    root_candidates = nodes - childrens
    root = next(iter(root_candidates)) if root_candidates else None
    return edges, root


def compute_layout(edges, root):
    """In-order traversal даёт x-координату, глубина - y-координату."""
    positions = {}
    x_counter = [0]

    def inorder(node, depth):
        if node is None:
            return
        left, right = edges.get(node, (None, None))
        inorder(left, depth + 1)
        positions[node] = (x_counter[0], -depth)
        x_counter[0] += 1
        inorder(right, depth + 1)

    inorder(root, 0)
    return positions


def draw(edges, root, positions):
    n = len(positions)
    max_depth = max(-y for _, y in positions.values()) if positions else 0

    # автоматический масштаб: чем больше узлов, тем шире фигура и меньше кружки,
    # чтобы они гарантированно не накладывались друг на друга по горизонтали
    fig_width = max(6, n * 0.55)
    fig_height = max(4, (max_depth + 1) * 1.3)

    fig, ax = plt.subplots(figsize=(fig_width, fig_height))

    # оценка расстояния между соседними узлами в points (72pt = 1 inch)
    # с учётом того, что часть ширины фигуры уходит на отступы
    usable_width_inches = fig_width * 0.92
    spacing_points = (usable_width_inches / max(n, 1)) * 72

    marker_diameter = max(6, spacing_points * 0.9)
    marker_size = marker_diameter**2
    fontsize = max(3, min(12, marker_diameter * 0.32))

    # рёбра
    for node, (left, right) in edges.items():
        if node not in positions:
            continue
        x0, y0 = positions[node]
        for child in (left, right):
            if child is not None and child in positions:
                x1, y1 = positions[child]
                ax.plot([x0, x1], [y0, y1], color="gray", zorder=1, linewidth=0.8)

    # узлы
    for node, (x, y) in positions.items():
        ax.scatter(
            [x],
            [y],
            s=marker_size,
            color="#89CFF0",
            edgecolor="black",
            linewidth=0.6,
            zorder=2,
        )
        ax.text(
            x,
            y,
            str(node),
            ha="center",
            va="center",
            fontsize=fontsize,
            fontweight="bold",
            zorder=3,
        )

    ax.set_xlim(-1, n)
    ax.set_axis_off()
    plt.tight_layout()
    plt.show()


def read_until_empty_line():
    # print("Enter tree:")
    lines = []
    while True:
        try:
            line = input()
        except EOFError:
            break
        if line.strip() == "":
            break
        lines.append(line)
    return "\n".join(lines)


def main():
    text = read_until_empty_line()
    if not text.strip():
        print("Parsing error")
        return

    edges, root = parse_lines(text)
    if root is None:
        print("Parsing error")
        return

    positions = compute_layout(edges, root)
    draw(edges, root, positions)


if __name__ == "__main__":
    main()
