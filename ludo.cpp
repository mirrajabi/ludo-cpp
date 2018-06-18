#include <iostream>
#include <conio.h>
#include <windows.h>
#include "rlutil.h"
#include <sstream>
#include <string>
#include <time.h>
#include <fstream>
//#include "stdafx.h"

using namespace std;
using namespace rlutil;

bool _finished = false;
string _winner;

bool _prepared = false;
int _preparationPass = 0;

enum Turns {PLAYER_ONE, PLAYER_TWO};
Turns _currentTurn;

const int MAP_SIZE = 11;
const int UPDATE_INTERVAL = 1000;

//Classes
class Piece;
class Player;

//User Defined
const char BLOCK = 'O';
const char PIECE_PLAYER_ONE = 'A';
const char PIECE_PLAYER_TWO = 'B';

//Methods
void Draw(char [MAP_SIZE][MAP_SIZE]);
void DrawShowOffScreen();
void Update();
void FixWindow();
void SetCursorPosition(short x,short y);
void RollDice();
void ClearMap();
void RepaintMap();
Player *GetPlayer(int id);
void LogText( const string &text );

void Log();
void ClearLogsText();
void AddLog(string log);

void InitiatePath();
COORD InitiateNode(int x,int y);

string map[MAP_SIZE][MAP_SIZE];
COORD Path[40];

COORD Out_A[4];
COORD Out_B[4];

COORD Houses_A[4];
COORD Houses_B[4];

class Piece
{
private :
    COORD _position;
    int _id;
    char _face;
public:
    int Step,Passed;
    void SetId(int id)
    {
        _id = id;
    }
    int GetId()
    {
        return _id;
    }

    void SetFace(char face)
    {
        _face = face;
    }
    char GetFace()
    {
        return _face;
    }

    void SetPosition(int x,int y)
    {
        _position.X = x;
        _position.Y = y;
    }
    COORD GetPosition()
    {
        return _position;
    }
    void SetStep(int step)
    {
        Step = step;
    }
    int GetStep()
    {
        return Step;
    }

    void SetPassed(int passed)
    {
        Passed = passed;
    }
    int GetPassed()
    {
        return Passed;
    }
};

class Player
{
private:
    string _name;
public:
    Player(string name)
    {
        _name = name;
        if(_name == "Player 1")
            Opponent = 2;
        if(_name == "Player 2")
            Opponent = 1;
        InitiatePieces();
    }
    int PiecesInsideCount;
    int PiecesOutsideCount;
    int PiecesInHouseCount;

    Piece PiecesInside[4];
    Piece PiecesOutside[4];
    Piece PiecesInHouse[4];

    int Opponent;

    void AddInsidePiece(Piece piece)
    {
        ReArrangePieces();
        for(int x = 3;x >= 0;x--)
        {
            PiecesInside[x+1] = PiecesInside[x];
        }
        PiecesInside[0] = piece;
    }

    void RemoveInsidePieceAtPosition(int index)
    {
        ReArrangePieces();
        for(int x = index;x < PiecesInsideCount;x++)
        {
            PiecesInside[x] = PiecesInside[x+1];
        }
    }

    void RemoveInsidePieceById(int id)
    {
        int index;
        for(int i = 0;i < 4;i++)
        {
            if(PiecesInside[i].GetId() == id)
                index = i;
        }
        RemoveInsidePieceAtPosition(index);
    }

    void AddOutsidePiece(Piece piece)
    {
        for(int x = 3;x >= 0;x--)
        {
            PiecesOutside[x+1] = PiecesOutside[x];
        }
        PiecesOutside[0] = piece;
    }

    void RemoveOutsidePieceAtPosition(int index)
    {
        for(int x = index;x < 3;x++)
        {
            PiecesOutside[x] = PiecesOutside[x+1];
        }
    }

    void ReArrangePieces()
    {
        Piece PiecesInRow[4];
        for(int i = 0;i<4;i++)
        {
            PiecesInRow[i] = PiecesInside[i];
        }
        for(int pass = 0;pass < 4;pass++)
        {
            for(int x = 0;x < 3;x++)
            {
                if(PiecesInRow[x].Passed < PiecesInRow[x + 1].Passed)
                {
                    Piece tempPiece = PiecesInRow[x];
                    PiecesInRow[x] = PiecesInRow[x+1];
                    PiecesInRow[x + 1] = tempPiece;
                }
            }
        }
        for(int i = 0;i<4;i++)
        {
            PiecesInside[i] = PiecesInRow[i];
        }
    }

