#define _POSIX_SOURCE
/* cpp includes */
#include <iostream>
#include <vector>
#include <algorithm>
#include <curses.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <cerrno>
#include <random>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
/* local includes */
#include "client.h"
//#include "cardlib.h"
/* defined ncurses macros */
#define KEY_SPACE ' '
//Origin of ncurses stdscr
#define ORIGIN_X 1
#define ORIGIN_Y 1
//Card background character
#define CARD_CHAR 178
//Card width and length
#define CARD_WIDTH 16
#define CARD_HEIGHT 5
//Space between rows and columns
#define ROW_OFFSET 1
#define COL_OFFSET 1
//Timer Window coordinates
#define TIME_WIN_X 20
#define TIME_WIN_Y 24

using namespace std;

/* Constants */
const char EXIT = 'x';
const char MESSAGE = 'm';
const char CARDS = 'c';
const char UPDATE = 'u';


vector<char> ACCEPTED_CHARS = { '1', '2', '3', '4', 
                                'q', 'w', 'e', 'r',
                                'a', 's', 'd', 'f', 
                                'z', 'x', 'c', 'v',
                                'O', 'N', 'U', 'I' };
// Globals 
Client *my_client;

WINDOW *score_win;
WINDOW *timer_win;
WINDOW *message_win;

char* user;

string uname_string = "";
string choice_string = "";

bool animate = false;
bool name_set = false;
bool game_started = false;

int cur_x1 = 0;
int cur_y1 = 0;
int cur_x2 = 0;
int cur_y2 = 0;

int uname_x;
int uname_y;
//end Globals


//Get name from user: allows for name editing
void get_user_name()
{
    int characters = 0;
    
    for ( ;; )
    {
        int ch = getch();
        switch( ch )
        {
            case KEY_BACKSPACE:
                if ( characters > 0 )
                {
                    characters -= 1;
                    getyx( stdscr, uname_y, uname_x );
                    mvaddch( uname_y, uname_x - 1, KEY_SPACE );
                    move( uname_y, uname_x - 1 );
                    uname_string.erase( uname_string.size() -1, 1 );
                }

            break;

            case '\n':
                user = new char[uname_string.length() + 1];
                strcpy( user, uname_string.c_str() );
                return;

            default:
                if ( ch > 32 && ch < 126 && characters < 15 )
                {
                    addch( ch );
                    uname_string += ( char )ch;
                    characters += 1;
                }
                break;
        }
        refresh();
    }
}


void sig_wrap_cleanup( int sig )
{
    echo();
    endwin();
    cout << "nCurses has exited. " << endl;
    // wrapper for sigaction to pass int to sig which is never used
    my_client->cleanup();
}


