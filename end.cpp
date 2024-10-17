#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <stdexcept>
#include <algorithm>
#include <climits>
#include <bits/stdc++.h>

using namespace std;

// -----------------------------
// Division Class
// -----------------------------
class Division {
private:
    int numTeams;
    vector<string> teamNames;
    vector<int> wins;
    vector<int> losses;
    vector<int> remainingGames;
    vector<vector<int>> remainingGamesMatrix;

    int stringToNumber(const string& s) const {
        return stoi(s);
    }

public:
    Division(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Error: File '" + filename + "' does not exist.");
        }

        string line;
        // Read number of teams
        if (!getline(file, line)) {
            throw runtime_error("Error: The input file is empty.");
        }

        numTeams = stringToNumber(line);
        if (numTeams < 2) {
            throw runtime_error("Error: At least two teams are required.");
        }

        // Read team data
        while (getline(file, line)) {
            if (line.empty()) continue; // Skip empty lines
            istringstream ss(line);
            string team;
            int win, loss, remaining;
            ss >> team >> win >> loss >> remaining;

            teamNames.push_back(team);
            wins.push_back(win);
            losses.push_back(loss);
            remainingGames.push_back(remaining);

            vector<int> gamesAgainst;
            for (int i = 0; i < numTeams; ++i) {
                int games;
                ss >> games;
                gamesAgainst.push_back(games);
            }
            remainingGamesMatrix.push_back(gamesAgainst);
        }

        // Validate input
        if (teamNames.size() != numTeams) {
            throw runtime_error("Error: Mismatch between number of teams and team data provided.");
        }

        file.close();
    }

    // Getter methods
    int getNumTeams() const { return numTeams; }
    const vector<string>& getTeamNames() const { return teamNames; }
    const vector<int>& getWins() const { return wins; }
    const vector<int>& getLosses() const { return losses; }
    const vector<int>& getRemainingGames() const { return remainingGames; }
    const vector<vector<int>>& getRemainingGamesMatrix() const { return remainingGamesMatrix; }
};

// -----------------------------
// Graph Class
// -----------------------------
class Graph {
private:
    int numVertices;
    vector<vector<int>> adjList; // adjacency list with capacities

public:
    // Graph(int vertices) : numVertices(vertices), adjList(vertices, vector<int>(vertices, 0)) {}
        Graph(int vertices) {
        numVertices = vertices;
        adjList = vector<vector<int>>(vertices, vector<int>(vertices, 0));
    }

    void addEdge(int from, int to, int capacity) {
        adjList[from][to] = capacity;
    }

    const vector<vector<int>>& getAdjList() const { return adjList; }
    int getNumVertices() const { return numVertices; }
};

// -----------------------------
// Ford-Fulkerson Class
// -----------------------------
// class FordFulkerson {
// private:
//     const Graph& graph;
//     int source;
//     int sink;
//     vector<int> parent;

//     bool bfs(const vector<vector<int>>& residual) {
//         parent.assign(graph.getNumVertices(), -1);
//         parent[source] = -2;
//         queue<pair<int, int>> q;
//         q.push({source, INT32_MAX});

//         while (!q.empty()) {
//             int current = q.front().first;
//             int flow = q.front().second;
//             q.pop();

//             for (int next = 0; next < graph.getNumVertices(); ++next) {
//                 if (parent[next] == -1 && residual[current][next] > 0) {
//                     parent[next] = current;
//                     int new_flow = min(flow, residual[current][next]);
//                     if (next == sink) {
//                         return true;
//                     }
//                     q.push({next, new_flow});
//                 }
//             }
//         }
//         return false;
//     }

// public:
//     FordFulkerson(const Graph& g, int s, int t) : graph(g), source(s), sink(t), parent(g.getNumVertices(), -1) {}

//     int getMaxFlow() {
//         vector<vector<int>> residual = graph.getAdjList();
//         int maxFlow = 0;

//         while (bfs(residual)) {
//             // Find the minimum residual capacity along the path filled by BFS
//             int flow = INT32_MAX;
//             int current = sink;
//             while (current != source) {
//                 int prev = parent[current];
//                 flow = min(flow, residual[prev][current]);
//                 current = prev;
//             }

//             // Update residual capacities of the edges and reverse edges
//             current = sink;
//             while (current != source) {
//                 int prev = parent[current];
//                 residual[prev][current] -= flow;
//                 residual[current][prev] += flow;
//                 current = prev;
//             }

