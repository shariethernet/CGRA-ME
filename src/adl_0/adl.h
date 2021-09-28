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

#ifndef XML_PARSER_HPP
#define XML_PARSER_HPP
#include "pugixml.hpp"

#include <utility>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <initializer_list>
#include <map>
#include <memory>
#include <functional>

#include <CGRA/Module.h>
#include <CGRA/CGRA.h>

namespace adl0 {

class ADL {
    private:
        template<typename T> // for strict typedef
            struct grid_pair {
                public:
                    int row;
                    int col;
                    grid_pair() : row(0), col(0) { }
                    grid_pair(int row, int col) : row(row), col(col) { }
                    bool operator==(const grid_pair<T> &other) const {
                        return std::make_pair(row, col) == std::make_pair(other.row, other.col);
                    }
                    bool operator!=(const grid_pair<T> &other) const {
                        return !(*this == other);
                    }
                    // default operator==;
            };

        std::ostream &debug_out = std::cout;

        struct TAG_pattern_size;
        struct TAG_area_size;
        struct TAG_grid_position;

    public:
        typedef grid_pair<TAG_pattern_size> pattern_size;
        typedef grid_pair<TAG_area_size> area_size;
        typedef grid_pair<TAG_grid_position> grid_position;

    public:
        enum class division {
            NW, N, NE, W, C, E, SW, S, SE, WHOLE, INVALID
        };

        struct grid_area {
            private:
                int top_, bottom_, left_, right_;
                const std::vector<division> divisions_;

                grid_position division_begin(division div) const noexcept ;

            public:

                int top(void) { return top_; }
                int bottom(void) { return bottom_; }
                int left(void) { return left_; }
                int right(void) { return right_; }
                class const_iterator : public std::iterator<std::output_iterator_tag, grid_position> {
                    friend struct grid_area;
                    private:
                    const grid_area *area_;
                    std::vector<division>::const_iterator div_it_, div_end_;
                    grid_position current_; // (row, col)

                    public:
                    const_iterator () = default;
                    protected:
                    const_iterator (const grid_area *area) noexcept :
                        area_(area),
                        div_it_(area->divisions_.begin()),
                        div_end_(area->divisions_.end()),
                        current_(initial_position(*area)) {
                            follow_valid_division();
                        };

                    const_iterator (const grid_area *area,
                            std::vector<division>::const_iterator div_begin,
                            std::vector<division>::const_iterator div_end
                            ) noexcept :
                        area_(area),
                        div_it_(div_begin),
                        div_end_(div_end),
                        current_(initial_position(*area, div_begin, div_end)) {
                            follow_valid_division();
                        }

                    private:
                    static grid_position initial_position(const grid_area &area,
                            std::vector<division>::const_iterator begin,
                            std::vector<division>::const_iterator end) {
                        if (begin == end) {
                            return area.division_begin(division::INVALID);
                        } else {
                            return area.division_begin(*begin);
                        }
                    }

                    static grid_position initial_position(const grid_area &area) {
                        const auto & divs = area.divisions_;
                        return initial_position(area, divs.begin(), divs.end());
                    }

                    void follow_valid_division(void) {
                        for (; div_it_ != div_end_; ++div_it_) {
                            current_ = area_->division_begin(*div_it_);
                            if (valid_position(*div_it_)) {
                                return;
                            }
                        }
                        invalidate();
                    }

                    public:
                    const grid_position& operator*() const noexcept {
                        return current_;
                    };

                    const_iterator& operator++() {
                        if (move_next_position_in_same_division()) {

                        } else {
                            ++div_it_;
                            follow_valid_division();
                        }
                        return *this;
                    }

                    int predictable_id(void) const;

                    bool in_division(const division &div) const {
                        return valid_position(div);
                    }

                    bool operator==(const const_iterator &other) const {
                        return current_ == other.current_; // ToDo: Is it okay to ignore compare with another grid_area's iterator?
                    }
                    bool operator!=(const const_iterator &other) const {
                        return !(*this == other);
                    }

                    void invalidate(void) noexcept {
                        current_ = area_->division_begin(division::INVALID);
                    }

                    private:
                    bool valid_position(division) const noexcept;
                    bool move_next_position_in_same_division(void) noexcept ;

                };
                typedef const_iterator iterator;

            public:
                grid_area(std::pair<int, int> topleft, std::pair<int, int> bottomright, std::vector<division> &&divs) :
                    top_(topleft.first), bottom_(bottomright.first),
                    left_(topleft.second), right_(bottomright.second),
                    divisions_(divs) {}
                grid_area(std::pair<int, int> topleft, std::pair<int, int> bottomright) :
                    grid_area(topleft, bottomright, {
                            division::WHOLE }) {}
                const_iterator begin() const { return iterator(this); }
                const_iterator end() const { return iterator(this, divisions_.end(), divisions_.end()); }
        };

