#include "logger.hpp"

#include <iostream>
#include <sstream>
#include "gene.hpp"
#include "protein.hpp"
#include "protein_store.hpp"
#include "node.hpp"
#include <cstdio>
#include <fstream>
#include <cmath>

Logger::Logger(Run *run) {
    this->run = run;
    
    //ensure statements are serialized so that using multiple openMP threads in the simulation (which may call logger member functions) won't mess up the order of transactions
    int rc = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    if (rc != SQLITE_OK) {
        cerr << "Error enabling sqlite serialized mode: " << rc << endl;
        exit(1);
    }

    //enable URI filenames so that we can share a single cache between multiple connections (so we can open multiple connections to the same in-memory database)
    // rc = sqlite3_config(SQLITE_CONFIG_URI, 1);
    // if (rc != SQLITE_OK) {
    //     cerr << "Error enabling sqlite URI filenames: " << rc << endl;
    //     exit(1);
    // }

    //we'll log to an in-memory database during the simulation (since it's an order of magnitude faster than a disk-based db)
    //Then, when the simulation ends, the user should call write_db() to write it out to disk in one shot
    //rc = sqlite3_open("file::memory:?cache=shared", &this->conn);
    rc = sqlite3_open(":memory:", &this->conn);
    if (rc) {
        cerr << "Cannot create in-memory database connection" << endl;
        exit(1);
    }

    //we won't be rolling back, so turn this off.
    rc = sqlite3_exec(this->conn, "PRAGMA journal_mode = OFF;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "Error turning off sqlite journaling: " << rc << endl;
    }

    this->gen_avg_fitness = -1.0f;
    this->create_tables();
}

