#include <stdio.h>
#include <string.h>
#include <limits.h>

#define MAX 10
#define INF 999999

// Structure to represent a node in the search tree
typedef struct
{
    int path[MAX];          // Partial tour
    int visited[MAX];       // Visited cities
    int level;              // Number of cities in current path
    int cost;               // Cost of current partial path
    int bound;              // Lower bound
} Node;

// Global variables
int n;
char city[MAX][30];
int cost[MAX][MAX];

int bestCost = INF;
int bestPath[MAX + 1];

int expandedNodes = 0;
int prunedNodes = 0;

/*
   Calculate the lower bound of a node.

   The bound is calculated using:
   Current path cost +
   minimum outgoing edge for every unvisited city.
*/
int calculateBound(Node *node)
{
    int bound = node->cost;
    int i, j;
    int minCost;

    // For the current city, estimate the cheapest outgoing edge
    int currentCity = node->path[node->level - 1];

    minCost = INF;

    for (j = 0; j < n; j++)
    {
        if (!node->visited[j] && cost[currentCity][j] < minCost)
        {
            minCost = cost[currentCity][j];
        }
    }

    if (node->level < n && minCost != INF)
    {
        bound += minCost;
    }

    // For every unvisited city, add its minimum outgoing edge
    for (i = 0; i < n; i++)
    {
        if (!node->visited[i])
        {
            minCost = INF;

            for (j = 0; j < n; j++)
            {
                if (i != j && cost[i][j] < minCost)
                {
                    minCost = cost[i][j];
                }
            }

            if (minCost != INF)
            {
                bound += minCost;
            }
        }
    }

    return bound;
}

/*
   Find an initial feasible tour using a simple
   nearest-neighbor approach.

   This gives us an initial upper bound (incumbent).
*/
void findInitialTour(int start)
{
    int visited[MAX] = {0};
    int path[MAX + 1];

    int current = start;
    int totalCost = 0;

    path[0] = start;
    visited[start] = 1;

    for (int level = 1; level < n; level++)
    {
        int next = -1;
        int minimum = INF;

        for (int j = 0; j < n; j++)
        {
            if (!visited[j] && cost[current][j] < minimum)
            {
                minimum = cost[current][j];
                next = j;
            }
        }

        if (next == -1 || minimum == INF)
        {
            return;
        }

        visited[next] = 1;
        path[level] = next;
        totalCost += cost[current][next];

        current = next;
    }

    // Return to starting city
    if (cost[current][start] == INF)
    {
        return;
    }

    totalCost += cost[current][start];
    path[n] = start;

    bestCost = totalCost;

    for (int i = 0; i <= n; i++)
    {
        bestPath[i] = path[i];
    }
}

/*
   Branch and Bound using a simple priority selection.

   Since this is a small-city mini project, we maintain
   live nodes in an array and select the node having
   the smallest lower bound.
*/
void branchAndBound(int start)
{
    Node liveNodes[10000];
    int liveCount = 0;

    // Create root node
    Node root;

    root.level = 1;
    root.cost = 0;

    for (int i = 0; i < n; i++)
    {
        root.visited[i] = 0;
    }

    root.path[0] = start;
    root.visited[start] = 1;

    root.bound = calculateBound(&root);

    liveNodes[liveCount++] = root;

    while (liveCount > 0)
    {
        // Find node having smallest lower bound
        int bestIndex = 0;

        for (int i = 1; i < liveCount; i++)
        {
            if (liveNodes[i].bound < liveNodes[bestIndex].bound)
            {
                bestIndex = i;
            }
        }

        // Remove selected node
        Node current = liveNodes[bestIndex];

        liveNodes[bestIndex] = liveNodes[liveCount - 1];
        liveCount--;

        // If lower bound cannot improve incumbent, prune
        if (current.bound >= bestCost)
        {
            prunedNodes++;
            continue;
        }

        expandedNodes++;

        // If all cities have been visited
        if (current.level == n)
        {
            int lastCity = current.path[current.level - 1];

            // Check whether we can return to starting city
            if (cost[lastCity][start] != INF)
            {
                int totalCost =
                    current.cost + cost[lastCity][start];

                if (totalCost < bestCost)
                {
                    bestCost = totalCost;

                    for (int i = 0; i < n; i++)
                    {
                        bestPath[i] = current.path[i];
                    }

                    bestPath[n] = start;
                }
            }

            continue;
        }

        // Generate children
        int currentCity = current.path[current.level - 1];

        for (int nextCity = 0; nextCity < n; nextCity++)
        {
            // Check if city is not visited
            if (!current.visited[nextCity] &&
                cost[currentCity][nextCity] != INF)
            {
                Node child;

                child.level = current.level + 1;

                child.cost =
                    current.cost + cost[currentCity][nextCity];

                // Copy path and visited information
                for (int i = 0; i < n; i++)
                {
                    child.visited[i] = current.visited[i];
                    child.path[i] = current.path[i];
                }

                child.path[current.level] = nextCity;
                child.visited[nextCity] = 1;

                // Calculate lower bound
                child.bound = calculateBound(&child);

                // Add only if it can improve current best
                if (child.bound < bestCost)
                {
                    if (liveCount < 10000)
                    {
                        liveNodes[liveCount++] = child;
                    }
                }
                else
                {
                    prunedNodes++;
                }
            }
        }
    }
}

