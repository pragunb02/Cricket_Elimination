// BaseballElimination.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <stdexcept>
#include <algorithm>
#include <climits>
#include <numeric>
#include <set>

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
    vector<vector<int>> gamesAgainstMatrix;

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
            if (!(ss >> team >> win >> loss >> remaining)) {
                throw runtime_error("Error: Invalid data format for team statistics.");
            }

            vector<int> gamesAgainst;
            for (int i = 0; i < numTeams; ++i) {
                int games;
                if (!(ss >> games)) {
                    throw runtime_error("Error: Insufficient games against data for team " + team);
                }
                gamesAgainst.push_back(games);
            }

            // Data consistency check
            int totalGamesAgainst = accumulate(gamesAgainst.begin(), gamesAgainst.end(), 0);
            if (totalGamesAgainst != remaining) {
                throw runtime_error("Error: Total games against other teams does not match remaining games for team " + team);
            }

            teamNames.push_back(team);
            wins.push_back(win);
            losses.push_back(loss);
            remainingGames.push_back(remaining);
            gamesAgainstMatrix.push_back(gamesAgainst);
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
    const vector<vector<int>>& getGamesAgainstMatrix() const { return gamesAgainstMatrix; }
};

// -----------------------------
// Graph Class
// -----------------------------
class Graph {
private:
    int numVertices;
    vector<vector<int>> capacityMatrix;

public:
    Graph(int vertices) : numVertices(vertices), capacityMatrix(vertices, vector<int>(vertices, 0)) {}

    void addEdge(int from, int to, int capacity) {
        capacityMatrix[from][to] = capacity;
    }

    const vector<vector<int>>& getCapacityMatrix() const { return capacityMatrix; }
    int getNumVertices() const { return numVertices; }
};

