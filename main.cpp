#include <iostream>
#include <cstring>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <climits>
#include <tuple>

//N>=3
static const int N = 3;

static const char EMPTY = '_';
static const char X = 'x'; //max
static const char O = 'o'; //min
static const char DRAW = 'D';

//used to shuffle successors
static unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
static std::default_random_engine generator (seed);
static std::uniform_int_distribution<int> toss(0,1);


struct State{

    State(bool _Xturn = true):depth(0),Xturn(_Xturn){
        memset(board,'_',N*N);
    }

    char board [N][N];
    int depth; //used in utility function and optimizes terminalState check
    bool Xturn; //helps generate successors

    //generate all possible moves the current player can make
    std::vector<State> getSuccessors(){
        std::vector<State> result;
        result.reserve(N*N-depth);
        for (int row=0;row<N;row++){
            for (int column=0;column<N;column++){
                if (board[row][column]==EMPTY){
                    State newState = *this;
                    newState.board[row][column] = (Xturn ? X : O);
                    newState.depth++;
                    newState.Xturn = !Xturn;
                    result.push_back(newState);
                }
            }
        }

        std::shuffle(result.begin(),result.end(),generator);
        return result;
    }


    //pretty print
    inline friend std::ostream& operator<<(std::ostream& os, const State& given){
        for (int row=0;row<N;row++){
            os<<'|';
            for (int column=0;column<N;column++){
                 os<<given.board[row][column]<<'|';
            }
            os<<std::endl;
        }
        os<<std::endl;
        return os;
    }


    //cheeck for terminal state
    std::pair<bool,char> isTerminalState()const{
        if (depth < ( 2*N - 1)){
            return { false, EMPTY};
        }

        //main
        bool terminalState = true;
        char sample = board[0][0];
        if (sample == EMPTY){
            terminalState = false;
        }
        for (int index = 1;index<N && terminalState;index++){
            if (board[index][index] != sample){
                terminalState = false;
            }
        }
        if (terminalState){
            return {true,sample};
        }

        //secondary
        terminalState = true;
        sample = board[N-1][0];
        if (sample == EMPTY){
            terminalState = false;
        }
        for (int index=1;index<N && terminalState;index++){
            if (board[N-1-index][index] != sample){
                terminalState = false;
            }
        }
        if (terminalState){
            return {true,sample};
        }

        //rows
        terminalState = true;
        for (int row=0;row<N;row++){
            terminalState = true;
            sample = board[row][0];
            if (sample == EMPTY){
                continue;
            }
            for (int column=1;column<N && terminalState;column++){
                if (board[row][column] != sample){
                    terminalState = false;
                }
            }
            if (terminalState){
                return {true,sample};
            }
        }

        //columns
        terminalState = true;
        for (int column=0;column<N;column++){
            terminalState = true;
            sample = board[0][column];
            if (sample == EMPTY){
                continue;
            }
            for (int row=1;row<N && terminalState;row++){
                if (board[row][column] != sample){
                    terminalState = false;
                }
            }
            if (terminalState){
                return {true,sample};
            }
        }

        if (depth == N*N){
            return {true,DRAW};
        }

        return {false,EMPTY};
    }

    int getUtility(char winner){
        if (winner == O){
            return -N*N-1+depth;
        }
        if (winner == X){
            return N*N+1-depth;
        }
        return 0;
    }
};

//forward Declarations
static inline std::pair<int,State> maxValue(State current, int alpha, int beta);
static inline std::pair<int,State> minValue(State current, int alpha, int beta);

//return best move
static inline State alphaBetaSearch(State root,bool maximizingPlayer){
    std::pair<int,State> bestChoice;
    bestChoice = maximizingPlayer ? maxValue(root,INT_MIN,INT_MAX) : minValue(root,INT_MIN,INT_MAX);
    return bestChoice.second;
}

static inline std::pair<int,State> maxValue(State current, int alpha, int beta){
    // if terminal, return
    std::pair<int,char> terminal = current.isTerminalState();
    if (terminal.first){
        return std::pair<int,State> ({current.getUtility(terminal.second),current});
    }
    //store best (max) choice in here
    std::pair<int,State> currentMaxValue (INT_MIN,State());
    //get child States
    std::vector<State> successors =  current.getSuccessors();
    //for each child state
    for (const State& option : successors){
        // get its min value
        std::pair<int,State> child = {minValue(option,alpha,beta).first,option};
        // if if the min value of the child is better than the current best max, get it as best
        if ( child.first > currentMaxValue.first){
            currentMaxValue = child;
        }
        //if current max is more than or equal to beta (upperBound for the upper node (which is a min node)), then cutoff
        //(no need to look at the other options because only the max is important, and the current max is more than beta (upperBound))
        if (currentMaxValue.first >= beta){
            return currentMaxValue;
        }
        //update alpha (lowerBound) if it has been raised
        if (currentMaxValue.first > alpha){
            alpha = currentMaxValue.first;
        }
    }
    return currentMaxValue;
}

