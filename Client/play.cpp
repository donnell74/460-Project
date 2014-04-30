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
const char GAMEOVER = 'o';
const char USERNAME = 'n';
const char KEYBOARD_LAYOUT = 'k';

// Globals 
Client *my_client;

WINDOW *score_win;
WINDOW *timer_win;
WINDOW *message_win;
WINDOW *legend_win;

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

char* keyboard;

string ukeyboard_string = "";
int ukeyboard_x;
int ukeyboard_y;
int keyboard_as_int = 0;
//end Globals


//Get name from user: allows for name editing
//|get_user_name
void get_user_name( string user_name )
{
    int characters = 0;
  
    if ( user_name != "" ) 
    {
        //Trim user name to 14
        if ( user_name.size() > 14 )
	{
	    user_name.substr( 0, 13 ); 
        }
      
        characters = user_name.size();
        uname_string = user_name;
        mvprintw( 21, 45, "%s", uname_string.c_str() );
    }
  
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

	    case 54:
                quit_options( game_started );
	        break;
 
            default:
                if ( ch > 32 && ch < 126 && characters < 14 )
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


//|sig_wrap_cleanup
void sig_wrap_cleanup( int sig )
{
    echo();
    endwin();
    cout << "nCurses has exited. " << endl;
    // wrapper for sigaction to pass int to sig which is never used
    my_client->cleanup();
}


//|splash_screen
void splash_screen()
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
    mvwprintw( stdscr, ORIGIN_Y + 20, ORIGIN_X + 10, 
               "Please Enter a Username: " );
    touchwin( stdscr );
    refresh();
    wrefresh( subwindow );
  
}


//|show_keyboard_layout
void show_keyboard_layout ( )
{
    // Alpha
    WINDOW* alpha = newwin( 9, 13, ORIGIN_Y + 10, ORIGIN_X + 10 );
    wborder( alpha, '|', '|', '-', '-', '+', '+', '+', '+' );
    mvwprintw( alpha, 1, 1, "Keyboard  1" );
    mvwprintw( alpha, 2, 1, "-----------" );
    mvwprintw( alpha, 3, 3, "A B C D" );
    mvwprintw( alpha, 4, 3, "E F G H" );
    mvwprintw( alpha, 5, 3, "I J K L" );
    mvwprintw( alpha, 6, 3, "M N O P" );
    mvwprintw( alpha, 7, 3, "   X   " );
    mvwprintw( stdscr, ORIGIN_Y + 22, ORIGIN_X + 10, 
               "Please Choose Your Keyboard Layout: " );
    
    // left QWERTY
    WINDOW* lqwerty = newwin( 9, 13, ORIGIN_Y + 10, ORIGIN_X + 23 );
    wborder( lqwerty, '|', '|', '-', '-', '+', '+', '+', '+' );
    mvwprintw( lqwerty, 1, 1, "Keyboard  2" );
    mvwprintw( lqwerty, 2, 1, "-----------" );
    mvwprintw( lqwerty, 3, 3, "1 2 3 4" );
    mvwprintw( lqwerty, 4, 3, "Q W E R" );
    mvwprintw( lqwerty, 5, 3, "A S D F" );
    mvwprintw( lqwerty, 6, 3, "Z X C V" );

    // left QWERTY
    WINDOW* rqwerty = newwin( 9, 13, ORIGIN_Y + 10, ORIGIN_X + 36 );
    wborder( rqwerty, '|', '|', '-', '-', '+', '+', '+', '+' );
    mvwprintw( rqwerty, 1, 1, "Keyboard  3" );
    mvwprintw( rqwerty, 2, 1, "-----------" );
    mvwprintw( rqwerty, 3, 3, "7 8 9 0" );
    mvwprintw( rqwerty, 4, 3, "U I O P" );
    mvwprintw( rqwerty, 5, 3, "J K L ;" );
    mvwprintw( rqwerty, 6, 3, "M , . /" );
    mvwprintw( rqwerty, 7, 3, "   N   " );

    // left QWERTY
    WINDOW* colemak = newwin( 9, 13, ORIGIN_Y + 10, ORIGIN_X + 49 );
    wborder( colemak, '|', '|', '-', '-', '+', '+', '+', '+' );
    mvwprintw( colemak, 1, 1, "Keyboard  4" );
    mvwprintw( colemak, 2, 1, "-----------" );
    mvwprintw( colemak, 3, 3, "1 2 3 4" );
    mvwprintw( colemak, 4, 3, "Q W F P" );
    mvwprintw( colemak, 5, 3, "A R S T" );
    mvwprintw( colemak, 6, 3, "Z X C V" );
    mvwprintw( colemak, 7, 3, "   N   " );


    touchwin( stdscr );
    refresh();
    wrefresh( alpha );
    wrefresh( lqwerty );
    wrefresh( rqwerty );
    wrefresh( colemak );
}