    void SetInsidePieces(int piecesCount)
    {
        PiecesInsideCount = piecesCount;
    }
    void SetOutsidePieces(int piecesCount)
    {
        PiecesOutsideCount = piecesCount;
    }
    void SetHousePieces(int piecesCount)
    {
        PiecesInHouseCount = piecesCount;
    }

    void SetName(string name)
    {
        _name = name;
        if(_name == "Player 1")
            Opponent = 2;
        if(_name == "Player 2")
            Opponent = 1;
    }
    string GetName()
    {
        return _name;
    }

    Piece &GetInsidePieceById(int id)
    {
        ReArrangePieces();
        int index;
        for(int i = 0;i<4;i++)
        {
            if(PiecesInside[i].GetId() == id)
                index = i;
        }
        return PiecesInside[index];
    }

    Piece &GetOutsidePieceById(int id)
    {
        int index;
        for(int i = 0;i<PiecesInsideCount;i++)
        {
            if(PiecesOutside[i].GetId() == id)
                index = i;
        }
        return PiecesOutside[index];
    }

    void InitiatePieces()
    {
        //Majbur shodam in kaaro konam =))) vojdaanan c++ is the worst
        Piece emptyPiece;
        emptyPiece.SetFace(' ');
        emptyPiece.SetPassed(-1);
        emptyPiece.SetStep(-1);
        PiecesInside[0] = emptyPiece;
        PiecesInside[1] = emptyPiece;
        PiecesInside[2] = emptyPiece;
        PiecesInside[3] = emptyPiece;

        PiecesInsideCount = 0;
        PiecesOutsideCount = 4;
        PiecesInHouseCount = 0;

        Piece p1;
        Piece p2;
        Piece p3;
        Piece p4;

        p1.Step = -1;
        p2.Step = -1;
        p3.Step = -1;
        p4.Step = -1;

        p1.Passed = -1;
        p2.Passed = -1;
        p3.Passed = -1;
        p4.Passed = -1;

        p1.SetId(1);
        p2.SetId(2);
        p3.SetId(3);
        p4.SetId(4);

        p1.SetPosition(-1,-1);
        p2.SetPosition(-1,-1);
        p3.SetPosition(-1,-1);
        p4.SetPosition(-1,-1);

        if(_name == "Player 1")
        {
            p1.SetFace('A');
            p2.SetFace('B');
            p3.SetFace('C');
            p4.SetFace('D');

        }
        if(_name == "Player 2")
        {
            p1.SetFace('W');
            p2.SetFace('X');
            p3.SetFace('Y');
            p4.SetFace('Z');
        }

        PiecesOutside[0] = p1;
        PiecesOutside[1] = p2;
        PiecesOutside[2] = p3;
        PiecesOutside[3] = p4;
    }

    void ClearOptions()
    {
        SetCursorPosition(75,27);
        cout << "                         ";
        SetCursorPosition(75,29);
        cout << "                         ";
        SetCursorPosition(75,33);
        cout << "                         ";
    }

    void PrintOptions(int dice)
    {
        SetCursorPosition(72,25);
        cout<<"Options : ";
        if(PiecesInsideCount > 0)
        {
            SetCursorPosition(75,27);
            cout<<GetName()<<" : Move? (1)";
            if(dice == 6 && PiecesInsideCount + PiecesInHouseCount < 4)
            {
                bool overflowOnEntrance = false;
                for(int i = 0;i<PiecesInsideCount;i++)
                {
                    if(PiecesInside[i].Passed == 0)
                        overflowOnEntrance = true;
                }
                if(!overflowOnEntrance)
                {
                    SetCursorPosition(75,29);
                    cout<<GetName()<<" : Bring in? (2)";
                }
            }
            SetCursorPosition(75,33);
            cout<<"Your Answer : ";
            int input;
            cin>>input;
            ClearOptions();
            if(input == 1)
            {
                for(int i = 0;i < PiecesInsideCount;i++)
                {
                    SetCursorPosition(75,27 + i);
                    bool overflow = false;
                    for(int j = 0;j< PiecesInsideCount;j++)
                    {
                        if(PiecesInside[i].Step + dice == PiecesInside[j].Step)
                        {
                            overflow = true;
                        }
                    }
                    if(!overflow)
                        cout<<"Move piece " << PiecesInside[i].GetFace() << "? ("<<(i + 1)<<") :";
                    else
                        cout<<"Cant move piece " << PiecesInside[i].GetFace() << ".";
                }
                SetCursorPosition(75,33);
                cout<<"Your Answer : ";
                int index;
                cin>>index;
                MovePiece(index - 1,dice);
            }
            else if(input == 2 && PiecesInsideCount + PiecesInHouseCount < 4)
            {
                BringPieceIn();
            }
        }
        else
        {
            if(dice == 6)
            {
                BringPieceIn();
            }
            else
                AddLog("No legal moves. Passing.");
        }
    }

