#include "logger.hpp"

#include <iostream>
#include "constants.hpp"
#include <sstream>
#include <boost/dynamic_bitset.hpp>
#include "gene.hpp"
#include "protein.hpp"
#include "protein_store.hpp"

Logger::Logger(Run *run) {
    this->run = run;
    
    //we'll log to an in-memory database during the simulation (since it's an order of magnitude faster than a disk-based db)
    //Then, when the simulation ends, the user should call write_db() to write it out to disk in one shot
    int rc = sqlite3_open(":memory:", &this->db);
    if (rc) {
        cerr << "Cannot create in-memory database" << endl;
    }

    this->create_tables();
}

//write the in-memory database to disk
void Logger::write_db() {
    sqlite3 *disk_db;
    int rc = sqlite3_open(LOG_DB.c_str(), &disk_db);
    if (rc) {
        cerr << "Cannot create disk database file" << endl;
    }
    
    sqlite3_backup *backup = sqlite3_backup_init(disk_db, "main", this->db, "main");
    sqlite3_backup_step(backup, -1); //pass -1 to copy entire db
    sqlite3_backup_finish(backup); //cleans up resources allocated for copy operation
}

void Logger::create_tables() {
    //grn
    stringstream sql;
    sql << "CREATE TABLE grn (";
    sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql << "ga_step INTEGER NOT NULL,";
    sql << "pop_index INTEGER NOT NULL";
    sql << ");";
    sqlite3_exec(this->db, sql.str().c_str(), NULL, NULL, NULL);

    //fitness
    sql = stringstream();
    sql << "CREATE TABLE fitness (";
    sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql << "fitness REAL NOT NULL,";
    sql << "grn_id INTEGER NOT NULL,";
    sql << "FOREIGN KEY(grn_id) REFERENCES grn(id)";
    sql << ");";
    sqlite3_exec(this->db, sql.str().c_str(), NULL, NULL, NULL);

    //gene
    sql = stringstream();
    sql << "CREATE TABLE gene (";
    sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql << "binding_seq TEXT NOT NULL,"; //not the most space-efficient storage method, but it's convenient and works for now
    sql << "output_seq TEXT NOT NULL,";
    sql << "threshold REAL NOT NULL,";
    sql << "output_rate REAL NOT NULL,";
    sql << "kernel_index INTEGER NOT NULL,";
    sql << "pos INTEGER NOT NULL,";
    sql << "grn_id INTEGER NOT NULL,";
    sql << "FOREIGN KEY(grn_id) REFERENCES grn(id)";
    sql << ");";
    sqlite3_exec(this->db, sql.str().c_str(), NULL, NULL, NULL);

    //gene_state
    sql = stringstream();
    sql << "CREATE TABLE gene_state (";
    sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql << "reg_step INTEGER NOT NULL,";
    sql << "active_output INTEGER NULL,";
    sql << "bound_protein INTEGER NULL,";
    sql << "gene_id INTEGER,";
    sql << "FOREIGN KEY(active_output) REFERENCES protein(id),"; // note: this and the line below reference the *db id*, not the *pid* (i.e. they do not reference the simulation protein id as their correspondingly-named data members in the Protein class do)
    sql << "FOREIGN KEY(bound_protein) REFERENCES protein(id),";
    sql << "FOREIGN KEY(gene_id) REFERENCES gene(id)";
    sql << ");";
    sqlite3_exec(this->db, sql.str().c_str(), NULL, NULL, NULL);
    
    //protein
    sql = stringstream();
    sql << "CREATE TABLE protein (";
    sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,"; //database id
    sql << "pid INTEGER NOT NULL,"; //protein id from simulation
    sql << "seq TEXT NOT NULL,";
    sql << "kernel_index INTEGER NOT NULL,";
    sql << "src_pos INTEGER NOT NULL,";
    sql << "grn_id INTEGER NOT NULL,";
    sql << "FOREIGN KEY(grn_id) REFERENCES grn(id)";
    sql << ");";
    sqlite3_exec(this->db, sql.str().c_str(), NULL, NULL, NULL);

    //protein_state
    sql = stringstream();
    sql << "CREATE TABLE protein_state (";
    sql << "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql << "reg_step INTEGER NOT NULL,";

    //conc columns
    for (int i = 0; i < this->run->num_genes; i++) {
        sql << "conc" << i << " REAL NOT NULL, ";
    }
    
    sql << "protein_id INTEGER NOT NULL,";
    sql << "grn_id INTEGER NOT NULL,";
    sql << "FOREIGN KEY(protein_id) REFERENCES protein(id),";
    sql << "FOREIGN KEY(grn_id) REFERENCES grn(id)";
    sql << ");";
    sqlite3_exec(this->db, sql.str().c_str(), NULL, NULL, NULL);
}