//|get_keyboard_layout
void get_keyboard_layout( )
{
  show_keyboard_layout();
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
          getyx( stdscr, ukeyboard_y, ukeyboard_x );
          mvaddch( ukeyboard_y, ukeyboard_x - 1, KEY_SPACE );
          move( ukeyboard_y, ukeyboard_x - 1 );
          ukeyboard_string.erase( ukeyboard_string.size() -1, 1 );
        }

        break;

      case '\n':
        keyboard = new char[ukeyboard_string.length() + 1];
        strcpy( keyboard, ukeyboard_string.c_str() );
        return;

      case 54:
        quit_options( game_started );
        break;

      default:
        if ( characters == 1 )
        {
          break;
        }

        if ( ch > 32 && ch < 126 && characters < 14 )
        {
          addch( ch );
          ukeyboard_string += ( char )ch;
          characters += 1;
        }
        break;
    }
    refresh();
  }
}



//|update_timer_win
void update_timer_win( string msg )
{

}


//|update_score_win
void update_score_win( string msg )
{
    touchwin( score_win );
    werase( score_win );

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
        column = ( i % 4 ) * 17;

        mvwprintw( score_win, row - 1, column, "%d.%s ", i + 1,
                   clients[i].c_str() );
    }
    wrefresh( score_win );

}


