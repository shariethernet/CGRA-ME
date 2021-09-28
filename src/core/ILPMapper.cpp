/*******************************************************************************
 * CGRA-ME Software End-User License Agreement
 *
 * The software programs comprising "CGRA-ME" and the documentation provided
 * with them are copyright by its authors S. Chin, K. Niu, N. Sakamoto, J. Zhao,
 * A. Rui, S. Yin, A. Mertens, J. Anderson, and the University of Toronto. Users
 * agree to not redistribute the software, in source or binary form, to other
 * persons or other institutions. Users may modify or use the source code for
 * other non-commercial, not-for-profit research endeavours, provided that all
 * copyright attribution on the source code is retained, and the original or
 * modified source code is not redistributed, in whole or in part, or included
 * in or with any commercial product, except by written agreement with the
 * authors, and full and complete attribution for use of the code is given in
 * any resulting publications.
 *
 * Only non-commercial, not-for-profit use of this software is permitted. No
 * part of this software may be incorporated into a commercial product without
 * the written consent of the authors. The software may not be used for the
 * design of a commercial electronic product without the written consent of the
 * authors. The use of this software to assist in the development of new
 * commercial CGRA architectures or commercial soft processor architectures is
 * also prohibited without the written consent of the authors.
 *
 * This software is provided "as is" with no warranties or guarantees of
 * support.
 *
 * This Agreement shall be governed by the laws of Province of Ontario, Canada.
 *
 * Please contact Prof. Anderson if you are interested in commercial use of the
 * CGRA-ME framework.
 ******************************************************************************/

#include <algorithm>

#include <assert.h>

#include <CGRA/Exception.h>
#include <CGRA/CGRA.h>
#include <CGRA/OpGraph.h>
#include <CGRA/ILPMapper.h>

#ifdef USE_GUROBI
#include <gurobi_c++.h>
#endif

#include <cstdio>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

ILPMapper::ILPMapper(std::shared_ptr<CGRA> cgra, int timelimit, const std::map<std::string, std::string> & args)
    : Mapper(cgra, timelimit)
{
#ifdef USE_GUROBI
    auto ilp_solver_it =args.find("ILPMapper.ilp_solver");
    if(ilp_solver_it != args.end())
    {
        std::string temp_solver_name = ilp_solver_it->second;
        std::transform(temp_solver_name.begin(), temp_solver_name.end(), temp_solver_name.begin(), ::tolower); // Transform into lowercase
        if(temp_solver_name == "scip")
        {
            std::cout << "[INFO] SCIP ILP Solver Specified" << std::endl;
            solvertype = ILPSolverType::SCIP;
        }
        else if(temp_solver_name == "gurobi")
        {
            std::cout << "[INFO] Gurobi ILP Solver Specified" << std::endl;
            solvertype = ILPSolverType::Gurobi;
        }
        else
        {
            std::cout << "[WARNING] Invalid ILP Solver Specified, Using Default SCIP Solver" << std::endl;
            solvertype = ILPSolverType::SCIP;
        }
    }
    else
    {
        std::cout << "[WARNING] No ILP Solver Specified, Using Default SCIP SCIP Solver" << std::endl;
        solvertype = ILPSolverType::SCIP;
    }
#else
    solvertype = ILPSolverType::SCIP;
#endif
    try
    {
        switch(solvertype)
        {
            case ILPSolverType::SCIP:
                scip_mipgap = std::stod(args.at("ILPMapper.scip_mip_gap"));
                scip_solnlimit = std::stoi(args.at("ILPMapper.scip_solution_limit"));
                return;
#ifdef USE_GUROBI
            case ILPSolverType::Gurobi:
                grb_mipgap = std::stod(args.at("ILPMapper.grb_mip_gap"));
                grb_solnlimit = std::stoi(args.at("ILPMapper.grb_solution_limit"));
                return;
#endif
        }
    }
    catch(const std::exception & e)
    {
        throw cgrame_error(std::string("ILPMapper Parameter Parsing Exception Thrown by: [") + e.what() + "] at File: " + std::string(__FILE__) + " Line: " + std::to_string(__LINE__));
    }
}

/*
ILPMapper::ILPMapper(CGRA* cgra, int timelimit, double mipgap, int solnlimit)
    : Mapper(cgra, timelimit)
{
    this->mipgap = mipgap;
    this->solnlimit = solnlimit;

}
*/