//Splash Screen
void splash_screen( char * name )
{
    clear();
    start_color();
    init_pair( 10, COLOR_BLUE, COLOR_BLACK );
    init_pair( 11, COLOR_CYAN, COLOR_BLACK );
    init_pair( 12, COLOR_MAGENTA, COLOR_BLACK );
    init_pair( 13, COLOR_YELLOW, COLOR_BLACK );
    wborder( stdscr, '|', '|', '-', '-', '+', '+', '+', '+' );

    vector<string>S = { R"(  /\\\\\\\\\\\      )",
                        R"(/\\\/////////\\\    )",
                        R"(\//\\\      \///    )",
                        R"(  \////\\\          )",
                        R"(      \////\\\      )",
                        R"(          \////\\\  )",
                        R"(    /\\\      \//\\\)",
                        R"(    \///\\\\\\\\\\\/)",
                        R"(       \/////////// )" };
		
    vector<string>E = { R"(/\\\\\\\\\\\\\\     )",
                        R"(\/\\///////////     )",
                        R"( \/\\\              )",
                        R"(  \/\\\\\\\\\       )",
                        R"(   \/\\\/////       )",
                        R"(    \/\\\           )",
                        R"(     \/\\\          )",
                        R"(      \/\\\\\\\\\\\\)",
                        R"(       \////////////)" };

    vector<string>T = { R"(/\\\\\\\\\\\\\\\ )",
                        R"(\///////\\\///// )",
                        R"(       \/\\\     )",
                        R"(        \/\\\    )",
                        R"(         \/\\\   )",
                        R"(          \/\\\  )",
                        R"(           \/\\\ )",
                        R"(            \/\\\)",
                        R"(             \///)" };


    int logo_offset = 6;

    //Draw the 'S'
    attron( COLOR_PAIR( 11 ) );
    for ( int i = 0; i < S.size(); i++ )
    {
        mvprintw( ORIGIN_Y + i, ORIGIN_X + logo_offset, S[i].c_str() );
    }                  
    attroff( COLOR_PAIR( 11 ) );

    //Draw the 'E'
    logo_offset += 20;    
    attron( COLOR_PAIR( 13 ) );
    for ( int j = 0; j < E.size(); j++ )
    {
        mvprintw( ORIGIN_Y + j, ORIGIN_X + logo_offset, E[j].c_str() );
    }                  
    attroff( COLOR_PAIR( 13 ) );

    //Draw the 'T'
    logo_offset += 20;    
    attron( COLOR_PAIR( 12 ) );
    for ( int k = 0; k < T.size(); k++ )
    {
        mvprintw( ORIGIN_Y + k, ORIGIN_X + logo_offset, T[k].c_str() );
    }                  
    attroff( COLOR_PAIR( 12 ) );

    //Project Details window
    WINDOW* subwindow = newwin( 8, 35, ORIGIN_Y + 10, ORIGIN_X + 20 );
    wborder( subwindow, '|', '|', '-', '-', '+', '+', '+', '+' );
    mvwprintw( subwindow, 1, 3, "CSC 460" );
    mvwprintw( subwindow, 2, 3, "Spring 2014" );
    mvwprintw( subwindow, 3, 3, "Project Team: Tim Williams" );
    mvwprintw( subwindow, 4, 3, "\t\t Greg Donnell" );
    mvwprintw( subwindow, 5, 3, "\t\t Matthew Duff" );
    touchwin( subwindow );
    mvwprintw( stdscr, ORIGIN_Y + 20, ORIGIN_X + 20, 
               "Please Enter a Username: " );
    touchwin( stdscr );
    refresh();
    wrefresh( subwindow );
  
}


void update_timer_win( string msg )
{

}


void update_score_win( string msg )
{
    touchwin( score_win );
    wclear( score_win );

    int row = 1;
    int column = 0;
    int pos = 0;

    vector<string>clients;

    while ( ( pos = msg.find( "<>" ) ) != string::npos )
    {
        clients.push_back( msg.substr( 0, pos ) );
        msg.erase( 0, pos + 2 );
    }

    for ( unsigned int i = 0; i < clients.size(); i++ )
    {
        if ( i % 4 == 0 )
        {
           row += 1;
           column = 0;
        }
        column = ( i % 4 ) * 15;

        mvwprintw( score_win, row - 1, column, "%d.%s ", i + 1,
                   clients[i].c_str() );
    }
    wrefresh( score_win );

}