//Draws card on screen at desired position
//|draw_card
void draw_card( int card, int number, int symbol, int shade, int color )
{
    int x, y, shade_ch, card_color;
    int columns[4] = { ORIGIN_X, ( ORIGIN_X + CARD_WIDTH + COL_OFFSET ), 
		     ( ORIGIN_X + 2 * ( CARD_WIDTH + COL_OFFSET ) ), 
		     ( ORIGIN_X + 3 * ( CARD_WIDTH + COL_OFFSET ) ) };

    int rows[3] = { ORIGIN_Y, ( ORIGIN_Y + CARD_HEIGHT + ROW_OFFSET ), 
		  ( ORIGIN_Y + 2 * ( CARD_HEIGHT + ROW_OFFSET ) ) };
    switch( card )
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
  
    move( y,x );
  
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

    switch( shade )
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

    switch( color )
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
    if( shade_ch == KEY_SPACE )
    {
        card_color += 3;
    }

    attron( COLOR_PAIR( card_color ) );

    switch( number )
    {
        case 0:
            switch( symbol )
	    {
	        case 0: 
	            mvaddch( y + 1, x + 7, shade_ch );
	            mvaddch( y + 2, x + 7, shade_ch );
	            mvaddch( y + 2, x + 6, shade_ch );
	            mvaddch( y + 2, x + 8, shade_ch );
	            mvaddch( y + 3, x + 7, shade_ch );
	            break;

	        case 1:
	            mvaddch( y + 1, x + 6, shade_ch );
	            mvaddch( y + 1, x + 7, shade_ch );
	            mvaddch( y + 1, x + 8, shade_ch );
	            mvaddch( y + 2, x + 8, shade_ch );
	            mvaddch( y + 3, x + 8, shade_ch );
	            mvaddch( y + 3, x + 7, shade_ch );
	            mvaddch( y + 3, x + 6, shade_ch );
	            mvaddch( y + 2, x + 6, shade_ch );
	            break;

	        case 2:
	            mvaddch( y + 3, x + 6, shade_ch );
	            mvaddch( y + 2, x + 7, shade_ch );
	            mvaddch( y + 1, x + 8, shade_ch );
	            break;

	        default:
	           break;
	    }
            break;
      
        case 1:
            switch( symbol )
	    {
	        case 0:
	            mvaddch( y + 1, x + 5, shade_ch );
	            mvaddch( y + 2, x + 5, shade_ch );
	            mvaddch( y + 2, x + 4, shade_ch );
	            mvaddch( y + 2, x + 6, shade_ch );
	            mvaddch( y + 3, x + 5, shade_ch );
	  
	            mvaddch( y + 1, x + 9,  shade_ch );
	            mvaddch( y + 2, x + 9,  shade_ch );
	            mvaddch( y + 2, x + 8,  shade_ch );
	            mvaddch( y + 2, x + 10, shade_ch );
	            mvaddch( y + 3, x + 9,  shade_ch );
	            break;
	  
	        case 1:
	            mvaddch( y + 1, x + 4, shade_ch );
	            mvaddch( y + 1, x + 5, shade_ch );
	            mvaddch( y + 1, x + 6, shade_ch );
	            mvaddch( y + 2, x + 6, shade_ch );
	            mvaddch( y + 3, x + 6, shade_ch );
	            mvaddch( y + 3, x + 5, shade_ch );
	            mvaddch( y + 3, x + 4, shade_ch );
	            mvaddch( y + 2, x + 4, shade_ch );

	            mvaddch( y + 1, x + 8,  shade_ch );
	            mvaddch( y + 1, x + 9,  shade_ch );
	            mvaddch( y + 1, x + 10, shade_ch );
	            mvaddch( y + 2, x + 10, shade_ch );
	            mvaddch( y + 3, x + 10, shade_ch );
	            mvaddch( y + 3, x + 9,  shade_ch );
	            mvaddch( y + 3, x + 8,  shade_ch );
	            mvaddch( y + 2, x + 8,  shade_ch );
	            break;

	        case 2:					
	            mvaddch( y + 3, x + 4,  shade_ch );
	            mvaddch( y + 2, x + 5,  shade_ch );
	            mvaddch( y + 1, x + 6,  shade_ch );

	            mvaddch( y + 3, x + 8,  shade_ch );
	            mvaddch( y + 2, x + 9,  shade_ch );
	            mvaddch( y + 1, x + 10, shade_ch );
	            break;

	        default:
	            break;
	}
        break;

        case 2:
            switch( symbol )
	    {
	        case 0:
	            mvaddch( y + 1, x + 3, shade_ch );
	            mvaddch( y + 2, x + 3, shade_ch );
	            mvaddch( y + 2, x + 2, shade_ch );
	            mvaddch( y + 2, x + 4, shade_ch );
	            mvaddch( y + 3, x + 3, shade_ch );
	  
	            mvaddch( y + 1, x + 7, shade_ch );
	            mvaddch( y + 2, x + 7, shade_ch );
	            mvaddch( y + 2, x + 6, shade_ch );
	            mvaddch( y + 2, x + 8, shade_ch );
	            mvaddch( y + 3, x + 7, shade_ch );
	  
                    mvaddch( y + 1, x + 11, shade_ch );
	            mvaddch( y + 2, x + 11, shade_ch );
	            mvaddch( y + 2, x + 10, shade_ch );
	            mvaddch( y + 2, x + 12, shade_ch );
	            mvaddch( y + 3, x + 11, shade_ch );
	            break;

	        case 1:
	            mvaddch( y + 1, x + 2, shade_ch );
	            mvaddch( y + 1, x + 3, shade_ch );
	            mvaddch( y + 1, x + 4, shade_ch );
	            mvaddch( y + 2, x + 4, shade_ch );
	            mvaddch( y + 3, x + 4, shade_ch );
	            mvaddch( y + 3, x + 3, shade_ch );
	            mvaddch( y + 3, x + 2, shade_ch );
	            mvaddch( y + 2, x + 2, shade_ch );

	            mvaddch( y + 1, x + 6, shade_ch );
	            mvaddch( y + 1, x + 7, shade_ch );
	            mvaddch( y + 1, x + 8, shade_ch );
	            mvaddch( y + 2, x + 8, shade_ch );
	            mvaddch( y + 3, x + 8, shade_ch );
	            mvaddch( y + 3, x + 7, shade_ch );
	            mvaddch( y + 3, x + 6, shade_ch );
	            mvaddch( y + 2, x + 6, shade_ch );

	            mvaddch( y + 1, x + 10, shade_ch );
	            mvaddch( y + 1, x + 11, shade_ch );
	            mvaddch( y + 1, x + 12, shade_ch );
	            mvaddch( y + 2, x + 12, shade_ch );
	            mvaddch( y + 3, x + 12, shade_ch );
	            mvaddch( y + 3, x + 11, shade_ch );
	            mvaddch( y + 3, x + 10, shade_ch );
	            mvaddch( y + 2, x + 10, shade_ch );
	            break;

	        case 2:
	            mvaddch( y + 3, x + 2, shade_ch );
	            mvaddch( y + 2, x + 3, shade_ch );
	            mvaddch( y + 1, x + 4, shade_ch );

	            mvaddch( y + 3, x + 6, shade_ch );
	            mvaddch( y + 2, x + 7, shade_ch );
	            mvaddch( y + 1, x + 8, shade_ch );

	            mvaddch( y + 3, x + 10, shade_ch );
	            mvaddch( y + 2, x + 11, shade_ch );
	            mvaddch( y + 1, x + 12, shade_ch );
	            break;
	    }
            break;

    default:
        break;
    }
  
    attroff( COLOR_PAIR( card_color ) );
    refresh();

}

