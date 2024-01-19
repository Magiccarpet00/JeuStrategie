#include <iostream>
#include <cmath>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include <list>
#include <map>
#include <iterator>

//taille du board 
#define BOARD_WIDTH 40
#define BOARD_HEIGHT 12

//le décalage du plateau
#define BOARD_OFFSET_X 10
#define BOARD_OFFSET_Y 4

using namespace std;

struct Cursor{
    int x,y;
}cursor;

class Player{
    private:
        int gold;
        int nbCastle;

    public:
        Player()
        {
            this->gold = 20;
            this->nbCastle = 1;
        }

        int modifGold(int amount)
        {
            int tmpGold = this->gold;
            tmpGold += amount;
            if(tmpGold < 0)
                return 0;
            else
                this->gold = tmpGold;
            return 1;
        }

        int getGold() {return this->gold;}

};

Player* playerRed = new Player();
Player* playerBlue = new Player();

Player* players[2] = {playerRed, playerBlue};
int idPlayer = 0;
Player* currentPlayer = players[idPlayer];

void nextTurn();
void draw();
int isSelectPawn();


enum ColorEffect {
    FRONT_RED,
    FRONT_BLUE,
    FRONT_DEFAULT,
    FRONT_GOLD,
    CURSOR,
    CURSOR_SELECTED,
    END_CURSOR_SELECTED,

    OPEN,
    M,
    ADD,
    END
};

enum TypePawn {
    CASTLE,
    FARMER,
    WARRIOR,
    KING
};

void createPawn(int x, int y, TypePawn typePawn, Player* owner);

static string debugBuffer;

static std::map<ColorEffect, string> fx;

void createFxMap()
{
    fx[OPEN] = "\033[";
    fx[M] = "m";
    fx[ADD] = ";";
    fx[FRONT_RED] = "31";
    fx[FRONT_BLUE] = "34";
    fx[FRONT_DEFAULT] = "39";
    fx[FRONT_GOLD] = "33";
    fx[CURSOR] = "47";
    fx[CURSOR_SELECTED] = "5";
    fx[END_CURSOR_SELECTED] = "25";
    fx[END] = "\033[0m";
}



//CELL
class Cell
{
    private :
        int x,y;
        string spr;
        string renderer;
        ColorEffect color;
    public:
        Cell(int _x,int _y,string _spr)
        {
            x = _x;
            y = _y;
            spr =_spr;
            color = FRONT_DEFAULT;
            updateRenderer();
        }

        void updateRenderer()
        {
            string s = "";
            int nbEffect = 0;
            if(isCursorSelected())
            {   
                nbEffect +=1;
                s += fx[OPEN];
                s += fx[CURSOR];

                if(isSelectPawn() == 1)
                {
                    nbEffect +=1;
                    s += fx[ADD];
                    s += fx[CURSOR_SELECTED];
                }

            }

            if(this->color == FRONT_RED)
            {
                if(nbEffect == 0) 
                    s += fx[OPEN];
                else        
                    s += fx[ADD];

                nbEffect +=1;
                s += fx[FRONT_RED];
            }
            
            if(this->color == FRONT_BLUE)
            {
                if(nbEffect == 0) 
                    s += fx[OPEN];
                else        
                    s += fx[ADD];

                nbEffect +=1;
                s += fx[FRONT_BLUE];
            }
            
            if(this->color == FRONT_DEFAULT)
            {
                if(nbEffect == 0) 
                    s += fx[OPEN];
                else        
                    s += fx[ADD];
                    
                nbEffect +=1;
                s += fx[FRONT_DEFAULT];
            }

            if(nbEffect != 0)
                s += fx[M];

            s += spr;

            if(isCursorSelected())
            {
                
            }
            s+= fx[END];
            this->renderer = s;
        }

        bool isCursorSelected()
        {
            if(x == cursor.x && y == cursor.y)
                return true;
            else
                return false;
        }

        int getX()
        {
            return x;
        }

        int getY()
        {
            return y;
        }

        string getRenderer()
        {
            return renderer;
        }

        void setSprite(string s)
        {
            spr = s;
        }

        void setColor(ColorEffect color)
        {
            this->color = color;
        }

        Cell* getCell()
        {
            return this;
        }
        
};

static std::list<Cell> board;
static int nbTurn = 0;

Cell* getCell(int x, int y)
{
    for (list<Cell>::iterator i = board.begin(); i != board.end(); ++i) 
    {
        if(i->getX() == x && i->getY() == y)
        {
            return i->getCell();
        }
    }   
    return NULL;
}

class Pawn
{
    protected:
        //GENERAL
        int x,y;
        string spr;
        Cell* currentCell;
        ColorEffect color;
        Player* owner;