    private:
        class WrapperModule {
            public:
                virtual ~WrapperModule () = default;
                virtual void addConfig(std::string name, std::vector<std::string> ConnectTo) = 0;
                virtual void addSubModule(std::string name, std::shared_ptr<WrapperModule> mod) = 0;
                virtual void addPrimitive(std::string name, std::string type, const std::map<std::string, std::string> &args = {}) = 0;
                virtual void addPort(std::string portname, port_type pt, unsigned size = DEFAULT_SIZE) = 0;
                virtual void addConnection(std::string from, std::string to) = 0;
                virtual void addWire(std::string wire) ;
                virtual void setConfiguration(const std::map<std::string, std::string> config) = 0;
                virtual std::unique_ptr<Module> instantiate(std::string name) = 0;

                void addConfig(Module &m, std::string name, std::vector<std::string> ConnectTo);
                void addSubModule(Module &m, std::string name, WrapperModule &mod);
                void addPrimitive(Module &m, std::string name, std::string type, const std::map<std::string, std::string> &args = {});
                void addPort(Module &m, std::string portname, port_type pt, unsigned size = DEFAULT_SIZE);
                void addConnection(Module &m, std::string from, std::string to);

                std::set<std::string> aliases_;
                std::map<std::string, std::string> correspondences_;
                bool isAlias(const std::string & alias) ;
                void addAlias(const std::string & alias);
                void addAliasSubstantia(const std::string & alias, const std::string & subst);
                std::string substantia(std::string alias);
        };

        class ADLModule : public Module {
            private:
                std::string cls_;
                virtual std::string GenericName() {
                    return cls_;
                }
            public:
                ADLModule (std::string name, std::string cls) : Module(name), cls_(cls) {};
        };

        class ADLModuleWrapper : public WrapperModule {
            private:
                std::string cls_;
                std::vector<std::pair<std::string, std::shared_ptr<WrapperModule> > > submodules;
                std::vector<std::pair<std::string, std::vector<std::string> > > configs;
                std::vector<std::tuple<std::string, std::string, std::map<std::string, std::string> > > primitives;
                std::vector<std::tuple<std::string, port_type, unsigned> > ports;
                std::vector<std::tuple<std::string, std::string> > connections;
            public:
                ADLModuleWrapper () = default;
                ADLModuleWrapper (std::string cls) : cls_(cls) {};
                virtual ~ADLModuleWrapper () = default;
                virtual void addSubModule(std::string name, std::shared_ptr<WrapperModule> mod);
                virtual void addPrimitive(std::string name, std::string mod, const std::map<std::string, std::string> &args = {});
                virtual void addPort(std::string portname, port_type pt, unsigned size = DEFAULT_SIZE);
                virtual void addConfig(std::string name, std::vector<std::string> ConnectTo);
                virtual void addConnection(std::string from, std::string to);
                virtual void setConfiguration(const std::map<std::string, std::string> config) override;
                virtual std::unique_ptr<Module> instantiate(std::string name);
                virtual std::unique_ptr<Module> instantiate(std::unique_ptr<Module> m);
        };

        class TopCGRAModule : public ADLModuleWrapper {
            private:
                CGRA &cgra;
            public:
                TopCGRAModule (CGRA &cgra) noexcept : cgra(cgra) {}
                virtual ~TopCGRAModule () = default;
                virtual void setConfiguration(const std::map<std::string, std::string> config) override;
                virtual std::unique_ptr<Module> instantiate(std::string name) override {
                    return ADLModuleWrapper::instantiate(std::unique_ptr<CGRA>(new CGRA(cgra)));
                };
                static const std::string config_prefix;
                static const std::string config_rows;
                static const std::string config_cols;
        };

    public:
        class environment {
            private:
                std::ostream * debug_out_;
                std::pair<grid_area::iterator, bool> position_; // In place of std::optional.
                std::pair<area_size, bool> area_size_; // In place of std::optional.
                std::map<std::string, std::function<std::string(environment, std::vector<std::string>)>> functions_;
                std::shared_ptr<WrapperModule> module_; // Current working module.
            public:
                typedef std::string glob_function(environment, std::vector<std::string>) ;
                environment() = default;
                environment(std::shared_ptr<WrapperModule> module) : module_(module) {}
                // environment(WrapperModule &module) : module_(&module) {}
                /*
                   void position(int row, int col) {
                   position_ = std::make_pair(grid_position(row, col), true);
                   }
                 */
                void debug_out(std::ostream &os) {
                    debug_out_ = &os;
                }
                std::ostream * debug_out(void) const {
                    return debug_out_;
                }
                void position(grid_area::iterator pos) {
                    position_ = std::make_pair(pos, true);
                }
                grid_position position(void) const {
                    if (position_.second)
                        return *position_.first;
                    else
                        throw std::runtime_error("No position");
                }
                bool in_division(division div) const {
                    if (position_.second)
                        return (position_.first).in_division(div);
                    else
                        throw std::runtime_error("No position");
                }
                void areaSize(int row, int col) {
                    area_size_ = std::make_pair(area_size(row, col), true);
                }
                area_size areaSize(void) const {
                    if (area_size_.second)
                        return area_size_.first;
                    else
                        throw std::runtime_error("No area size");
                }
                void function(std::string funcname, std::function<std::string(environment, std::vector<std::string>)> func) {
                    if (debug_out())
                        *debug_out() << "add function: " << funcname << std::endl;
                    functions_[funcname] = [func, funcname](environment env, std::vector<std::string> args){
                        auto os = env.debug_out();
                        if (os) {
                            *os << "expansion: " << funcname << ":";
                            for (auto && arg : args) {
                                *os << " (" << arg << ")";
                            }
                            *os << std::endl;
                        }
                        auto && ret = func(env, args);

                        if (os) {
                            *os << funcname << ":";
                            for (auto && arg : args) {
                                *os << " (" << arg << ")";
                            }
                            *os << " -> (" << ret << ")" << std::endl;
                        }
                        return ret;
                    };
                }
                const std::function<std::string(environment, std::vector<std::string>)> function(std::string funcname) const {
                    auto f = functions_.find(funcname);
                    if (f != functions_.end())
                        return (*f).second;
                    else {
                        std::cerr << "function " << funcname << " is not found!" << std::endl;
                        throw std::runtime_error(std::string("no function named: ") + funcname);
                    }
                }
                void module(std::shared_ptr<WrapperModule> module) {
                    module_ = module;
                }
                WrapperModule &module(void) const {
                    return *module_;
                }
        };