//|draw_card_IDs
void draw_card_IDs()
{
    for( int i = 0; i < 3; i++ )
    {
          for( int j = 0; j < 4; j++ )
          {
	      move( ORIGIN_Y + ( ( i ) * ( CARD_HEIGHT + ROW_OFFSET ) ),
	            ORIGIN_X + ( ( j ) * ( CARD_WIDTH + COL_OFFSET ) ) );
	      addch( ( int )ACCEPTED_CHARS[keyboard_as_int][i * 4 + j] );
	  
	  }
    }
    refresh();
}


//|in_accepted_chars
bool in_accepted_chars( int ch )
{
    for ( auto chars : ACCEPTED_CHARS[keyboard_as_int] )
    {
        if ( ch == ( int )chars )
        {
            return true;
        }
    }
    return false;
}


//|get_card
int get_card( int ch )
{
    // before you replace this with a switch statement
    // switch statement does not let none constant values
    // for cases
    if ( ch == ACCEPTED_CHARS[keyboard_as_int][0] )
    {
      return 1;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][1] )
    {
      return 2;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][2] )
    {
      return 3;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][3] )
    {
      return 4;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][4] )
    {
      return 5;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][5] )
    {
      return 6;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][6] )
    {
      return 7;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][7] )
    {
      return 8;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][8] )
    {
      return 9;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][9] )
    {
      return 10;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][10] )
    {
      return 11;
    }
    else if ( ch == ACCEPTED_CHARS[keyboard_as_int][11] )
    {
      return 12;
    }
    else
    {
        return -1;
    }
}


//|get_card_coords
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


//|print_card_stats
void print_card_stats( int card )
{
    werase( message_win );
    
    if ( choice_string == "nnn" )
    {
    
        mvwprintw( message_win, 0, 0, 
                   "CLIENT MESSAGE: You've selected NO SET." );
    }
    else if ( choice_string.size() == 0 )
    {
        
        mvwprintw( message_win, 0, 0, 
                   "CLIENT MESSAGE: You've selected nothing." );
    }
    else
    {
        mvwprintw( message_win, 0, 0, 
                   "CLIENT MESSAGE: You've selected %s", 
                   choice_string.c_str() );
    }
    wrefresh( message_win );
}


