# import re
import pygame
import math

class Graph:
    def __init__(self, graph_matrix: list[list[float]]) -> None:
        self.m = len(graph_matrix)
        self.n = len(graph_matrix)
        self.col_index = []
        self.row_index = []
        self.values = []

        # Convert to compressed sparse row format
        col_length = 0
        for i in range(self.m):
            self.row_index.append(col_length)
            for j in range(self.n):
                if int(graph_matrix[i][j]) != 0:
                    self.col_index.append(j)
                    self.values.append(graph_matrix[i][j])
                    col_length += 1
        self.row_index.append(col_length)

    def __str__(self):
        '''Print the graph in graphX ASM format'''
        result = ''
        result += f'.row_index\n{", ".join(map(str, self.row_index))}\n\n'
        result += f'.col_index\n{", ".join(map(str, self.col_index))}\n\n'
        result += f'.values\n{", ".join(map(str, self.values))}\n\n'
        return result
    
def main():
    # Nodes and edges are just stored as a list of tuples
    # Nodes are (x, y)
    # Edges are (node_1, node_2, weight, direction)
    nodes = []
    edges = []

    # Used for drawing nodes and edges
    current_node = -1
    start, end = -1, -1

    # Used for adding edge weights
    weight_text = ''
    popup_rect = pygame.Rect(950, 475, 100, 50) # (100x50 centered in screen)
    popup_active = False
    current_edge = -1

    def intersect_point(x, y):
        '''Get an intersecting node if it exists'''
        target_node = -1

        for i in range(len(nodes)):
            # Hash map is stored as entries with (index, x_pos, y_pos)
            if (nodes[i][0] - x)**2 + (nodes[i][1] - y)**2 < 15**2:
                target_node = i
        return target_node
    
    def to_adjacency_matrix():
        '''Convert the nodes and edges to an adjacency matrix'''
        n = len(nodes)
        matrix = [[0 for _ in range(n)] for _ in range(n)]
        for a, b, w, direction in edges:
            if direction == 0:
                matrix[a][b] = w
                matrix[b][a] = w
            elif direction == 1:
                matrix[a][b] = w
            else:
                matrix[b][a] = w

        return matrix
    
    def dist_to_line(p, v, w):
        '''Calculate the squared distance from a point to a line segment'''
        # Distance of line
        l2 = (v[0] - w[0])**2 + (v[1] - w[1])**2
        if l2 == 0:
            # Reduces to a single point if length is 0
            return (p[0] - v[0])**2 + (p[1] - v[1])**2
        
        # Found this on stack overflow, not sure exactly how it works
        # https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
        t = ((p[0] - v[0]) * (w[0] - v[0]) + (p[1] - v[1]) * (w[1] - v[1])) / l2
        t = max(0, min(1, t))
        p2 = (v[0] + t * (w[0] - v[0]), v[1] + t * (w[1] - v[1]))
        return (p[0] - p2[0])**2 + (p[1] - p2[1])**2

    def closest_edge(x, y):
        '''Find the closest edge to a position'''
        min_edge = -1
        min_dist = float('inf')
        for i, (a, b, _, _) in enumerate(edges):
            a = nodes[a]
            b = nodes[b]
            dist = dist_to_line((x, y), a, b)

            if dist < min_dist:
                min_dist = dist
                min_edge = i

        return min_edge

    # Initialize pygame
    pygame.init()
    screen = pygame.display.set_mode((2000, 1000))
    running = True
    font = pygame.font.SysFont('Arial', 15)

    while running:
        '''Event loop'''
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            if popup_active:
                if event.type == pygame.KEYDOWN:
                    # Typing in the input box
                    if event.key == pygame.K_RETURN:
                        # Done entering weight
                        popup_active = False
                        e = edges[current_edge]
                        edges[current_edge] = (e[0], e[1], int(weight_text), e[3])
                    elif event.key == pygame.K_BACKSPACE:
                        # Backspace input
                        weight_text = weight_text[:-1]
                    else:
                        # Default input
                        weight_text += event.unicode
            else:
                if event.type == pygame.MOUSEBUTTONDOWN:
                    if event.button == 1:
                        # Left button click
                        i = intersect_point(*event.pos)
                        if i < 0:
                            # Not intersecting, add to nodes and spatial hash
                            nodes.append((event.pos[0], event.pos[1]))
                            current_node = len(nodes)-1
                        else:
                            # Intersecting, handle intersection
                            start = i
                elif event.type == pygame.MOUSEBUTTONUP:
                    if event.button == 1:
                        # Left button unclick
                        i = intersect_point(*event.pos)
                        if i < 0:
                            # Incorrect drag
                            start = -1
                        elif i == current_node:
                            current_node = -1
                            pass
                        else:
                            # Drag from one node to another
                            end = i
                            if start >= 0 and end >= 0:
                                edges.append((start, end, 1, 0))
                                start, end = -1, -1
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_s:
                        # Print out the graph in CSR format
                        matrix = to_adjacency_matrix()
                        graph = Graph(matrix)
                        print(graph)
                    elif event.key == pygame.K_w:
                        # Add an edge weight
                        current_edge = closest_edge(*pygame.mouse.get_pos())
                        weight_text = ''
                        popup_active = True
                    elif event.key == pygame.K_e:
                        # Toggle edge from undirected to directed
                        e = closest_edge(*pygame.mouse.get_pos())
                        edge = edges[e]
                        new_direction = ((edge[3] + 2) % 3) - 1
                        edges[e] = (edge[0], edge[1], edge[2], new_direction)
        
        # White background
        screen.fill((255, 255, 255))

        # Draw the edges first so the nodes show up in front of them
        for a, b, w, d in edges:
            pygame.draw.line(screen, (0, 0, 0), nodes[a], nodes[b], 2)

            # Check for direction
            if d > 0:
                dest = nodes[b]
                src = nodes[a]
            elif d < 0:
                dest = nodes[a]
                src = nodes[b]
            else:
                dest = None
                src = None

            if dest is not None and src is not None:            
                # Destination to source vector (point back)
                vec = (src[0] - dest[0], src[1] - dest[1])
                
                # Calculate center
                center = (dest[0] + 0.4*vec[0], dest[1] + 0.4*vec[1])
                pygame.draw.circle(screen, (255, 0, 0), center, 5)

            # Put the edge weight on the center of the line
            line_center = ((nodes[a][0] + nodes[b][0]) // 2, (nodes[a][1] + nodes[b][1]) // 2)
            text_surface = font.render(str(w), True, (0, 0, 0))
            text_rect = text_surface.get_rect(center=line_center)
            pygame.draw.rect(screen, (255, 255, 255), text_rect)
            screen.blit(text_surface, text_rect)

        # If the user is currently drawing an edge draw the edge as it's created
        if start >= 0:
            pygame.draw.line(screen, (255, 0, 0), nodes[start], pygame.mouse.get_pos(), 2)

        # Draw the nodes
        for i, (x, y) in enumerate(nodes):
            pygame.draw.circle(screen, (0, 0, 0), (x, y), 15)

            # Put the node ID on the center of the node
            text_surface = font.render(str(i), True, (255, 255, 255))
            text_rect = text_surface.get_rect(center=(x, y))
            screen.blit(text_surface, text_rect)

        # Draw the popup last so it overlays everything else
        if popup_active:
            # Draw popup background 
            pygame.draw.rect(screen, (100, 100, 100), (925, 450, 150, 100)) # Input background
            pygame.draw.rect(screen, (255, 255, 255), popup_rect, 2) # Input box border

            # Render and blit input text
            text_surface = font.render(weight_text, True, (255, 255, 255))
            screen.blit(text_surface, (popup_rect.x + 5, popup_rect.y + 5))

            # Adjust weight width dynamically
            popup_rect.w = max(100, text_surface.get_width() + 10)

        # Flip double buffers
        pygame.display.flip()

pygame.quit()

if __name__ == '__main__':
    main()