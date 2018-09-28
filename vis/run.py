class Run():
    def __init__(self, conn):
        #get the run id (db row id)
        sql = "SELECT max(id) FROM run;"
        rs = conn.select(sql, [], [int])
        self.run_id = rs[0][0]

        #get all other stuff
        sql = "SELECT pop_size, ga_steps, reg_steps, mut_prob, mut_prob_limit, mut_step, cross_frac, cross_frac_limit, cross_step, num_genes, gene_bits, min_protein_conc, max_protein_conc, alpha, beta, decay_rate, initial_proteins, max_proteins, max_mut_float, max_mut_bits, fitness_log_interval, binding_method, graph_results, log_grns, log_reg_steps, log_code_with_fitness, growth_start, growth_end, growth_sample_interval, growth_seq, growth_threshold, term_cutoff, code_start, code_end, code_sample_interval FROM run;"
        
        rs = conn.select(sql, [], [int, int, int, float, float, float, float, float, float, int, int, float, float, float, float, float, int, int, float, int, int, str, int, int, int, int, int, int, int, str, float, float, int, int, int])
        
        self.pop_size, self.ga_steps, self.reg_steps, self.mut_prob, self.mut_prob_limit, self.mut_step, self.cross_frac, self.cross_frac_limit, self.cross_step, self.num_genes, self.gene_bits, self.min_protein_conc, self.max_protein_conc, self.alpha, self.beta, self.decay_rate, self.initial_proteins, self.max_proteins, self.max_mut_float, self.max_mut_bits, self.fitness_log_interval, self.binding_method, self.graph_results, self.log_grns, self.log_reg_steps, self.log_code_with_fitness, self.growth_start, self.growth_end, self.growth_sample_interval, self.growth_seq, self.growth_threshold, self.term_cutoff, self.code_start, self.code_end, self.code_sample_interval = rs[0]

    def __str__(self):
        info += "run_id: {}\n".format(self.run_id)
        info += "pop_size: {}\n".format(self.pop_size)
        info += "ga_steps: {}\n".format(self.ga_steps)
        info += "reg_steps: {}\n".format(self.reg_steps)
        info += "mut_prob: {}\n".format(self.mut_prob)
        info += "mut_prob_limit: {}\n".format(self.mut_prob_limit)
        info += "mut_step: {}\n".format(self.mut_step)
        info += "cross_frac: {}\n".format(self.cross_frac)
        info += "cross_frac_limit: {}\n".format(self.cross_frac_limit)
        info += "cross_step: {}\n".format(self.cross_step)
        info += "num_genes: {}\n".format(self.num_genes)
        info += "gene_bits: {}\n".format(self.gene_bits)
        info += "min_protein_conc: {}\n".format(self.min_protein_conc)
        info += "max_protein_conc: {}\n".format(self.max_protein_conc)
        info += "alpha: {}\n".format(self.alpha)
        info += "beta: {}\n".format(self.beta)
        info += "decay_rate: {}\n".format(self.decay_rate)
        info += "initial_proteins: {}\n".format(self.initial_proteins)
        info += "max_proteins: {}\n".format(self.max_proteins)
        info += "max_mut_float: {}\n".format(self.max_mut_float)
        info += "max_mut_bits: {}\n".format(self.max_mut_bits)
        info += "fitness_log_interval: {}".format(self.fitness_log_interval)
        info += "binding_method: {}".format(self.binding_method)
        info += "graph_results: {}".format(self.graph_results)
        info += "log_grns: {}".format(self.log_grns)
        info += "log_reg_steps: {}".format(self.log_reg_steps)
        info += "log_code_with_fitness: {}".format(self.log_code_with_fitness)
        info += "growth_start: {}".format(self.growth_start)
        info += "growth_end: {}".format(self.growth_end)
        info += "growth_sample_interval: {}".format(self.growth_sample_interval)
        info += "growth_seq: {}".format(self.growth_seq)
        info += "growth_threshold: {}".format(self.growth_threshold)
        info += "term_cutoff: {}".format(self.term_cutoff)
        info += "code_start: {}".format(self.code_start)
        info += "code_end: {}".format(self.code_end)
        info += "code_sample_interval: {}".format(self.code_sample_interval)

        return info
