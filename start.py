import numpy as np
from numpy.linalg import cond, matrix_rank

A = np.array([[1,1,0],[0,1,0],[1,0,1]])

print("condition number of A:", cond(A))

print("rank of A:", matrix_rank(A))

y = np.array([[1],[2],[1]])

A_y = np.concatenate((A, y), axis=1)

print("Augmented matrix:", A_y)

print("rank of augmented matrix:", matrix_rank(A_y))

coefficients = np.linalg.solve(A, y)