/*
   Validate the input matrix.
*/
int validateMatrix()
{
    for (int i = 0; i < n; i++)
    {
        if (cost[i][i] != 0)
        {
            printf("\nError: Diagonal values must be 0.\n");
            return 0;
        }

        for (int j = 0; j < n; j++)
        {
            if (cost[i][j] < 0)
            {
                printf("\nError: Costs cannot be negative.\n");
                return 0;
            }
        }
    }

    return 1;
}

/*
   Check whether at least one complete tour is possible.
*/
int tourPossible(int start)
{
    int visited[MAX] = {0};

    int current = start;
    visited[start] = 1;

    for (int count = 1; count < n; count++)
    {
        int next = -1;
        int minimum = INF;

        for (int j = 0; j < n; j++)
        {
            if (!visited[j] && cost[current][j] < minimum)
            {
                minimum = cost[current][j];
                next = j;
            }
        }

        if (next == -1 || minimum == INF)
        {
            return 0;
        }

        visited[next] = 1;
        current = next;
    }

    if (cost[current][start] == INF)
    {
        return 0;
    }

    return 1;
}

int main()
{
    int start;
    char startName[30];

    printf("============================================\n");
    printf("   TRAVELLING SALESMAN PROBLEM (TSP)\n");
    printf("        USING BRANCH AND BOUND\n");
    printf("============================================\n");

    // Input number of cities
    printf("\nEnter number of cities (2-%d): ", MAX);
    scanf("%d", &n);

    if (n < 2 || n > MAX)
    {
        printf("Invalid number of cities.\n");
        return 0;
    }

    // Input city names
    printf("\nEnter city names:\n");

    for (int i = 0; i < n; i++)
    {
        printf("City %d: ", i);
        scanf("%29s", city[i]);
    }

    // Input cost matrix
    printf("\nEnter the cost matrix.\n");
    printf("Use %d for INF (no direct path).\n\n", INF);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("Cost from %s to %s: ",
                   city[i], city[j]);

            scanf("%d", &cost[i][j]);

            if (cost[i][j] == INF)
            {
                cost[i][j] = INF;
            }
        }
    }

    // Validate matrix
    if (!validateMatrix())
    {
        return 0;
    }

    // Input starting city
    printf("\nEnter starting city: ");
    scanf("%29s", startName);

    start = -1;

    for (int i = 0; i < n; i++)
    {
        if (strcmp(startName, city[i]) == 0)
        {
            start = i;
            break;
        }
    }

    if (start == -1)
    {
        printf("\nError: Starting city not found.\n");
        return 0;
    }

    // Check whether a tour is possible
    if (!tourPossible(start))
    {
        printf("\n============================================\n");
        printf("No complete tour exists from %s.\n", city[start]);
        printf("============================================\n");
        return 0;
    }

    // Find initial feasible solution
    findInitialTour(start);

    if (bestCost == INF)
    {
        printf("\nNo feasible tour exists.\n");
        return 0;
    }

    printf("\nInitial feasible tour found.");
    printf("\nInitial upper bound = %d\n", bestCost);

    // Reset statistics
    expandedNodes = 0;
    prunedNodes = 0;

    // Run Branch and Bound
    branchAndBound(start);

    // Display result
    printf("\n============================================\n");
    printf("              FINAL RESULT\n");
    printf("============================================\n");

    printf("\nBest Tour:\n");

    for (int i = 0; i <= n; i++)
    {
        printf("%s", city[bestPath[i]]);

        if (i < n)
        {
            printf(" -> ");
        }
    }

    printf("\n\nMinimum Total Cost = %d\n", bestCost);

    printf("\nSearch Statistics:\n");
    printf("--------------------------------------------\n");
    printf("Expanded Nodes : %d\n", expandedNodes);
    printf("Pruned Nodes   : %d\n", prunedNodes);
    printf("--------------------------------------------\n");

    printf("\nBranch and Bound completed successfully.\n");

    return 0;
}