    void HandleDice(int dice)
    {
        ReArrangePieces();
        PrintOptions(dice);
    }

    void MoveToHouse(int id)
    {
        ReArrangePieces();
        PiecesInHouse[PiecesInHouseCount] = GetInsidePieceById(id);
        if(_name == "Player 1")
            PiecesInHouse[PiecesInHouseCount].SetPosition(Houses_A[PiecesInHouseCount].X,Houses_A[PiecesInHouseCount].Y);
        else
            PiecesInHouse[PiecesInHouseCount].SetPosition(Houses_B[PiecesInHouseCount].X,Houses_B[PiecesInHouseCount].Y);

        ReArrangePieces();
        RemoveInsidePieceById(id);

        PiecesInHouseCount++;
        PiecesInsideCount--;

        if(PiecesInHouseCount == 4)
        {
            //Finish
            _finished = true;
            _winner = GetName();
            AddLog("Game Finished!");
            AddLog("The winner is " + GetName());
        }
    }

    void BringPieceIn()
    {
        int Step;

        if(_name == "Player 1")
            Step = 10;
        if(_name == "Player 2")
            Step = 30;

        PiecesOutside[0].SetPosition(Path[Step].X,Path[Step].Y);
        PiecesOutside[0].Step = Step;
        PiecesOutside[0].Passed = 0;

        AddInsidePiece(PiecesOutside[0]);
        RemoveOutsidePieceAtPosition(0);

        ostringstream ost;
        ost <<"Piece brought in : "<< PiecesInside[0].GetFace();
        AddLog(ost.str());


        for(int i = 0;i<GetPlayer(Opponent)->PiecesInsideCount;i++)
        {
            if(PiecesInside[0].Step == GetPlayer(Opponent)->PiecesInside[i].Step)
            {
                GetPlayer(Opponent)->DropOut(GetPlayer(Opponent)->PiecesInside[i].GetId(),PiecesInside[0].GetFace());
            }
        }

        PiecesInsideCount++;
        PiecesOutsideCount--;
    }

    void MovePiece(int id,int step)
    {
        COORD startPos = Path[PiecesInside[id].Step];

        PiecesInside[id].Step += step;
        PiecesInside[id].Passed +=step;

        if(PiecesInside[id].Step > 39)
        {
            PiecesInside[id].Step = PiecesInside[id].Step - 40;
        }
        if(PiecesInside[id].Passed > 39)
        {
            //Log move
            AddLog(GetName() + " moved to house.");
            MoveToHouse(PiecesInside[id].GetId());
        }
        else
        {
            //Log move
            AddLog(GetName() + " moved : ");
            ostringstream ost;
            ost << PiecesInside[id].GetFace() << " : " << startPos.X << ", " << startPos.Y << " -> " << Path[PiecesInside[id].Step].X << ", " << Path[PiecesInside[id].Step].Y;
            AddLog(ost.str());

            for(int i = 0;i<GetPlayer(Opponent)->PiecesInsideCount;i++)
            {
                if(PiecesInside[id].Step == GetPlayer(Opponent)->PiecesInside[i].Step)
                {
                    GetPlayer(Opponent)->DropOut(GetPlayer(Opponent)->PiecesInside[i].GetId(),PiecesInside[id].GetFace());
                }
            }
            PiecesInside[id].SetPosition(Path[PiecesInside[id].Step].X,Path[PiecesInside[id].Step].Y);
        }
    }