        //GAMEPLAY
        int power;
        int max_health;
        int current_health;
        int max_move;
        int move;
        int prod;
        int cost;
        int atk;

    public:
        Pawn(int _x, int _y, string _spr, Player* owner)
        {   
            x = _x;
            y = _y;
            spr = _spr;
            atk = 1;
            currentCell = getCell(x,y);
            this->owner = owner;
            if(owner == playerRed) color = FRONT_RED;
            if(owner == playerBlue) color = FRONT_BLUE;
        }

        void updateSprite()
        {
            currentCell->setSprite(spr);
            currentCell->setColor(color);
        }
        
        bool isCursorSelected()
        {
            if(x == cursor.x && y == cursor.y)
                return true;
            else
                return false;
        }

        void moveAttack(int _x, int _y);
        void die();

        void getDamage(int amount)
        {
            this->current_health -= amount;
            if(this->current_health <= 0) die();
        }

        virtual void refresh()
        {
            this->move = this->max_move;
            this->atk = 1;
        }

        int getX() {return x;}
        int getY() {return y;}
        int getPower() {return power;}
        int getMax_health() {return max_health;}
        int getCurrent_health() {return current_health;}
        int getMaxMoveSpeed() {return max_move;}
        int getMoveSpeed() {return move;}
        int getProd() {return prod;}
        int getCost() {return cost;}
        Player* getOwner() {return owner;}
        virtual string getName() = 0;

        virtual void framing()
        {
            return;
        }

        virtual void createUnits(TypePawn typePawn)
        {
            return;
        }

        virtual void transform()
        {
            return;
        }

        virtual void autoProd()
        {
            return;
        }
        
};

static std::list<Pawn*> panws;
static Pawn* selectedPawn = NULL;

Pawn* selectPawn(int x, int y)
{
    for (auto const& i : panws) 
        if(i->getX() == x && i->getY() == y)
            return i;

    return NULL;
}

int isSelectPawn()
{
    if(selectedPawn == NULL)
        return 0;
    else
        return 1;
}

//Move and Attack
void Pawn::moveAttack(int _x, int _y)
{
    if(currentPlayer == owner) 
    {
        if(move > 0 && selectPawn(x+_x, y+_y) == NULL)
        {
            //MOVE
            currentCell->setColor(FRONT_DEFAULT);
            x += _x;
            y += _y;
            currentCell = getCell(x,y);
            move--;
            cursor.x = x;
            cursor.y = y;
        }
        else
        {
            //ATTACK
            Pawn* pawnToAttack = selectPawn(x+_x, y+_y);
            if(pawnToAttack != NULL && 
            pawnToAttack->getOwner() != this->owner &&
            atk == 1)
            {
                atk--;
                pawnToAttack->getDamage(this->power);
            }
        }   
    }
}

void Pawn::die()
{
    Pawn* pawnToDie = selectPawn(x,y);
    panws.remove(pawnToDie);
    delete pawnToDie;
}

class Warrior : public Pawn
{
    private:

    public:
        Warrior(int _x, int _y, Player* owner) : Pawn(_x, _y, "G", owner)
        {
            this->power = 5;
            this->max_health = 10;
            this->current_health = max_health;
            this->max_move = 3;
            this->move = max_move;
            this->prod = 0;
            this->cost = 10;
        }

        string getName()
        {
            return "GUERRIER";
        }
};

class Farmer : public Pawn 
{
    private:
        int nbFarm;

    public:
        Farmer(int _x, int _y, Player* owner) : Pawn(_x, _y, "P", owner)
        {
            this->power = 0;
            this->max_health = 1;
            this->current_health = max_health;
            this->max_move = 2;
            this->move = max_move;
            this->prod = 5;
            this->cost = 20;
            this->nbFarm = 1;
        }

        string getName()
        {
            return "PAYSAN";
        }

        void framing()
        {
            if(this->nbFarm > 0) {
                this->nbFarm--;
                this->getOwner()->modifGold(this->prod);
            }
        }

        virtual void refresh() override
        {
            this->move = this->max_move;
            this->atk = 1;
            this->nbFarm =1;
        }
};

class Castle : public Pawn
{
    private:

    public:
        Castle(int _x, int _y, Player* owner) : Pawn(_x, _y, "C", owner)
        {
            this->power = 0;
            this->max_health = 20;
            this->current_health = max_health;
            this->max_move = 0;
            this->move = max_move;
            this->prod = 2;
            this->cost = 15;
        }

