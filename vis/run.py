class Run():
    def __init__(self, conn):
        sql = "SELECT pop_size, ga_steps, reg_steps, mut_prob, cross_frac, num_genes, gene_bits, min_protein_conc, max_protein_conc, alpha, beta, decay_rate, initial_proteins, max_mut_float, max_mut_bits, fitness_log_interval FROM run;"
        rs = conn.execute(sql)
        self.pop_size, self.ga_steps, self.reg_steps, self.mut_prob, self.cross_frac, self.num_genes, self.gene_bits, self.min_protein_conc, self.max_protein_conc, self.alpha, self.beta, self.decay_rate, self.initial_proteins, self.max_mut_float, self.max_mut_bits, self.fitness_log_interval = rs.fetchone()

    def __str__(self):
        info = "pop_size: {}\n".format(self.pop_size)
        info += "ga_steps: {}\n".format(self.ga_steps)
        info += "reg_steps: {}\n".format(self.reg_steps)
        info += "mut_prob: {}\n".format(self.mut_prob)
        info += "cross_frac: {}\n".format(self.cross_frac)
        info += "num_genes: {}\n".format(self.num_genes)
        info += "gene_bits: {}\n".format(self.gene_bits)
        info += "min_protein_conc: {}\n".format(self.min_protein_conc)
        info += "max_protein_conc: {}\n".format(self.max_protein_conc)
        info += "alpha: {}\n".format(self.alpha)
        info += "beta: {}\n".format(self.beta)
        info += "decay_rate: {}\n".format(self.decay_rate)
        info += "initial_proteins: {}\n".format(self.initial_proteins)
        info += "max_mut_float: {}\n".format(self.max_mut_float)
        info += "max_mut_bits: {}\n".format(self.max_mut_bits)
        info += "fitness_log_interval: {}".format(self.fitness_log_interval)

        return info