// This is the main mapping function
// true on success, false on failure
Mapping ILPMapper::mapOpGraph(std::shared_ptr<OpGraph> opgraph, int II)
{
    std::cout << "[INFO] Mapping DFG Onto CGRA Architecture..." << std::endl;

    // Create result obj
    Mapping mapping_result(cgra, II, opgraph);
    
    ILPMapperStatus mapper_status = ILPMapperStatus::UNLISTED_STATUS;
    switch(solvertype)
    {
        case ILPSolverType::SCIP:
            mapper_status = SCIPMap(opgraph.get(), II, &mapping_result);
            break;
#ifdef USE_GUROBI
        case ILPSolverType::Gurobi:
            mapper_status = GurobiMap(opgraph.get(), II, &mapping_result);
            break;
#endif
    }
    if(mapper_status == ILPMapperStatus::INFEASIBLE)
    {
        std::cout << "[INFO] CGRA Mapping Infeasible" << std::endl;
        std::cout << "MapperTimeout: 0" << std::endl;
        std::cout << "Mapped: 0" << std::endl;
        mapping_result.setMapped(false);
    }
    else if(mapper_status == ILPMapperStatus::OPTIMAL_FOUND)
    {
        std::cout << "[INFO] Optimal CGRA Mapping Found" << std::endl;
        std::cout << "MapperTimeout: 0" << std::endl;
        std::cout << "Mapped: 1" << std::endl;
        mapping_result.setMapped(true);
    }
    else if(mapper_status == ILPMapperStatus::SUBOPTIMAL_FOUND)
    {
        std::cout << "[INFO] Suboptimal CGRA Mapping Found" << std::endl;
        std::cout << "MapperTimeout: 0" << std::endl;
        std::cout << "Mapped: 1" << std::endl;
        mapping_result.setMapped(true);
    }
    else if(mapper_status == ILPMapperStatus::TIMEOUT)
    {
        std::cout << "[INFO] CGRA Mapping Timed Out" << std::endl;
        std::cout << "MapperTimeout: 1" << std::endl;
        std::cout << "Mapped: 0" << std::endl;
        mapping_result.setMapped(false);
    }
    else if(mapper_status == ILPMapperStatus::INTERRUPTED)
    {
        std::cout << "[INFO] CGRA Mapping Interrupted" << std::endl;
        std::cout << "MapperTimeout: 0" << std::endl;
        std::cout << "Mapped: 0" << std::endl;
        mapping_result.setMapped(false);
    }
    else if(mapper_status == ILPMapperStatus::UNLISTED_STATUS)
    {
        std::cout << "[ERROR] CGRA Mapping Results in Unlisted Status" << std::endl;
        std::cout << "[INFO] Please Report this Bug to Xander Chin(xan@ece.utoronto.ca)" << std::endl;
        std::cout << "MapperTimeout: 0" << std::endl;
        std::cout << "Mapped: 0" << std::endl;
        mapping_result.setMapped(false);
    }

    return mapping_result;
}