//             maxFlow += flow;
//         }

//         return maxFlow;
//     }
// };
class FordFulkerson {
private:
    const Graph& graph;
    int source;
    int sink;
    vector<int> parent;
    vector<vector<int>> residual;

    bool bfs() {
        parent.assign(graph.getNumVertices(), -1);
        parent[source] = -2;
        queue<int> q;
        q.push(source);

        while (!q.empty()) {
            int current = q.front();
            q.pop();

            for (int next = 0; next < graph.getNumVertices(); ++next) {
                if (parent[next] == -1 && residual[current][next] > 0) {
                    parent[next] = current;
                    if (next == sink) {
                        return true;
                    }
                    q.push(next);
                }
            }
        }
        return false;
    }

public:
    FordFulkerson(const Graph& g, int s, int t)
        : graph(g), source(s), sink(t), parent(g.getNumVertices(), -1), residual(g.getAdjList()) {}

    int getMaxFlow() {
        int maxFlow = 0;

        while (bfs()) {
            // Find bottleneck capacity
            int pathFlow = INT_MAX;
            for (int v = sink; v != source; v = parent[v]) { 
                // we move from sink to source (because of parent vector we maintain)
                int u = parent[v];
                pathFlow = min(pathFlow, residual[u][v]);
            }

            // Update residual capacities
            for (int v = sink; v != source; v = parent[v]) {
                int u = parent[v];
                residual[u][v] -= pathFlow;
                residual[v][u] += pathFlow;
            }

            maxFlow += pathFlow;
        }

        return maxFlow;
    }

    const vector<vector<int>>& getResidualGraph() const {
        return residual;
    }
};


// -----------------------------
// FlowNetwork Class
// -----------------------------
class FlowNetwork {
private:
    Graph graph;
    int source;
    int sink;
    int leftTeam;
    vector<string> teamNames;

public:
    FlowNetwork(const Division& division, int teamIndex) 
        : graph(0), source(0), sink(0), leftTeam(teamIndex), teamNames(division.getTeamNames()) {
        int numTeams = division.getNumTeams();
        // Calculate number of game nodes
        int numGames = 0;
        for (int i = 0; i < numTeams; ++i) {
            for (int j = i + 1; j < numTeams; ++j) {
                if (i != teamIndex && j != teamIndex) {
                    int gamesLeft = division.getRemainingGamesMatrix()[i][j];
                    if (gamesLeft > 0) {
                        numGames++;
                    }
                }
            }
        }

        // Total nodes = source + game nodes + team nodes + sink
        int totalNodes = 2 + numGames + (numTeams - 1);
        graph = Graph(totalNodes);
        source = 0;
        sink = totalNodes - 1;

        int gameNodeStart = 1;
        int teamNodeStart = gameNodeStart + numGames;

        // Add edges from source to game nodes
        int gameNode = gameNodeStart;
        const auto& remainingGames = division.getRemainingGamesMatrix();
        for (int i = 0; i < numTeams; ++i) {
            if (i == teamIndex) continue;
            for (int j = i + 1; j < numTeams; ++j) {
                if (j == teamIndex) continue;
                int gamesLeft = remainingGames[i][j];
                if (gamesLeft > 0) {
                    graph.addEdge(source, gameNode, gamesLeft);
                    // Connect game node to team nodes
                    // Adjust team node index based on whether team index is greater than teamIndex
                    int teamNodeI = teamNodeStart + i - (i > teamIndex ? 1 : 0);
                    int teamNodeJ = teamNodeStart + j - (j > teamIndex ? 1 : 0);
                    graph.addEdge(gameNode, teamNodeI, INT32_MAX);
                    graph.addEdge(gameNode, teamNodeJ, INT32_MAX);
                    gameNode++;
                }
            }
        }

        // Calculate the maximum number of games Team X can win
        int maxPossibleWins = division.getWins()[teamIndex] + division.getRemainingGames()[teamIndex];

        // Add edges from team nodes to sink
        for (int i = 0; i < numTeams; ++i) {
            if (i == teamIndex) continue;
            int capacity = maxPossibleWins - division.getWins()[i];
            if (capacity < 0) {
                capacity = 0; // Team already has more wins than Team X can achieve
            }
            int teamNode = teamNodeStart + i - (i > teamIndex ? 1 : 0);
            graph.addEdge(teamNode, sink, capacity);
        }
    }

