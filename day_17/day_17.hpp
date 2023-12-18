#pragma once

#include <iostream>
#include <utility>
#include <queue>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 17

NAMESPACE_DEF(DAY) {

// stuff is slow, toggling for running them 1 at a time.
#define DO_SOLUTION_1 true
#define DO_SOLUTION_2 true

struct Node;
struct Edge;

using NodePtr = std::shared_ptr<Node>;

struct Edge {
    explicit Edge(NodePtr p, int c) : to(std::move(p)), cost(c) {}

    NodePtr to;
    int cost;
};

struct Node {
    Node(int x, int y, std::string label) : x(x), y(y), label(std::move(label)) {}

    void addEdge(Edge&& e) { edges.emplace_back(e); }

    void assignId(int _id) { id = _id; }

    int x;
    int y;
    int id = -4096; // just a big negative number to hopefully segfualt If I try using nodeIDs in a vector without assignment.
    std::string label;
    std::list<Edge> edges;
};

std::ostream& operator<<(std::ostream& os, const Node& n) {
    os << "Node {\n";
    os << "\tx: " << n.x << ", y: " << n.y << "\n";
    os << "\tlbl: " << n.label << "\n";
    os << "\tedges:\n";
    for (auto& e : n.edges) {
        os << "\t\t" << e.to->label << " (" << e.cost << ")\n";
    }
    os << "}";
    return os;
}

struct DijkstraComparator {
    //std::map<const Node*, const Node*> prev;
    //std::map<const Node *, int> dist;
    std::vector<const Node *> prev;
    std::vector<int> dist; // index i represents node id i.

    bool operator()(const Node * a, const Node * b) {
        return dist[a->id] > dist[b->id];
    }

    // assumes the requested node exists in the maps :)
    std::pair<const Node *, int> data (const Node * n) const {
//        auto * pre = prev.find(n)->second;
//        int cost = dist.find(n)->second;
        auto * pre = prev[n->id];
        int cost = dist[n->id];

        return std::make_pair(pre, cost);
    }
};

constexpr int DIJKSTRA_INFINITY = std::numeric_limits<int>::max();

class DijkstraPriorityQueue : private std::priority_queue<const Node *, std::vector<const Node *>, DijkstraComparator> {
public:

    /**
     * Initializes Dijkstra's Algorithm, by preparing the priority queue:
     * Every node in the supplied container has its distance set to infinity (INT_MAX),
     * And its predecessor set to nullptr.
     *
     * Behavior is undefined if a node 'n' is encountered during shortest path calculation,
     * such that 'n' is not part of the input graph 'nodes'.
     */
    explicit DijkstraPriorityQueue(const std::vector<NodePtr>& nodes) {
        auto& comp = this->comp;

        int expected = 0; // could technically be relaxed as long as things map bijectively to [0: N).
        for (auto& ptr : nodes) {
            if (ptr->id != expected) {
                throw std::logic_error("Linear IDs expected in Dijkstra solver.");
            }
            expected++;
        }

        comp.dist.resize(nodes.size(), DIJKSTRA_INFINITY);
        comp.prev.resize(nodes.size(), nullptr);
    }

    // mimicing the wikipedia algo on Dijkstra involving a priority queue.
    // Calculates the shortest path to every node from 'from' if 'to' is nullptr.
    // 'From' was defined at construction time.
    // Be aware that using the returned object for shortest path calcs is only accurate for the 'to'<-'from' path, if 'to' is not nullptr.
    // Only general shortest-path-from-source can be looked up if the algorithm is run in foll.
    const DijkstraComparator& calculate_shortest_path(const Node * src, const Node * to) {
        this->comp.dist[src->id] = 0;
        this->push(src);

        // vertex priority queue is updated as new elements are discovered.
        while (! is_empty()) {
            auto * u = extract_min();
            std::cout << visited.size() << ". "; // progress reporting :) it is that slow :)

            if (visited.contains(u->id)) continue; // we already visited this node.

            visited.emplace(u->id);

            if (u == to) { // the shortest path is calculated, we can stop now.
                break;
            }

            // for each neighbour of u...
            for (auto& edge : u->edges) {
                auto * v = edge.to.get();
                int alt = this->comp.dist[u->id] + edge.cost;
                // update the neighbour if it is cheaper to reach it from u...
                if (alt < this->comp.dist[v->id]) {
                    this->comp.dist[v->id] = alt;
                    this->comp.prev[v->id] = u;

                    this->push(v);
                }
            }
        }

        std::cout << "\n"; // cleanup for progress reporting.
        return this->comp;
    }

private:
    std::set<int> visited;

    [[nodiscard]] bool is_empty() const {
        return this->empty();
    }

    // Remove and return the best vertex
    const Node * extract_min() {
        auto * next = this->top();
        this->pop();
        return next;
    }
};

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::vector<std::vector<int>> grid;
        grid.emplace_back(); // assumes the grid is not empty :)
        int c;
        while ((c = input.get()) != EOF) {
            if (c == '\n') {
                grid.emplace_back();
            } else {
                grid.back().emplace_back(c - '0');
            }
        }

        int nrows = static_cast<int>(grid.size());
        int ncols = static_cast<int>(grid.back().size());
        for (auto& row : grid) {
            if (row.size() != ncols) {
                throw std::logic_error("Not a rectangular grid.");
            }
        }

#if DO_SOLUTION_1
        makeNormalCrucibleAugmentedGraph(nrows, ncols, grid);
#endif
#if DO_SOLUTION_2
        makeUltraCrucibleAugmentedGraph(nrows, ncols, grid);
#endif

    }

    void v1() const override {
#if DO_SOLUTION_1
        auto srciter = std::find_if(crucibleAugmentedGraph.begin(), crucibleAugmentedGraph.end(), [](auto& nodeptr) {
            return nodeptr->label == "SRC";
        });
        if (srciter == crucibleAugmentedGraph.end()) {
            throw std::logic_error("Could not find the hardcoded SRC node.");
        }

        auto trgiter = std::find_if(crucibleAugmentedGraph.begin(), crucibleAugmentedGraph.end(), [](auto& nodeptr) {
            return nodeptr->label == "TRG";
        });
        if (trgiter == crucibleAugmentedGraph.end()) {
            throw std::logic_error("Could not find the hardcoded TRG node.");
        }

        DijkstraPriorityQueue solver(crucibleAugmentedGraph);

        auto * src = srciter->get();
        auto * trg = trgiter->get();
        auto result = solver.calculate_shortest_path(src, trg);

// Uncomment to visualise path.
//        std::cout << "Dijkstra reports path from " << src->label << " to " << trg->label << " costs: " << result.data(trg).second << "\nLike this:\n";
//        const auto * here = trg;
//        while (here != nullptr) {
//            std::cout << "\t" << here->label << "\n";
//            here = result.data(here).first;
//        }

        reportSolution(result.data(trg).second);
#else
        reportSolution(0);
#endif
    }

    void v2() const override {
#if DO_SOLUTION_2
        auto srciter = std::find_if(ultraCrucibleAugmentedGraph.begin(), ultraCrucibleAugmentedGraph.end(), [](auto& nodeptr) {
            return nodeptr->label == "SRC";
        });
        if (srciter == ultraCrucibleAugmentedGraph.end()) {
            throw std::logic_error("Could not find the hardcoded SRC node.");
        }

        auto trgiter = std::find_if(ultraCrucibleAugmentedGraph.begin(), ultraCrucibleAugmentedGraph.end(), [](auto& nodeptr) {
            return nodeptr->label == "TRG";
        });
        if (trgiter == ultraCrucibleAugmentedGraph.end()) {
            throw std::logic_error("Could not find the hardcoded TRG node.");
        }

        DijkstraPriorityQueue solver(ultraCrucibleAugmentedGraph);

        auto * src = srciter->get();
        auto * trg = trgiter->get();
        auto result = solver.calculate_shortest_path(src, trg);

// Uncomment to visualise path.
//        std::cout << "Dijkstra reports path from " << src->label << " to " << trg->label << " costs: " << result.data(trg).second << "\nLike this:\n";
//        const auto * here = trg;
//        while (here != nullptr) {
//            std::cout << "\t" << here->label << "\n";
//            here = result.data(here).first;
//        }

        reportSolution(result.data(trg).second);
#else
        reportSolution(0);
#endif
    }

    void parseBenchReset() override {
        crucibleAugmentedGraph.clear();
        ultraCrucibleAugmentedGraph.clear();
    }