Logger::~Logger() {
    sqlite3_close(this->db);
}

void Logger::log_fitnesses(int ga_step, vector<float> *fitnesses) {
    int rc;
    string grn_sel_sql = "SELECT id, pop_index FROM grn WHERE ga_step = ?;";
    sqlite3_stmt *grn_sel_stmt;
    sqlite3_prepare_v3(this->db, grn_sel_sql.c_str(), grn_sel_sql.size() + 1, 0, &grn_sel_stmt, NULL);

    string fitness_sql = "INSERT INTO fitness (fitness, grn_id) VALUES (?, ?);";
    sqlite3_stmt *fitness_stmt;
    sqlite3_prepare_v3(this->db, fitness_sql.c_str(), fitness_sql.size() + 1, SQLITE_PREPARE_PERSISTENT, &fitness_stmt, NULL);

    int bind_index = 1;
    sqlite3_bind_int(grn_sel_stmt, bind_index++, ga_step);
    
    while((rc = sqlite3_step(grn_sel_stmt)) == SQLITE_ROW) {
        int grn_id = sqlite3_column_int(grn_sel_stmt, 0);
        int pop_index = sqlite3_column_int(grn_sel_stmt, 1);

        bind_index = 1;
        sqlite3_bind_double(fitness_stmt, bind_index++, (double) (*fitnesses)[pop_index]);
        sqlite3_bind_int(fitness_stmt, bind_index++, grn_id);

        rc = sqlite3_step(fitness_stmt);
        if (rc != SQLITE_DONE) {
            cerr << "Error inserting fitness." << endl;
        }

        sqlite3_reset(fitness_stmt);
        sqlite3_clear_bindings(fitness_stmt);
    }
    if (rc != SQLITE_DONE) {
        cerr << "Error selecting grns." << endl;
    }

    sqlite3_finalize(grn_sel_stmt);
    sqlite3_finalize(fitness_stmt);
}

void Logger::log_ga_step(int ga_step, vector<Grn> *grns) {
    int rc;
    int bind_index;
    string grn_sql = "INSERT INTO grn (ga_step, pop_index) VALUES (?, ?);";
    sqlite3_stmt *grn_stmt;
    sqlite3_prepare_v3(this->db, grn_sql.c_str(), grn_sql.size() + 1, SQLITE_PREPARE_PERSISTENT, &grn_stmt, NULL);

    string gene_sql = "INSERT INTO gene (binding_seq, output_seq, threshold, output_rate, kernel_index, pos, grn_id) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *gene_stmt;
    sqlite3_prepare_v3(this->db, gene_sql.c_str(), gene_sql.size() + 1, SQLITE_PREPARE_PERSISTENT, &gene_stmt, NULL);

    //insert grns
    Grn *grn;
    for (int i = 0; i < (int) grns->size(); i++) {
        grn = &(*grns)[i];
        bind_index = 1;
        sqlite3_bind_int(grn_stmt, bind_index++, ga_step);
        sqlite3_bind_int(grn_stmt, bind_index++, i);

        rc = sqlite3_step(grn_stmt);
        if (rc != SQLITE_DONE) {
            cerr << "Error inserting grn: " << rc << endl;
        }
        int grn_id = sqlite3_last_insert_rowid(this->db);

        //insert genes
        Gene *gene;
        for (int j = 0; j < this->run->num_genes; j++) {
            gene = &grn->genes[j];

            bind_index = 1;
            string binding_str;
            boost::to_string(gene->binding_seq, binding_str);
            sqlite3_bind_text(gene_stmt, bind_index++, binding_str.c_str(), binding_str.size(), SQLITE_STATIC);

            string output_str;
            boost::to_string(gene->output_seq, output_str);
            sqlite3_bind_text(gene_stmt, bind_index++, output_str.c_str(), output_str.size(), SQLITE_STATIC);

            sqlite3_bind_double(gene_stmt, bind_index++, (double) gene->threshold);
            sqlite3_bind_double(gene_stmt, bind_index++, (double) gene->output_rate);
            sqlite3_bind_int(gene_stmt, bind_index++, gene->kernel_index);
            sqlite3_bind_int(gene_stmt, bind_index++, gene->pos);
            sqlite3_bind_int(gene_stmt, bind_index++, grn_id);
            if (sqlite3_step(gene_stmt) != SQLITE_DONE) {
                cerr << "Error inserting gene." << endl;
            }
            sqlite3_reset(gene_stmt);
            sqlite3_clear_bindings(gene_stmt);
        }

        sqlite3_reset(grn_stmt);
        sqlite3_clear_bindings(grn_stmt);
    }

    sqlite3_finalize(grn_stmt);
    sqlite3_finalize(gene_stmt);
}