    void DropOut(int id, char hit)
    {
        ostringstream str;
        str<<"Hit : "  << "P"<<3 - Opponent<< ":" <<  hit << " -> "  << "P"<<Opponent<<":" << GetInsidePieceById(id).GetFace();
        AddLog(str.str());

        GetInsidePieceById(id).SetPosition(-1,-1);
        GetInsidePieceById(id).SetPassed(-1);
        GetInsidePieceById(id).SetStep(-1);

        AddOutsidePiece(GetInsidePieceById(id));
        RemoveInsidePieceById(GetInsidePieceById(id).GetId());
        PiecesOutsideCount++;
        PiecesInsideCount--;

        Log();
    }
};

//Players
Player *Player_A;
Player *Player_B;
int _move = 1;
int main()
{
    ClearLogsText();
    AddLog("*Game Started*");
    AddLog("");
    srand((unsigned) time(0));
    Player_A = new Player("Player 1");
    Player_B = new Player("Player 2");
    InitiatePath();

    while(!_prepared)
    {
        DrawShowOffScreen();
    }
    FixWindow();
    while(!_finished)
    {
        ostringstream ost;
        ost<<"Move #" <<_move<<" : ";
        AddLog(ost.str());
        Update();
        LogText("----------------");
        AddLog("");
        _move++;
        Sleep(UPDATE_INTERVAL);
        system("cls");
    }
    system("color 0F");
    string text = "Game has finished!\n\r\rThe winner is : ";
    for(int i = 0;i<text.size();i++)
    {
        Beep(1000,50);
        cout<<text[i];
        if(text[i] == '\n')
            Sleep(1000);
    }
    Sleep(1000);
    for (int i=0; i<_winner.size(); i++)
    {
        Beep (2000 + i*100,100);
        cout<<_winner[i];
    }
    Beep(300,500);
    Sleep(50);
    Beep(300,500);
    Sleep(50);
    Beep(300,500);
    Sleep(50);
    Beep(250,500);
    Sleep(50);
    Beep(350,250);
    Beep(300,500);
    Sleep(50);
    cout<<endl<<endl<<endl<<"Press any key to exit...";
    getch();
    return 0;
}

Player *GetPlayer(int id)
{
    return (id == 1 ? Player_A : Player_B);
}
void DrawShowOffScreen()
{
    system("color 0F");
    string text = "Ludo Game \nMohammad Mirrajabi\nhttp://Mirrajabi.ir";
    for(int i = 0;i<text.size();i++)
    {
        Beep(2000,50);
        cout<<text[i];
        if(text[i] == '\n')
            Sleep(1000);
    }
    cout<<endl<<endl<<endl;
    Sleep(3000);
    setColor(12);
    cout<<"Game will start in...";
    for(int i = 0;i<3;i++)
    {
        cout<<endl<<(3 - i)<<"...";
        Beep(50,200);
        Beep(50,200);
        Sleep(500);
    }
    system("color 07");
    system("cls");
    _prepared = true;
}
void Draw(string map[MAP_SIZE][MAP_SIZE])
{
    for(int i = 1; i<=MAP_SIZE; i++)
    {
        for(int j = 1; j<=MAP_SIZE; j++)
        {

            if(map[i-1][j-1] != "   ")
            {
                if(map[i-1][j-1] != "x")
                {
                    SetCursorPosition((i - 1) * 6 + 1,j * 4 - 1);
                    setColor(15);
                    cout<<" --- ";
                    SetCursorPosition((i - 1) * 6 + 1,j * 4 + 1);
                    setColor(15);
                    cout<<" --- ";
                }
            }
            SetCursorPosition((i - 1) * 6,j * 4);
            if(map[i-1][j-1][0] == BLOCK)
            {
                setColor(15);
                cout<<" | ";
                setColor(2);
                cout<<map[i-1][j-1];
                setColor(15);
                cout<<" | ";
            }
            else
            {
                string x = map[i-1][j-1];
                if(x == "*" || x == "A" || x == "B" || x == "C" || x == "D" || x == "W" || x == "X" || x == "Y" || x == "Z")
                {
                    setColor(14);
                    cout<<" | ";
                    if(x == "A" || x == "B" || x == "C" || x == "D")
                        setColor(12);
                    if(x == "W" || x == "X" || x == "Y" || x == "Z")
                        setColor(14);

                    cout<<map[i-1][j-1];
                    setColor(14);
                    cout<<" | ";
                }
                else if(map[i-1][j-1] == "x")
                {
                }
                else
                    cout<<map[i-1][j-1];
            }
        }
    }
}