    // bool isEliminated() const {
    //     FordFulkerson ff(graph, source, sink);
    //     int maxFlow = ff.getMaxFlow();
    //     cout<<"mAX FLOW"<<endl;
    //     cout<<maxFlow<<endl;
    //     // Calculate total remaining games (from source)
    //     int totalGames = 0;
    //     cout<<"remaining games"<<endl;
    //     const auto& adjList = graph.getAdjList();
    //     for (int j = 0; j < adjList[source].size(); ++j) {
    //         totalGames += adjList[source][j];
    //         cout<<(adjList[source][j])<<endl;
    //     }
    //     cout<<totalGames<<endl;
    //     return maxFlow < totalGames;
    // }
    bool isEliminated() const {
    // Print adjacency list before max-flow computation
    cout << "Adjacency List (Original Capacities):" << endl;
    const auto& adjList = graph.getAdjList();
    for (size_t i = 0; i < adjList.size(); ++i) {
        for (size_t j = 0; j < adjList[i].size(); ++j) {
            if (adjList[i][j] > 0) {
                cout << "Edge from " << i << " to " << j << " with capacity " << adjList[i][j] << endl;
            }
        }
    }

    // Run Ford-Fulkerson algorithm
    FordFulkerson ff(graph, source, sink);
    int maxFlow = ff.getMaxFlow();
    cout << "MAX FLOW" << endl;
    cout << maxFlow << endl;

    // Calculate total remaining games (from source)
    int totalGames = 0;
    for (size_t j = 0; j < adjList[source].size(); ++j) {
        totalGames += adjList[source][j];
    }
    cout << "Total remaining games: " << totalGames << endl;

    // Print residual capacities after max-flow computation
    cout << "Residual Capacities After Max Flow:" << endl;
    const auto& residual = ff.getResidualGraph();
    for (size_t i = 0; i < residual.size(); ++i) {
        for (size_t j = 0; j < residual[i].size(); ++j) {
            if (adjList[i][j] > 0) { // Only print edges that existed in the original graph
                cout << "Edge from " << i << " to " << j << " has residual capacity " << residual[i][j] << endl;
            }
        }
    }

    // Optionally, print the flow along each edge
    cout << "Flow Along Each Edge:" << endl;
    for (size_t i = 0; i < residual.size(); ++i) {
        for (size_t j = 0; j < residual[i].size(); ++j) {
            int flow = adjList[i][j] - residual[i][j];
            if (flow > 0) {
                cout << "Edge from " << i << " to " << j << " carries flow " << flow << endl;
            }
        }
    }

    return maxFlow < totalGames;
}

};

// -----------------------------
// Main Function
// -----------------------------
int main() {
    string filename;
    cout << "Enter File Name:\n";
    cin >> filename;
    cout << endl;

    try {
        Division division(filename);
        int numTeams = division.getNumTeams();

        for (int teamIndex = 0; teamIndex < numTeams; ++teamIndex) {
            // Trivial elimination check
            int maxPossibleWins = division.getWins()[teamIndex] + division.getRemainingGames()[teamIndex];
            bool triviallyEliminated = false;
            string eliminationReason = "";

            for (int i = 0; i < numTeams; ++i) {
                if (i == teamIndex) continue;
                if (division.getWins()[i] > maxPossibleWins) {
                    triviallyEliminated = true;
                    eliminationReason = division.getTeamNames()[i] + " already has more wins than Team " + division.getTeamNames()[teamIndex] + " can achieve.";
                    break;
                }
            }

            if (triviallyEliminated) {
                cout << division.getTeamNames()[teamIndex] << " is trivially eliminated.\n";
                cout << eliminationReason << "\n\n";
            } else {
                // Non-trivial elimination check using Flow Network
                FlowNetwork fn(division, teamIndex);
                if (fn.isEliminated()) {
                    cout << division.getTeamNames()[teamIndex] << " is eliminated.\n";
                    // Additional details can be implemented here (e.g., identifying the certificate of elimination)
                } else {
                    cout << division.getTeamNames()[teamIndex] << " is not eliminated.\n";
                }
                cout << "\n";
            }
        }
    } catch (const exception& e) {
        cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