void Logger::log_reg_step(int ga_step, int reg_step, Grn *grn, int pop_index) {
    int rc;
    
    //the grn should already be in the database. Get it's id.
    string grn_sql = "SELECT id FROM grn WHERE ga_step = ? AND pop_index = ?;";
    sqlite3_stmt *grn_stmt;
    rc = sqlite3_prepare_v3(this->db, grn_sql.c_str(), grn_sql.size() + 1, 0, &grn_stmt, NULL);
    
    sqlite3_bind_int(grn_stmt, 1, ga_step);
    sqlite3_bind_int(grn_stmt, 2, pop_index);
    if (sqlite3_step(grn_stmt) != SQLITE_ROW) {
        cerr << "Error selecting grn. ga_step: " << ga_step << ", pop_index: " << pop_index << endl;
    }
    int grn_id = sqlite3_column_int(grn_stmt, 0);
    sqlite3_finalize(grn_stmt);

    //insert proteins
    string protein_sel_sql = "SELECT id FROM protein WHERE grn_id = ? AND pid = ?;";
    sqlite3_stmt *protein_sel_stmt;
    sqlite3_prepare_v3(this->db, protein_sel_sql.c_str(), protein_sel_sql.size() + 1, SQLITE_PREPARE_PERSISTENT, &protein_sel_stmt, NULL);

    string protein_ins_sql = "INSERT INTO protein (pid, seq, kernel_index, src_pos, grn_id) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt *protein_ins_stmt;
    sqlite3_prepare_v3(this->db, protein_ins_sql.c_str(), protein_ins_sql.size() + 1, SQLITE_PREPARE_PERSISTENT, &protein_ins_stmt, NULL);

    stringstream pstate_sql;
    stringstream pstate_vals;
    pstate_sql << "INSERT INTO protein_state (reg_step, ";
    pstate_vals << "?";
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
    sqlite3_prepare_v3(this->db, pstate_final_sql.c_str(), pstate_final_sql.size() + 1, SQLITE_PREPARE_PERSISTENT, &pstate_stmt, NULL);

    //go through all proteins in the grn's store. Some of the proteins may have been inserted on previous iterations.
    //In these cases, we only need to insert a new protein_state record
    ProteinStore *store = &grn->proteins;
    Protein *protein;
    int bind_index;

    for (int pid : *store) {
        int protein_id; //db id
        protein = store->get(pid);
    
        //first, check if protein has already been inserted on a previous iteration and, if so, get its database id
        bool inserted = false;
        bind_index = 1;
        sqlite3_bind_int(protein_sel_stmt, bind_index++, grn_id);
        sqlite3_bind_int(protein_sel_stmt, bind_index++, pid);
        rc = sqlite3_step(protein_sel_stmt);
        if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
            cerr << "Error selecting protein (1)." << endl;
        }
        if (rc == SQLITE_ROW) {
            inserted = true;
            protein_id = sqlite3_column_int(protein_sel_stmt, 0);
        }

        sqlite3_reset(protein_sel_stmt);
        sqlite3_clear_bindings(protein_sel_stmt);

        //otherwise, insert it and retreive the resulting id
        if (!inserted) {
            bind_index = 1;
            sqlite3_bind_int(protein_ins_stmt, bind_index++, pid);

            string seq_str;
            boost::to_string(protein->seq, seq_str);
            sqlite3_bind_text(protein_ins_stmt, bind_index++, seq_str.c_str(), seq_str.size(), SQLITE_STATIC);

            sqlite3_bind_int(protein_ins_stmt, bind_index++, protein->kernel_index);
            sqlite3_bind_int(protein_ins_stmt, bind_index++, protein->src_pos);
            sqlite3_bind_int(protein_ins_stmt, bind_index++, grn_id);

            if (sqlite3_step(protein_ins_stmt) != SQLITE_DONE) {
                cerr << "Error inserting protein." << endl;
            }
            
            protein_id = sqlite3_last_insert_rowid(this->db);

            sqlite3_reset(protein_ins_stmt);
            sqlite3_clear_bindings(protein_ins_stmt);
        }

        //insert protein_state
        bind_index = 1;
        sqlite3_bind_int(pstate_stmt, bind_index++, reg_step);
        for (int i = 0; i < this->run->num_genes; i++) {
            sqlite3_bind_double(pstate_stmt, bind_index++, (double) protein->concs[i]);
        }
        sqlite3_bind_int(pstate_stmt, bind_index++, protein_id);
        sqlite3_bind_int(pstate_stmt, bind_index++, grn_id);

        if ((rc = sqlite3_step(pstate_stmt)) != SQLITE_DONE) {
            cerr << "Error inserting protein_state: " << rc << endl;
        }

        sqlite3_reset(pstate_stmt);
        sqlite3_clear_bindings(pstate_stmt);
    }

    //insert gene_state
    string gstate_ins_sql = "INSERT INTO gene_state (reg_step, active_output, bound_protein, gene_id) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *gstate_ins_stmt;
    sqlite3_prepare_v3(this->db, gstate_ins_sql.c_str(), gstate_ins_sql.size() + 1, 0, &gstate_ins_stmt, NULL);

    string gstate_selp_sql = "SELECT id FROM protein WHERE pid = ? AND grn_id = ?;";
    sqlite3_stmt *gstate_selp_stmt;
    sqlite3_prepare_v3(this->db, gstate_selp_sql.c_str(), gstate_selp_sql.size() + 1, 0, &gstate_selp_stmt, NULL);

    string gstate_selg_sql = "SELECT id FROM gene WHERE grn_id = ? AND pos = ?";
    sqlite3_stmt *gstate_selg_stmt;
    sqlite3_prepare_v3(this->db, gstate_selg_sql.c_str(), gstate_selg_sql.size() + 1, 0, &gstate_selg_stmt, NULL);

    Gene *gene;
    for (int i = 0; i < this->run->num_genes; i++) {
        gene = &grn->genes[i];

        //get the db id of the gene (should have already been inserted)
        bind_index = 1;
        sqlite3_bind_int(gstate_selg_stmt, bind_index++, grn_id);
        sqlite3_bind_int(gstate_selg_stmt, bind_index++, i);

        if (sqlite3_step(gstate_selg_stmt) != SQLITE_ROW) {
            cerr << "Error selecting gene." << endl;
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
        }
        
        sqlite3_reset(gstate_ins_stmt);
        sqlite3_clear_bindings(gstate_ins_stmt);
    }
    
    sqlite3_finalize(protein_sel_stmt);
    sqlite3_finalize(protein_ins_stmt);
    sqlite3_finalize(pstate_stmt);
    sqlite3_finalize(gstate_ins_stmt);
    sqlite3_finalize(gstate_selp_stmt);
    sqlite3_finalize(gstate_selg_stmt);
}
