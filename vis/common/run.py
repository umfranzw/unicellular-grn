from collections import OrderedDict

class Run():
    DB_MODE = 0
    IPC_MODE = 1
    def __init__(self, db=None, ipc=None):
        if db is not None:
            self.db = db
            self.mode = Run.DB_MODE
            
        elif ipc is not None:
            self.ipc = ipc
            self.mode = Run.IPC_MODE

        else:
            print('No database connection provided')
            
        self._set_id()
        self._set_attrs()

    def _select(self, sql, args, result_types=None):
        rs = None
        if self.mode == Run.DB_MODE:
            self.db.cur.execute(sql, args)
            rs = self.db.cur

        else:
            rs = self.ipc.select(sql, args, result_types)

        return rs

    def _set_id(self):
        #get the run id (db row id)
        sql = "SELECT max(id) FROM run;"
        rs = self._select(sql, (), (int,))
        row = rs.fetchone()
        self.run_id = row[0] - 1 #sqlite starts ids at 1. Subtract 1 to make this the same as the db file name

    def _set_attrs(self):
        #get all other stuff
        cols = OrderedDict((
            ("pop_size", int),
            ("ga_steps", int),
            ("reg_steps", int),
            ("mut_prob", float),
            ("mut_prob_limit", float),
            ("mut_step", float),
            ("cross_frac", float),
            ("cross_frac_limit", float),
            ("cross_step", float),
            ("num_genes", int),
            ("gene_bits", int),
            ("min_protein_conc", float),
            ("max_protein_conc", float),
            ("decay_rate", float),
            ("initial_proteins", int),
            ("max_proteins", int),
            ("max_pgm_size", int),
            ("max_mut_float", float),
            ("max_mut_bits", int),
            ("fitness_log_interval", int),
            ("binding_seq_play", int),
            ("graph_results", int),
            ("log_grns", int),
            ("log_reg_steps", int),
            ("log_code_with_fitness", int),
            ("growth_start", int),
            ("growth_end", int),
            ("growth_sample_interval", int),
            ("growth_seq", str),
            ("growth_threshold", float),
            ("term_cutoff", float),
            ("code_start", int),
            ("code_end", int),
            ("code_sample_interval", int),
            ("fix_rng_seed", int),
            ("log_mode", str),
        ))
        sql = ("SELECT " + ", ".join(cols.keys()) + " FROM run;")

        rs = self._select(sql, (), cols.values())
        row = list(rs.fetchone())

        col_names = list(cols.keys())
        for i in range(len(col_names)):
            setattr(self, col_names[i], row[i])

    def __str__(self):
        info = "run_id: {}\n".format(self.run_id)
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
        info += "decay_rate: {}\n".format(self.decay_rate)
        info += "initial_proteins: {}\n".format(self.initial_proteins)
        info += "max_proteins: {}\n".format(self.max_proteins)
        info += "max_pgm_size: {}\n".format(self.max_pgm_size)
        info += "max_mut_float: {}\n".format(self.max_mut_float)
        info += "max_mut_bits: {}\n".format(self.max_mut_bits)
        info += "fitness_log_interval: {}\n".format(self.fitness_log_interval)
        info += "binding_seq_play: {}\n".format(self.binding_seq_play)
        info += "graph_results: {}\n".format(self.graph_results)
        info += "log_grns: {}\n".format(self.log_grns)
        info += "log_reg_steps: {}\n".format(self.log_reg_steps)
        info += "log_code_with_fitness: {}\n".format(self.log_code_with_fitness)
        info += "growth_start: {}\n".format(self.growth_start)
        info += "growth_end: {}\n".format(self.growth_end)
        info += "growth_sample_interval: {}\n".format(self.growth_sample_interval)
        info += "growth_seq: {}\n".format(self.growth_seq)
        info += "growth_threshold: {}\n".format(self.growth_threshold)
        info += "term_cutoff: {}\n".format(self.term_cutoff)
        info += "code_start: {}\n".format(self.code_start)
        info += "code_end: {}\n".format(self.code_end)
        info += "code_sample_interval: {}\n".format(self.code_sample_interval)
        info += "fix_rng_seed: {}\n".format(self.fix_rng_seed)
        info += "log_mode: {}\n".format(self.log_mode)

        return info
