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

#include <tuple>
#include <vector>
#include "adl.h"

namespace adl0 {

std::vector<std::string> ADL::tokenize_list(const std::string &lis)
{
    std::stringstream ss(lis);
    std::vector<std::string> args;

    std::string str;
    while (ss >> str) {
        args.push_back(str);
    }

    return args;
}

std::vector<std::string> ADL::parse_function_call(const std::string &expr)
{
    return tokenize_list(expr);
}

std::string ADL::expand_name(const std::string &tpl, environment env)
{
    // std::vector<std::unique_ptr<std::stringstream>> streams;
    std::vector<std::stringstream*> streams;
    int parenDepth = 0;

    streams.push_back(new std::stringstream);

    for (auto ch : tpl) {
        if (ch == '(') {
            ++parenDepth;
            // std::unique_ptr<std::stringstream> ss(new std::stringstream);
            streams.push_back(new std::stringstream);
        } else if (ch == ')') {
            auto & funccall = *streams.at(parenDepth--);
            auto & output = *streams.at(parenDepth);
            auto args = parse_function_call(funccall.str());
            if (args.size() == 0) {
                // do nothing
            } else {
                output << env.function(args.front())(env, args);
            }
            streams.pop_back();
        } else {
            *streams.back() << ch;
        }
    }
    return streams.front()->str();
}

std::function<std::string(ADL::environment, std::vector<std::string>)> ADL::const_function(std::string name, std::string content)
{
    return [name, content](ADL::environment env, std::vector<std::string> args) {
        if (args.size() != 1) {
            std::cerr << "Wrong number of arguments for constant " << name << std::endl;
        }
        return content;
    };
}

std::function<std::string(ADL::environment, std::vector<std::string>)> ADL::user_function(std::string name, std::string content)
{
    return [name, content](ADL::environment env, std::vector<std::string> args) {
        std::vector<std::string> expr;
        auto && body = parse_function_call(content);
        expr.reserve(body.size() + args.size() + 2);
        expr.push_back("$");
        expr.insert(expr.end(), body.begin(), body.end());
        expr.push_back("$");
        expr.insert(expr.end(), std::next(args.begin()), args.end());
        std::cout << "user function call: " << name << ":";
        for (auto && arg : expr) {
            std::cout << " (" << arg << ")";
        }
        std::cout << std::endl;
        return function_apply(env, expr);
    };
}

std::tuple<
std::vector<std::string>::iterator,
    std::vector<std::vector<std::string> >,
    std::vector<std::string> >
parse_function_definition(
        std::vector<std::string>::iterator it,
        std::vector<std::string>::iterator it_end
        )
{
    std::vector<std::vector<std::string> > arglists;
    std::vector<std::string> templates;
    // function description
    for (; it != it_end; ++it) {
        arglists.push_back(std::vector<std::string>());
        // Arguments
        for (; it != it_end; ++it) {
            auto arg = *it;
            if (!arg.compare("->")) {
                ++it;
                break;
            } else if (!arg.compare("$"))
                break;
            arglists.back().push_back(arg);
        }
        // Template
        std::ostringstream oss;
        bool first = true;
        for (; it != it_end; ++it) {
            auto arg = *it;
            if (!arg.compare(";")) {
                break;
            } else if (!arg.compare("$"))
                break;
            if (first) {
                first = false;
            } else {
                oss << " ";
            }
            oss << arg;
            first = false;
        }
        templates.push_back(oss.str());
        if (it != it_end && !it->compare("$"))
            break;
    }

    if (!!(*it).compare("$")) {
        std::cerr << "Function body is not terminated" << std::endl;
        throw std::runtime_error("Function body is not terminated");
    }
    ++it;

    return make_tuple(it, arglists, templates);
}

    std::pair<std::map<std::string, std::string>, bool>
pattern_match_clause(
        std::vector<std::string>::iterator it,
        std::vector<std::string>::iterator it_end,
        std::vector<std::string> arglist
        )
{
    std::cout << "pattern";

    bool success = true;
    std::map<std::string, std::string> assignment;
    for (auto && arg : arglist) {
        std::cout << " (" << arg << ")";
        if (!arg.compare("...")) { // variable argument
            std::ostringstream oss;
            bool first = true;
            for (; it != it_end; ++it) {
                if (first) {
                    first = false;
                } else {
                    oss << " ";
                }
                oss << *it;
            }
            assignment["..."] = oss.str();
            break;
        } else if (it == it_end) {
            std::cout << "line end" << *it;
            success = false;
            break;
        } else if (arg.at(0) == '$') {
            std::cout << ":" << *it;
            auto varname = arg.substr(1, std::string::npos);
            assignment[varname] = *it;
            continue;
        } else if (!!arg.compare(*it)) {
            std::cout << ":" << *it;
            success = false;
            break;
        }
        std::cout << ":" << *it;
        ++it;
    }

    if (success) {
        std::cout << " matched" << std::endl;
        return make_pair(assignment, true);
    } else {
        std::cout << " failed" << std::endl;
        return make_pair(std::map<std::string, std::string>(), false);
    }
}

std::tuple<bool, std::map<std::string, std::string>, int>
pattern_match(
        std::vector<std::string>::iterator it,
        std::vector<std::string>::iterator it_end,
        std::vector<std::vector<std::string> > arglists
        ) {
    // function arguments
    for (int i = 0; i < arglists.size(); ++i) {
        const auto & arglist = arglists.at(i);
        auto assignment = pattern_match_clause(it, it_end, arglist);
        if (assignment.second) {
            return make_tuple(true, assignment.first, i);
        }
    }
    return make_tuple(false, std::map<std::string, std::string>(), -1);
}

std::string ADL::function_apply(environment env, std::vector<std::string> args)
{
    auto it = args.begin();
    if (it != args.end()) ++it; // skip the function name

    bool a = false;
    auto lambda = parse_function_definition(it, args.end());
    it = std::get<0>(lambda);
    auto funcargs = std::get<1>(lambda);
    auto funcbodies = std::get<2>(lambda);
    auto match = pattern_match(it, args.end(), funcargs);
    auto success = std::get<0>(match);

    if (success) {
        auto assignment = std::get<1>(match);
        auto body = funcbodies.at(std::get<2>(match));
        for (auto && p : assignment) {
            auto && name = p.first;
            auto && content = p.second;
            std::cout << name << " -> " << content << std::endl;
            env.function(name, [content](environment, std::vector<std::string>){ return content; });
        }
        return expand_name(body, env);
    } else {
        std::cerr << "Pattern doesn't match" << std::endl;
        return "";
    }

}

}

