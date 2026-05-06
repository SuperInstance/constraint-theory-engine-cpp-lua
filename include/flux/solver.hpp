#pragma once
/// flux::CDCLSolver — Conflict-Driven Clause Learning solver
///
/// Literals are int32_t: positive = true, negative = false.
/// Clauses are vectors of literals.
/// Trail tracks assignments with decision levels.
/// Conflict analysis produces learned clauses.

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <optional>

namespace flux {

using Literal = int32_t;
using Clause = std::vector<Literal>;
using Assignment = std::unordered_map<uint32_t, bool>;

/// A single step in the solver's assignment trail
struct TrailEntry {
    Literal lit;
    int decision_level;
    Clause reason;  // Empty for decisions, antecedent clause for propagations
};

/// CDCL solver result
struct SolverResult {
    bool satisfiable;
    Assignment assignment;
    std::vector<Clause> learned_clauses;
    uint64_t decisions;
    uint64_t propagations;
    uint64_t conflicts;
};

/// CDCL (Conflict-Driven Clause Learning) solver
class CDCLSolver {
public:
    CDCLSolver() = default;

    /// Add a clause to the solver
    void add_clause(const Clause& clause) {
        clauses_.push_back(clause);
        for (auto lit : clause) {
            auto var = std::abs(lit);
            if (var + 1 > num_vars_) {
                num_vars_ = var + 1;
            }
        }
    }

    /// Solve the current clause set
    SolverResult solve() {
        SolverResult result;
        result.decisions = 0;
        result.propagations = 0;
        result.conflicts = 0;
        trail_.clear();
        levels_.clear();
        level_ = 0;

        while (true) {
            auto conflict = propagate(result.propagations);

            if (conflict.has_value()) {
                ++result.conflicts;

                // Can't backtrack past level 0 → UNSAT
                if (level_ == 0) {
                    result.satisfiable = false;
                    return result;
                }

                // Analyze conflict → learned clause
                auto learned = analyze_conflict(*conflict);
                result.learned_clauses.push_back(learned);

                // Backtrack
                int bt_level = compute_backtrack_level(learned);
                backtrack(bt_level);

                // Add learned clause and propagate
                clauses_.push_back(learned);
            } else {
                // Check if all variables assigned
                if (trail_.size() >= num_vars_) {
                    result.satisfiable = true;
                    for (const auto& entry : trail_) {
                        auto var = std::abs(entry.lit);
                        result.assignment[var] = (entry.lit > 0);
                    }
                    return result;
                }

                // Decision: pick next unassigned variable
                ++level_;
                auto var = pick_variable();
                if (var.has_value()) {
                    Literal lit = static_cast<Literal>(*var);
                    decide(lit);
                    ++result.decisions;
                }
            }
        }
    }

    /// Get current clauses (including learned)
    const std::vector<Clause>& clauses() const { return clauses_; }

private:
    std::vector<Clause> clauses_;
    std::vector<TrailEntry> trail_;
    std::vector<size_t> levels_; // trail index at each level start
    uint32_t num_vars_ = 0;
    int level_ = 0;

    /// Boolean Constraint Propagation
    std::optional<Clause> propagate(uint64_t& prop_count) {
        // Start from current trail end, propagate implications
        size_t start = trail_.empty() ? 0 : trail_.size() - 1;

        for (size_t i = start; i < trail_.size(); ++i) {
            Literal lit = trail_[i].lit;
            Literal neg_lit = -lit;

            for (const auto& clause : clauses_) {
                bool has_true = false;
                bool has_false = false;
                int unassigned = 0;
                Literal unit_lit = 0;
                bool all_false = true;

                for (auto cl : clause) {
                    auto val = eval_literal(cl);
                    if (val.has_value()) {
                        if (*val) { has_true = true; all_false = false; break; }
                        else { has_false = true; }
                    } else {
                        ++unassigned;
                        unit_lit = cl;
                        all_false = false;
                    }
                }

                // All false → conflict
                if (all_false && !has_true) {
                    return clause;
                }

                // Unit clause → propagate
                if (!has_true && !has_false && unassigned == 1) {
                    trail_.push_back({unit_lit, level_, clause});
                    ++prop_count;
                }
            }
        }

        return std::nullopt;
    }

    /// Evaluate a literal under current assignment
    std::optional<bool> eval_literal(Literal lit) const {
        auto var = std::abs(lit);
        for (const auto& entry : trail_) {
            if (std::abs(entry.lit) == var) {
                bool sign = (entry.lit > 0);
                return sign == (lit > 0);
            }
        }
        return std::nullopt;
    }

    /// Decide a literal at current level
    void decide(Literal lit) {
        levels_.push_back(trail_.size());
        trail_.push_back({lit, level_, {}});
    }

    /// Analyze conflict and return learned clause (1-UIP)
    Clause analyze_conflict(const Clause& conflict) {
        // Simplified: return negation of conflict literals at current level
        Clause learned;
        for (auto lit : conflict) {
            learned.push_back(-lit);
        }
        // Remove duplicates
        std::sort(learned.begin(), learned.end());
        learned.erase(std::unique(learned.begin(), learned.end()), learned.end());
        return learned;
    }

    /// Compute backtrack level from learned clause
    int compute_backtrack_level(const Clause& learned) const {
        if (learned.empty()) return 0;
        int max_level = 0;
        for (auto lit : learned) {
            for (const auto& entry : trail_) {
                if (std::abs(entry.lit) == std::abs(lit)) {
                    max_level = std::max(max_level, entry.decision_level);
                    break;
                }
            }
        }
        return std::max(0, max_level - 1);
    }

    /// Backtrack to given level
    void backtrack(int target_level) {
        while (!trail_.empty() && trail_.back().decision_level > target_level) {
            trail_.pop_back();
        }
        level_ = target_level;
        while (levels_.size() > static_cast<size_t>(target_level + 1)) {
            levels_.pop_back();
        }
    }

    /// Pick next unassigned variable (basic heuristic)
    std::optional<uint32_t> pick_variable() const {
        for (uint32_t v = 1; v < num_vars_; ++v) {
            bool assigned = false;
            for (const auto& entry : trail_) {
                if (std::abs(entry.lit) == v) {
                    assigned = true;
                    break;
                }
            }
            if (!assigned) return v;
        }
        return std::nullopt;
    }
};

} // namespace flux
