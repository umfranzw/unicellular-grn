#include "diffusion.hpp"
#include <cmath>

bool Diffusion::diffuse(Protein *p) {
    float F = 0.3; // * dt / pow(dx, 2);
    float right = 1.0;
    float left = 1.0;
    bool above_threshold = false;

    vector<float> prev_concs = p->concs;
    
    //update position 0
    p->concs[0] = prev_concs[0] + F * (-2.0f * prev_concs[0] + left * prev_concs[1]);
    above_threshold = above_threshold || (p->concs[0] > p->run->min_protein_conc);
    
    for (int i = 1; i < p->run->num_genes - 1; i++) {
        float new_conc = prev_concs[i] + F * (right * prev_concs[i - 1] - 2.0f * prev_concs[i] + left * prev_concs[i + 1]);
        p->concs[i] = min(new_conc, p->run->max_protein_conc); //ensure we don't go over the max
        above_threshold = above_threshold || (p->concs[i] > p->run->min_protein_conc);
    }
    //update position n-1
    p->concs[p->run->num_genes - 1] = prev_concs[p->run->num_genes - 1] + F * (right * prev_concs[p->run->num_genes - 2] - 2.0f * prev_concs[p->run->num_genes - 1]);
    above_threshold = above_threshold || (p->concs[p->run->num_genes - 1] > p->run->min_protein_conc);

    return above_threshold;
}