static SCIP_RETCODE scip_run_solver(ILPMapperStatus & mapperstatus, MRRG * mrrg, OpGraph * opgraph, double timelimit, double scip_mipgap, int scip_solnlimit, Mapping* mapping_result)
{
    SCIP* scip;
    SCIP_CALL( SCIPcreate(&scip) );
    SCIP_CALL( SCIPincludeDefaultPlugins(scip) );

    SCIP_CALL( SCIPsetRealParam(scip, "limits/gap", scip_mipgap) );
    if(timelimit != 0.0)
        SCIP_CALL( SCIPsetRealParam(scip, "limits/time", timelimit) );

    if(scip_solnlimit != 0)
        SCIP_CALL( SCIPsetIntParam(scip,"limits/solutions", scip_solnlimit) );


    const int num_dfg_vals  = opgraph->val_nodes.size();
    const int num_dfg_ops   = opgraph->op_nodes.size();
    const int num_mrrg_r    = mrrg->routing_nodes.size();
    const int num_mrrg_f    = mrrg->function_nodes.size();

    std::map<OpGraphVal*, std::map<MRRGNode*, SCIP_VAR**> > R_var_map;
    std::map<OpGraphVal*, std::map<MRRGNode*, SCIP_VAR**> > S_var_map;
    std::map<OpGraphOp*,  std::map<MRRGNode*, SCIP_VAR**> > F_var_map;

    // Create variables
    const int count_R = num_dfg_vals * num_mrrg_r;
    SCIP_VAR** R = new SCIP_VAR*[count_R];

    const int count_F = num_dfg_ops * num_mrrg_f;
    SCIP_VAR** F = new SCIP_VAR*[count_F];

    std::vector<std::pair<SCIP_VAR**, int>> S; // Vecotr of VAR array and number of fanouts
    for(auto & val : opgraph->val_nodes)
    {
        for(auto & r : mrrg->routing_nodes)
        {
            int num_fanouts = val->output.size();
            if(num_fanouts > 1)
            {
                S.emplace_back(std::make_pair(new SCIP_VAR*[num_fanouts], num_fanouts));
            }
            else
            {
                S.emplace_back(std::make_pair(nullptr, num_fanouts));
            }
        }
    }

    // Create the problem
    SCIP_CALL( SCIPcreateProbBasic(scip, "cgrame_map") );

    for(int i = 0; i < count_R; ++i)
    {
        // Objective function is implied here
        SCIP_CALL( SCIPcreateVarBasic(scip, &R[i], "R", 0.0, 1.0, 1.0, SCIP_VARTYPE_BINARY) );
        SCIP_CALL( SCIPaddVar(scip, R[i]) );
    }

    for(int i = 0; i < count_F; ++i)
    {
        SCIP_CALL( SCIPcreateVarBasic(scip, &F[i], "F", 0.0, 1.0, 0.0, SCIP_VARTYPE_BINARY) );
        SCIP_CALL( SCIPaddVar(scip, F[i]) );
    }

    for(auto & s : S)
    {
        if(s.second <= 1)
            continue;
        else
        {
            for(int i = 0; i < s.second; ++i)
            {
                SCIP_CALL( SCIPcreateVarBasic(scip, &(s.first[i]), "S", 0.0, 1.0, 0.0, SCIP_VARTYPE_BINARY) );
                SCIP_CALL( SCIPaddVar(scip, s.first[i]) );
            }
        }
    }

    // Populate maps
    int constr_count = 0;
    int j = 0;
    for(auto & val : opgraph->val_nodes)
    {
        int i = 0;
        for(auto & r : mrrg->routing_nodes)
        {
            R_var_map[val][r] = &R[j * num_mrrg_r + i];
            SCIP_CALL( SCIPchgVarName(scip, *R_var_map[val][r], ("R_" + std::to_string(j) + "_" + std::to_string(i)).c_str()) );
            auto temp_it = S.begin() + (j * num_mrrg_r + i);
            if(temp_it->second > 1)
            {
                S_var_map[val][r] = temp_it->first;
                for(int k = 0; k < val->output.size(); k++)
                {
                    SCIP_CALL( SCIPchgVarName(scip, S_var_map[val][r][k], ("R_" + std::to_string(j) + "_" + std::to_string(i) + "_" + std::to_string(k)).c_str()) );
                    SCIP_CONS* temp_cons;
                    SCIP_VAR* temp_var[2] = {*(R_var_map[val][r]), S_var_map[val][r][k]};
                    SCIP_Real temp_real[2] = {1.0, -1.0};
                    SCIP_CALL( SCIPcreateConsBasicLinear(scip, &temp_cons, ("sub_val_" + std::to_string(constr_count++)).c_str(), 2, temp_var, temp_real, 0.0, SCIPinfinity(scip)) );
                    SCIP_CALL( SCIPaddCons(scip, temp_cons) );
                    SCIP_CALL( SCIPreleaseCons(scip, &temp_cons) );
                }
            }
            else
                S_var_map[val][r] = R_var_map[val][r];
            i++;
        }
        j++;
    }

    int p = 0;
    for(auto & op : opgraph->op_nodes)
    {
        int q = 0;
        for(auto & f : mrrg->function_nodes)
        {
            F_var_map[op][f] = &F[p * num_mrrg_f + q];
            SCIP_CALL( SCIPchgVarName(scip, *F_var_map[op][f], ("F_" + std::to_string(p) + "_" + std::to_string(q)).c_str()) );
            q++;
        }
        p++;
    }

    //Constraint 1
    constr_count = 0;
    for(int i = 0; i < num_mrrg_r; i++)
    {
        SCIP_CONS* constraint;
        SCIP_Real coeffs[num_dfg_vals];
        SCIP_VAR* vars [num_dfg_vals];

        for(int j = 0; j < num_dfg_vals; j++)
        {
            coeffs[j] = 1.0;
            vars[j] = R[j * num_mrrg_r + i];
        }

        SCIP_CALL( SCIPcreateConsBasicLinear(scip, &constraint, ("route_exclusivity_" + std::to_string(constr_count++)).c_str(), num_dfg_vals, vars, coeffs, -SCIPinfinity(scip), 1.0) );
        SCIP_CALL( SCIPaddCons(scip, constraint) );
        SCIP_CALL( SCIPreleaseCons(scip, &constraint) );
    }

    // Constraint 2
    constr_count = 0;
    for(int p = 0; p < num_mrrg_f; p++)
    {
        SCIP_CONS* constraint;
        SCIP_Real coeffs[num_dfg_ops];
        SCIP_VAR* vars [num_dfg_ops];

        for(int q = 0; q < num_dfg_ops; q++)
        {
            coeffs[q] = 1.0;
            vars[q] = F[q * num_mrrg_f + p];
        }

        SCIP_CALL( SCIPcreateConsBasicLinear(scip, &constraint, ("function_unit_exclusivity_" + std::to_string(constr_count++)).c_str(), num_dfg_ops, vars, coeffs, -SCIPinfinity(scip), 1.0) );
        SCIP_CALL( SCIPaddCons(scip, constraint) );
        SCIP_CALL( SCIPreleaseCons(scip, &constraint) );
    }

    // Constraint 3
    constr_count = 0;
    for(int q = 0; q < num_dfg_ops; q++)
    {
        SCIP_CONS* constraint;
        SCIP_Real coeffs[num_mrrg_f];
        SCIP_VAR* vars [num_mrrg_f];

        for(int p = 0; p < num_mrrg_f; p++)
        {
            coeffs[p] = 1.0;
            vars[p] = F[q * num_mrrg_f + p];
        }

        SCIP_CALL( SCIPcreateConsBasicLinear(scip, &constraint, ("ensure_all_ops_mapped_" + std::to_string(constr_count++)).c_str(), num_mrrg_f, vars, coeffs, 1.0, 1.0) );
        SCIP_CALL( SCIPaddCons(scip, constraint) );
        SCIP_CALL( SCIPreleaseCons(scip, &constraint) );
    }

    // Constraint 4 - Fanout Routing
    int constr_count1 = 0;
    int constr_count2 = 0;
    for(auto &val: opgraph->val_nodes)
    {
        for(auto &r: mrrg->routing_nodes)
        {
            int val_fanouts = val->output.size();
            for(int i = 0; i < val_fanouts; i++)
            {
                std::vector<SCIP_VAR*> sum_of_fanouts;
                std::vector<SCIP_Real> coeff_sum_of_fanouts;
                int fanout_count = 0;
                for(auto &mrrg_fanout : r->fanout)
                {
                    if(mrrg_fanout->type == MRRG_NODE_ROUTING)
                    {
                        sum_of_fanouts.push_back(S_var_map[val][mrrg_fanout][i]);
                        coeff_sum_of_fanouts.push_back(1.0);
                        fanout_count++;
                    }
                    else if(mrrg_fanout->type == MRRG_NODE_FUNCTION)
                    {
                        auto &op =  val->output[i];
                        int operand = val->output_operand[i];

                        if(mrrg_fanout->fanin.size() > operand && mrrg_fanout->fanin.at(operand) == r)
                        {
                            sum_of_fanouts.push_back(*(F_var_map[op][mrrg_fanout]));
                            coeff_sum_of_fanouts.push_back(1.0);
                            fanout_count++;
                        }
                    }
                    else
                    {
                        assert(0);
                    }
                }

                SCIP_CONS* constraint;
                sum_of_fanouts.push_back(S_var_map[val][r][i]);
                coeff_sum_of_fanouts.push_back(-1.0);
                SCIP_CALL( SCIPcreateConsBasicLinear(scip, &constraint, ("fanout_routing_" + std::to_string(constr_count1++)).c_str(), sum_of_fanouts.size(), &sum_of_fanouts[0], &coeff_sum_of_fanouts[0], 0.0, SCIPinfinity(scip)) );
                SCIP_CALL( SCIPaddCons(scip, constraint) );
                SCIP_CALL( SCIPreleaseCons(scip, &constraint) );
            }
        }
    }

    // MUX Exclusivity Constraint
    constr_count = 0;
    for(auto &val: opgraph->val_nodes)
    {
        for(auto &r: mrrg->routing_nodes)
        {
            std::vector<SCIP_VAR*> sum_of_fanins;
            std::vector<SCIP_Real> coeff_sum_of_fanins;
            int fanin_count = r->fanin.size();
            if(fanin_count > 1)
            {
                for(auto &fanin : r->fanin)
                {
                    assert(fanin->type == MRRG_NODE_ROUTING);

                    // TODO: Clean up this debug code
                    if(fanin_count > 1)
                    {
                        if(fanin->fanout.size() != 1)
                        {

                            std::cout << "Candidate MUX node is: " << *r << std::endl;

                            for(auto &print_fanins : r->fanin)
                                std::cout << *print_fanins << "->" << *r << std::endl;

                            std::cout << "Problem fanin is: " << *fanin << std::endl;

                            for(auto &print_fanouts : fanin->fanout)
                                std::cout << *fanin << "->" << *print_fanouts << std::endl;

                        }
                        assert(fanin->fanout.size() == 1);
                    }

                    sum_of_fanins.push_back(*(R_var_map[val][fanin]));
                    coeff_sum_of_fanins.push_back(1.0);
                }
                SCIP_CONS* constraint;
                sum_of_fanins.push_back(*(R_var_map[val][r]));
                coeff_sum_of_fanins.push_back(-1.0);
                SCIP_CALL( SCIPcreateConsBasicLinear(scip, &constraint, ("mux_exclusivity_" + std::to_string(constr_count++)).c_str(), sum_of_fanins.size(), &sum_of_fanins[0], &coeff_sum_of_fanins[0], 0.0, 0.0) );
                SCIP_CALL( SCIPaddCons(scip, constraint) );
                SCIP_CALL( SCIPreleaseCons(scip, &constraint) );
            }
        }
    }

    // Constraint 5 - FU Fanout
    // XXX: this assumes single output nodes in both OpGraph and MRRG
    constr_count = 0;
    for(auto &op: opgraph->op_nodes)
    {
        for(auto &f: mrrg->function_nodes)
        {
            if(op->output)
            {
                OpGraphVal* val = op->output;
                assert(f->fanout.size() == 1);
                for(auto & r : f->fanout)
                {
                    int val_fanouts = val->output.size();
                    for(int i = 0; i < val_fanouts; i++)
                    {
                        SCIP_CONS* constraint;
                        SCIP_VAR* var[2] = {*(F_var_map[op][f]), S_var_map[val][r][i]};
                        SCIP_Real coeff[2] = {1, -1};
                        SCIP_CALL( SCIPcreateConsBasicLinear(scip, &constraint, ("function_unit_fanout_" + std::to_string(constr_count++)).c_str(), 2, var, coeff, 0.0, 0.0) );
                        SCIP_CALL( SCIPaddCons(scip, constraint) );
                        SCIP_CALL( SCIPreleaseCons(scip, &constraint) );
                    }
                }
            }
        }
    }

    // Constraint 7 - FU supported Op legality
    constr_count = 0;
    for(auto &op: opgraph->op_nodes)
    {
        for(auto &f: mrrg->function_nodes)
        {
            if(!f->canMapOp(op))
            {
                SCIP_CONS* constraint;
                SCIP_Real coeff[1] = {1.0};
                SCIP_CALL( SCIPcreateConsBasicLinear(scip, &constraint, ("op_support_" + std::to_string(constr_count++)).c_str(), 1, F_var_map[op][f], coeff, 0.0, 0.0) );
                SCIP_CALL( SCIPaddCons(scip, constraint) );
                SCIP_CALL( SCIPreleaseCons(scip, &constraint) );
            }
        }
    }

#ifdef WRITE_PROB
    FILE* fp = std::fopen("SCIP_Problem.lp", "w");
    SCIP_CALL( SCIPprintOrigProblem(scip, fp, "lp", FALSE) );
    std::fclose(fp);
#endif

    SCIP_CALL( SCIPsolve(scip) ); // Solve the problem

    SCIP_Status status = SCIPgetStatus(scip);

    if(status == SCIP_STATUS_INFEASIBLE)
    {
        mapperstatus = ILPMapperStatus::INFEASIBLE;
    }
    else if(status == SCIP_STATUS_TIMELIMIT)
    {
        mapperstatus = ILPMapperStatus::TIMEOUT;
    }
    else if(status == SCIP_STATUS_USERINTERRUPT)
    {
        mapperstatus = ILPMapperStatus::INTERRUPTED;
    }
    else if(status == SCIP_STATUS_OPTIMAL || status == SCIP_STATUS_SOLLIMIT || status == SCIP_STATUS_GAPLIMIT)
    {
        SCIP_SOL * sol = SCIPgetBestSol(scip);
        if(sol == nullptr)
            throw cgrame_mapper_error("Unable to Get Solution After Solving");
        // Prepare result vector
        for(auto & val : opgraph->val_nodes)
            val->fanout_result.resize(val->output.size());

// TODO:
/*
        // Detailed mapping
        for(auto & val : opgraph->val_nodes)
        {
            for(int fanout_id = 0; fanout_id < val->output.size(); ++fanout_id)
            {
                for(auto & r : mrrg->routing_nodes)
                {
                    if(SCIPgetSolVal(scip, sol, S_var_map[val][r][fanout_id]) == 1.0)
                    {
                        val->fanout_result.at(fanout_id).push_back(r);
                    }
                }
            }
        }
*/        
        for(auto & val : opgraph->val_nodes)
        {
            for(auto & r : mrrg->routing_nodes)
            {
                if(SCIPgetSolVal(scip, sol, *R_var_map[val][r]) == 1.0)
                {
                    mapping_result->mapMRRGNode(val, r);
                }
            }
        }
        for(auto & op : opgraph->op_nodes)
        {
            for(auto & f : mrrg->function_nodes)
            {
                if(SCIPgetSolVal(scip, sol, *F_var_map[op][f]) == 1.0)
                {
                    mapping_result->mapMRRGNode(op, f);
                }
            }
        }
        mapperstatus = (status == SCIP_STATUS_OPTIMAL) ? ILPMapperStatus::OPTIMAL_FOUND : ILPMapperStatus::SUBOPTIMAL_FOUND;
    }
    else
        mapperstatus = ILPMapperStatus::UNLISTED_STATUS;

    for(int i = 0; i < count_R; ++i)
    {
        SCIP_CALL( SCIPreleaseVar(scip, &R[i]) );
    }
    delete[] R;

    for(int i = 0; i < count_F; ++i)
    {
        SCIP_CALL( SCIPreleaseVar(scip, &F[i]) );
    }
    delete[] F;

    for(auto & s : S)
    {
        if(s.second <= 1)
            continue;
        else
        {
            for(int i = 0; i < s.second; ++i)
                SCIP_CALL( SCIPreleaseVar(scip, &s.first[i]) );
            delete[] s.first;
        }
    }

    SCIP_CALL( SCIPfreeTransform(scip) );
    SCIP_CALL( SCIPfree(&scip) );
    return SCIP_OKAY;
}