        virtual void createUnits(TypePawn typePawn) override
        {

            int cost;
            switch (typePawn)
            {
                case WARRIOR:
                    cost = 10;
                    break;
                case FARMER:
                    cost = 20;
                    break;
                case KING:
                    cost = 10;
                    break;
            }

            for(float i = 0; i< M_PI*2;i+=M_PI_2)
            {
                int x = (int)sin(i) + this->x;
                int y = (int)cos(i) + this->y;

                if(selectPawn(x,y) == NULL && owner->modifGold(-cost))
                {
                    createPawn(x,y,typePawn,this->owner);
                    return;
                }
            }
        }

        string getName()
        {
            return "CHATEAU";
        }

        void autoProd()
        {
            owner->modifGold(this->prod);
        }
};

class King : public Pawn
{
    private:

    public:
        King(int _x, int _y, Player* owner) : Pawn(_x, _y, "S", owner)
        {
            this->power = 3;
            this->max_health = 5;
            this->current_health = max_health;
            this->max_move = 1;
            this->move = max_move;
            this->prod = 0;
            this->cost = 10;
        }

        void transform()
        {
            if(owner->modifGold(-15))
            {
                getDamage(99);
                createPawn(x,y,CASTLE, owner);
                selectedPawn = NULL;
            }
        }

        string getName()
        {
            return "SEIGNEUR";
        }


};


//GAME
void updateHUD_createPawn();

void updateHUD()
{
    //INFO Player
    cout << "\033[1;1H" + fx[END];
    if(idPlayer == 0) 
        cout <<  fx[OPEN] + fx[FRONT_RED] + fx[M] + "TOUR DE ROUGE" + fx[END];
    if(idPlayer == 1)
        cout <<  fx[OPEN] + fx[FRONT_BLUE] + fx[M] + "TOUR DE BLEU" + fx[END];
    cout <<  fx[OPEN] + fx[FRONT_GOLD] + fx[M] + "\tOr: " + to_string(currentPlayer->getGold()) + fx[END];

    //INFO selected Pawn
    cout << "\033[2;1H" + fx[END];
    for (auto const& i : panws) {

        if(i->isCursorSelected())
        {

            if(selectedPawn != NULL) cout << "SELECT[";

            cout <<
            i->getName() <<
            "   Pv:" << 
            to_string(i->getCurrent_health()) <<
            "/" << 
            to_string(i->getMax_health()) <<
            "   Depl:" <<
            to_string(i->getMoveSpeed()) <<
            "/" << 
            to_string(i->getMaxMoveSpeed());

            if(selectedPawn != NULL) cout << "]";
        }
    }

    //INFO action
    int h = BOARD_HEIGHT + BOARD_OFFSET_Y+1;
    cout << "\033[" <<h<< ";1H" + fx[END];

    if(selectedPawn != NULL &&
       selectedPawn->getName() == "CHATEAU" && 
       selectedPawn->getOwner() == currentPlayer)
    {
        cout << "g: GUERRIER";
        cout << fx[OPEN] + fx[FRONT_GOLD] + fx[M] + " (10 or)  " +fx[END];
        cout << "p: PAYSAN";
        cout << fx[OPEN] + fx[FRONT_GOLD] + fx[M] + " (20 or)  " +fx[END];
        cout << "s: SEIGNEUR";
        cout << fx[OPEN] + fx[FRONT_GOLD] + fx[M] + " (10 or)  " +fx[END];
    }

    else if(selectedPawn != NULL &&
            selectedPawn->getName() == "PAYSAN" && 
            selectedPawn->getOwner() == currentPlayer)
    {
        cout << "p: pass     *: select     r: recolte";
        cout << fx[OPEN] + fx[FRONT_GOLD] + fx[M] + " (+5 or) " +fx[END];
    }

    else if (selectedPawn != NULL &&
             selectedPawn->getName() == "SEIGNEUR" && 
             selectedPawn->getOwner() == currentPlayer)
    {
        cout << "p: pass     *: select     t: transform";
        cout << fx[OPEN] + fx[FRONT_GOLD] + fx[M] + " (15 or) " +fx[END];
    }

    else
    {
        cout << "p: pass     *: select";
    }
}

void updateHUD_createPawn()
{
    

}

void updateMatix()
{   
    for (list<Cell>::iterator i = board.begin(); i != board.end(); ++i) 
    {
        i->setSprite(".");
    }

    for (auto const& i : panws) {
        i->updateSprite();
    }

    for (list<Cell>::iterator i = board.begin(); i != board.end(); ++i) 
    {
        i->updateRenderer();
    }
}

void draw()
{
    cout << "\033" << "[2J";
    for (Cell c : board) 
    {
        cout << "\033" << "[" << to_string(c.getY()+BOARD_OFFSET_Y) << ";" <<  to_string(c.getX() +BOARD_OFFSET_X) << "H";
        cout << c.getRenderer();
    }
}