//|animate_cards
void animate_cards( vector<int>cards, int rate )
{
    vector<int>xls;
    vector<int>xhs;
    vector<int>yls;
    vector<int>yhs;
    vector<int>mids;

    int row;
    int column;
    int mid;
    int x1;
    int x2;
    int y1; 
    int y2;

    for ( int c = 0; c < cards.size(); c++ )
    {
        row = get_card_coords( cards[c] )[0] - 48;
        column = get_card_coords( cards[c] )[1] - 48;
        x1 = ORIGIN_X + ( (column - 1 ) * ( CARD_WIDTH + COL_OFFSET ) );
        x2 = x1 + CARD_WIDTH - 1;
        y1 = ORIGIN_Y + ( ( row - 1 ) * ( CARD_HEIGHT+ROW_OFFSET ) );
        y2 = y1 + CARD_HEIGHT - 1;
        mid = ( x1 + x2 ) / 2;
      
        xls.push_back( x1 );
        xhs.push_back( x2 );
        yls.push_back( y1 );
        yhs.push_back( y2 );
        mids.push_back( mid );
    }
  
    //Vanish animation
    for ( int i = 0; i < CARD_WIDTH / 2; i++ )
    {
        for ( int j = 0; j < CARD_HEIGHT; j++ )
	{
	    for ( int w = 0; w < cards.size(); w++ )
	    {
	        mvaddch( yls[w] + j, xls[w] + i, KEY_SPACE );
	        mvaddch( yls[w] + j, xhs[w] - i, KEY_SPACE );
	    }
	}
        usleep( rate );
        refresh();
    }
  
    //Reappear animation
    for ( int k = 0; k < CARD_WIDTH / 2; k++ )
    {
        for ( int l = 0; l < CARD_HEIGHT; l++ )
	{
	    for ( int z = 0; z < cards.size(); z++ )
	    {
	        mvaddch( yls[z] + l, mids[z] + 1 + k, CARD_CHAR );
	        mvaddch( yls[z] + l, mids[z] - k, CARD_CHAR );
	    }
	}
        usleep( rate );
        refresh();
    }

    draw_card_IDs();
}


//|dehighlight_card
void dehighlight_card(int card)
{
    int x1;
    int x2;
    int y1;
    int y2; 
    int column;
    int row;
    row = get_card_coords( card )[0] - 48;
    column = get_card_coords( card )[1] - 48;
  
    x1 = ORIGIN_X + ( ( column - 1) * ( CARD_WIDTH + COL_OFFSET ) ) + 1;
    x2 = x1 - 1 +CARD_WIDTH-1;
    y1 = ORIGIN_Y + ( ( row - 1 ) * ( CARD_HEIGHT + ROW_OFFSET ) );
    y2 = y1 + CARD_HEIGHT - 1;

    //Clear previous highlight
    if( x2 != 0 )
    {
        //1.Left Border
        move( y1 - 1, x1 - 2 );
        addch (32 );
        for ( int i = 0; i < CARD_HEIGHT + 1; i++ )
	{
	    move( y1 + i, x1 - 2 );
	    addch( 32 );
	}

        //2.Top Border
        move( y1 - 1, x1 - 1 );
        addch( 32 );
        for ( int j = 0; j < CARD_WIDTH; j++ )
	{
	    move( y1 - 1, j + x1 );
	    addch( 32 );
	}

        //3.Right Border
        move( y1 - 1, x2 + 1 );
        addch( 32 );
        for ( int k = 0; k < CARD_HEIGHT; k++ )
	{
	    move( y1 + k, x2 + 1 );
	    addch( 32 );
	}

        //4.Bottom Border
        move( y2 + 1, x1 - 1 );
        addch( 32 );
        for ( int l = 0; l < CARD_WIDTH; l++ )
	{
	    move( y2 + 1, l + x1 );
	    addch( 32 );
	}

    }
}


//|highlight_card
void highlight_card( int card )
{
    int column;
    int row;
    int x1;
    int x2;
    int y1;
    int y2;
    row = get_card_coords( card )[0] - 48;
    column = get_card_coords( card )[1] - 48;
  
    x1 = ORIGIN_X + ( ( column - 1 ) * ( CARD_WIDTH + COL_OFFSET ) ) + 1;
    x2 = x1 - 1 +CARD_WIDTH - 1;
    y1 = ORIGIN_Y + ( ( row - 1 ) * ( CARD_HEIGHT+ROW_OFFSET ) );
    y2 = y1 + CARD_HEIGHT - 1;
  
    //Set current highlighted x and y marks
    cur_x1 = x1;
    cur_x2 = x2;
    cur_y1 = y1;
    cur_y2 = y2;
  
    //Draw border around selected card
    attron( COLOR_PAIR( 1 ) );
    //1.Left Border
    mvaddch( y1 - 1, x1 - 2, CARD_CHAR );
    for( int i = 0; i < CARD_HEIGHT + 1; i++ )
    {
      mvaddch( y1 + i, x1 - 2, CARD_CHAR );
    }

    //2.Top Border
    mvaddch( y1 - 1, x1 - 1, CARD_CHAR );
    for ( int j = 0; j < CARD_WIDTH; j++ )
    {
        mvaddch( y1 - 1, j + x1, CARD_CHAR );
    }

    //3.Right Border
    mvaddch(y1-1, x2+1, CARD_CHAR);
    for ( int k = 0; k < CARD_HEIGHT; k++ )
    {
        mvaddch( y1 + k, x2 + 1, CARD_CHAR );
    }

    //4.Bottom Border
    mvaddch( y2 + 1, x1 - 1, CARD_CHAR );
    for ( int l = 0; l < CARD_WIDTH; l++ )
    {
        mvaddch( y2 + 1, l + x1, CARD_CHAR );
    }

    attroff( COLOR_PAIR( 1 ) );
}