static inline std::pair<int,State> minValue(State current, int alpha, int beta){
    // if terminal, return
    std::pair<int,char> terminal = current.isTerminalState();
    if (terminal.first){
        return std::pair<int,State> ({current.getUtility(terminal.second),current});
    }
    //store best (min) choice in here
    std::pair<int,State> currentMinValue (INT_MAX,State());
    //get child States
    std::vector<State> successors =  current.getSuccessors();
    //for each child state
    for (const State& option : successors){
        // get its max value
        std::pair<int,State> child = {maxValue(option,alpha,beta).first,option};
        // if the max value of the child is better than the current best min, get it as best
        if ( child.first < currentMinValue.first){
            currentMinValue = child;
        }
        //if current min is less than or equal to alpha (lowerBound for the upper node (which is a max node)), then cutoff
        //(no need to look at the other options because only the min is important, and the current min is less than alpha (lowerBound))
        if (currentMinValue.first <= alpha){
            return currentMinValue;
        }
        //update beta (upperBound) if it has been lowered
        if (currentMinValue.first < beta){
            beta = currentMinValue.first;
        }
    }
    return currentMinValue;
}

void botVSbot(){
    //ai vs ai
    State s(true);
    bool turn = true;
    std::pair<bool,char> currentResult;
    while(!((currentResult = s.isTerminalState()).first)){
        s = alphaBetaSearch(s,turn);
        std::cout<<s;
        std::cin.get();
        turn = !turn;
    }

    if (currentResult.second == DRAW){
        std::cout<<"Game ends in a draw!\n";
        return;
    }
    turn ? (std::cout<<X<<" wins the game!\n") : (std::cout<<O<<" wins the game!\n");
}

void humanVSbot(){

    //who goes first?
    bool humanIsFirst = toss(generator);
    //who gets sign X ?
    bool humanGetsX = toss(generator);

    int x = 0;
    int y = 0;

    char humanSign = humanGetsX ? X : O;

    humanGetsX ? (std::cout<<"You are with x, bot is with o\n") : (std::cout<<"You are with o, bot is with x\n");
    humanIsFirst ? (std::cout<<"You go first!\n") : (std::cout<<"Bot goes first !\n");

    if (humanIsFirst){
        State s(humanGetsX);
        std::cout<<s;
        std::pair<bool,char> currentResult;
        while(!((currentResult = s.isTerminalState()).first)){
            do{
                std::cout<<"insert row and column of a free square [(x,y), 1<=x<=3, 1<=y<=3]"<<std::endl;
                std::cin>>x>>y;
            }while(s.board[x-1][y-1] != EMPTY || x<1 || x>3 || y<1 ||y >3);
            s.board[x-1][y-1] = humanSign;
            s.depth++;
            std::cout<<s;
            if (!((currentResult = s.isTerminalState()).first)){
                s.Xturn = !s.Xturn;
                s = alphaBetaSearch(s,s.Xturn);
                std::cout<<"Bot move:\n";
                std::cout<<s;
            }
        }

        if (currentResult.second == DRAW){
            std::cout<<"Game ends in a draw!\n";
            return;
        }
        (currentResult.second == X) ? (std::cout<<X<<" wins the game!\n") : (std::cout<<O<<" wins the game!\n");
    }

     if (!humanIsFirst){
        State s(!humanGetsX);
        std::cout<<s;
        std::pair<bool,char> currentResult;
        while(!((currentResult = s.isTerminalState()).first)){
            s = alphaBetaSearch(s,s.Xturn);
            std::cout<<"Bot move:\n";
            std::cout<<s;
            if (!((currentResult = s.isTerminalState()).first)){
                do{
                    std::cout<<"insert row and column of a free square [(x,y), 1<=x<=3, 1<=y<=3]"<<std::endl;
                    std::cin>>x>>y;
                }while(s.board[x-1][y-1] != EMPTY || x<1 || x>3 || y<1 ||y >3);
                s.board[x-1][y-1] = humanSign;
                s.depth++;
                std::cout<<s;
                s.Xturn = !s.Xturn;
            }
        }

        if (currentResult.second == DRAW){
            std::cout<<"Game ends in a draw!\n";
            return;
        }
        (currentResult.second == X) ? (std::cout<<X<<" wins the game!\n") : (std::cout<<O<<" wins the game!\n");
    }


}


int main(const int argc, const char** argv){
    if ( (argc > 1) && ((strcmp((*(argv+1)),"bot"))==0)){
        botVSbot();
        return 0;
    }
    if ( (argc > 1) && ((strcmp((*(argv+1)),"human"))==0)){
        humanVSbot();
        return 0;
    }



    //botVSbot();
    humanVSbot();

    return 0;
}