void debug()
{
    cout << "\033[3;1H" + fx[END];

    cout << "DEBUG : " << debugBuffer;
    debugBuffer.clear();

}



//INPUT
void inputGlobal();
void inputCastle();
void inputKing();

void input()
{
    if(    selectedPawn != NULL
        && selectedPawn->getName() == "CHATEAU" 
        && selectedPawn->getOwner() == currentPlayer)
    {
        inputCastle();
    }
    else
    {
        inputGlobal();
    }
}

void inputGlobal()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) 
    {
        switch (c)
        {
        // DEPLACEMENT DU CURSEUR
        case 'z':
            if(selectedPawn != NULL)
                selectedPawn->moveAttack(0,-1);
            else
                cursor.y -= 1;
            break;

        case 'd':
            if(selectedPawn != NULL)
                selectedPawn->moveAttack(1,0);
            else = getchar()) != '\n'
                cursor.x += 1;
            break;

        case 's':
            if(selectedPawn != NULL)
                selectedPawn->moveAttack(0,1);
            else
                cursor.y += 1;
            break;

        case 'q':
            if(selectedPawn != NULL)
                selectedPawn->moveAttack(-1,0);
            else
                cursor.x -= 1;
            break;

        // SELECTION SUR CURSEUR
        case '*':
            if(selectPawn(cursor.x, cursor.y) != NULL )
            {
                if(selectedPawn == NULL)
                    selectedPawn = selectPawn(cursor.x, cursor.y);
                else
                    selectedPawn = NULL;
            }
            break;

        case 'p':
            nextTurn();
            break;

        case 'r':
            if(selectedPawn != NULL)
                selectedPawn->framing();
            break;

        case 't':
            if(selectedPawn != NULL)
                selectedPawn->transform();
            break;

        default:
            break;
        }
    }
}

void inputCastle()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        switch(c)
        {
            case 'g':
                selectedPawn->createUnits(WARRIOR);
                break;
                
            case 'p':
                selectedPawn->createUnits(FARMER);
                break;

            case 's':
                selectedPawn->createUnits(KING);
                break;

            case '*':
                selectedPawn = NULL;
                break;
        }
    }
}


    

void gameLoop()
{
    while(1)
    {
        nbTurn++;
        updateMatix();
        draw();
        updateHUD();
        // debug();    
        input();
    }
}

int setUpShell()
{
    struct termios term, term_orig;

    if(tcgetattr(0, &term_orig)) {
        printf("tcgetattr failed\n");
        return -1;
    }
    
    term = term_orig;
    term.c_lflag &= ~ICANON;
    term.c_lflag &= ~ECHO;

    if (tcsetattr(0, TCSANOW, &term)) {
        printf("tcsetattr failed\n");
        return 1;
    }

    return 0;
}

void createBoard()
{
    for (int i = 0; i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < BOARD_WIDTH; j++)
        {
            Cell newCell (j,i,".");
            board.push_back(newCell);
        }
    }
}

void createPawn()
{
    // Pawn newPawn(3,3, "G", FRONT_RED);
    // panws.push_back(newPawn);
    Pawn* newWarrior = new Warrior(3,3,playerRed);
    panws.push_back(newWarrior);

    Pawn* newWarrior2 = new Warrior(6,3,playerBlue);
    panws.push_back(newWarrior2);

    Pawn* newWarrior3 = new Warrior(2,1,playerRed);
    panws.push_back(newWarrior3);

    Pawn* newFarmer = new Farmer(2,3,playerRed);
    panws.push_back(newFarmer);

    Pawn* newCastle = new Castle(4,1,playerRed);
    panws.push_back(newCastle);

    Pawn* newKing = new King(4,2,playerRed);
    panws.push_back(newKing);
}

void createPawn(int x, int y, TypePawn typePawn, Player* owner)
{
    Pawn* newPawn;
    switch (typePawn)
    {
    case WARRIOR:
        newPawn = new Warrior(x,y,owner);
        break;

    case FARMER:
        newPawn = new Farmer(x,y,owner);
        break;

    case CASTLE:
        newPawn = new Castle(x,y,owner);
        break; 
    
    case KING:
        newPawn = new King(x,y,owner);
        break;
    }
    panws.push_back(newPawn);
}

void nextTurn()
{

    idPlayer = (idPlayer+1)%2;
    currentPlayer = players[idPlayer];
    selectedPawn = NULL;

    for (auto const& i : panws) 
        if(i->getOwner() == currentPlayer)
        {
            i->refresh();
            i->autoProd(); 
        }
}

int main()
{
    cursor.x = 0;
    cursor.y = 0;

    createFxMap();
    setUpShell();
    createBoard();
    createPawn();
    gameLoop();
    return 0;
}
