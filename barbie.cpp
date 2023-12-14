#include <iostream> 
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
using namespace std;

struct Path {
    vector<int> path;
    int costo ;
    int numberOfLinks;
    bool isOccupied ;
    bool isNull = false; //usata per ritornare nel caso in cui non siano trovati path possibili
};
struct link{
    int nodoTo;
    int costo;
};
struct nodo{
  vector<link> vic;
  bool occupied = false;
  int costDistance = -1;
  int linkDistance = -1;
};

struct minimumPathPassingThroughOccupiedNodes {
    int indiceNodo;
    int costoOccupiedPath = -1;
    int lenghtOccupiedPath = -1;
};


vector<nodo> grafo;
vector<int> parents;
vector<int> minPath; 
vector<minimumPathPassingThroughOccupiedNodes> occupiedNodes;
int C, S, M;

//setta le distanze da 0 a C-1 di grafo globale
void setDistance(  ) ;
void printPath( ofstream&, const vector<int> ) ;
//return true se il path contiene nodi occupied
bool isPathOccupied( const vector<int> ) ;
//inserisce in minpath globale i nodi per cui passa il minPath, usando vettore parents globale
void minimumPath( const int  , const int  ) ;
void printGraph( const vector<nodo> ) ;
Path minimumCostNonOccupiedPath() ;
Path minimumCostPath() ;
Path minimumDistanceNonOccupiedPath() ;
Path minimunDistancePath() ;
vector<int> getPath(const vector<int> , const int  , int  ) ;
void occupiedPath(vector<nodo>);


int main() {
    ifstream input("input.txt") ;
    ofstream output("output.txt") ;

    input >> C >> S ;
    grafo.resize(C) ;
    parents.resize(C) ;
    parents[0] = -1;


    for (int i = 0; i < S ; i++) {
        int from, to, costo;
        input >> from >> to >> costo;
        grafo[from].vic.push_back({ to, costo }) ;
        grafo[to].vic.push_back({from, costo}) ;
    }

    input >> M;
    //se M = 0 vuol dire che non ci sono città occupate quindi printiamo -1 e il minpath
    
    setDistance() ;
    minPath = getPath( parents, 0 , C-1 ) ;
    printPath( output,  minPath ) ;
    if ( M == 0 ) {       // GB aggiunto caso per M=0
        output << "-1" << endl;
        output << minPath.size() << endl;
        printPath( output, minPath ) ;
    } else if(M==C-2) {   //LS aggiunto caso in cui tutte città occupate tranne partenza e arrivo
        output << "-2" << endl;
        output << minPath.size() << endl;
        printPath( output, minPath ) ;
    }else{
        //get dei nodi occupati
        for (int i = 0; i < M; i++) {
            int tmp;
            input >> tmp ;
            grafo[tmp].occupied = true;
            minimumPathPassingThroughOccupiedNodes nodeToAdd;
            nodeToAdd.indiceNodo = tmp;
            
            occupiedNodes.push_back(nodeToAdd);
        }
        
        printGraph( grafo ) ;

        

        //Path path = minimumCostNonOccupiedPath();  

        

        

        if ( isPathOccupied(minPath) ) { //caso in cui non abbiamo trovato un cammino minimo non occupato
            output << "-2" << endl;
            output << minPath.size() << endl;
            printPath( output, minPath ) ;
        } else { //abbiamo trovato un minPath non occupato
            int k = 100000*C;
            occupiedPath(grafo);
            // k = (OccupiedCost - minimumPathCost) / (minimumPathLenght - OccupiedLenght)
            for(minimumPathPassingThroughOccupiedNodes pathToConsider : occupiedNodes)
            {
                /* cout << " i : " << pathToConsider.indiceNodo
                     << "costoOccupiedPath: " << pathToConsider.costoOccupiedPath 
                     << "  path.costo: " << grafo[C-1].costDistance
                     << "  path.numberOfLinks: " << grafo[C-1].linkDistance
                     << "  lenghtOccupiedPath: " << pathToConsider.lenghtOccupiedPath << endl; */

                int tmp = (pathToConsider.costoOccupiedPath - grafo[C-1].costDistance)/(grafo[C-1].linkDistance - pathToConsider.lenghtOccupiedPath);

                if(tmp > 0)
                {
                    k = min(k,tmp);
                }
                
            }
            output << k-1 << endl;
            output << minPath.size() << endl;
            printPath( output, minPath ) ;
        }
    }



    return 0;
}
Path minimunDistancePath() {
    vector<int> localParent;
    localParent.resize(C) ;

    queue<int> q;
    grafo[0].costDistance = 0;
    grafo[0].linkDistance = 0;
    q.push(0) ;
    while ( !q.empty() ) {
        int nodoAtt = q.front();
        q.pop();
        for( link linkVic : grafo[nodoAtt].vic ) {
            int nodoVic = linkVic.nodoTo;
            int costo = linkVic.costo;
            if ( (grafo[nodoVic].linkDistance == -1 || 
                  grafo[nodoVic].linkDistance > grafo[nodoAtt].linkDistance + 1) 
                ) {
                grafo[nodoVic].costDistance = grafo[nodoAtt].costDistance + costo;
                grafo[nodoVic].linkDistance = grafo[nodoAtt].linkDistance + 1;
                localParent[nodoVic] = nodoAtt;
                q.push(nodoVic) ;
            }
        }
    }
    vector<int> path;
    path = getPath( localParent, 0, C-1 ) ;
    Path returnPath ; // isNull è a false di default
    returnPath.path = path;
    returnPath.costo = grafo[C-1].costDistance;
    returnPath.numberOfLinks = grafo[C-1].linkDistance;
    returnPath.isOccupied = isPathOccupied( path ) ;
    return returnPath;
}