//Draws card on screen at desired position
void draw_card(int card, int number, int symbol, int shade, int color)
{
  int x, y, shade_ch, card_color;
  int columns[4] = {ORIGIN_X, (ORIGIN_X+CARD_WIDTH+COL_OFFSET), 
		    (ORIGIN_X+2*(CARD_WIDTH+COL_OFFSET)), 
		    (ORIGIN_X+3*(CARD_WIDTH+COL_OFFSET))};

  int rows[3] = {ORIGIN_Y, (ORIGIN_Y+CARD_HEIGHT+ROW_OFFSET), 
		 (ORIGIN_Y+2*(CARD_HEIGHT+ROW_OFFSET))};
  switch(card)
	{
	case 1:
	  y = rows[0];
	  x = columns[0];
	  break;
	case 2:
	  y = rows[0];
	  x = columns[1];
	  break;
	case 3:
	  y = rows[0];
	  x = columns[2];
	  break;
	case 4:
	  y = rows[0];
	  x = columns[3];
	  break;
	case 5:
	  y = rows[1];
	  x = columns[0];
	  break;
	case 6:
	  y = rows[1];
	  x = columns[1];
	  break;
	case 7:
	  y = rows[1];
	  x = columns[2];
	  break;
	case 8:
	  y = rows[1];
	  x = columns[3];
	  break;
	case 9:
	  y = rows[2];
	  x = columns[0];
	  break;
	case 10:
	  y = rows[2];
	  x = columns[1];
	  break;
	case 11:
	  y = rows[2];
	  x = columns[2];
	  break;
	case 12:
	  y = rows[2];
	  x = columns[3];
	  break;
	default:
	  break;
    }
  
  move(y,x);
  
    for( int j = 1; j < CARD_HEIGHT + 1; j++ )
    {
        for( int k = 0; k < CARD_WIDTH; k++ )
	{
            if ( number == 9 )
            {
                addch( KEY_SPACE );
            }
	    else
            {
                addch( CARD_CHAR );
            }
	}
        move( y + j, x );
    }
    
    if ( number == 9 )
    {
        return;
    }

    switch(shade)
    {
    case 0:
      shade_ch = 64;
      break;
    case 1 :
      shade_ch = 111;
      break;
    case 2:
      shade_ch = KEY_SPACE;
      break;
    default:
      break;
    }

  switch(color)
    {
    case 0:
      card_color = 2;
      break;
    case 1:
      card_color = 3;
      break;
    case 2:
      card_color = 4;
      break;
    default:
      break;
    }

  //Configure card
  if(shade_ch == KEY_SPACE)
    {
      card_color += 3;
    }

  attron(COLOR_PAIR(card_color));
  switch(number)
    {
    case 0:
      switch(symbol)
	{
	case 0: 
	  mvaddch(y+1, x+7, shade_ch);
	  mvaddch(y+2, x+7, shade_ch);
	  mvaddch(y+2, x+6, shade_ch);
	  mvaddch(y+2, x+8, shade_ch);
	  mvaddch(y+3, x+7, shade_ch);
	  break;

	case 1:
	  mvaddch(y+1, x+6, shade_ch);
	  mvaddch(y+1, x+7, shade_ch);
	  mvaddch(y+1, x+8, shade_ch);
	  mvaddch(y+2, x+8, shade_ch);
	  mvaddch(y+3, x+8, shade_ch);
	  mvaddch(y+3, x+7, shade_ch);
	  mvaddch(y+3, x+6, shade_ch);
	  mvaddch(y+2, x+6, shade_ch);
	  break;

	case 2:
	  mvaddch(y+3, x+6, shade_ch);
	  mvaddch(y+2, x+7, shade_ch);
	  mvaddch(y+1, x+8, shade_ch);
	  break;

	default:
	  break;
	}
      break;
      
    case 1:
      switch(symbol)
	{
	case 0:
	  mvaddch(y+1, x+5, shade_ch);
	  mvaddch(y+2, x+5, shade_ch);
	  mvaddch(y+2, x+4, shade_ch);
	  mvaddch(y+2, x+6, shade_ch);
	  mvaddch(y+3, x+5, shade_ch);
	  
	  mvaddch(y+1, x+9, shade_ch);
	  mvaddch(y+2, x+9, shade_ch);
	  mvaddch(y+2, x+8, shade_ch);
	  mvaddch(y+2, x+10, shade_ch);
	  mvaddch(y+3, x+9, shade_ch);
	  break;
	  
	case 1:
	  mvaddch(y+1, x+4, shade_ch);
	  mvaddch(y+1, x+5, shade_ch);
	  mvaddch(y+1, x+6, shade_ch);
	  mvaddch(y+2, x+6, shade_ch);
	  mvaddch(y+3, x+6, shade_ch);
	  mvaddch(y+3, x+5, shade_ch);
	  mvaddch(y+3, x+4, shade_ch);
	  mvaddch(y+2, x+4, shade_ch);

	  mvaddch(y+1, x+8, shade_ch);
	  mvaddch(y+1, x+9, shade_ch);
	  mvaddch(y+1, x+10, shade_ch);
	  mvaddch(y+2, x+10, shade_ch);
	  mvaddch(y+3, x+10, shade_ch);
	  mvaddch(y+3, x+9, shade_ch);
	  mvaddch(y+3, x+8, shade_ch);
	  mvaddch(y+2, x+8, shade_ch);
	  break;

	case 2:					
	  mvaddch(y+3, x+4, shade_ch);
	  mvaddch(y+2, x+5, shade_ch);
	  mvaddch(y+1, x+6, shade_ch);

	  mvaddch(y+3, x+8, shade_ch);
	  mvaddch(y+2, x+9, shade_ch);
	  mvaddch(y+1, x+10, shade_ch);
	  break;
	default:
	  break;
	}
      break;
    case 2:
      switch(symbol)
	{
	case 0:
	  mvaddch(y+1, x+3, shade_ch);
	  mvaddch(y+2, x+3, shade_ch);
	  mvaddch(y+2, x+2, shade_ch);
	  mvaddch(y+2, x+4, shade_ch);
	  mvaddch(y+3, x+3, shade_ch);
	  
	  mvaddch(y+1, x+7, shade_ch);
	  mvaddch(y+2, x+7, shade_ch);
	  mvaddch(y+2, x+6, shade_ch);
	  mvaddch(y+2, x+8, shade_ch);
	  mvaddch(y+3, x+7, shade_ch);
	  
          mvaddch(y+1, x+11, shade_ch);
	  mvaddch(y+2, x+11, shade_ch);
	  mvaddch(y+2, x+10, shade_ch);
	  mvaddch(y+2, x+12, shade_ch);
	  mvaddch(y+3, x+11, shade_ch);
	  
	  break;

	case 1:
	  mvaddch(y+1, x+2, shade_ch);
	  mvaddch(y+1, x+3, shade_ch);
	  mvaddch(y+1, x+4, shade_ch);
	  mvaddch(y+2, x+4, shade_ch);
	  mvaddch(y+3, x+4, shade_ch);
	  mvaddch(y+3, x+3, shade_ch);
	  mvaddch(y+3, x+2, shade_ch);
	  mvaddch(y+2, x+2, shade_ch);

	  mvaddch(y+1, x+6, shade_ch);
	  mvaddch(y+1, x+7, shade_ch);
	  mvaddch(y+1, x+8, shade_ch);
	  mvaddch(y+2, x+8, shade_ch);
	  mvaddch(y+3, x+8, shade_ch);
	  mvaddch(y+3, x+7, shade_ch);
	  mvaddch(y+3, x+6, shade_ch);
	  mvaddch(y+2, x+6, shade_ch);

	  mvaddch(y+1, x+10, shade_ch);
	  mvaddch(y+1, x+11, shade_ch);
	  mvaddch(y+1, x+12, shade_ch);
	  mvaddch(y+2, x+12, shade_ch);
	  mvaddch(y+3, x+12, shade_ch);
	  mvaddch(y+3, x+11, shade_ch);
	  mvaddch(y+3, x+10, shade_ch);
	  mvaddch(y+2, x+10, shade_ch);

	  break;

	case 2:
	  mvaddch(y+3, x+2, shade_ch);
	  mvaddch(y+2, x+3, shade_ch);
	  mvaddch(y+1, x+4, shade_ch);

	  mvaddch(y+3, x+6, shade_ch);
	  mvaddch(y+2, x+7, shade_ch);
	  mvaddch(y+1, x+8, shade_ch);

	  mvaddch(y+3, x+10, shade_ch);
	  mvaddch(y+2, x+11, shade_ch);
	  mvaddch(y+1, x+12, shade_ch);
	  break;
	}
      break;

    default:
      break;
    }
  
  attroff(COLOR_PAIR(card_color));
  refresh();

}


