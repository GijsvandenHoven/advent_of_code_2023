#pragma once

#include <iostream>
#include <ranges>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 25

NAMESPACE_DEF(DAY) {

struct Edge;
struct Node {
    std::string name;
    int id;

    std::vector<std::shared_ptr<Edge>> edges;
};

struct Edge { // edges are undirected.
    std::shared_ptr<Node> a;
    std::shared_ptr<Node> b;

    int util = 0;
};

std::ostream& operator<<(std::ostream& os, const Edge& e) {
    os << "Edge (" << e.a->name << "/" << e.b->name << ") util: " << e.util;
    return os;
}

using Connection = std::pair<std::string, std::vector<std::string>>;

class Graph : public std::map<std::string, std::shared_ptr<Node>> {
public:
    [[maybe_unused]] void reset() {
        for (auto& [a, b] : *this) {
            for (auto& e : b->edges) {
                e->util = 0;
            }
        }
    }

    template <size_t N> void getHighestUtilEdge(std::array<std::shared_ptr<Edge>, N>& out) {
        for (auto& [_, n] : *this) {
            BFSWithEdgeUtil(n);
        }

        std::set<std::shared_ptr<Edge>> edges;
        for (auto& [_, n] : *this) {
            for (auto& e : n->edges) {
                edges.emplace(e);
            }
        }

        std::vector<std::shared_ptr<Edge>> sortedByEdgeUtil(edges.begin(), edges.end());
        std::sort(sortedByEdgeUtil.begin(), sortedByEdgeUtil.end(), [](auto& a, auto& b){
            return a->util < b->util;
        });

//        std::cout << "sort result\n";
//        for (auto& e : sortedByEdgeUtil) {
//            std::cout << *e << "\n";
//        }

        for (int i = 0; i < N; ++i) {
            int index = static_cast<int>(sortedByEdgeUtil.size()) - 1 - i;
            out[i] = sortedByEdgeUtil[index];
        }
    }

    void removeEdges(auto iterBegin, auto iterEnd) {
        int eraseCount = 0;
        while (iterBegin != iterEnd) {
            for (auto& [str, n] : *this) {
                auto iterToRemovedSection = std::remove_if(n->edges.begin(), n->edges.end(), [&removing = *iterBegin](auto& maybe){
                    return maybe == removing;
                });

                // std::cout << "Erase at " << n->name << " ? " << (iterToRemovedSection != n->edges.end()) << "\n";
                eraseCount += (iterToRemovedSection != n->edges.end());
                n->edges.erase(iterToRemovedSection, n->edges.end());
            }
            ++iterBegin;
        }
    }

    [[nodiscard]] std::pair<int,int> BFSClusterSize(
            const std::shared_ptr<Node>& cluster1,
            const std::shared_ptr<Node>& cluster2
    ) {
        auto BFS = [](Node * start) -> int {
            std::set<Node *> visited;
            std::queue<Node *> work;
            auto addToQueue = [&visited, &work](Node * unit){ work.emplace(unit); visited.emplace(unit); };

            addToQueue(start);
            while (! work.empty()) {
                auto * unit = work.front();
                work.pop();

                for (auto& e : unit->edges) {
                    auto * other = (e->a->id == unit->id ? e->b.get() : e->a.get());
                    if (! visited.contains(other)) {
                        addToQueue(other);
                    }
                }
            }

            return static_cast<int>(visited.size());
        };

        int a = BFS(cluster1.get());
        int b = BFS(cluster2.get());

        if (a + b != this->size()) throw std::logic_error("This graph is currently not a 2-cluster.");
        return {a, b};
    }

private:
    void BFSWithEdgeUtil(std::shared_ptr<Node>& start) {
        // std::cout << "BFS on " << start->name << "\n";
        std::queue<std::pair<int, Node *>> horizon;
        std::vector<Edge *> parentEdges(this->size(), nullptr); // node id to parent edge.
        std::vector<Node *> orderedParenting;
        orderedParenting.reserve(this->size());

        horizon.emplace(0, start.get());
        while (! horizon.empty()) {
            auto [d, unit] = horizon.front();
            horizon.pop();

            for (auto& edge : unit->edges) {
                auto * other = edge->a.get() == unit ? edge->b.get() : edge->a.get();
                auto& maybeE = parentEdges[other->id];
                if (maybeE == nullptr && other != start.get()) { // this node was not visited yet.
                    // std::cout << "Node " << other->name << " has dist " << d+1 << " through " << unit->name << "\n";
                    horizon.emplace(d + 1, other);
                    maybeE = edge.get();
                    orderedParenting.emplace_back(other); // sorted ascending by distance, def. of BFS.
                }
            }
        }

        // std::cout << "Edge util update\n";
        std::vector<bool> coverage(this->size(), false);
        std::function<void(Node *, int)> backtrack = [&backtrack, s = start.get(), &coverage, &parentEdges](Node * n, int power){
            coverage[n->id] = true;

            if (s == n) return;

            auto edge = parentEdges[n->id];
            auto * parent = (edge->a->id == n->id ? edge->b.get() : edge->a.get());
            edge->util += power;

            bool parentIsNovel = ! coverage[parent->id];

            backtrack(parent, power + parentIsNovel);
        };

        // backtracking to update edge counts with fewer comparisons total.
        // Starting with the highest distance to cover as much as possible and not have to repeat as much.
        for (auto& node : std::ranges::reverse_view(orderedParenting)) {
            bool touched = coverage[node->id];

            if (! touched) {
                backtrack(node, 1);
            }
        }

//        for (auto& p : *this) {
//            std::cout << p.first << ":\n";
//            for (auto& e : p.second->edges) {
//                std::cout << "\tEdge " << e << " (" << e->a->name << "/" << e->b->name << ") has util " << e->util << "\n";
//            }
//        }
    }
};

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        std::vector<std::string> lines;
        while (std::getline(input, line)) {
            lines.emplace_back(std::move(line));
        }

        auto add3Chars = [](std::istringstream& i, std::ostringstream& o) {
            o << static_cast<char>(i.get());
            o << static_cast<char>(i.get());
            o << static_cast<char>(i.get());
        };
        for (const auto& l : lines) {
            std::istringstream s(l);
            std::ostringstream buf;

            add3Chars(s, buf);
            blueprint.emplace_back(buf.str(), std::vector<std::string>());
            buf.str(std::string());

            s.ignore(2); // ': '

            bool thereIsMore = true;
            while (thereIsMore) {
                add3Chars(s, buf);
                blueprint.back().second.emplace_back(buf.str());
                buf.str(std::string());

                thereIsMore = (s.get() != EOF); // we are expecting a space if there is more, followed by another 3 char label.
            }
        }
    }

    void v1() const override {
        Graph g;
        makeGraph(g);
        std::array<std::shared_ptr<Edge>, 3> mostUsed;
        g.getHighestUtilEdge(mostUsed);
        g.removeEdges(mostUsed.begin(), mostUsed.end());
        auto [a, b] = g.BFSClusterSize(mostUsed[0]->a, mostUsed[0]->b);

        reportSolution(a * b);
    }

    void v2() const override {
        reportSolution("*Virtually pushes the button to get star 50*");
    }

    void parseBenchReset() override {
        blueprint.clear();
    }

private:
    std::vector<Connection> blueprint;

    void makeGraph(Graph& g) const {
        auto exists = [&g](const std::string& n){
            auto iter = std::find_if(g.begin(), g.end(), [&n](auto& pair){
                return pair.first == n;
            });
            return std::make_pair(iter != g.end(), iter);
        };

        int idc = 0;
        for (auto& [n, con] : blueprint) {
            auto [n_exist, n_iter] = exists(n);
            std::shared_ptr<Node> nptr;
            if (n_exist) {
                nptr = n_iter->second;
            } else {
                nptr = std::make_shared<Node>(n, idc++);
                g.emplace(n, nptr);
            }

            for (auto& c : con) {
                auto [c_exist, c_iter] = exists(c);
                std::shared_ptr<Node> cptr;
                if (c_exist) {
                    cptr = c_iter->second;
                } else {
                    cptr = std::make_shared<Node>(c, idc++);
                    g.emplace(c, cptr);
                }

                auto e = std::make_shared<Edge>();

                e->a = nptr;
                e->b = cptr;

                nptr->edges.emplace_back(e);
                cptr->edges.emplace_back(e);
            }
        }
    }
};

} // namespace

#undef DAY