private:
    std::vector<NodePtr> crucibleAugmentedGraph; // size = (W*H*4) - (2xH+2xW) + 4; x4 from augment, -(..) from pruning, +4 from src,trg and the 2 saved from the prune because of it.
    std::vector<NodePtr> ultraCrucibleAugmentedGraph;

    static void createEdges(
            const NodePtr& src,
            const std::vector<NodePtr>& nodeList,
            const std::vector<std::string>& trgs,
            const std::vector<std::vector<int>>& costGrid
    ) {
        for (auto& seekLabel : trgs) {
            auto maybeTarget = std::find_if(nodeList.begin(), nodeList.end(), [&seekLabel](const auto& n){
                return n->label == seekLabel;
            });

            if (maybeTarget == nodeList.end()) continue;

            auto& trg = *maybeTarget;

            // calculate the cost of this edge we are about to add.
            // The cost is the sum of the trg node's cost and every other node required to connect to src.
            // The behavior is undefined if src cannot be reached from trg in a straight line.
            // Moves over the 'cost grid', not existing edges.
            int sx = src->x;
            int sy = src->y;
            int tx = trg->x;
            int ty = trg->y;

            int cost = 0;
            if (sx == tx) { // vertical move.
                while (sy != ty) {
                    cost += costGrid[ty][tx];
                    ty = (sy > ty ? ty+1 : ty-1);
                }
            } else if (sy == ty) { // horizontal move.
                while (sx != tx) {
                    cost += costGrid[ty][tx];
                    tx = (sx > tx ? tx+1 : tx-1);
                }
            } else {
                throw std::logic_error("sx != tx && sy != ty (" + std::to_string(sx) + ", " + std::to_string(tx) + ", " + std::to_string(sy) + ", " + std::to_string(ty));
            }

            Edge e(trg, cost);
            src->addEdge(std::move(e));
        }
    }

    void makeNormalCrucibleAugmentedGraph(int nrows, int ncols, const std::vector<std::vector<int>>& grid) {

        auto toLabel = [](int x, int y, const char * d) { return std::to_string(x) + '.' + std::to_string(y) + d; };

        // for every coordinate in the grid create 4 nodes, one for each direction.
        std::vector<NodePtr> nodes;
        nodes.reserve(nrows * ncols * 4);
        auto dirs = { "N", "E", "S", "W" };
        for (auto d : dirs) {
            for (int i = 0; i < nrows; ++i) {
                for (int j = 0; j < ncols; ++j) {
                    nodes.emplace_back(std::make_shared<Node>(j, i, toLabel(j, i, d)));
                }
            }
        }

        // connect the nodes appropriately, with the correctly calculated cost.
        int iterCount = 0;
        std::cout << "Connecting Normal Crucible Augmented Graph. " << nodes.size() << " units of work to do. Reporting every 1000.\n";
        for (auto& n : nodes) {
            if (++iterCount % 1000 == 0) {
                std::cout << iterCount << " .. ";
            }

            std::vector<std::string> connectLabels; // not necessarily every node label to connect to actually exists, this is by design.
            char type = n->label.back();
            int x = n->x;
            int y = n->y;
            switch(type) {
                default: throw std::logic_error("Unknown NodePtr::label.back(), expected a char representing cardinal direction.");
                case 'N': // if north is chosen, our next options are east and west.
                    connectLabels.emplace_back(std::move(toLabel(x, y-1, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-2, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-3, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-1, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-2, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-3, "W")));
                    break;
                case 'E': // if east is chosen, our next options are north and south.
                    connectLabels.emplace_back(std::move(toLabel(x+1, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x+2, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x+3, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x+1, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x+2, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x+3, y, "S")));
                    break;
                case 'S': // if south is chosen, our next options are east and west.
                    connectLabels.emplace_back(std::move(toLabel(x, y+1, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+2, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+3, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+1, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+2, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+3, "W")));
                    break;
                case 'W': // if west is chosen, our next options are north and south.
                    connectLabels.emplace_back(std::move(toLabel(x-1, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x-2, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x-3, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x-1, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x-2, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x-3, y, "S")));
                    break;
            }

            // We now have all the lables to seek in the vector, to create edges for this node.
            createEdges(n, nodes, connectLabels, grid);
        }

        // finally, connect a src node to the starting points, and a target node to the ends. Coordinates are irrelevant for these.
        auto trg = std::make_shared<Node>(1'000'000'000, 1'000'000'000, "TRG");
        auto src = std::make_shared<Node>(-1'000'000'000, -1'000'000'000, "SRC");

        auto labelSeekPredicate = [](const std::string& lbl){
            return [&lbl](NodePtr& n) { return n->label == lbl; };
        };
        auto cs1 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(0, 0, "E")));
        auto cs2 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(0, 0, "S")));

        auto ct1 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(ncols-1, nrows-1, "N")));
        auto ct2 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(ncols-1, nrows-1, "S")));
        auto ct3 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(ncols-1, nrows-1, "E")));
        auto ct4 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(ncols-1, nrows-1, "W")));

        auto assertExistence = [end = nodes.end()](auto iter) {
            if (iter == end) {
                throw std::logic_error("Trying to connect non-existing node to src or trg");
            }
        };

        assertExistence(cs1); assertExistence(cs2);
        assertExistence(ct1); assertExistence(ct2); assertExistence(ct3); assertExistence(ct4);

        {
            Edge se1(*cs1, 0); src->addEdge(std::move(se1));
            Edge se2(*cs2, 0); src->addEdge(std::move(se2));
        }
        {
            Edge te1(trg, 0); (*ct1)->addEdge(std::move(te1));
            Edge te2(trg, 0); (*ct2)->addEdge(std::move(te2));
            Edge te3(trg, 0); (*ct3)->addEdge(std::move(te3));
            Edge te4(trg, 0); (*ct4)->addEdge(std::move(te4));
        }
        std::cout << "all nodes connected\npruning unconnected nodes & dead edges.\n";

        // cleanup / pruning: Edges other than "TRG" with 0 outgoing edges should not go into the graph.
        // These pruned nodes should also have edges with their name on it removed.
        std::vector<NodePtr> pruned;
        std::for_each(nodes.begin(), nodes.end(), [this, &pruned](auto node) {
            if (node->edges.empty()) {
                pruned.push_back(node);
            } else {
                this->crucibleAugmentedGraph.push_back(node);
            }
        });

        // also add the src and trg to the graph, of course.
        crucibleAugmentedGraph.push_back(src);
        crucibleAugmentedGraph.push_back(trg);

        for (auto& p : pruned) {
            std::for_each(crucibleAugmentedGraph.begin(), crucibleAugmentedGraph.end(), [&p](auto &node) {
                node->edges.erase(
                        std::remove_if(
                                node->edges.begin(),
                                node->edges.end(),
                                [&p](auto& e) {
                                    return p == e.to;
                                }),
                        node->edges.end()
                );
            });
        }

        // assign IDs to allow Dijkstra to look up nodes in constant time and O|V| memory.
        int node_id = 0;
        for (auto& n : crucibleAugmentedGraph) {
            n->assignId(node_id);
            ++node_id;
        }
        std::cout << "Augmented graph created with " << crucibleAugmentedGraph.size() << " nodes\n";
    }

    // good old code duplication for part 2, applying the edge connections as per the rules of ultra crucibles.
    // Behavior is the same, except for the edge connecting. Fortunately the connect-edges helper function is reusable.
    // The switch adding target labels for what to connect to however, is not.
    // Duplicated code sections: Node creation, Node pruning.
    // Adapted / Non-trivially duplicated: Edge label vector building to pass to createEdges
    void makeUltraCrucibleAugmentedGraph(int nrows, int ncols, const std::vector<std::vector<int>>& grid) {
        auto toLabel = [](int x, int y, const char * d) { return std::to_string(x) + '.' + std::to_string(y) + d; };

        // for every coordinate in the grid create 4 nodes, one for each direction.
        std::vector<NodePtr> nodes;
        nodes.reserve(nrows * ncols * 4);
        auto dirs = { "N", "E", "S", "W" };
        for (auto d : dirs) {
            for (int i = 0; i < nrows; ++i) {
                for (int j = 0; j < ncols; ++j) {
                    nodes.emplace_back(std::make_shared<Node>(j, i, toLabel(j, i, d)));
                }
            }
        }

        // connect the nodes appropriately, with the correctly calculated cost.
        // In ultra crucible mode, movement is 4 to 10 tiles.
        // Thus, edges connect to nodes 4 to 10 tiles over.
        // The rules for turning remain the same (only orthogonal, no continue or turn-around)
        int iterCount = 0;
        std::cout << "Connecting Ultra Crucible Augmented Graph. " << nodes.size() << " units of work to do. Reporting every 1000.\n";
        for (auto& n : nodes) {
            if (++iterCount % 1000 == 0) {
                std::cout << iterCount << " .. ";
            }

            std::vector<std::string> connectLabels; // not necessarily every node label to connect to actually exists, this is by design.
            char type = n->label.back();
            int x = n->x;
            int y = n->y;
            switch(type) {
                default: throw std::logic_error("Unknown NodePtr::label.back(), expected a char representing cardinal direction.");
                case 'N': // if north is chosen, our next options are east and west.
                    connectLabels.emplace_back(std::move(toLabel(x, y-4, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-5, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-6, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-7, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-8, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-9, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-10, "E")));

                    connectLabels.emplace_back(std::move(toLabel(x, y-4, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-5, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-6, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-7, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-8, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-9, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y-10, "W")));
                    break;
                case 'E': // if east is chosen, our next options are north and south.
                    connectLabels.emplace_back(std::move(toLabel(x+4, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x+5, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x+6, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x+7, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x+8, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x+9, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x+10, y, "N")));

                    connectLabels.emplace_back(std::move(toLabel(x+4, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x+5, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x+6, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x+7, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x+8, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x+9, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x+10, y, "S")));
                    break;
                case 'S': // if south is chosen, our next options are east and west.
                    connectLabels.emplace_back(std::move(toLabel(x, y+4, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+5, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+6, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+7, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+8, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+9, "E")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+10, "E")));

                    connectLabels.emplace_back(std::move(toLabel(x, y+4, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+5, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+6, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+7, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+8, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+9, "W")));
                    connectLabels.emplace_back(std::move(toLabel(x, y+10, "W")));
                    break;
                case 'W': // if west is chosen, our next options are north and south.
                    connectLabels.emplace_back(std::move(toLabel(x-4, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x-5, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x-6, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x-7, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x-8, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x-9, y, "N")));
                    connectLabels.emplace_back(std::move(toLabel(x-10, y, "N")));

                    connectLabels.emplace_back(std::move(toLabel(x-4, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x-5, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x-6, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x-7, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x-8, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x-9, y, "S")));
                    connectLabels.emplace_back(std::move(toLabel(x-10, y, "S")));
                    break;
            }
            // We now have all the lables to seek in the vector, to create edges for this node.
            createEdges(n, nodes, connectLabels, grid);
        }

        // finally, connect a src node to the starting points, and a target node to the ends. Coordinates are irrelevant for these.
        auto trg = std::make_shared<Node>(1'000'000'000, 1'000'000'000, "TRG");
        auto src = std::make_shared<Node>(-1'000'000'000, -1'000'000'000, "SRC");

        auto labelSeekPredicate = [](const std::string& lbl){
            return [&lbl](NodePtr& n) { return n->label == lbl; };
        };
        auto cs1 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(0, 0, "E")));
        auto cs2 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(0, 0, "S")));

        auto ct1 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(ncols-1, nrows-1, "N")));
        auto ct2 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(ncols-1, nrows-1, "S")));
        auto ct3 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(ncols-1, nrows-1, "E")));
        auto ct4 = std::find_if(nodes.begin(), nodes.end(), labelSeekPredicate(toLabel(ncols-1, nrows-1, "W")));

        auto assertExistence = [end = nodes.end()](auto iter) {
            if (iter == end) {
                throw std::logic_error("Trying to connect non-existing node to src or trg");
            }
        };

        assertExistence(cs1); assertExistence(cs2);
        assertExistence(ct1); assertExistence(ct2); assertExistence(ct3); assertExistence(ct4);

        {
            Edge se1(*cs1, 0); src->addEdge(std::move(se1));
            Edge se2(*cs2, 0); src->addEdge(std::move(se2));
        }
        {
            Edge te1(trg, 0); (*ct1)->addEdge(std::move(te1));
            Edge te2(trg, 0); (*ct2)->addEdge(std::move(te2));
            Edge te3(trg, 0); (*ct3)->addEdge(std::move(te3));
            Edge te4(trg, 0); (*ct4)->addEdge(std::move(te4));
        }
        std::cout << "all connected\npruning unconnected nodes & dead edges.\n";

        // cleanup / pruning: Edges other than "TRG" with 0 outgoing edges should not go into the graph.
        // These pruned nodes should also have edges with their name on it removed.
        std::vector<NodePtr> pruned;
        std::for_each(nodes.begin(), nodes.end(), [this, &pruned](auto node) {
            if (node->edges.empty()) {
                pruned.push_back(node);
            } else {
                this->ultraCrucibleAugmentedGraph.push_back(node);
            }
        });

        // also add the src and trg to the graph, of course.
        ultraCrucibleAugmentedGraph.push_back(src);
        ultraCrucibleAugmentedGraph.push_back(trg);

        for (auto& p : pruned) {
            std::for_each(ultraCrucibleAugmentedGraph.begin(), ultraCrucibleAugmentedGraph.end(), [&p](auto &node) {
                node->edges.erase(
                        std::remove_if(
                                node->edges.begin(),
                                node->edges.end(),
                                [&p](auto& e) {
                                    return p == e.to;
                                }),
                        node->edges.end()
                );
            });
        }

        // assign IDs to allow Dijkstra to look up nodes in constant time and O|V| memory.
        int node_id = 0;
        for (auto& n : ultraCrucibleAugmentedGraph) {
            n->assignId(node_id);
            ++node_id;
        }
        std::cout << "Augmented graph created with " << ultraCrucibleAugmentedGraph.size() << " nodes\n";
    }
};

} // namespace

#undef DAY
#undef DO_SOLUTION_1
#undef DO_SOLUTION_2