void draw_card_IDs()
{
    for( int i = 0; i < 3; i++ )
    {
          for( int j = 0; j < 4; j++ )
          {
	      move( ORIGIN_Y + ( ( i ) * ( CARD_HEIGHT + ROW_OFFSET ) ),
	            ORIGIN_X + ( ( j ) * ( CARD_WIDTH + COL_OFFSET ) ) );
	      addch( ( int )ACCEPTED_CHARS[i * 4 + j] );
	  
	  }
    }
    refresh();
}



bool in_accepted_chars( int ch )
{
    for ( auto chars : ACCEPTED_CHARS )
    {
        if ( ch == ( int )chars )
        {
            return true;
        }
    }
    return false;
}


//int get_card(int row, int column)
int get_card(int ch)
{
  //Retrieve card by row and column
  //return row + (3 * (row-1) + (column-1)
  //int ID[16] = {49,50,51,52,81,87,69,82,65,83,68,70,90,88,67,86};

   switch(ch)
     {
     case 49:
       return 1;
     case 50:
       return 2;
     case 51:
       return 3;
     case 52:
       return 4;
     case 113:
       return 5;
     case 119:
       return 6;
     case 101:
       return 7;
     case 114:
       return 8;
     case 97:
       return 9;
     case 115:
       return 10;
     case 100:
       return 11;
     case 102:
       return 12;
     default:
       return -1;
     }
}