//write the in-memory database to disk
void Logger::write_db() {
    if (this->run->log_grns) {
        sqlite3 *disk_conn;
        stringstream path;
        path << this->run->log_dir << "/run" << this->run->file_index << ".db";
        string path_str = path.str();
    
        //check if file already exists
        ifstream fin(path_str);
        if (fin) {
            //show no mercy!
            remove(path_str.c_str());
        }
                                                              
        int rc = sqlite3_open(path_str.c_str(), &disk_conn);
        if (rc) {
            cerr << "Cannot create disk database file" << endl;
            exit(1);
        }
        rc = sqlite3_exec(disk_conn, "PRAGMA journal_mode = OFF;", nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            cerr << "Error turning off sqlite journaling: " << rc << endl;
        }
    
        sqlite3_backup *backup = sqlite3_backup_init(disk_conn, "main", this->conn, "main");
        sqlite3_exec(disk_conn, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
        sqlite3_backup_step(backup, -1); //pass -1 to copy entire db
        sqlite3_exec(disk_conn, "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
        sqlite3_backup_finish(backup); //cleans up resources allocated for copy operation

        sqlite3_close(disk_conn);
    }
}

void Logger::print_results(int total_iters) {
    cout << "********" << endl;
    cout << "Results:" << endl;
    cout << "********" << endl;
    cout << "total iterations: " << total_iters << endl;
    cout << endl;
    cout << "grn:" << endl;
    cout << "run-best fitness: " << this->grn_bests.get_run_best_fitness() << endl;
    cout << "run-best pop_index: " << this->grn_bests.get_run_best_index() << endl;
    cout << "run-best iter (0-based): " << this->grn_bests.get_run_best_iter() << endl;
    cout << endl;
    cout << "phenotype:" << endl;
    cout << "size: " << this->ptype_bests.get_run_best()->size() << endl;
    cout << "filled_nodes: " << this->ptype_bests.get_run_best()->get_num_filled_nodes() << endl;
    cout << "unfilled_nodes: " << this->ptype_bests.get_run_best()->get_num_unfilled_nodes() << endl;
    cout << this->ptype_bests.get_run_best()->to_code() << endl;
}

Logger::~Logger() {
    sqlite3_close(this->conn);
}

void Logger::create_tables() {
    sqlite3_exec(this->conn, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    
    //run
    stringstream run_sql;
    run_sql << "CREATE TABLE run (";
    run_sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    run_sql << "seed TEXT NOT NULL,"; //unsigned 64-bit ints are not supported, so store as text
    run_sql << "pop_size INTEGER NOT NULL,";
    run_sql << "ga_steps INTEGER NOT NULL,";
    run_sql << "reg_steps INTEGER NOT NULL,";
    run_sql << "mut_prob REAL NOT NULL,";
    run_sql << "mut_prob_limit REAL NOT NULL,";
    run_sql << "mut_step REAL NOT NULL,";
    run_sql << "cross_frac REAL NOT NULL,";
    run_sql << "cross_frac_limit REAL NOT NULL,";
    run_sql << "cross_step REAL NOT NULL,";
    run_sql << "num_genes INTEGER NOT NULL,";
    run_sql << "gene_bits INTEGER NOT NULL,";
    run_sql << "min_protein_conc REAL NOT NULL,";
    run_sql << "max_protein_conc REAL NOT NULL,";
    run_sql << "decay_rate REAL NOT NULL,";
    run_sql << "initial_proteins INTEGER NOT NULL,";
    run_sql << "max_proteins INTEGER NOT NULL,";
    run_sql << "max_pgm_size INTEGER NOT NULL,";
    run_sql << "max_mut_float REAL NOT NULL,";
    run_sql << "max_mut_bits INTEGER NOT NULL,";
    run_sql << "fitness_log_interval INTEGER NOT NULL,";
    run_sql << "binding_seq_play INTEGER NOT NULL,";
    run_sql << "graph_results INTEGER NOT NULL,";
    run_sql << "log_grns INTEGER NOT NULL,";
    run_sql << "log_reg_steps INTEGER NOT NULL,";
    run_sql << "log_code_with_fitness INTEGER NOT NULL,";
    run_sql << "growth_start INTEGER NOT NULL,";
    run_sql << "growth_end INTEGER NOT NULL,";
    run_sql << "growth_sample_interval INTEGER NOT NULL,";
    run_sql << "growth_threshold INT NOT NULL,";
    run_sql << "term_cutoff FLOAT NOT NULL,";
    run_sql << "code_start INTEGER NOT NULL,";
    run_sql << "code_end INTEGER NOT NULL,";
    run_sql << "code_sample_interval INTEGER NOT NULL,";
    run_sql << "fix_rng_seed INTEGER NOT NULL,";
    run_sql << "log_mode TEXT NOT NULL";
    run_sql << ");";
    sqlite3_exec(this->conn, run_sql.str().c_str(), NULL, NULL, NULL);

    //fitness
    stringstream fit_sql;
    fit_sql << "CREATE TABLE fitness (";
    fit_sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    fit_sql << "fitness REAL NOT NULL,";
    fit_sql << "pop_index INTEGER NOT NULL,";
    fit_sql << "ga_step INTEGER NOT NULL,";
    fit_sql << "code TEXT NULL,";
    fit_sql << "str TEXT NULL";
    fit_sql << ");";
    sqlite3_exec(this->conn, fit_sql.str().c_str(), NULL, NULL, NULL);

    if (this->run->log_grns) {
        //grn
        stringstream grn_sql;
        grn_sql << "CREATE TABLE grn (";
        grn_sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
        grn_sql << "ga_step INTEGER NOT NULL,";
        grn_sql << "pop_index INTEGER NOT NULL,";
        grn_sql << "growth_seq TEXT NOT NULL,";
        grn_sql << "split_pt INTEGER NULL";
        grn_sql << ");";
        sqlite3_exec(this->conn, grn_sql.str().c_str(), NULL, NULL, NULL);

        //create indices
        stringstream grn_index_sql;
        grn_index_sql << "CREATE INDEX index0 ON grn(ga_step);";
        sqlite3_exec(this->conn, grn_index_sql.str().c_str(), NULL, NULL, NULL);

        stringstream grn_index2_sql;
        grn_index2_sql << "CREATE UNIQUE INDEX index1 ON grn(ga_step, pop_index);";
        sqlite3_exec(this->conn, grn_index2_sql.str().c_str(), NULL, NULL, NULL);

        //gene
        stringstream gene_sql;
        gene_sql << "CREATE TABLE gene (";
        gene_sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
        gene_sql << "binding_seq TEXT NOT NULL,"; //not the most space-efficient storage method, but it's convenient and works for now
        gene_sql << "output_seq TEXT NOT NULL,";
        gene_sql << "threshold REAL NOT NULL,";
        gene_sql << "output_rate REAL NOT NULL,";
        gene_sql << "pos INTEGER NOT NULL,";
        gene_sql << "grn_id INTEGER NOT NULL,";
        gene_sql << "FOREIGN KEY(grn_id) REFERENCES grn(id)";
        gene_sql << ");";
        sqlite3_exec(this->conn, gene_sql.str().c_str(), NULL, NULL, NULL);

        //create index
        stringstream gene_index_sql;
        gene_index_sql << "CREATE UNIQUE INDEX index0 ON gene(grn_id, pos);";
        sqlite3_exec(this->conn, gene_index_sql.str().c_str(), NULL, NULL, NULL);
    }

    //note: must log_grns in order to log_reg_steps
    if (this->run->log_grns && this->run->log_reg_steps) {
        //gene_state
        stringstream gs_sql;
        gs_sql << "CREATE TABLE gene_state (";
        gs_sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
        gs_sql << "reg_step INTEGER NOT NULL,";
        gs_sql << "active_output INTEGER NULL,";
        gs_sql << "bound_protein INTEGER NULL,";
        gs_sql << "gene_id INTEGER,";
        gs_sql << "FOREIGN KEY(active_output) REFERENCES protein(id),"; // note: this and the line below referenceconn id*, not the *pid* (i.e. they do not reference the simulation protein id as their correspondingly-named data members in the Protein class do)
        gs_sql << "FOREIGN KEY(bound_protein) REFERENCES protein(id),";
        gs_sql << "FOREIGN KEY(gene_id) REFERENCES gene(id)";
        gs_sql << ");";
        sqlite3_exec(this->conn, gs_sql.str().c_str(), NULL, NULL, NULL);

        //protein
        stringstream protein_sql;
        protein_sql << "CREATE TABLE protein (";
        protein_sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,"; //database id
        protein_sql << "pid INTEGER NOT NULL,"; //protein id from simulation
        protein_sql << "seq TEXT NOT NULL,";
        protein_sql << "src_pos INTEGER NOT NULL,";
        protein_sql << "grn_id INTEGER NOT NULL,";
        protein_sql << "FOREIGN KEY(grn_id) REFERENCES grn(id)";
        protein_sql << ");";
        sqlite3_exec(this->conn, protein_sql.str().c_str(), NULL, NULL, NULL);

        //create index
        stringstream protein_index_sql;
        protein_index_sql << "CREATE UNIQUE INDEX index0 ON protein(grn_id, pid);";
        sqlite3_exec(this->conn, protein_index_sql.str().c_str(), NULL, NULL, NULL);

        //protein_state
        stringstream ps_sql;
        ps_sql << "CREATE TABLE protein_state (";
        ps_sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
        ps_sql << "reg_step INTEGER NOT NULL,";
        ps_sql << "interactions INTEGER NOT NULL,";

        //conc columns
        for (int i = 0; i < this->run->num_genes; i++) {
            ps_sql << "conc" << i << " REAL NOT NULL, ";
        }
    
        ps_sql << "protein_id INTEGER NOT NULL,";
        ps_sql << "grn_id INTEGER NOT NULL,";
        ps_sql << "FOREIGN KEY(protein_id) REFERENCES protein(id),";
        ps_sql << "FOREIGN KEY(grn_id) REFERENCES grn(id)";
        ps_sql << ");";
        sqlite3_exec(this->conn, ps_sql.str().c_str(), NULL, NULL, NULL);

        //tree
        stringstream tree_sql;
        tree_sql << "CREATE TABLE tree (";
        tree_sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
        tree_sql << "ptype_state_id INTEGER NOT NULL,";
        tree_sql << "FOREIGN KEY(ptype_state_id) REFERENCES ptype_state(id)";
        tree_sql << ");";
        sqlite3_exec(this->conn, tree_sql.str().c_str(), NULL, NULL, NULL);

        //node
        stringstream node_sql;
        node_sql << "CREATE TABLE node (";
        node_sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
        node_sql << "tree_id INTEGER NOT NULL,";
        node_sql << "parent_id INTEGER NULL,";
        node_sql << "descr TEXT NULL,";
        node_sql << "FOREIGN KEY(tree_id) REFERENCES tree(id),";
        node_sql << "FOREIGN KEY(parent_id) REFERENCES node(id)";
        node_sql << ");";
        sqlite3_exec(this->conn, node_sql.str().c_str(), NULL, NULL, NULL);
        
        //ptype_state (phenotype)
        stringstream ptype_sql;
        ptype_sql << "CREATE TABLE ptype_state (";
        ptype_sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
        ptype_sql << "grn_id INTEGER NOT NULL,";
        ptype_sql << "reg_step INTEGER NOT NULL,";
        ptype_sql << "tree_str TEXT NULL,";
        ptype_sql << "size INTEGER NOT NULL,";
        ptype_sql << "height INTEGER NOT NULL,";
        ptype_sql << "filled_nodes NOT NULL,";
        ptype_sql << "FOREIGN KEY(grn_id) REFERENCES grn(id)";
        ptype_sql << ");";
        sqlite3_exec(this->conn, ptype_sql.str().c_str(), NULL, NULL, NULL);
    }
    sqlite3_exec(this->conn, "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
}

void Logger::log_run() {
    int rc;
    string run_sql = "INSERT INTO run (seed, pop_size, ga_steps, reg_steps, mut_prob, mut_prob_limit, mut_step, cross_frac, cross_frac_limit, cross_step, num_genes, gene_bits, min_protein_conc, max_protein_conc, decay_rate, initial_proteins, max_proteins, max_pgm_size, max_mut_float, max_mut_bits, fitness_log_interval, binding_seq_play, graph_results, log_grns, log_reg_steps, log_code_with_fitness, growth_start, growth_end, growth_sample_interval, growth_threshold, term_cutoff, code_start, code_end, code_sample_interval, fix_rng_seed, log_mode) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *run_stmt;
    sqlite3_prepare_v2(this->conn, run_sql.c_str(), run_sql.size() + 1, &run_stmt, NULL);

    int bind_index = 1;
    stringstream seed_buf;
    seed_buf << this->run->rand->seed;
    string seed_str = seed_buf.str();
    sqlite3_bind_text(run_stmt, bind_index++, seed_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->pop_size);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->ga_steps);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->reg_steps);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->mut_prob);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->mut_prob_limit);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->mut_step);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->cross_frac);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->cross_frac_limit);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->cross_step);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->num_genes);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->gene_bits);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->min_protein_conc);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->max_protein_conc);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->decay_rate);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->initial_proteins);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->max_proteins);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->max_pgm_size);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->max_mut_float);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->max_mut_bits);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->fitness_log_interval);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->binding_seq_play);

    sqlite3_bind_int(run_stmt, bind_index++, (int) this->run->graph_results);
    sqlite3_bind_int(run_stmt, bind_index++, (int) this->run->log_grns);
    sqlite3_bind_int(run_stmt, bind_index++, (int) this->run->log_reg_steps);
    sqlite3_bind_int(run_stmt, bind_index++, (int) this->run->log_code_with_fitness);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->growth_start);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->growth_end);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->growth_sample_interval);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->growth_threshold);
    sqlite3_bind_double(run_stmt, bind_index++, (double) this->run->term_cutoff);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->code_start);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->code_end);
    sqlite3_bind_int(run_stmt, bind_index++, this->run->code_sample_interval);
    sqlite3_bind_int(run_stmt, bind_index++, (int) this->run->fix_rng_seed);
    sqlite3_bind_text(run_stmt, bind_index++, this->run->log_mode.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_exec(this->conn, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    rc = sqlite3_step(run_stmt);
    sqlite3_exec(this->conn, "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_DONE) {
        cerr << "Error inserting run." << endl;
        exit(1);
    }

    sqlite3_finalize(run_stmt);
}

void Logger::log_fitnesses(int ga_step, vector<Grn*> *pop, vector<Phenotype*> *phenotypes, vector<float> *fitnesses) {
    //note: ga_step may be < 0 when logging initial fitnesses
    if (this->should_sample(ga_step)) {
        int rc;
        string fitness_sql = "INSERT INTO fitness (fitness, pop_index, ga_step, code, str) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt *fitness_stmt;
        sqlite3_prepare_v2(this->conn, fitness_sql.c_str(), fitness_sql.size() + 1, &fitness_stmt, NULL);

        this->gen_avg_fitness = 0.0f;
        int bind_index;

        sqlite3_exec(this->conn, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
        for (int i = 0; i < this->run->pop_size; i++) {
            float fitness = (*fitnesses)[i];
            this->gen_avg_fitness += fitness;

            if (ga_step == -1 || (fitness < this->grn_bests.get_gen_best_fitness())) {
                Grn *best_grn = new Grn((*pop)[i]); //create a copy
                this->grn_bests.update_gen_best(best_grn, i, fitness);
                Phenotype *best_ptype = new Phenotype((*phenotypes)[i]); //create a copy
                this->ptype_bests.update_gen_best(best_ptype, i, fitness);

                this->grn_bests.update_run_best(best_grn, i, fitness, ga_step);
                this->ptype_bests.update_run_best(best_ptype, i, fitness, ga_step);
            }

            bind_index = 1;
            sqlite3_bind_double(fitness_stmt, bind_index++, (double) (*fitnesses)[i]);
            sqlite3_bind_int(fitness_stmt, bind_index++, i);
            sqlite3_bind_int(fitness_stmt, bind_index++, ga_step);

            if (this->run->log_code_with_fitness) {
                Phenotype *ptype = (*phenotypes)[i];
                string code = ptype->to_code();
                if (code.size() > 0) {
                    sqlite3_bind_text(fitness_stmt, bind_index++, code.c_str(), -1, SQLITE_TRANSIENT);
                }
                else {
                    sqlite3_bind_null(fitness_stmt, bind_index++);
                }
                string str = ptype->to_str();
                if (str.size() > 0) {
                    sqlite3_bind_text(fitness_stmt, bind_index++, str.c_str(), -1, SQLITE_TRANSIENT);
                }
                else {
                    sqlite3_bind_null(fitness_stmt, bind_index++);
                }
            }
            else {
                sqlite3_bind_null(fitness_stmt, bind_index++);
                sqlite3_bind_null(fitness_stmt, bind_index++);
            }
            
            rc = sqlite3_step(fitness_stmt);
            if (rc != SQLITE_DONE) {
                cerr << "Error inserting fitness. Error code: " << rc << endl;
                cerr << "Data to insert: " << (*fitnesses)[i] << ", " << i << ", " << ga_step << endl;
                exit(1);
            }
            sqlite3_reset(fitness_stmt);
            sqlite3_clear_bindings(fitness_stmt);
        }
        this->grn_bests.gen_done();
        this->ptype_bests.gen_done();

        sqlite3_exec(this->conn, "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
        
        this->gen_avg_fitness /= (float) this->run->pop_size;

        sqlite3_finalize(fitness_stmt);

        //print status
        if (ga_step < 0) {
            cout << "********" << endl;
            cout << "initial:" << endl;
            cout << "********" << endl;
            cout << "seed: " << this->run->rand->seed << endl;
        }
        else {
            //we're going to be looking at this output a lot, so we might as well make it pretty...
            string msg = "iteration ";
            stringstream div;
            for (int i = 0; i < (int) msg.length() + (int) log10(ga_step + 1) + 1 + 1; i++) { //last +1 is for the ":" below
                div << "*";
            }
            cout << div.str() << endl;
            cout << msg << ga_step + 1 << ":" << endl;
            cout << div.str() << endl;
        }
        cout << "mut_prob: " << this->run->mut_prob << endl;
        cout << "cross_frac: " << this->run->cross_frac << endl;
        cout << endl;
        cout << "grn:" << endl;
        cout << "gen best fitness: " << this->grn_bests.get_gen_best_fitness() << endl;
        cout << "gen avg fitness: " << this->gen_avg_fitness << endl;
        cout << "run-best fitness: " << this->grn_bests.get_run_best_fitness() << endl;
        cout << "run-best pop_index: " << this->grn_bests.get_run_best_index() << endl;
        cout << endl;
        cout << "phenotype:" << endl;
        cout << "size: " << this->ptype_bests.get_run_best()->size() << endl;
        cout << "filled_nodes: " << this->ptype_bests.get_run_best()->get_num_filled_nodes() << endl;
        cout << "unfilled_nodes: " << this->ptype_bests.get_run_best()->get_num_unfilled_nodes() << endl;
        cout << this->ptype_bests.get_run_best()->to_code() << endl;
        
        cout << endl;
        cout.flush();
    }
}

float Logger::get_fitness_val(int ga_step, string *sql_fcn) {
    string sql = "SELECT " + *sql_fcn + "(fitness) FROM fitness WHERE ga_step = ?;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(this->conn, sql.c_str(), sql.size() + 1, &stmt, NULL);

    int bind_index = 1;
    sqlite3_bind_int(stmt, bind_index++, ga_step);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        cerr << "Error selecting fitness: " << rc << endl;
        exit(1);
    }

    float result = (float) sqlite3_column_double(stmt, 0);
    
    sqlite3_finalize(stmt);

    return result;
}

float Logger::get_run_best_fitness() {
    return this->grn_bests.get_run_best_fitness();
}

float Logger::get_gen_avg_fitness() {
    return this->gen_avg_fitness;
}

bool Logger::should_sample(int ga_step) {
    return ga_step <= 0 || ga_step == this->run->ga_steps - 1 || (ga_step + 1) % this->run->fitness_log_interval == 0;
}

void Logger::log_ga_step(int ga_step, vector<Grn*> *grns, BestInfo<RegSnapshot*> *bests) {
    if (this->run->log_grns) {
        if (this->run->log_mode == "all" &&
            this->should_sample(ga_step)) {
            this->log_ga_step(ga_step, grns, 0);
        }
    }
}

void Logger::log_split_pt(int ga_step, int pop_index, int split_pt) {
    sqlite3_exec(this->conn, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    
    string sql = "UPDATE grn SET split_pt = ? WHERE ga_step = ? AND pop_index = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(this->conn, sql.c_str(), sql.size() + 1, &stmt, NULL);

    int bind_index = 1;
    sqlite3_bind_int(stmt, bind_index++, split_pt);
    sqlite3_bind_int(stmt, bind_index++, ga_step);
    sqlite3_bind_int(stmt, bind_index++, pop_index);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "Error updating grn with split_pt." << endl;
        cerr << sqlite3_errmsg(this->conn) << endl;
        exit(1);
    }

    sqlite3_finalize(stmt);

    sqlite3_exec(this->conn, "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
}

void Logger::log_ga_step(int ga_step, vector<Grn*> *grns, int pop_index_offset=0) {
    int rc;
    int bind_index;
    string grn_sql = "INSERT INTO grn (ga_step, pop_index, growth_seq) VALUES (?, ?, ?);";
    sqlite3_stmt *grn_stmt;
    sqlite3_prepare_v2(this->conn, grn_sql.c_str(), grn_sql.size() + 1, &grn_stmt, NULL);

    string gene_sql = "INSERT INTO gene (binding_seq, output_seq, threshold, output_rate, pos, grn_id) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *gene_stmt;
    sqlite3_prepare_v2(this->conn, gene_sql.c_str(), gene_sql.size() + 1, &gene_stmt, NULL);

    //insert grns
    sqlite3_exec(this->conn, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    Grn *grn;
    for (int i = 0; i < (int) grns->size(); i++) {
        grn = (*grns)[i];
        bind_index = 1;
        sqlite3_bind_int(grn_stmt, bind_index++, ga_step);
        sqlite3_bind_int(grn_stmt, bind_index++, pop_index_offset + i);
        sqlite3_bind_text(grn_stmt, bind_index++, grn->growth_seq->to_str().c_str(), -1, SQLITE_TRANSIENT);

        rc = sqlite3_step(grn_stmt);
        if (rc != SQLITE_DONE) {
            cerr << "Error inserting grn: " << rc << endl;
            exit(1);
        }
        int grn_id = sqlite3_last_insert_rowid(this->conn);

        //insert genes
        Gene *gene;
        for (int j = 0; j < this->run->num_genes; j++) {
            gene = grn->genes[j];

            bind_index = 1;
            string binding_str = gene->binding_seq->to_str();
            sqlite3_bind_text(gene_stmt, bind_index++, binding_str.c_str(), -1, SQLITE_TRANSIENT);

            string output_str = gene->output_seq->to_str();
            sqlite3_bind_text(gene_stmt, bind_index++, output_str.c_str(), -1, SQLITE_TRANSIENT);

            sqlite3_bind_double(gene_stmt, bind_index++, (double) gene->threshold);
            sqlite3_bind_double(gene_stmt, bind_index++, (double) gene->output_rate);
            sqlite3_bind_int(gene_stmt, bind_index++, gene->pos);
            sqlite3_bind_int(gene_stmt, bind_index++, grn_id);
            rc = sqlite3_step(gene_stmt);
            if (rc != SQLITE_DONE) {
                cerr << "Error inserting gene." << endl;
                exit(1);
            }
            sqlite3_reset(gene_stmt);
            sqlite3_clear_bindings(gene_stmt);
        }

        sqlite3_reset(grn_stmt);
        sqlite3_clear_bindings(grn_stmt);
    }
    sqlite3_exec(this->conn, "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);

    sqlite3_finalize(grn_stmt);
    sqlite3_finalize(gene_stmt);
}

void Logger::log_reg_snapshot(RegSnapshot *snappy) {
    for (int i = -1; i < this->run->reg_steps; i++) {
        Grn *grn = snappy->grns[i];
        Phenotype *ptype = snappy->ptypes[i];
        this->log_reg_step(snappy->ga_step, i, grn, snappy->pop_index, ptype);
    }
}

void Logger::log_reg_step(int ga_step, int reg_step, Grn *grn, int pop_index, Phenotype *ptype) {
    //note: must log_grns in order to log_reg_steps
    if (this->run->log_grns && this->run->log_reg_steps) {
        if (this->should_sample(ga_step)) {
            int rc;
    
            //the grn should already be in the database. Get it's id.
            string grn_sql = "SELECT id FROM grn WHERE ga_step = ? AND pop_index = ?;";
            sqlite3_stmt *grn_stmt;
            rc = sqlite3_prepare_v2(this->conn, grn_sql.c_str(), grn_sql.size() + 1, &grn_stmt, NULL);
    
            sqlite3_bind_int(grn_stmt, 1, ga_step);
            sqlite3_bind_int(grn_stmt, 2, pop_index);
            rc = sqlite3_step(grn_stmt);
            int grn_id = -1;
            if (rc == SQLITE_ERROR || rc == SQLITE_MISUSE) {
                cerr << "Error selecting grn. ga_step: " << ga_step << ", pop_index: " << pop_index << endl;
                cerr << sqlite3_errmsg(this->conn) << endl;
                exit(1);
            }
            else if (rc == SQLITE_DONE) {
                //The logging mode is "best" and the grn has not been inserted yet, so insert it
                vector<Grn*> temp;
                temp.push_back(grn);
                this->log_ga_step(ga_step, &temp, pop_index);
                sqlite3_reset(grn_stmt);
                rc = sqlite3_step(grn_stmt);
            }
            
            if (rc == SQLITE_ROW) {
                grn_id = sqlite3_column_int(grn_stmt, 0);
            }
            else {
                cerr << "Error selecting grn. ga_step: " << ga_step << ", pop_index: " << pop_index << endl;
                cerr << sqlite3_errmsg(this->conn) << endl;
                exit(1);
            }
            sqlite3_finalize(grn_stmt);

            //insert proteins
            string protein_sel_sql = "SELECT id FROM protein WHERE grn_id = ? AND pid = ?;";
            sqlite3_stmt *protein_sel_stmt;
            sqlite3_prepare_v2(this->conn, protein_sel_sql.c_str(), protein_sel_sql.size() + 1, &protein_sel_stmt, NULL);

            string protein_ins_sql = "INSERT INTO protein (pid, seq, src_pos, grn_id) VALUES (?, ?, ?, ?)";
            sqlite3_stmt *protein_ins_stmt;
            sqlite3_prepare_v2(this->conn, protein_ins_sql.c_str(), protein_ins_sql.size() + 1, &protein_ins_stmt, NULL);

            stringstream pstate_sql;
            stringstream pstate_vals;
            pstate_sql << "INSERT INTO protein_state (reg_step, interactions, ";
            pstate_vals << "?, ?";
            for (int i = 0; i < this->run->num_genes; i++) {
                pstate_sql << "conc" << i;
                pstate_vals << ", ?";
                if (i < this->run->num_genes - 1) {
                    pstate_sql << ", ";
                }
            }
            pstate_sql << ", protein_id, grn_id) VALUES (" << pstate_vals.str() << ", ?, ?" << ");";
            string pstate_final_sql = pstate_sql.str();
            sqlite3_stmt *pstate_stmt;
            sqlite3_prepare_v2(this->conn, pstate_final_sql.c_str(), pstate_final_sql.size() + 1, &pstate_stmt, NULL);

            //go through all proteins in the grn's store. Some of the proteins may have been inserted on previous iterations.
            //In these cases, we only need to insert a new protein_state record
            ProteinStore *store = grn->proteins;
            Protein *protein;
            int bind_index;

            sqlite3_exec(this->conn, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
            for (int pid : *store) {
                int protein_id = -1; //db id
                protein = store->get(pid);
    
                //first, check if protein has already been inserted on a previous iteration and, if so, get its database id
                bool inserted = false;
                bind_index = 1;
                sqlite3_bind_int(protein_sel_stmt, bind_index++, grn_id);
                sqlite3_bind_int(protein_sel_stmt, bind_index++, pid);
                rc = sqlite3_step(protein_sel_stmt);
                if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
                    cerr << "Error selecting protein (1): " << rc << endl;
                    exit(1);
                }
                if (rc == SQLITE_ROW) {
                    inserted = true;
                    protein_id = sqlite3_column_int(protein_sel_stmt, 0); //db id
                }

                sqlite3_reset(protein_sel_stmt);
                sqlite3_clear_bindings(protein_sel_stmt);

                //otherwise, insert it and retreive the resulting id
                if (!inserted) {
                    bind_index = 1;
                    sqlite3_bind_int(protein_ins_stmt, bind_index++, pid);

                    string seq_str = protein->seq->to_str();
                    sqlite3_bind_text(protein_ins_stmt, bind_index++, seq_str.c_str(), -1, SQLITE_TRANSIENT);

                    sqlite3_bind_int(protein_ins_stmt, bind_index++, protein->src_pos);
                    sqlite3_bind_int(protein_ins_stmt, bind_index++, grn_id);

                    rc = sqlite3_step(protein_ins_stmt); 
                    if (rc != SQLITE_DONE) {
                        cerr << "Error inserting protein." << endl;
                        exit(1);
                    }
                    protein_id = sqlite3_last_insert_rowid(this->conn); //db id

                    sqlite3_reset(protein_ins_stmt);
                    sqlite3_clear_bindings(protein_ins_stmt);
                }

                //insert protein_state
                bind_index = 1;
                sqlite3_bind_int(pstate_stmt, bind_index++, reg_step);
                sqlite3_bind_int(pstate_stmt, bind_index++, protein->interactions);
                for (int i = 0; i < this->run->num_genes; i++) {
                    sqlite3_bind_double(pstate_stmt, bind_index++, (double) protein->concs[i]);
                }
                sqlite3_bind_int(pstate_stmt, bind_index++, protein_id);
                sqlite3_bind_int(pstate_stmt, bind_index++, grn_id);

                if ((rc = sqlite3_step(pstate_stmt)) != SQLITE_DONE) {
                    cerr << "Error inserting protein_state: " << rc << endl;
                    cerr << sqlite3_errmsg(this->conn) << endl;
                    exit(1);
                }

                sqlite3_reset(pstate_stmt);
                sqlite3_clear_bindings(pstate_stmt);
            }

            //insert gene_state
            string gstate_ins_sql = "INSERT INTO gene_state (reg_step, active_output, bound_protein, gene_id) VALUES (?, ?, ?, ?);";
            sqlite3_stmt *gstate_ins_stmt;
            sqlite3_prepare_v2(this->conn, gstate_ins_sql.c_str(), gstate_ins_sql.size() + 1, &gstate_ins_stmt, NULL);

            string gstate_selp_sql = "SELECT id FROM protein WHERE pid = ? AND grn_id = ?;";
            sqlite3_stmt *gstate_selp_stmt;
            sqlite3_prepare_v2(this->conn, gstate_selp_sql.c_str(), gstate_selp_sql.size() + 1, &gstate_selp_stmt, NULL);

            string gstate_selg_sql = "SELECT id FROM gene WHERE grn_id = ? AND pos = ?";
            sqlite3_stmt *gstate_selg_stmt;
            sqlite3_prepare_v2(this->conn, gstate_selg_sql.c_str(), gstate_selg_sql.size() + 1, &gstate_selg_stmt, NULL);

            Gene *gene;
            for (int i = 0; i < this->run->num_genes; i++) {
                gene = grn->genes[i];

                //get the db id of the gene (should have already been inserted)
                bind_index = 1;
                sqlite3_bind_int(gstate_selg_stmt, bind_index++, grn_id);
                sqlite3_bind_int(gstate_selg_stmt, bind_index++, i);

                if (sqlite3_step(gstate_selg_stmt) != SQLITE_ROW) {
                    cerr << "Error selecting gene. grn_id: " << grn_id << ", pos: " << i << endl;
                    exit(1);
                }

                int gene_id = sqlite3_column_int(gstate_selg_stmt, 0);

                sqlite3_reset(gstate_selg_stmt);
                sqlite3_clear_bindings(gstate_selg_stmt);

                //get the db id of the active_output protein (if any)
                int active_output_id = -1;
                if (gene->active_output >= 0) {
                    bind_index = 1;
                    sqlite3_bind_int(gstate_selp_stmt, bind_index++, gene->active_output);
                    sqlite3_bind_int(gstate_selp_stmt, bind_index++, grn_id);
                    sqlite3_bind_int(gstate_selp_stmt, bind_index++, gene->pos);

                    if (sqlite3_step(gstate_selp_stmt) != SQLITE_ROW) {
                        cerr << "Error selecting protein (2)." << endl;
                        cerr << "grn_id: " << grn_id << endl;
                        cerr << "gene index: " << i << endl;
                        cerr << "active_output: " << gene->active_output << endl;
                        sqlite3_exec(this->conn, "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
                        this->write_db();
                        exit(1);
                    }

                    active_output_id = sqlite3_column_int(gstate_selp_stmt, 0);

                    sqlite3_reset(gstate_selp_stmt);
                    sqlite3_clear_bindings(gstate_selp_stmt);
                }

                //get the db id of the bound_protein (if any)
                int bound_protein_id = -1;
                if (gene->bound_protein >= 0) {
                    bind_index = 1;
                    sqlite3_bind_int(gstate_selp_stmt, bind_index++, gene->bound_protein);
                    sqlite3_bind_int(gstate_selp_stmt, bind_index++, grn_id);

                    if ((rc = sqlite3_step(gstate_selp_stmt)) != SQLITE_ROW) {
                        cerr << "Error selecting protein (3). rc: " << rc << ", pid: " << gene->bound_protein << ", grn_id: " << grn_id << endl;
                        exit(1);
                    }

                    bound_protein_id = sqlite3_column_int(gstate_selp_stmt, 0);

                    sqlite3_reset(gstate_selp_stmt);
                    sqlite3_clear_bindings(gstate_selp_stmt);
                }

                bind_index = 1;
                sqlite3_bind_int(gstate_ins_stmt, bind_index++, reg_step);
                if (active_output_id >= 0) {
                    sqlite3_bind_int(gstate_ins_stmt, bind_index++, active_output_id);
                }
                else {
                    sqlite3_bind_null(gstate_ins_stmt, bind_index++);
                }

                if (bound_protein_id >= 0) {
                    sqlite3_bind_int(gstate_ins_stmt, bind_index++, bound_protein_id);
                }
                else {
                    sqlite3_bind_null(gstate_ins_stmt, bind_index++);
                }
                sqlite3_bind_int(gstate_ins_stmt, bind_index++, gene_id);

                if (sqlite3_step(gstate_ins_stmt) != SQLITE_DONE) {
                    cerr << "Error inserting gene_state." << endl;
                    exit(1);
                }
        
                sqlite3_reset(gstate_ins_stmt);
                sqlite3_clear_bindings(gstate_ins_stmt);
            }

            //insert phenotype state
            sqlite3_stmt *ptype_stmt;
            string ptype_sql = "INSERT INTO ptype_state (grn_id, reg_step, tree_str, size, height, filled_nodes) VALUES (?, ?, ?, ?, ?, ?);";
            sqlite3_prepare_v2(this->conn, ptype_sql.c_str(), ptype_sql.size() + 1, &ptype_stmt, NULL);
            
            bind_index = 1;
            sqlite3_bind_int(ptype_stmt, bind_index++, grn_id);
            sqlite3_bind_int(ptype_stmt, bind_index++, reg_step);
            string tree_str = ptype->to_str();
            if (tree_str == "") {
                sqlite3_bind_null(ptype_stmt, bind_index++);
            }
            else {
                sqlite3_bind_text(ptype_stmt, bind_index++, tree_str.c_str(), -1, SQLITE_TRANSIENT);
            }
            sqlite3_bind_int(ptype_stmt, bind_index++, ptype->size());
            sqlite3_bind_int(ptype_stmt, bind_index++, ptype->height());
            sqlite3_bind_int(ptype_stmt, bind_index++, ptype->get_num_filled_nodes());

            rc = sqlite3_step(ptype_stmt);
            if (rc != SQLITE_DONE) {
                cerr << "Error inserting phenotype state: " << rc << endl;
                cerr << sqlite3_errmsg(this->conn) << endl;
                exit(1);
            }

            //insert tree
            if (ptype->size() > 0) {
                int ptype_id = sqlite3_last_insert_rowid(this->conn);
                sqlite3_stmt *tree_stmt;
                string tree_sql = "INSERT INTO tree (ptype_state_id) VALUES (?);";
                sqlite3_prepare_v2(this->conn, tree_sql.c_str(), tree_sql.size() + 1, &tree_stmt, NULL);
                bind_index = 1;
                sqlite3_bind_int(tree_stmt, bind_index++, ptype_id);
                if ((rc = sqlite3_step(tree_stmt)) != SQLITE_DONE) {
                    cerr << "Error inserting tree: " << rc << endl;
                    cerr << sqlite3_errmsg(this->conn) << endl;
                    exit(1);
                }
                int tree_id = sqlite3_last_insert_rowid(this->conn);

                //insert nodes
                Node *root = ptype->tree->get_node(0);
                if (root != nullptr) {
                    this->insert_node(ptype->tree, tree_id, root, -1);
                }
                sqlite3_finalize(tree_stmt);
            }

            
            sqlite3_exec(this->conn, "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
    
            sqlite3_finalize(protein_sel_stmt);
            sqlite3_finalize(protein_ins_stmt);
            sqlite3_finalize(pstate_stmt);
            sqlite3_finalize(gstate_ins_stmt);
            sqlite3_finalize(gstate_selp_stmt);
            sqlite3_finalize(gstate_selg_stmt);
            sqlite3_finalize(ptype_stmt);
        }
    }
}

void Logger::insert_node(Tree *tree, int tree_id, Node *cur, int parent_id) {
    sqlite3_stmt *stmt;
    string sql = "INSERT INTO node (tree_id, parent_id, descr) VALUES (?, ?, ?);";
    sqlite3_prepare_v2(this->conn, sql.c_str(), sql.size() + 1, &stmt, NULL);
    int bind_index = 1;
    sqlite3_bind_int(stmt, bind_index++, tree_id);
    if (parent_id == -1) {
        sqlite3_bind_null(stmt, bind_index++);
    }
    else {
        sqlite3_bind_int(stmt, bind_index++, parent_id);
    }

    if (cur->instr == nullptr) {
        sqlite3_bind_null(stmt, bind_index++);
    }
    else {
        string desc = cur->instr->to_str();
        sqlite3_bind_text(stmt, bind_index++, desc.c_str(), -1, SQLITE_TRANSIENT);
    }

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "Error inserting node: " << rc << endl;
        cerr << sqlite3_errmsg(this->conn) << endl;
        exit(1);
    }

    sqlite3_finalize(stmt);

    int new_parent_id = sqlite3_last_insert_rowid(this->conn);
    Node *child;
    for (int child_id : cur->children) {
        child = tree->get_node(child_id);
        this->insert_node(tree, tree_id, child, new_parent_id);
    }
}
