from ctypes import c_uint, c_float, Structure

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

    def to_c(self):
        class graph_t(Structure):
            _fields_ = [
                ('m', c_uint), 
                ('n', c_uint),
                ('col_index', c_uint * len(self.col_index)),
                ('row_index', c_uint * len(self.row_index)),
                ('values', c_float * len(self.values)),
            ]

        g = graph_t(self.m, self.n, self.col_index, self.row_index, self.values)
        return g