string get_card_coords( int card )
{
    switch( card )
    {
        case 1:
            return "11";

        case 2:
            return "12";

        case 3:
            return "13";

        case 4:
            return "14";
    
        case 5:
            return "21";

        case 6:
            return "22";

        case 7:
            return "23";

        case 8:
            return "24";

        case 9:
            return "31";

        case 10:
            return "32";

        case 11:
            return "33";

        case 12:
            return "34";

        default:
            return "-1";
    }
}


void print_card_stats( int card )
{
    //mvprintw( 32, 10, "Choice String:%s", choice_string.c_str() );
    wclear( message_win );
    mvwprintw( message_win, 0, 0, "SERVER MESSAGE: You've selected %s", 
               choice_string.c_str() );
    wrefresh( message_win );
}

void animate_cards(vector<int>cards, int rate)
{
  vector<int>xls;
  vector<int>xhs;
  vector<int>yls;
  vector<int>yhs;
  vector<int>mids;

    int row, column, mid, x1, x2, y1, y2;
    for( int c = 0; c < cards.size(); c++)
    {
      row = get_card_coords( cards[c] )[0] - 48;
      column = get_card_coords( cards[c] )[1] - 48;
      x1 = ORIGIN_X + ((column - 1)*(CARD_WIDTH+COL_OFFSET));
      x2 = x1 + CARD_WIDTH-1;
      y1 = ORIGIN_Y + ((row - 1)*(CARD_HEIGHT+ROW_OFFSET));
      y2 = y1+CARD_HEIGHT-1;
      mid = (x1+x2)/2;
      
      xls.push_back(x1);
      xhs.push_back(x2);
      yls.push_back(y1);
      yhs.push_back(y2);
      mids.push_back(mid);
    }
  
  //Vanish animation
  for(int i=0; i<CARD_WIDTH/2; i++)
    {
      for(int j=0; j<CARD_HEIGHT; j++)
	{
	  for(int w=0; w<cards.size(); w++)
	    {
	      mvaddch(yls[w]+j, xls[w]+i, KEY_SPACE);
	      mvaddch(yls[w]+j, xhs[w]-i, KEY_SPACE);
	    }
	}
      usleep(rate);
      refresh();
    }
  
  //Reappear animation
  for(int k=0; k<CARD_WIDTH/2; k++)
    {
      for(int l=0; l<CARD_HEIGHT; l++)
	{
	  for(int z=0; z<cards.size(); z++)
	    {
	      mvaddch(yls[z]+l, mids[z]+1+k, CARD_CHAR);
	      mvaddch(yls[z]+l, mids[z]-k, CARD_CHAR);
	    }
	}
      usleep(rate);
      refresh();
    }

  draw_card_IDs();
}


void dehighlight_card(int card)
{
    int x1, x2, y1, y2, column, row;
    row = get_card_coords( card )[0] - 48;
    column = get_card_coords( card )[1] - 48;
  
    x1 = ORIGIN_X + ((column - 1)*(CARD_WIDTH+COL_OFFSET))+1;
    x2 = x1 - 1 +CARD_WIDTH-1;
    y1 = ORIGIN_Y + ((row - 1)*(CARD_HEIGHT+ROW_OFFSET));
    y2 = y1+CARD_HEIGHT-1;

    //Clear previous highlight
    if( x2 != 0 )
    {
      //1.Left Border
      move(y1-1, x1-2);
      addch(32);
      for(int i=0; i<CARD_HEIGHT+1; i++)
	{
	  move(y1+i, x1-2);
	  addch(32);
	}

      //2.Top Border
      move(y1-1, x1-1);
      addch(32);
      for(int j=0; j<CARD_WIDTH; j++)
	{
	  move(y1-1, j+x1-1+1);
	  addch(32);
	}

      //3.Right Border
      move(y1-1, x2+1);
      addch(32);
      for(int k=0; k<CARD_HEIGHT; k++)
	{
	  move(y1+k, x2+1);
	  addch(32);
	}

      //4.Bottom Border
      move(y2+1, x1-1);
      addch(32);
      for(int l=0; l<CARD_WIDTH; l++)
	{
	  move(y2+1, l+x1-1+1);
	  addch(32);
	}

    }
}


