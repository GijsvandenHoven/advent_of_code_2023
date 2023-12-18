#pragma once

#include <iostream>
#include <utility>
#include <queue>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 17

NAMESPACE_DEF(DAY) {

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

    int x;
    int y;
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
    std::map<const Node*, const Node*> prev;
    std::map<const Node *, int> dist;
    bool operator()(const Node * a, const Node * b) {
        return dist[a] > dist[b];
    }
};

class DijkstraPriorityQueue : public std::priority_queue<
        Node *,
        std::vector<Node *>,
        DijkstraComparator
> {
public:
    explicit DijkstraPriorityQueue(const std::vector<NodePtr>& nodes) {
        auto& comp = this->comp;
        for (auto& n : nodes) {
            auto * nptr = n.get();
            comp.dist[nptr] = std::numeric_limits<int>::max();
            comp.prev[nptr] = nullptr;

            this->push(nptr);
        }
    }

    DijkstraComparator& get_comparator() {
        return this->comp;
    }
};

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::vector<std::vector<int>> grid;
        grid.emplace_back(); // assumes the grid is not empty :)
        int c = 0;
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

        std::cout << "nrows: " << nrows << ", ncols: " << ncols << "\n";

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

        std::cout << "nodes created\n";

        // connect the nodes appropriately, with the correctly calculated cost.
        int iterCount = 0;
        std::cout << "Connecting Augmented Graph." << nodes.size() << " units of work to do. Reporting every 1000.\n";
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

            // std::cout << "\ttry create edges\n";
            // We now have all the lables to seek in the vector, to create edges for this node.
            createEdges(n, nodes, connectLabels, grid);
        }

        std::cout << "nodes connected\n";
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
        std::cout << "src & trg connected\npruning unconnected nodes & dead edges.";

        // cleanup / pruning: Edges other than "TRG" with 0 outgoing edges should not go into the graph.
        // These pruned nodes should also have edges with their name on it removed.
        std::vector<NodePtr> pruned;
        std::for_each(nodes.begin(), nodes.end(), [this, &pruned](auto node) {
            if (node->edges.empty()) {
                // std::cout << "node " << node->label << " has 0 outgoing edges.\n";
                pruned.push_back(node);
            } else {
                this->augmentedGraph.push_back(node);
            }
        });

        // also add the src and trg to the graph, of course.
        augmentedGraph.push_back(src);
        augmentedGraph.push_back(trg);

        for (auto& p : pruned) {
            std::for_each(augmentedGraph.begin(), augmentedGraph.end(), [&p](auto &node) {
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

        std::cout << "Augmented graph created with " << augmentedGraph.size() << " nodes\n";
    }

    void v1() const override {
        std::vector<NodePtr> test;
        auto x = std::make_shared<Node>(0, 0, "A");
        auto y = std::make_shared<Node>(1, 1, "B");
        auto z = std::make_shared<Node>(2, 2, "C");

        auto e1 = Edge(y, 10);
        x->addEdge(std::move(e1));

        auto e2 = Edge(z, 5);
        y->addEdge(std::move(e2));

        auto e3 = Edge(x, 2);
        z->addEdge(std::move(e3));

        test.push_back(x);
        test.push_back(y);
        test.push_back(z);

        DijkstraPriorityQueue q(test);

        std::cout << "COMPARATOR OBJ TEST\n";

        auto& c = q.get_comparator();
        auto& d = c.dist;
        auto& p = c.prev;

        std::cout << "DISTS\n";
        for (auto& kvp : d) {
            std::cout << "\t" << kvp.first << ", " << kvp.second << "\n";
        }

        std::cout << "PREV\n";
        for (auto& kvp : p) {
            std::cout << "\t" << kvp.first << ", " << kvp.second << "\n";
        }

        reportSolution(0);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {
        augmentedGraph.clear();
    }

private:
    std::vector<NodePtr> augmentedGraph; // size = (W*H*4) - (2xH+2xW) + 4; x4 from augment, -(..) from pruning, +4 from src,trg and the 2 saved from the prune because of it.

    static void dijkstra() {

    }

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

            // std::cout << "Going to connect " << src->label << " with: " << (*maybeTarget)->label << ".\n";
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

            // std::cout << "edge from " << src->label << " to " << (*maybeTarget)->label << " costs " << cost << ".\n";
            Edge e(trg, cost);
            src->addEdge(std::move(e));
        }
    }
};

} // namespace

#undef DAY