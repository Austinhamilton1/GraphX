from ctypes import c_uint, c_float, Structure
import re

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
        result += f'.row_index\n{", ".join(map(str, self.row_index))}\n'
        result += f'.col_index\n{", ".join(map(str, self.col_index))}\n'
        result += f'.values\n{", ".join(map(str, self.values))}\n'
        return result


def main():
    '''Take a graph matrix and return a graphX ASM formatted graph'''
    m = int(input('Enter the number of nodes> '))

    # Consume the graph matrix
    graph_matrix = [[None for _ in range(m)] for _ in range(m)]
    for i in range(m):
        row = input(f'row {i+1}> ')
        values = re.split(r'[,\s]+', row)
        for j in range(m):
            graph_matrix[i][j] = int(values[j])

    graph = Graph(graph_matrix)
    print(graph)

if __name__ == '__main__':
    main()