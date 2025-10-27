graph = {
    0: [],
    1: [2],
    2: [1],
    3: [0, 1],
    4: [1, 3, 5],
    5: [1, 4],
    6: [1, 4],
    7: [1, 4],
    8: [1, 4],
    9: [4],
    10: [4],
}

def pagerank(graph, damping=0.85, max_iter=100, tol=1e-6):
    N = len(graph)
    ranks = {node: 1.0 / N for node in graph}

    # Precompute outgoing link counts
    out_counts = {node: len(out) for node, out in graph.items()}

    for _ in range(max_iter):
        new_ranks = {}

        # Base rank from random jump
        base = (1.0 - damping) / N

        for node in graph:
            new_rank = base
            # Sum contributions from nodes that link to this one
            for v in graph:
                if node in graph[v] and out_counts[v] > 0:
                    new_rank += damping * (ranks[v] / out_counts[v])
            new_ranks[node] = new_rank

        # Check convergence
        diff = sum(abs(new_ranks[n] - ranks[n]) for n in graph)
        ranks = new_ranks
        if diff < tol:
            break

    return ranks

print(pagerank(graph, max_iter=10))