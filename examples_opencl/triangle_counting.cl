// OpenCL kernel for triangle counting (classic approach)
__kernel void count_triangles(
    __global const uint* vertices,      // Vertex offsets in CSR format
    __global const uint* edges,         // Edge destinations in CSR format
    __global uint* triangle_count,      // Output: total triangle count
    const uint num_vertices             // Number of vertices in the graph
) {
    uint u = get_global_id(0);
    if (u >= num_vertices) return;

    uint u_start = vertices[u];
    uint u_end = vertices[u + 1];
    uint local_triangles = 0;

    for (uint i = u_start; i < u_end; i++) {
        uint v = edges[i];
        if (u >= v) continue; // Only consider u < v

        uint v_start = vertices[v];
        uint v_end = vertices[v + 1];

        // For each neighbor w of v where v < w
        for (uint j = v_start; j < v_end; j++) {
            uint w = edges[j];
            if (v >= w) continue; // Only consider v < w

            // Binary search for w in u's neighbor list
            uint left = u_start, right = u_end;
            while (left < right) {
                uint mid = left + (right - left) / 2;
                if (edges[mid] < w) {
                    left = mid + 1;
                } else {
                    right = mid;
                }
            }
            if (left < u_end && edges[left] == w) {
                local_triangles++;
            }
        }
    }
    if (local_triangles > 0) {
        atomic_add(triangle_count, local_triangles);
    }
} 