//|show_game_screen
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
    legend_win = newwin( 10, 10, 1, 70 ); 
    mvwprintw( legend_win, 0, 0, "=LEGEND=" );
    mvwprintw( legend_win, 1, 0, "========" );
    mvwprintw( legend_win, 2, 0, "'6'" );
    mvwprintw( legend_win, 3, 0, "--Quit" );
    mvwprintw( legend_win, 4, 0, "'n'" );
    mvwprintw( legend_win, 5, 0, "--No Set" );
    mvwprintw( legend_win, 6, 0, "Space Bar" );
    mvwprintw( legend_win, 7, 0, "--Guess" );
    mvwprintw( legend_win, 8, 0, "========" );
    refresh();
    wrefresh( legend_win );
    wrefresh( message_win );
}


//|quit_options
void quit_options( bool game_started )
{
    
    WINDOW* quit_win = newwin( 8, 35, ORIGIN_Y + 10, ORIGIN_X + 20 );
    wborder( quit_win, '|', '|', '-', '-', '+', '+', '+', '+' );
    mvwprintw( quit_win, 1, 3, "Do you wan to quit?" );

    if ( game_started )
    {
        mvwprintw( quit_win, 2, 3, "--press 1, 2, or 3--" );
        mvwprintw( quit_win, 3, 3, "1. Quit" );
        mvwprintw( quit_win, 4, 3, "2. Stay and Watch" );
        mvwprintw( quit_win, 5, 3, "3. Keep Playing" );
        touchwin( quit_win );
        wrefresh( quit_win );

        int ch;
        ch = wgetch( quit_win );

        switch ( ch )
        {
            case 49:
                endwin();
                my_client->cleanup();
                break;

            case 50:
                nocbreak();
                touchwin( stdscr );
                mvwprintw( message_win, 0, 0, 
                           "CLIENT MESSAGE: Press 'Ctrl' + 'c' to quit" );
                touchwin( score_win );
                refresh();
                wrefresh( message_win );
                wrefresh( score_win );
                break;

            case 51:
                touchwin( stdscr );
                mvwprintw( message_win, 0, 0, 
                           "CLIENT MESSAGE: Welcome back!" );
                touchwin( score_win );
                touchwin( legend_win );
                refresh();
                wrefresh( message_win );
                wrefresh( score_win );
                wrefresh( legend_win );
                break;
        
                default:
                    break;
        }
    }
    else
    {   
        mvwprintw( quit_win, 2, 3, "--press 1 or 2--" );
        mvwprintw( quit_win, 3, 3, "1. Quit" );
        mvwprintw( quit_win, 4, 3, "2. Stay and Log in" );
        touchwin( quit_win );
        wrefresh( quit_win );

        int ch;
        ch = wgetch( quit_win );

        switch ( ch )
        {
            case 49:
                endwin();
                exit( EXIT_SUCCESS );
                break;

            case 50:
                splash_screen();
                break;

            default:
                break;
        }
    }
}


