# EdgeLab

For undirected graph loading / generation:
 - only add edges where src <= dest. (up to user to keep / remove self loops)
Then, when building undirected graphs:
 - iterate vertices in order
 - add inverse edges to each of your neighbors (if yourID < neighID)
This ensures that weighted undirected graphs loaded/generated with duplicate edges with unique weights,
will result in a final graph with each pair of directed edges having the same weight.