string _logs[10];
int _lastLog = 0;
void Log()
{
    for(int y = 0;y < 50;y++)
    {
        setColor(0);
        for(int x = 0;x < 29;x++)
        {
            SetCursorPosition(71 + x,y);
            cout<< "#";
        }
    }
    setColor(14);
    for(int i = 0;i<50;i++)
    {
        SetCursorPosition(70,i);
        cout<< '|';
    }
    for(int i = 0;i<100;i++)
    {
        SetCursorPosition(i,50);
        cout<< '-';
    }
    SetCursorPosition(72,1);
        cout<< "Logs :";
    for(int i = 0;i<=9;i++)
    {
        SetCursorPosition(75,(i + 2)*2);
            cout<<_logs[i];
    }
}
void AddLog(string log)
{
    LogText(log);
    if(_lastLog <= 9)
    {
        _logs[_lastLog] = log;
        _lastLog++;
    }
    else
    {
        for(int i = 0;i<9;i++)
        {
            _logs[i] = _logs[i + 1];
        }
        _logs[9] = log;
    }
}

void ClearLogsText()
{
    ofstream ofs;
    ofs.open("log.txt", ofstream::out | ofstream::trunc);
    ofs.close();
}

string basemap[11][11];
void ClearMap()
{
    for(int i = 0; i<MAP_SIZE; i++)
    {
        for(int j = 0; j<MAP_SIZE; j++)
            if((i == 4 || i==6 || j==4 || j==6) && ( j != 5 && i != 5))
                basemap[i][j] = BLOCK;
            else if((i == 0 && j == 5) || (i == MAP_SIZE - 1 && j == 5) || (i == 5 && j == MAP_SIZE - 1) || (i == 5 && j == 0))
                basemap[i][j] = BLOCK;
            else if(i == 5 && j != 5)
                basemap[i][j] = 'x';
            else
                basemap[i][j] = "   ";
    }
}

void RepaintMap()
{
    ClearMap();
    for(int i = 0;i<MAP_SIZE;i++)
    {
        for(int j = 0;j<MAP_SIZE;j++)
        {
            map[i][j] = basemap[i][j];
        }
    }
    //Inside Pieces
    for(int i = 0;i<Player_A->PiecesInsideCount;i++)
    {
        if(Player_A->PiecesInside[i].GetPosition().X != -1)
            map[Player_A->PiecesInside[i].GetPosition().X][Player_A->PiecesInside[i].GetPosition().Y] = Player_A->PiecesInside[i].GetFace();
    }
    for(int i = 0;i<Player_B->PiecesInsideCount;i++)
    {
        if(Player_B->PiecesInside[i].GetPosition().X != -1)
            map[Player_B->PiecesInside[i].GetPosition().X][Player_B->PiecesInside[i].GetPosition().Y] = Player_B->PiecesInside[i].GetFace();
    }

    //Pieces in house
    for(int i = 0;i<Player_A->PiecesInHouseCount;i++)
    {
        if(Player_A->PiecesInHouse[i].GetPosition().X != -1)
            map[Player_A->PiecesInHouse[i].GetPosition().X][Player_A->PiecesInHouse[i].GetPosition().Y] = Player_A->PiecesInHouse[i].GetFace();
    }
    for(int i = 0;i<Player_B->PiecesInHouseCount;i++)
    {
        if(Player_B->PiecesInHouse[i].GetPosition().X != -1)
            map[Player_B->PiecesInHouse[i].GetPosition().X][Player_B->PiecesInHouse[i].GetPosition().Y] = Player_B->PiecesInHouse[i].GetFace();
    }

    //Pieces outside
    for(int i = 0;i<Player_A->PiecesOutsideCount;i++)
    {
        Player_A->PiecesOutside[i].SetPosition(Out_A[i].X,Out_A[i].Y);
        map[Player_A->PiecesOutside[i].GetPosition().X][Player_A->PiecesOutside[i].GetPosition().Y] = Player_A->PiecesOutside[i].GetFace();
    }
    for(int i = 0;i<Player_B->PiecesOutsideCount;i++)
    {
        Player_B->PiecesOutside[i].SetPosition(Out_B[i].X,Out_B[i].Y);
        map[Player_B->PiecesOutside[i].GetPosition().X][Player_B->PiecesOutside[i].GetPosition().Y] = Player_B->PiecesOutside[i].GetFace();
    }

    Draw(map);
}