Path minimumDistanceNonOccupiedPath() {
    vector<int> localParent;
    localParent.resize(C) ;

    queue<int> q;
    grafo[0].costDistance = 0;
    grafo[0].linkDistance = 0;
    grafo[C-1].linkDistance = -1; //per sapere se non troveremo path possibili
    q.push(0) ;
    while ( !q.empty() ) {
        int nodoAtt = q.front();
        q.pop();
        for( link linkVic : grafo[nodoAtt].vic ) {
            int nodoVic = linkVic.nodoTo;
            int costo = linkVic.costo;
            if ( (grafo[nodoVic].linkDistance == -1 || 
                  grafo[nodoVic].linkDistance > grafo[nodoAtt].linkDistance + 1) && 
                  !grafo[nodoVic].occupied  //ricerco solo nodi non occupati
                ) {
                grafo[nodoVic].costDistance = grafo[nodoAtt].costDistance + costo;
                grafo[nodoVic].linkDistance = grafo[nodoAtt].linkDistance + 1;
                localParent[nodoVic] = nodoAtt;
                q.push(nodoVic) ;
            }
        }
    }
    if ( grafo[C-1].linkDistance == -1 ) { //non esiste path non occupato
        Path returnPath ;
        returnPath.isNull = true;
        return returnPath ;
    } else { //esiste path non occupato
        vector<int> path;
        path = getPath( localParent, 0, C-1 ) ;
        Path returnPath ; // isNull è a false di default
        returnPath.path = path;
        returnPath.costo = grafo[C-1].costDistance;
        returnPath.numberOfLinks = grafo[C-1].linkDistance;
        returnPath.isOccupied = isPathOccupied( path ) ;
        return returnPath;
    }
}
Path minimumCostPath() {
    vector<int> localParent;
    localParent.resize(C) ;

    queue<int> q;
    grafo[0].costDistance = 0;
    grafo[0].linkDistance = 0;
    q.push(0) ;
    while ( !q.empty() ) {
        int nodoAtt = q.front();
        q.pop();
        for( link linkVic : grafo[nodoAtt].vic ) {
            int nodoVic = linkVic.nodoTo;
            int costo = linkVic.costo;
            if ( (grafo[nodoVic].costDistance == -1 || 
                  grafo[nodoVic].costDistance > grafo[nodoAtt].costDistance + costo)
                ) {
                grafo[nodoVic].costDistance = grafo[nodoAtt].costDistance + costo;
                grafo[nodoVic].linkDistance = grafo[nodoAtt].linkDistance + 1 ;
                localParent[nodoVic] = nodoAtt;
                q.push(nodoVic) ;
            }
        }
    }
    vector<int> path;
    path = getPath( localParent, 0, C-1 ) ;
    Path returnPath ; // isNull è a false di default
    returnPath.path = path;
    returnPath.costo = grafo[C-1].costDistance;
    returnPath.numberOfLinks = grafo[C-1].linkDistance;
    returnPath.isOccupied = isPathOccupied( path ) ;
    return returnPath;
}