void highlight_card(int card)
{
    int column,row,x1,x2,y1,y2;
    row = get_card_coords( card )[0] - 48;
    column = get_card_coords( card )[1] - 48;
  
  x1 = ORIGIN_X + ((column - 1)*(CARD_WIDTH+COL_OFFSET))+1;
  x2 = x1 - 1 +CARD_WIDTH-1;
  y1 = ORIGIN_Y + ((row - 1)*(CARD_HEIGHT+ROW_OFFSET));
  y2 = y1+CARD_HEIGHT-1;
  
  //Set current highlighted x and y marks
  cur_x1 = x1;
  cur_x2 = x2;
  cur_y1 = y1;
  cur_y2 = y2;
  
  //Draw border around selected card
  attron(COLOR_PAIR(1));
  //1.Left Border
  mvaddch(y1-1, x1-2, CARD_CHAR);
  for(int i=0; i<CARD_HEIGHT+1; i++)
    {
      mvaddch(y1+i, x1-2, CARD_CHAR);
    }

  //2.Top Border
  mvaddch(y1-1, x1-1, CARD_CHAR);
  for(int j=0; j<CARD_WIDTH; j++)
  {
    mvaddch(y1-1, j+x1-1+1, CARD_CHAR);
  }

  //3.Right Border
  mvaddch(y1-1, x2+1, CARD_CHAR);
  for(int k=0; k<CARD_HEIGHT; k++)
    {
      mvaddch(y1+k, x2+1, CARD_CHAR);
    }

  //4.Bottom Border
  mvaddch(y2+1, x1-1, CARD_CHAR);
  for(int l=0; l<CARD_WIDTH; l++)
    {
      mvaddch(y2+1, l+x1-1+1, CARD_CHAR);
    }

  attroff(COLOR_PAIR(1));
}


void show_game_screen()
{
    clear();
    int row, column;
    start_color();

    init_pair( 1, COLOR_RED, COLOR_BLACK );
    init_pair( 2, COLOR_RED, COLOR_WHITE );
    init_pair( 3, COLOR_MAGENTA, COLOR_WHITE );
    init_pair( 4, COLOR_BLUE, COLOR_WHITE );
    init_pair( 5, COLOR_RED, COLOR_RED );
    init_pair( 6, COLOR_MAGENTA, COLOR_MAGENTA );
    init_pair( 7, COLOR_BLUE, COLOR_BLUE );
 
    getmaxyx (stdscr, row, column );
    score_win = newwin( 4, 80, 21, 0 );
    message_win = newwin( 1, 80, 20, 0 );
    mvwprintw( message_win, 0, 0, "SERVER MESSAGE: Make a guess %s!", 
               uname_string.c_str() );
    refresh();
    wrefresh( message_win );

}


//|handle_input
void handle_input()
{
    int card, ch;
    if( game_started )
    {
        ch = getch();

        switch(ch)
	{
	    case 54:
	        endwin();
	        my_client->cleanup();
	        break;

	    case 110:
	        choice_string = "nnn";
	        break;
	
	    case KEY_SPACE:
	    {
	        vector<int>cards;

	        if ( choice_string.size() == 3 )
	        {
                    my_client->send_message( choice_string );
	            for( int d = 0; d < choice_string.size(); d++ )
		    {
		        cards.push_back( get_card( ( int )
                                       ( choice_string[d] ) ) );
		    } 
	        }
                break;
            }
            default:
                //mvprintw( 33, 10, "Key Pressed:%c", ch );
                mvwprintw( message_win, 0, 0, 
                           "SERVER MESSAGE: you've pressed %c", ch );
                wrefresh( message_win );
                break;
        }


        if ( choice_string.find( ( char )ch ) == -1 && 
            choice_string.size() == 3 )
	{
	    //Empty choice string
	    choice_string = "";
	    //Delete all card highlights
	    for( int i = 1; i < 13; i++ )
	    {
	      dehighlight_card( i );
	    }
	 
	}

        if ( choice_string.find( ( char )ch ) == -1 && 
            in_accepted_chars( ch ) )
	{
	    choice_string += ( ( char )ch );
	    highlight_card( get_card( ch ) );
	    goto resume;
	 
	}
      
        if ( choice_string.find( ( char )ch ) !=-1 && 
            in_accepted_chars( ch ) )
	{
	    choice_string.erase( choice_string.find( ( char )ch ), 1 );
	    dehighlight_card( get_card ( ch ) );
	    goto resume;
	 
	}
      
    resume:
        move( 32,18 );
        clrtoeol();
        print_card_stats( ch );
        clrtoeol();
        refresh();
    }
}