void Update()
{
    RepaintMap();
    RollDice();
    Log();
}
void RollDice()
{
    int dice = (rand() % 6) + 1;

    //Temporary cheat :D
    /*
    SetCursorPosition(0,54);
    cout<<"Dice must roll : ";
    cin>>dice;*/

    ostringstream stream;
    stream<<"Dice rolled " <<dice << "!";
    switch(_currentTurn)
    {
    case PLAYER_ONE:
        AddLog("Player 1's Turn.");
        AddLog(stream.str());
        Log();
        Player_A->HandleDice(dice);
        if(dice != 6)
            _currentTurn = PLAYER_TWO;
        break;
    case PLAYER_TWO:
        AddLog("Player 2's Turn.");
        AddLog(stream.str());
        Log();
        Player_B->HandleDice(dice);
        if(dice != 6)
            _currentTurn = PLAYER_ONE;
        break;
    }
    if(dice == 6)
        AddLog("Bonus!");
}
void FixWindow()
{
    SetConsoleTitle("Ludo");
    system("color 17");
}

void SetCursorPosition(short x, short y)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = { x, y };

    SetConsoleCursorPosition(hStdout, position);
}

void LogText( const string &text )
{
    ofstream logFile(
        "log.txt", ios_base::out | ios_base::app );
    logFile << text << endl;
}

void InitiatePath()
{
    Path[0] = InitiateNode(0,4);
    Path[1] = InitiateNode(1,4);
    Path[2] = InitiateNode(2,4);
    Path[3] = InitiateNode(3,4);
    Path[4] = InitiateNode(4,4);
    Path[5] = InitiateNode(4,3);
    Path[6] = InitiateNode(4,2);
    Path[7] = InitiateNode(4,1);
    Path[8] = InitiateNode(4,0);
    Path[9] = InitiateNode(5,0);
    Path[10] = InitiateNode(6,0);//Player 1 start position
    Path[11] = InitiateNode(6,1);
    Path[12] = InitiateNode(6,2);
    Path[13] = InitiateNode(6,3);
    Path[14] = InitiateNode(6,4);
    Path[15] = InitiateNode(7,4);
    Path[16] = InitiateNode(8,4);
    Path[17] = InitiateNode(9,4);
    Path[18] = InitiateNode(10,4);
    Path[19] = InitiateNode(10,5);
    Path[20] = InitiateNode(10,6);
    Path[21] = InitiateNode(9,6);
    Path[22] = InitiateNode(8,6);
    Path[23] = InitiateNode(7,6);
    Path[24] = InitiateNode(6,6);
    Path[25] = InitiateNode(6,7);
    Path[26] = InitiateNode(6,8);
    Path[27] = InitiateNode(6,9);
    Path[28] = InitiateNode(6,10);
    Path[29] = InitiateNode(5,10);
    Path[30] = InitiateNode(4,10);//Player 2 start position
    Path[31] = InitiateNode(4,9);
    Path[32] = InitiateNode(4,8);
    Path[33] = InitiateNode(4,7);
    Path[34] = InitiateNode(4,6);
    Path[35] = InitiateNode(3,6);
    Path[36] = InitiateNode(2,6);
    Path[37] = InitiateNode(1,6);
    Path[38] = InitiateNode(0,6);
    Path[39] = InitiateNode(0,5);

    Out_A[0] = InitiateNode(8,1);
    Out_A[1] = InitiateNode(9,1);
    Out_A[2] = InitiateNode(8,2);
    Out_A[3] = InitiateNode(9,2);

    Out_B[0] = InitiateNode(1,8);
    Out_B[1] = InitiateNode(2,8);
    Out_B[2] = InitiateNode(1,9);
    Out_B[3] = InitiateNode(2,9);

    Houses_A[0] = InitiateNode(5,1);
    Houses_A[1] = InitiateNode(5,2);
    Houses_A[2] = InitiateNode(5,3);
    Houses_A[3] = InitiateNode(5,4);

    Houses_B[0] = InitiateNode(5,9);
    Houses_B[1] = InitiateNode(5,8);
    Houses_B[2] = InitiateNode(5,7);
    Houses_B[3] = InitiateNode(5,6);

}

COORD InitiateNode(int x,int y)
{
    COORD node;
    node.X = x;
    node.Y = y;

    return node;
}