//|handle_input
void handle_input()
{
    int card, ch;
    if( game_started )
    {
        ch = getch();

        switch( ch )
	{
	    case 54:
	        quit_options( game_started );
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
		        cards.push_back( get_card( 
                                       choice_string[d] ) );
		    } 
	        }
                break;
            }
            default:
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
             in_accepted_chars( toupper(ch) ) )
	{
	    choice_string += ( ( char )ch );
	    highlight_card( get_card( toupper(ch) ) );
	    goto resume;
	 
	}
      
        if ( choice_string.find( ( char )ch ) != -1 && 
             in_accepted_chars( toupper( ch ) ) )
	{
	    choice_string.erase( choice_string.find( ( char )ch ), 1 );
	    dehighlight_card( get_card ( toupper( ch ) ) );
	    goto resume;
	 
	}
      
    resume:
        move( 32,18 );
        clrtoeol();
        if ( in_accepted_chars( ch ) || choice_string == "nnn" )
        {
            print_card_stats( ch );
        }
        clrtoeol();
        refresh();
    }
}


//|bitcode_parser
string bitcode_parser( char bitcode )
{
    int bit = ( int )bitcode;
    int remainder;
    int padding;
    string card_string = "";
    string result = "";

    if ( bit < 0 )
    {
	bit = 128 - ( -1 ) * bit + 128;
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



//|display_game_over
void display_game_over( string msg )
{
  clear();

  wborder( stdscr, '|', '|', '-', '-', '+', '+', '+', '+' );
  int pos = 0;
  int row = 5;

  vector<string>clients;

  mvprintw( 0, 30, "Scoreboard");

  while ( ( pos = msg.find( "<>" ) ) != string::npos )
  {
      clients.push_back( msg.substr( 0, pos ) );
      msg.erase( 0, pos + 2 );
  }

  for ( unsigned int i = 0; i < clients.size(); i++ )
  {
      row += 1;
      size_t found = clients[i].find(user);
      if (found!=string::npos && clients[i][found+strlen(user)] == ' ')
        attron( COLOR_PAIR( 11 ) );

      mvprintw( row - 1, 30, "%d.%s ", i + 1,
                 clients[i].c_str() );

      attroff( COLOR_PAIR( 11 ) );
  }
  refresh();
}


//|handle_server_msg
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
        
        case GAMEOVER:
            display_game_over( msg.substr( 1 ) );
            break;

        case MESSAGE:
            wmove( message_win, 0, 0 );
            wclrtoeol( message_win );
            mvwprintw( message_win, 0, 0, "SERVER MESSAGE: %s", 
                       msg.substr( 1 ).data() );
            wrefresh( message_win );
            break;

        case USERNAME:
            wmove( stdscr, 0, 0 );
            wclrtoeol( stdscr );
            mvwprintw( stdscr, 22, 11, 
                       "You have been connected with username: %s", 
                       msg.substr( 1 ).data() );
            strncpy( user, msg.substr( 1 ).c_str(), msg.substr( 1 ).size() );
            wrefresh( stdscr );
            break;

        case KEYBOARD_LAYOUT:
            keyboard_as_int = msg.at(1) - '1'; //convert to int minus 1 
            break;

        case CARDS:
        {
            if ( game_started == false )
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
            break;
        
        }
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

//|main
int main( int argc, char *argv[] )
{
    char LOCALHOST[] = "127.0.0.1";
    //Initialize ncurses
    int row;
    int column;
    initscr();
    keypad( stdscr, TRUE );
    curs_set( 0 );
    noecho();
    splash_screen();
    timer_win = newwin( 1, 17, TIME_WIN_Y, TIME_WIN_X );
  
    switch( argc )
    {
        case 2:
	{
	    get_user_name( "" );
      get_keyboard_layout( );
	    my_client = new Client( atoi( argv[1] ), LOCALHOST, user, ukeyboard_string );
	}
	break;

        case 3:
	{
	    get_user_name( string( argv[2], strlen( argv[2] ) ) );
      get_keyboard_layout( );
	    my_client = new Client( atoi( argv[1] ), LOCALHOST, user, keyboard );
	}

	break;

        default:
	{
	    cerr << "Usage: ./client <port> username" << endl;
	    exit( EXIT_SUCCESS );
	}
	break;
    }

      // bind TERM to cleanup
      struct sigaction action = {};

      action.sa_handler = sig_wrap_cleanup;
      sigaction( SIGINT, &action, nullptr );
      sigaction( SIGWINCH, &action, nullptr );
      my_client->wait_for_input();
      cout << "nCurses has exited. " << endl;
      endwin();
      my_client->cleanup();

}


