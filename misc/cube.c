#include <stdio.h>

// Vertex numbering
//
//     ^   6_________ 7
//    y|   /:       /|
//        / :      / |    ^
//      2/________/3 |   /z
//       | 4:.....|..|5
//       | .      |  /
//       |.       | /   x
//       |________|/   -->
//      0          1


// wound right
const int quads[6][4] = {
    { 0, 2, 3, 1 },
    { 0, 1, 5, 4 },
    { 0, 4, 6, 2 },
    { 1, 3, 7, 5 },
    { 2, 6, 7, 3 },
    { 4, 5, 7, 6 }
};


int
main() {
    int v[14] = { 0, 1 }; // vertex sequence
    int u[24] = { 0 }; // half-triangles (quarter-quads) already filled
    int h0[14] = { 0 }; // which half-triangle was filled by vertex
    int h1[14] = { 0 };

    for (int i = 2; i >= 2;) { // vertex number
        int l = i % 2; // left winding
        // find which quad contains the triangle this vertex makes
        for (int q = 0; q < 6; q++) // quad
            for (int h = 0; h < 4; h++) // start coordinate in quad
                for (int j = 0; j < 3; j++) { // start coordinate in triangle
                    if (v[i-(j+2*!l)%3] == quads[q][ h     ] &&
                        v[i-(j+1   )%3] == quads[q][(h+1)%4] &&
                        v[i-(j+2* l)%3] == quads[q][(h+2)%4]) {
                        h0[i] = q*4 +  h;
                        h1[i] = q*4 + (h+1)%4;
                        if (u[h0[i]] || u[h1[i]])
                            goto bad;
                        else
                            goto good;
                    }
                }
        goto bad; // not in quad

    good:
        if (i < 13) {
            u[h0[i]] = 1;
            u[h1[i]] = 1;
            i++;
            continue;
        } else {
            printf("// ");
            for (int i = 0; i < 14; i++)
                printf("%d ", v[i]);
                //printf("    %d, %d, %d, 1,\n", v[i]&1, v[i]/2&1, v[i]/4&1);
            printf("\n");
            printf("const GLfloat cube_strip[] = {\n");
            for (int i = 0; i < 14; i++)
                printf("    %d, %d, %d, 1,\n", v[i]&1, v[i]>>1&1, v[i]>>2&1);
            printf("};\n");
            //return 0;
            // fall through to list all sequences
        }

    bad:
        // backtrack
        for (;;)
            if (++v[i] == 8) {
                v[i] = 0;
                i--;
                u[h0[i]] = 0;
                u[h1[i]] = 0;
            } else
                break;
    }
}