string bitcode_parser( char bitcode )
{
    int bit = ( int )bitcode;
    int remainder;
    int padding;
    string card_string = "";
    string result = "";

    if ( bit < 0 )
      {
	bit = 128 - (-1)*bit + 128;
      }
    
    if ( bit == 255 )
    {
        mvprintw( 43, 0, "9999" );
        return "9999";
    }

    if ( bit == 200 )
    {
        return "0000";
    }

    for ( ;; )
    {
        if ( bit <= 2 )
        {
            card_string += to_string( bit );
            break;
        }
        remainder = bit % 4;
        bit = bit >> 2;
        card_string += to_string( remainder );
    }
    
    //Reverse string
    for ( int j = card_string.size(); j > 0; j-- )
    {
        result += card_string[j -1];
    }

    padding = 4 - result.size();

    for ( int i = 0; i < padding; i++ )
    {
        result.insert( 0, "0" );
    }

    return result;
}


void handle_server_msg()
{
  string msg = my_client->get_next_msg();
  switch( msg.front() )
  {
    case EXIT:
      printw( "%s", msg.substr( 1 ).data() );
      printw( "%s", "\n" );
      refresh();
      endwin();
      my_client->cleanup();
      break;

    case MESSAGE:
      wmove( message_win, 0, 0 );
      wclrtoeol( message_win );
      mvwprintw( message_win, 0, 0, "SERVER MESSAGE: %s", 
                 msg.substr( 1 ).data() );
      wrefresh( message_win );
      break;

    case CARDS:
        {
        if( game_started == false )
	{
	  show_game_screen();
	  game_started = true;
	}
      
        vector<string>cards;
        vector<int>idxs;
        msg = msg.substr( 1 );
        int pos;

        while ( ( pos = msg.find( ";" ) ) != string::npos )
        {
            cards.push_back( msg.substr( 0, pos ) );
            msg.erase( 0, pos + 1 );
        }
        
        for ( int j = 0; j < cards.size(); j++ )
        {
            idxs.push_back( stoi( cards[j].substr( 2 ).data() ) + 1 );
        }

        //Animate cards
        animate_cards( idxs, 20000 );
	
        //Draw
        for ( int i = 0; i < cards.size(); ++i )
        {
            string c_string = bitcode_parser( cards[i][0] );
            draw_card( idxs[i], c_string[0] - 48, c_string[1] - 48,
                       c_string[2] - 48, c_string[3] - 48 ); 
        }

        draw_card_IDs();
        }
        break;

    case UPDATE:
        update_score_win( msg.substr( 1, msg.size() - 1 ) );
        break;

    default:
        break;
    }

    refresh();

    if ( ! my_client->get_past_data_read().empty() )
    {
        handle_server_msg();
    }   
}


int main( int argc, char *argv[] )
{
  char LOCALHOST[] = "127.0.0.1";

  if ( argc == 1 || argc > 3 )
  {
    cerr << "Usage: ./client <port>" << endl;
    exit(EXIT_SUCCESS);
  }
  else
  {
    //Initialize ncurses
    int row, column;
    initscr();
    keypad( stdscr, TRUE );
    curs_set(0);
    noecho();

    splash_screen( user );
    timer_win = newwin( 1, 17, TIME_WIN_Y, TIME_WIN_X );
    get_user_name();

    if ( argc == 2 )
    {
      my_client = new Client( atoi( argv[1] ), LOCALHOST, user );
    }
    else
    {
      my_client = new Client( atoi( argv[1] ), argv[2], user );
    }  
    
    // bind TERM to cleanup
    struct sigaction action = {};

    action.sa_handler = sig_wrap_cleanup;
    sigaction( SIGINT, &action, nullptr );
    
    my_client->wait_for_input();
    cout << "nCurses has exited. " << endl;
    endwin();
    my_client->cleanup();

  }

}
