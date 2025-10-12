# import re
import pygame

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
    nodes = []
    edges = []

    def intersect_point(x, y):
        target_node = -1
        for i in range(len(nodes)):
            if (nodes[i][0] - x)**2 + (nodes[i][1] - y)**2 < 15**2:
                target_node = i
        return target_node


    pygame.init()
    screen = pygame.display.set_mode((2000, 1000))
    running = True

    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:
                    intersects = intersect_point(*event.pos)
                    if intersects < 0:
                        nodes.append(event.pos)
                    else:
                        print(intersects)
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_s:
                    # Export to CRS here
                    pass

        screen.fill((255, 255, 255))
        for a, b in edges:
            pygame.draw.line(screen, (0, 0, 0), nodes[a], nodes[b], 2)
        for i, (x, y) in enumerate(nodes):
            pygame.draw.circle(screen, (0, 0, 0), (x, y), 15)

        pygame.display.flip()

# def main():
#     '''Take a graph matrix and return a graphX ASM formatted graph'''
#     m = int(input('Enter the number of nodes> '))

#     # Consume the graph matrix
#     graph_matrix = [[None for _ in range(m)] for _ in range(m)]
#     for i in range(m):
#         row = input(f'row {i+1}> ')
#         values = re.split(r'[,\s]+', row)
#         for j in range(m):
#             graph_matrix[i][j] = int(values[j])

#     graph = Graph(graph_matrix)
#     print(graph)

if __name__ == '__main__':
    main()