// -----------------------------
// Ford-Fulkerson Class (Edmonds-Karp Algorithm)
// -----------------------------
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
        : graph(g), source(s), sink(t), parent(g.getNumVertices(), -1), residual(g.getCapacityMatrix()) {}

    int getMaxFlow() {
        int maxFlow = 0;

        while (bfs()) {
            // Find bottleneck capacity
            int pathFlow = INT_MAX;
            for (int v = sink; v != source; v = parent[v]) {
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

    const vector<int>& getParent() const {
        return parent;
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
    int teamIndex; // Index of the team being analyzed
    int numTeams;
    int numGames;
    int gameNodeStart;
    int teamNodeStart;
    vector<string> teamNames;
    vector<int> wins;
    vector<int> remainingGames;

    int adjustedTeamNode(int originalIndex) const {
        return teamNodeStart + originalIndex - (originalIndex > teamIndex ? 1 : 0);
    }

public:
    FlowNetwork(const Division& division, int teamIdx)
        : teamIndex(teamIdx),
          teamNames(division.getTeamNames()),
          wins(division.getWins()),
          remainingGames(division.getRemainingGames()),
          numTeams(division.getNumTeams()),
          numGames(0),  // Initialize to 0, will calculate in the constructor body
          graph(1),  // Temporary initialization, will be properly set later
          source(0),
          sink(1)  // Temporary value, will be updated
    {
        // Calculate number of game nodes
        const auto& gamesAgainst = division.getGamesAgainstMatrix();
        for (int i = 0; i < numTeams; ++i) {
            for (int j = i + 1; j < numTeams; ++j) {
                if (i != teamIndex && j != teamIndex) {
                    int gamesLeft = gamesAgainst[i][j];
                    if (gamesLeft > 0) {
                        numGames++;
                    }
                }
            }
        }

        // Total nodes = source + game nodes + team nodes + sink
        int totalNodes = 2 + numGames + (numTeams - 1);
        
        // Properly initialize the graph with the correct number of nodes
        graph = Graph(totalNodes);
        sink = totalNodes - 1;

        gameNodeStart = 1;
        teamNodeStart = gameNodeStart + numGames;

        // Add edges from source to game nodes and from game nodes to team nodes
        int gameNode = gameNodeStart;
        for (int i = 0; i < numTeams; ++i) {
            if (i == teamIndex) continue;
            for (int j = i + 1; j < numTeams; ++j) {
                if (j == teamIndex) continue;
                int gamesLeft = gamesAgainst[i][j];
                if (gamesLeft > 0) {
                    // Edge from source to game node
                    graph.addEdge(source, gameNode, gamesLeft);

                    // Edges from game node to team nodes
                    int teamNodeI = adjustedTeamNode(i);
                    int teamNodeJ = adjustedTeamNode(j);
                    graph.addEdge(gameNode, teamNodeI, INT_MAX);
                    graph.addEdge(gameNode, teamNodeJ, INT_MAX);

                    gameNode++;
                }
            }
        }

        // Calculate the maximum number of games Team X can win
        int maxPossibleWins = wins[teamIndex] + remainingGames[teamIndex];

        // Add edges from team nodes to sink
        for (int i = 0; i < numTeams; ++i) {
            if (i == teamIndex) continue;
            int capacity = maxPossibleWins - wins[i];
            if (capacity < 0) {
                capacity = 0; // Team already has more wins than Team X can achieve
            }
            int teamNode = adjustedTeamNode(i);
            graph.addEdge(teamNode, sink, capacity);
        }
    }

    bool isEliminated(bool verbose = false) const {
        // Run Ford-Fulkerson algorithm
        FordFulkerson ff(graph, source, sink);
        int maxFlow = ff.getMaxFlow();

        // Calculate total remaining games (from source)
        int totalGames = 0;
        const auto& capacityMatrix = graph.getCapacityMatrix();
        for (int capacity : capacityMatrix[source]) {
            totalGames += capacity;
        }

        if (verbose) {
            cout << "Team: " << teamNames[teamIndex] << endl;
            cout << "Max Flow: " << maxFlow << endl;
            cout << "Total Remaining Games: " << totalGames << endl;
        }

        return maxFlow < totalGames;
    }

    vector<string> getEliminationCertificate() const {
        // Run Ford-Fulkerson algorithm
        FordFulkerson ff(graph, source, sink);
        ff.getMaxFlow();

        // Perform a BFS on the residual graph to find the reachable nodes from source
        vector<bool> visited(graph.getNumVertices(), false);
        queue<int> q;
        q.push(source);
        visited[source] = true;

        const auto& residual = ff.getResidualGraph();

        while (!q.empty()) {
            int current = q.front();
            q.pop();

            for (int next = 0; next < graph.getNumVertices(); ++next) {
                if (!visited[next] && residual[current][next] > 0) {
                    visited[next] = true;
                    q.push(next);
                }
            }
        }

        // Collect team nodes that are reachable from the source
        vector<string> certificate;
        for (int i = 0; i < numTeams; ++i) {
            if (i == teamIndex) continue;
            int teamNode = adjustedTeamNode(i);
            if (visited[teamNode]) {
                certificate.push_back(teamNames[i]);
            }
        }

        return certificate;
    }
};

// -----------------------------
// Main Function
// -----------------------------
int main() {
    string filename;
    cout << "Enter the input file name: ";
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
                    eliminationReason = division.getTeamNames()[i] + " already has more wins than " + division.getTeamNames()[teamIndex] + " can achieve.";
                    break;
                }
            }

            if (triviallyEliminated) {
                cout << division.getTeamNames()[teamIndex] << " is trivially eliminated." << endl;
                cout << "Reason: " << eliminationReason << endl;
                cout << endl;
            } else {
                // Non-trivial elimination check using Flow Network
                FlowNetwork fn(division, teamIndex);
                if (fn.isEliminated()) {
                    vector<string> certificate = fn.getEliminationCertificate();
                    cout << division.getTeamNames()[teamIndex] << " is eliminated." << endl;
                    cout << "They are eliminated by the subset { ";
                    for (const auto& team : certificate) {
                        cout << team << " ";
                    }
                    cout << "}." << endl;
                    cout << endl;
                } else {
                    cout << division.getTeamNames()[teamIndex] << " is not eliminated." << endl;
                    cout << endl;
                }
            }
        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