Path minimumCostNonOccupiedPath() {
    vector<int> localParent;
    localParent.resize(C) ;

    queue<int> q;
    grafo[0].costDistance = 0;
    grafo[0].linkDistance = 0;
    grafo[C-1].costDistance = -1; //per sapere se non troveremo path possibili
    q.push(0) ;
    while ( !q.empty() ) {
        int nodoAtt = q.front();
        q.pop();
        for( link linkVic : grafo[nodoAtt].vic ) {
            int nodoVic = linkVic.nodoTo;
            int costo = linkVic.costo;
            if ( (grafo[nodoVic].costDistance == -1 || 
                  grafo[nodoVic].costDistance > grafo[nodoAtt].costDistance + costo) && 
                  !grafo[nodoVic].occupied  //ricerco solo nodi non occupati
                ) {
                grafo[nodoVic].costDistance = grafo[nodoAtt].costDistance + costo;
                grafo[nodoVic].linkDistance = grafo[nodoAtt].linkDistance + 1;
                localParent[nodoVic] = nodoAtt;
                q.push(nodoVic) ;
            }
        }
    }
    if ( grafo[C-1].costDistance == -1 ) { //non esiste path non occupato
        Path returnPath ;
        returnPath.isNull = true;
        return returnPath ;
    } else { //esiste path non occupato
        vector<int> path;
        path = getPath( localParent, 0, C-1 ) ;
        Path returnPath ; // isNull è a false di default
        returnPath.path = path;
        returnPath.costo = grafo[C-1].costDistance;
        returnPath.numberOfLinks = grafo[C-1].linkDistance;
        returnPath.isOccupied = isPathOccupied( path ) ;
        return returnPath;
    }
}





void setDistance(  )  {
    queue<int> q;
    q.push(0) ;
    grafo[0].costDistance = 0;
    grafo[0].linkDistance = 0;

    while ( !q.empty() ) {
        int nodoAtt = q.front();
        q.pop();
        for( link linkVic : grafo[nodoAtt].vic ) {
            int nodoVic = linkVic.nodoTo;
            int costo = linkVic.costo;
            if ( grafo[nodoVic].costDistance == -1 ||
                 grafo[nodoVic].costDistance > grafo[nodoAtt].costDistance + costo
                ) {
                grafo[nodoVic].linkDistance = grafo[nodoAtt].linkDistance + 1;
                grafo[nodoVic].costDistance = grafo[nodoAtt].costDistance + costo;
                parents[nodoVic] = nodoAtt;
                q.push(nodoVic) ;
            }
        }
    }
}

void occupiedPath(vector<nodo> tmp_grafo)  {

    for (int j = 0; j < occupiedNodes.size(); j++) //minimumPathPassingThroughOccupiedNodes mppton : occupiedNodes
    {
        int i = occupiedNodes[j].indiceNodo;
        queue<int> q;
        q.push(C-1) ;
        

        vector<int> distances(C,-1);
        vector<int> costs(C,0);
        distances[C-1] = 0;
        costs[C-1] = 0;

        while ( !q.empty() ) {
            int nodoAtt = q.front();
            q.pop();
            for( link linkVic : tmp_grafo[nodoAtt].vic ) {

                
                int nodoVic = linkVic.nodoTo;
                int costo = linkVic.costo;
                if ( (distances[nodoVic] == -1 ||
                    costs[nodoVic] > costs[nodoAtt] + costo) &&
                    nodoVic != 0
                    ) {
                    distances[nodoVic] = distances[nodoAtt] + 1;
                    costs[nodoVic] = costs[nodoAtt] + costo;
                    
                    //if(nodoVic == i) break;

                    q.push(nodoVic) ;
                }
            }
        }
        occupiedNodes[j].costoOccupiedPath = costs[i] + grafo[i].costDistance;
        occupiedNodes[j].lenghtOccupiedPath = distances[i] + grafo[i].linkDistance;

    }    
}




bool isPathOccupied( const vector<int> path ) {
    for( int i : path ) {
        if ( grafo[i].occupied ) {
            return true;
        }
    }
    return false;
}

void minimumPath( const int start , const int stop ) {
    if( start == stop ) {
        minPath.push_back( stop ) ;
    } else {
        minimumPath( start, parents[stop] ) ;
        minPath.push_back( stop ) ;
    }
}

//dato un vettore di parent, costruisce il path
vector<int> getPath(const vector<int> localParent, const int start , int stop ) {
    vector<int> path;
    stack<int> s;
    do {
        s.push( stop ) ;
        stop = localParent[stop] ;
    } while ( stop != start ) ;
    while ( !s.empty() ) {
        path.push_back(s.top()) ;
        s.pop();
    }
    return path;
}

void printPath( ofstream &output, const vector<int> path ) {
    for ( int i = 0; i < path.size(); i++ ) {
        //cout << path[i] << " " ;
        output << path[i] << " " ;
    }
}

//GB stampa il nodo, se è occupato un asterisco, e tutti i vicini con il costo
void printGraph( const vector<nodo> grafo ) {
    for ( int i = 0; i < grafo.size() ; i++ ) {
        cout << "i: " << i ;
        if ( grafo[i].occupied ) {
            cout << "*" ;
        }
        for ( link l : grafo[i].vic ) {
            cout << " --> (" << l.nodoTo << " - " << l.costo << ")" ;
        }
        cout << endl;
    }
}


// Casi di test:
//In 6 casi su 20, tutti gli archi hanno lo stesso tempo di percorrenza;
//In 4 casi su 20, o nessuna città è occupata, o tutte le città sono occupate (B ed A escluse) 