ILPMapperStatus ILPMapper::SCIPMap(OpGraph* opgraph, int II, Mapping* mapping_result)
{
    // Create mrrg
    MRRG* mrrg = cgra->getMRRG(II).get();

    ILPMapperStatus mapperstatus;

    SCIP_RETCODE retcode = scip_run_solver(mapperstatus, mrrg, opgraph, timelimit, scip_mipgap, scip_solnlimit, mapping_result);
    if(retcode != SCIP_OKAY)
        throw cgrame_mapper_error("SCIP Error Code: " + std::to_string(retcode));
    return mapperstatus;
}

#ifdef USE_GUROBI
ILPMapperStatus ILPMapper::GurobiMap(OpGraph* opgraph, int II, Mapping* mapping_result)
{
    // Create mrrg
    MRRG* mrrg = cgra->getMRRG(II).get();

    // Create gurobi instance
    GRBEnv env = GRBEnv();
    env.set(GRB_DoubleParam_MIPGap, grb_mipgap);
    if(timelimit != 0.0)
        env.set(GRB_DoubleParam_TimeLimit, timelimit);

    if(grb_solnlimit != 0)
        env.set(GRB_IntParam_SolutionLimit, grb_solnlimit);

#ifdef GUROBI_TUNING
    //env.set(GRB_IntParam_MIPFocus, 1); // Focus on finding Feasible solutions
    env.set(GRB_IntParam_MIPFocus, 3); // Focus on Objective Bound
    env.set(GRB_DoubleParam_Heuristics, 0.01); // Percentage time spent in feadsibility heuristics
    env.set(GRB_IntParam_Cuts, 3); // Very Agressive cut generation
    env.set(GRB_IntParam_Presolve, 2); // Agressive presolve
    env.set(GRB_IntParam_VarBranch, 2); // Maximum Infeasibility Branching
    //    env.set(GRB_IntParam_VarBranch, 3); // Strong Branching
    // env.set(GRB_IntParam_BarHomogeneous, 1); // barrier something something.... ???
#endif
    GRBModel model = GRBModel(env);

    try
    {
        const int num_dfg_vals  = opgraph->val_nodes.size();
        const int num_dfg_ops   = opgraph->op_nodes.size();
        const int num_mrrg_r    = mrrg->routing_nodes.size();
        const int num_mrrg_f    = mrrg->function_nodes.size();

        // Create variables and map of variables to MRRG nodes
        const int count_R = num_dfg_vals * num_mrrg_r;
        std::unique_ptr<GRBVar> Runique(model.addVars(count_R, GRB_BINARY));
        GRBVar * R = Runique.get();

        const int count_F = num_dfg_ops * num_mrrg_f;
        std::unique_ptr<GRBVar> Funique(model.addVars(count_F, GRB_BINARY));
        GRBVar * F = Funique.get();

        std::vector<std::pair<GRBVar *, int>> S; // Vector of VAR array and number of fanouts
        for(auto & val : opgraph->val_nodes)
        {
            for(auto & r : mrrg->routing_nodes)
            {
                int num_fanouts = val->output.size();
                if(num_fanouts > 1)
                {
                    S.emplace_back(std::make_pair(model.addVars(num_fanouts, GRB_BINARY), num_fanouts));
                }
                else
                {
                    S.emplace_back(std::make_pair(nullptr, num_fanouts));
                }
            }
        }

        // Integrate new variables
        model.update();

        std::map<OpGraphVal*, std::map<MRRGNode*, GRBVar*> > R_var_map;
        std::map<OpGraphVal*, std::map<MRRGNode*, GRBVar*> > S_var_map;
        std::map<OpGraphOp*,  std::map<MRRGNode*, GRBVar*> > F_var_map;

        // Populate maps
        int constr_count = 0;
        int j = 0;
        for(auto & val : opgraph->val_nodes)
        {
            int i = 0;
            for(auto & r : mrrg->routing_nodes)
            {
                R_var_map[val][r] = &R[j * num_mrrg_r + i];
                R_var_map[val][r]->set(GRB_StringAttr_VarName, "R_" + std::to_string(j) + "_" + std::to_string(i));
                auto temp_it = S.begin() + (j * num_mrrg_r + i);
                if(temp_it->second > 1)
                {
                    S_var_map[val][r] = temp_it->first;
                    for(int k = 0; k < val->output.size(); k++)
                    {
                        S_var_map[val][r][k].set(GRB_StringAttr_VarName, "R_" + std::to_string(j) + "_" + std::to_string(i) + "_" + std::to_string(k));
                        model.addConstr(*(R_var_map[val][r]) >= S_var_map[val][r][k], "sub_val" + std::to_string(constr_count++));
                    }
                }
                else
                {
                    S_var_map[val][r] = R_var_map[val][r];
                }
                i++;
            }
            j++;
        }
        int p = 0;
        for(auto & op : opgraph->op_nodes)
        {
            int q = 0;
            for(auto & f : mrrg->function_nodes)
            {
                F_var_map[op][f] = &F[p * num_mrrg_f + q];
                F_var_map[op][f]->set(GRB_StringAttr_VarName, "F_" + std::to_string(p) + "_" + std::to_string(q));
                q++;
            }
            p++;
        }

        // Create and Set objective
        GRBLinExpr objective;
        double coeffs[count_R];
        for(int i = 0; i < count_R; i++)
        {
            coeffs[i] = 1.0;
        }

        objective.addTerms(coeffs, R, count_R);
        model.setObjective(objective, GRB_MINIMIZE);

        // Constraint 1 - Route Exclusivity
        constr_count = 0;
        for(int i = 0; i < num_mrrg_r; i++)
        {
            GRBLinExpr constraint;
            double coeffs[num_dfg_vals];
            GRBVar vars [num_dfg_vals];

            for(int j = 0; j < num_dfg_vals; j++)
            {
                coeffs[j] = 1.0;
                vars[j] = R[j * num_mrrg_r + i];
            }

            constraint.addTerms(coeffs, vars, num_dfg_vals);

            model.addConstr(constraint <= 1,"route_exclusivity_" + std::to_string(constr_count++));
        }

        // Constraint 2
        constr_count = 0;
        for(int p = 0; p < num_mrrg_f; p++)
        {
            GRBLinExpr constraint;
            double coeffs[num_dfg_ops];
            GRBVar vars [num_dfg_ops];

            for(int q = 0; q < num_dfg_ops; q++)
            {
                coeffs[q] = 1.0;
                vars[q] = F[q * num_mrrg_f + p];
            }

            constraint.addTerms(coeffs, vars, num_dfg_ops);

            model.addConstr(constraint <= 1, "function_unit_exclusivity_" + std::to_string(constr_count++));
        }

        // Constraint 3
        constr_count = 0;
        for(int q = 0; q < num_dfg_ops; q++)
        {
            GRBLinExpr constraint;
            double coeffs[num_mrrg_f];
            GRBVar vars [num_mrrg_f];

            for(int p = 0; p < num_mrrg_f; p++)
            {
                coeffs[p] = 1.0;
                vars[p] = F[q * num_mrrg_f + p];
            }

            constraint.addTerms(coeffs, vars, num_mrrg_f);

            model.addConstr(constraint == 1, "ensure_all_ops_mapped_" + std::to_string(constr_count++));
        }

        // Constraint 4 - Fanout Routing
        int constr_count1 = 0;
        int constr_count2 = 0;
        for(auto &val: opgraph->val_nodes)
        {
            for(auto &r: mrrg->routing_nodes)
            {
                int val_fanouts = val->output.size();
                for(int i = 0; i < val_fanouts; i++)
                {
                    GRBLinExpr sum_of_fanouts;
                    int fanout_count = 0;
                    for(auto &mrrg_fanout : r->fanout)
                    {
                        if(mrrg_fanout->type == MRRG_NODE_ROUTING)
                        {
                            sum_of_fanouts += S_var_map[val][mrrg_fanout][i];
                            fanout_count++;
                        }
                        else if(mrrg_fanout->type == MRRG_NODE_FUNCTION)
                        {
                            auto &op =  val->output[i];
                            int operand = val->output_operand[i];

                            if(mrrg_fanout->fanin.size() > operand && mrrg_fanout->fanin[operand] == r)
                            {
                                sum_of_fanouts += *(F_var_map[op][mrrg_fanout]);
                                fanout_count++;
                            }
                        }
                        else
                        {
                            assert(0);
                        }
                    }

                    model.addConstr(sum_of_fanouts >= S_var_map[val][r][i], "fanout_routing_" + std::to_string(constr_count1++));

#ifdef CONSTRAIN_S_VALS
                    GRBLinExpr sum_of_fanins;
                    int fanin_count = r->fanin.size();
                    if(fanin_count > 1)
                    {
                        for(auto &fanin : r->fanin)
                        {
                            assert(fanin->type == MRRG_NODE_ROUTING);
                            // TODO: Clean up this debug code
                            if(fanin_count > 1)
                            {
                                if(fanin->fanout.size() != 1)
                                {

                                    std::cout << "Candidate MUX node is: " << *r << std::endl;

                                    for(auto &print_fanins : r->fanin)
                                        std::cout << *print_fanins << "->" << *r << std::endl;

                                    std::cout << "Problem fanin is: " << *fanin << std::endl;

                                    for(auto &print_fanouts : fanin->fanout)
                                        std::cout << *fanin << "->" << *print_fanouts << std::endl;

                                }
                                assert(fanin->fanout.size() == 1);
                            }

                            sum_of_fanins += S_var_map[val][fanin][i];
                        }
                        model.addConstr(sum_of_fanins == S_var_map[val][r][i], "mux_exclusivity_" + std::to_string(constr_count2++));
                    }
#endif
                }
            }
        }

#ifndef CONSTRAIN_S_VALS
        constr_count = 0;
        for(auto &val: opgraph->val_nodes)
        {
            for(auto &r: mrrg->routing_nodes)
            {
                GRBLinExpr sum_of_fanins;
                int fanin_count = r->fanin.size();
                if(fanin_count > 1)
                {
                    for(auto &fanin : r->fanin)
                    {
                        assert(fanin->type == MRRG_NODE_ROUTING);
                        // TODO: Clean up this debug code
                        if(fanin_count > 1)
                        {
                            if(fanin->fanout.size() != 1)
                            {

                                std::cout << "Candidate MUX node is: " << *r << std::endl;

                                for(auto &print_fanins : r->fanin)
                                    std::cout << *print_fanins << "->" << *r << std::endl;

                                std::cout << "Problem fanin is: " << *fanin << std::endl;

                                for(auto &print_fanouts : fanin->fanout)
                                    std::cout << *fanin << "->" << *print_fanouts << std::endl;

                            }
                            assert(fanin->fanout.size() == 1);
                        }

                        sum_of_fanins += *(R_var_map[val][fanin]);
                    }
                    model.addConstr(sum_of_fanins == *(R_var_map[val][r]), "mux_exclusivity_" + std::to_string(constr_count++));
                }
            }
        }
#endif

        // Constraint 5 - FU Fanout
        // XXX: this assumes single output nodes in both OpGraph and MRRG
        constr_count = 0;
        for(auto &op: opgraph->op_nodes)
        {
            for(auto &f: mrrg->function_nodes)
            {
                if(op->output)
                {
                    OpGraphVal* val = op->output;
                    assert(f->fanout.size() == 1);
                    for(auto & r : f->fanout)
                    {
                        int val_fanouts = val->output.size();
                        for(int i = 0; i < val_fanouts; i++)
                        {
                            model.addConstr(*(F_var_map[op][f]) == S_var_map[val][r][i], "function_unit_fanout_" + std::to_string(constr_count++));
                        }
                    }
                }
            }
        }

        // Constraint 7 - FU supported Op legality
        for(auto &op: opgraph->op_nodes)
        {
            for(auto &f: mrrg->function_nodes)
            {
                if(!f->canMapOp(op))
                    model.addConstr(*(F_var_map[op][f]) == 0, "op_support_" + std::to_string(constr_count++));
            }
        }

        // Update all of the constraints and variables in the model
        model.update();

#ifdef WRITE_PROB
        model.write("Gurobi_Problem.lp");
#endif

        // Optimize model
        model.optimize();
        double ilp_runtime = model.get(GRB_DoubleAttr_Runtime);
        std::cout << "Gurobi Runtime: " << ilp_runtime << std::endl;

        int status = model.get(GRB_IntAttr_Status);
        if(status == GRB_INFEASIBLE)
        {
#ifdef DEBUG_ILP
            model.computeIIS();
            model.write("Gurobi_Debug.ilp");
#endif
            return ILPMapperStatus::INFEASIBLE;
        }
        else if(status == GRB_TIME_LIMIT)
        {
            return ILPMapperStatus::TIMEOUT;
        }
        else if(status == GRB_INTERRUPTED)
        {
            return ILPMapperStatus::INTERRUPTED;
        }
        else if(status == GRB_OPTIMAL || status == GRB_SUBOPTIMAL || status == GRB_SOLUTION_LIMIT)
        {
            // Prepare result vector
            for(auto & val : opgraph->val_nodes)
                val->fanout_result.resize(val->output.size());
//TODO:
/*
            // Detailed mapping
            for(auto & val : opgraph->val_nodes)
            {
                for(int fanout_id = 0; fanout_id < val->output.size(); ++fanout_id)
                {
                    for(auto & r : mrrg->routing_nodes)
                    {
                        if(S_var_map[val][r][fanout_id].get(GRB_DoubleAttr_X) == 1.0)
                        {
                            val->fanout_result.at(fanout_id).push_back(r);
                        }
                    }
                }
            }
*/            
            for(auto & val : opgraph->val_nodes)
            {
                for(auto & r : mrrg->routing_nodes)
                {
                    if(R_var_map[val][r]->get(GRB_DoubleAttr_X) == 1.0)
                    {
                        mapping_result->mapMRRGNode(val, r);
                    }
                }
            }
            for(auto & op : opgraph->op_nodes)
            {
                for(auto & f : mrrg->function_nodes)
                {
                    if(F_var_map[op][f]->get(GRB_DoubleAttr_X) == 1.0)
                    {
                        mapping_result->mapMRRGNode(op, f);
                    }
                }
            }
            return (status == GRB_OPTIMAL) ? ILPMapperStatus::OPTIMAL_FOUND : ILPMapperStatus::SUBOPTIMAL_FOUND;
        }
        else
            return ILPMapperStatus::UNLISTED_STATUS;
    }
    catch(GRBException e)
    {
        throw cgrame_mapper_error("Gurobi Error Code: " + std::to_string(e.getErrorCode()) + " Message: " + std::string(e.getMessage()));
    }
    catch(...)
    {
        throw cgrame_mapper_error("Gurobi Unknown Exception During Mapper");
    }
}
#endif