    private:
        std::map<std::string, std::shared_ptr<WrapperModule> > modules;
        std::map<std::string, std::pair<environment, pugi::xml_node> > moduleTemplates;
        std::map<std::string, pugi::xml_node> syntaxSugarTemplates;

    public:
        std::string parse_wrap_rel(std::string rel, int currentRow, int maxRow, int currentCol, int maxCol, std::vector<std::pair<std::string, int>> counters);
        std::shared_ptr<environment> parse_result;
        static division read_division(const std::string &str);

        void debug_print_node(const std::string name, const pugi::xml_node connection);
        void connection_declaration(environment env, const pugi::xml_node connection);
        void port_declaration(environment env, const pugi::xml_node port);
        void wire_declaration(environment env, const pugi::xml_node wire);
        void pattern_declaration(environment env, const pugi::xml_node pattern);
        std::string module_declaration(environment env, const pugi::xml_node module);
        std::string module_registration(const std::string & name, const std::string & mode);
        std::string module_instantiation(environment env, const pugi::xml_node module);
        void block_declaration(environment env, const pugi::xml_node block);
        void architecture_declaration(environment env, const pugi::xml_node arch);
        void cgra_declaration(environment env, const pugi::xml_node doc);

        void user_declaration(environment env, const pugi::xml_node node);
        void template_interpreter(environment env, pugi::xml_node target, const pugi::xml_node templ);

        bool conditional(environment env, const pugi::xml_node node);
        void macro_definition(environment &env, const pugi::xml_node node);

        static std::vector<std::string> tokenize_list(const std::string &str);
        template<typename T>
            static std::vector<T> tokenize_list_as(const std::string &str)
            {
                std::stringstream ss(str);
                std::vector<T> args;

                T x;
                while (ss >> x) {
                    args.push_back(x);
                    std::cout << x << " <- SOME OPCODE?\n";
                }

                return args;
            }

        static std::vector<std::string> parse_function_call(const std::string &expr);
        void add_default_functions(environment &end);

        std::function<std::string(ADL::environment, std::vector<std::string>)> const_function(std::string name, std::string content);
        std::function<std::string(ADL::environment, std::vector<std::string>)> user_function(std::string name, std::string content);

        template<typename T, typename S>
            std::function<std::string(ADL::environment, std::vector<std::string>)> accum_function(std::string name, T init, std::function<std::pair<T,bool>(T,S)> update)
            {
                return [name, init, update](ADL::environment env, std::vector<std::string> args) {
                    auto it = args.begin();
                    if (it != args.end()) ++it; // skip the function name

                    T a = init;
                    for (; it != args.end(); ++it) {
                        auto arg = *it;
                        std::istringstream iss(arg);
                        S b;
                        iss >> b;
                        auto p = update(a,b);
                        a = p.first;
                        if (!p.second)
                            break;
                    }
                    return std::to_string(a);
                };
            }

        static std::string function_apply(environment env, std::vector<std::string> args);
        static std::string expand_name(const std::string &name, environment env);

        template<typename T>
            static T expand_as(const std::string &expr, environment env, T def = 0)
            {
                auto result = expand_name(expr, env);
                std::istringstream iss(std::move(result));
                T b;
                iss >> b;
                if (iss.fail()) {
                    return def;
                } else {
                    return b;
                }
            }


    public:
        /**
         * Returned pointer may be invalidated after this object is destructed.
         *
         * ToDo: Replace raw pointers in Module classes with smart pointers OR use GC to solve this problem.
         */
        std::unique_ptr<CGRA> parse(const pugi::xml_node doc);
};

// Main parser function
std::shared_ptr<CGRA> parseADL(std::string arch);

}

#endif
