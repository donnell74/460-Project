#define _POSIX_SOURCE
/* cpp includes */
#include <iostream>
#include <vector>
#include <algorithm>
#include <curses.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <random>
#include <string.h>
#include <stdio.h>
/* local includes */
#include "client.h"
#include "cardlib.h"
/* defined ncurses macros */
#define KEY_SPACE ' '
//Origin of ncurses stdscr
#define ORIGIN_X 1
#define ORIGIN_Y 1
//Card background character
#define CARD_CHAR 219
//Card width and length
#define CARD_WIDTH 16
#define CARD_HEIGHT 5
//Space between rows and columns
#define ROW_OFFSET 1
#define COL_OFFSET 1

using namespace std;

/* Constants */
const char EXIT = 'x';
const char MESSAGE = 'm';
const char CARDS = 'c';

// last line of ACCEPTED_CHARS is for command chars
//response
vector<char> ACCEPTED_CHARS = {'1', '2', '3', '4', 
                               'Q', 'W', 'E', 'R',
                               'A', 'S', 'D', 'F', 
                               'Z', 'X', 'C', 'V',
                               'O', 'N', 'U', 'I',
			       'T'}; 
Client *my_client;
string choice_string = "";
bool animate = false;
int cur_x1 = 0;
int cur_y1 = 0;
int cur_x2 = 0;
int cur_y2 = 0;
random_device rd;
default_random_engine e1(rd());


void sig_wrap_cleanup( int sig )
{
    echo();
    endwin();
    cout << "nCurses has exited. " << endl;
    // wrapper for sigaction to pass int to sig which is never used
    my_client->cleanup();
}


//|<handle_input>
void handle_input()
{
  /*
  cbreak();
   
  vector<char> select = {};
  
  for ( int i = 0; ; ++i )
  { 

    int c = getch();

    if ( c == ERR || ( c == ' ' && select.size() == 3 ) ) 
    {
      break;
    }
    
    c = toupper( c );

    //If already selected, deselect it and go to top.
    if ( find( select.begin(), select.end(), c ) != select.end() )
    {
       //add code to deselect the card here
       select.erase( remove( select.begin(), select.end(), c ), select.end() );
       refresh();
       continue;
    }

    //If acceptable char and not a selected card, select it.
    if ( find(ACCEPTED_CHARS.begin(), ACCEPTED_CHARS.end(), c ) != 
         ACCEPTED_CHARS.end() && 
         find (select.begin(), select.end(), c ) == select.end() )  
    {
      //add code to select card here
      select.push_back( c );
      refresh();
    }
  }

  string inp( select.begin(), select.end() );
  printw( "%s", inp.data() );
  printw( "%s", "\n" );
  refresh();

  */
  //Send substing in case of left overs.
  //my_client->send_message( inp.substr( 0,3 ) );
  //my_client->send_message( inp );
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
      my_client->cleanup();
      break;

    case MESSAGE:
      mvprintw( 22, 2, "%s", "Message Received: \n" );
      mvprintw( 23, 2, "%s", msg.substr( 1 ).data() );
      refresh(); 
      break;

    case CARDS:
      if ( DEBUG )
      {
	printw("CARDS NOM NOM NOM");
	printw("Size: %d",msg.size());
	
	for ( int i = 1; i < (int) msg.size(); ++i )
	{
          printw("Received card with bitcode: %d", int( msg[i] ) );
	}
      }
      break;

    default:
      break;
  }

  if ( ! my_client->get_past_data_read().empty() )
  {
    handle_server_msg();
  }
}

//Random number generator wrapper
int rand()
{
  uniform_int_distribution<int> uniform_dist( 0, 2 );
  int random_number = uniform_dist( e1 );

  return random_number;
}


//Splash Screen
void splash_screen( char * name)
{
  clear();
  box(stdscr, '|', '-');				
  mvprintw(ORIGIN_Y, ORIGIN_X,"     /\\\\\\\\\\\\\\\\\\\\\\    /\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\  /\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\");         
  mvprintw(ORIGIN_Y+1, ORIGIN_X,"    /\\\\\\/////////\\\\\\ \\/\\\\\\///////////  \\///////\\\\\\/////");     
  mvprintw(ORIGIN_Y+2, ORIGIN_X,"    \\//\\\\\\      \\///  \\/\\\\\\                   \\/\\\\\\    ");         
  mvprintw(ORIGIN_Y+3, ORIGIN_X,"      \\////\\\\\\         \\/\\\\\\\\\\\\\\\\\\\\\\           \\/\\\\\\   ");     
  mvprintw(ORIGIN_Y+4, ORIGIN_X,"          \\////\\\\\\      \\/\\\\\\///////            \\/\\\\\\  ");         
  mvprintw(ORIGIN_Y+5, ORIGIN_X,"              \\////\\\\\\   \\/\\\\\\\                   \\/\\\\\\ ");         
  mvprintw(ORIGIN_Y+6, ORIGIN_X,"        /\\\\\\      \\//\\\\\\  \\/\\\\\\                   \\/\\\\\\");       
  mvprintw(ORIGIN_Y+7, ORIGIN_X,"        \\///\\\\\\\\\\\\\\\\\\\\\\/   \\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\       \\/\\\\\\");       
  mvprintw(ORIGIN_Y+8, ORIGIN_X,"           \\///////////     \\///////////////        \\///");
  refresh();
  
  //Project Details window
  WINDOW* subwindow = newwin(8,35,ORIGIN_Y+10, ORIGIN_X+16);
  box(subwindow, 0, 0);
  mvwprintw(subwindow, 1, 3, "CSC 460");
  mvwprintw(subwindow, 2, 3, "Spring 2014");
  mvwprintw(subwindow, 3, 3, "Project Team: Greg Donnell");
  mvwprintw(subwindow, 4, 3, "\t\t Matt Duff");
  mvwprintw(subwindow, 5, 3, "\t\t Tim Williams");
  wrefresh(subwindow);

  touchwin(subwindow);
  wrefresh(subwindow);
  mvwprintw(stdscr, ORIGIN_Y+20, ORIGIN_X+20, "Please Enter a Username: ");
  refresh();
  getnstr(name, sizeof 14);
  noecho();
}

//Draws card on screen at desired position
void draw_card(int card, int number, int symbol, int shape, int color)
{
  int x, y, sym_ch, card_color;
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
  
  for(int j=1; j<CARD_HEIGHT+1; j++)
    {
      for(int k=0; k<CARD_WIDTH; k++)
	{
	  addch(CARD_CHAR);
	}
      move(y+j, x);
    }
  
  switch(symbol)
    {
    case 0:
      sym_ch = 64;
      break;
    case 1 :
      sym_ch = 111;
      break;
    case 2:
      sym_ch = KEY_SPACE;
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
  if(sym_ch == KEY_SPACE)
    {
      card_color += 4;
    }

  attron(COLOR_PAIR(card_color));
  switch(number)
    {
    case 0:
      switch(shape)
	{
	case 0: 
	  mvaddch(y+1, x+7, sym_ch);
	  mvaddch(y+2, x+7, sym_ch);
	  mvaddch(y+2, x+6, sym_ch);
	  mvaddch(y+2, x+8, sym_ch);
	  mvaddch(y+3, x+7, sym_ch);
	  break;

	case 1:
	  mvaddch(y+1, x+6, sym_ch);
	  mvaddch(y+1, x+7, sym_ch);
	  mvaddch(y+1, x+8, sym_ch);
	  mvaddch(y+2, x+8, sym_ch);
	  mvaddch(y+3, x+8, sym_ch);
	  mvaddch(y+3, x+7, sym_ch);
	  mvaddch(y+3, x+6, sym_ch);
	  mvaddch(y+2, x+6, sym_ch);
	  break;

	case 2:
	  mvaddch(y+3, x+6, sym_ch);
	  mvaddch(y+2, x+7, sym_ch);
	  mvaddch(y+1, x+8, sym_ch);
	  break;

	default:
	  break;
	}
      break;
      
    case 1:
      switch(shape)
	{
	case 0:
	  mvaddch(y+1, x+5, sym_ch);
	  mvaddch(y+2, x+5, sym_ch);
	  mvaddch(y+2, x+4, sym_ch);
	  mvaddch(y+2, x+6, sym_ch);
	  mvaddch(y+3, x+5, sym_ch);
	  
	  mvaddch(y+1, x+9, sym_ch);
	  mvaddch(y+2, x+9, sym_ch);
	  mvaddch(y+2, x+8, sym_ch);
	  mvaddch(y+2, x+10, sym_ch);
	  mvaddch(y+3, x+9, sym_ch);
	  break;
	  
	case 1:
	  mvaddch(y+1, x+4, sym_ch);
	  mvaddch(y+1, x+5, sym_ch);
	  mvaddch(y+1, x+6, sym_ch);
	  mvaddch(y+2, x+6, sym_ch);
	  mvaddch(y+3, x+6, sym_ch);
	  mvaddch(y+3, x+5, sym_ch);
	  mvaddch(y+3, x+4, sym_ch);
	  mvaddch(y+2, x+4, sym_ch);

	  mvaddch(y+1, x+8, sym_ch);
	  mvaddch(y+1, x+9, sym_ch);
	  mvaddch(y+1, x+10, sym_ch);
	  mvaddch(y+2, x+10, sym_ch);
	  mvaddch(y+3, x+10, sym_ch);
	  mvaddch(y+3, x+9, sym_ch);
	  mvaddch(y+3, x+8, sym_ch);
	  mvaddch(y+2, x+8, sym_ch);
	  break;

	case 2:					
	  mvaddch(y+3, x+4, sym_ch);
	  mvaddch(y+2, x+5, sym_ch);
	  mvaddch(y+1, x+6, sym_ch);

	  mvaddch(y+3, x+8, sym_ch);
	  mvaddch(y+2, x+9, sym_ch);
	  mvaddch(y+1, x+10, sym_ch);
	  break;
	default:
	  break;
	}
      break;
    case 2:
      switch(shape)
	{
	case 0:
	  mvaddch(y+1, x+3, sym_ch);
	  mvaddch(y+2, x+3, sym_ch);
	  mvaddch(y+2, x+2, sym_ch);
	  mvaddch(y+2, x+4, sym_ch);
	  mvaddch(y+3, x+3, sym_ch);
	  
	  mvaddch(y+1, x+7, sym_ch);
	  mvaddch(y+2, x+7, sym_ch);
	  mvaddch(y+2, x+6, sym_ch);
	  mvaddch(y+2, x+8, sym_ch);
	  mvaddch(y+3, x+7, sym_ch);
	  
          mvaddch(y+1, x+11, sym_ch);
	  mvaddch(y+2, x+11, sym_ch);
	  mvaddch(y+2, x+10, sym_ch);
	  mvaddch(y+2, x+12, sym_ch);
	  mvaddch(y+3, x+11, sym_ch);
	  
	  break;

	case 1:
	  mvaddch(y+1, x+2, sym_ch);
	  mvaddch(y+1, x+3, sym_ch);
	  mvaddch(y+1, x+4, sym_ch);
	  mvaddch(y+2, x+4, sym_ch);
	  mvaddch(y+3, x+4, sym_ch);
	  mvaddch(y+3, x+3, sym_ch);
	  mvaddch(y+3, x+2, sym_ch);
	  mvaddch(y+2, x+2, sym_ch);

	  mvaddch(y+1, x+6, sym_ch);
	  mvaddch(y+1, x+7, sym_ch);
	  mvaddch(y+1, x+8, sym_ch);
	  mvaddch(y+2, x+8, sym_ch);
	  mvaddch(y+3, x+8, sym_ch);
	  mvaddch(y+3, x+7, sym_ch);
	  mvaddch(y+3, x+6, sym_ch);
	  mvaddch(y+2, x+6, sym_ch);

	  mvaddch(y+1, x+10, sym_ch);
	  mvaddch(y+1, x+11, sym_ch);
	  mvaddch(y+1, x+12, sym_ch);
	  mvaddch(y+2, x+12, sym_ch);
	  mvaddch(y+3, x+12, sym_ch);
	  mvaddch(y+3, x+11, sym_ch);
	  mvaddch(y+3, x+10, sym_ch);
	  mvaddch(y+2, x+10, sym_ch);

	  break;

	case 2:
	  mvaddch(y+3, x+2, sym_ch);
	  mvaddch(y+2, x+3, sym_ch);
	  mvaddch(y+1, x+4, sym_ch);

	  mvaddch(y+3, x+6, sym_ch);
	  mvaddch(y+2, x+7, sym_ch);
	  mvaddch(y+1, x+8, sym_ch);

	  mvaddch(y+3, x+10, sym_ch);
	  mvaddch(y+2, x+11, sym_ch);
	  mvaddch(y+1, x+12, sym_ch);
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
  int letter = 97;
  
  for(int i=0; i<3; i++)
    {
      for(int j=0; j<4; j++)
	{
	  move(ORIGIN_Y + ((i)*(CARD_HEIGHT+ROW_OFFSET)),
	       ORIGIN_X + ((j)*(CARD_WIDTH+COL_OFFSET)));
	  addch(letter);
	  letter+=1;
	  
	}
    }
}

void submenu_actions(WINDOW* win)
{
  mvwprintw(win,0,4,"Menu");
  mvwprintw(win,2,3,"1. Option");
  mvwprintw(win,3,3,"2. Option");
  mvwprintw(win,4,3,"3. Close");
  
  for(;;)
  {
    int ch = wgetch(win);
    int highlight = 1;

    switch(ch)
      {
      case 1:
	mvwprintw(win,5,3,"1");
	break;

      case 2:
	mvwprintw(win,5,3,"2");
	break;

      case 3:
	mvwprintw(win,5,3,"3");
	break;
	
      case KEY_UP:
	highlight-=1;
	break;

      case KEY_DOWN:
	highlight+=1;
	break;

      case '\r':
	break;

      default:
	mvwprintw(win,5,3,"%c",ch);
	wrefresh(win);
	usleep(200000);
	mvwprintw(win,5,3,"    ");
	wmove(win,5,3);
	break;
      }
	
    wrefresh(win);
  }
}

//int get_card(int row, int column)
int get_card(int card)
{
  //Retrieve card by row and column
  //return row + (3 * (row-1) + (column-1));
}

void print_card_stats(int card)
{
  mvprintw(31,10,"Card:%c", card);
  mvprintw(32,10,"Choice String:%s", choice_string.c_str());
  mvprintw(33,10,"Random:%d", rand());
}

void animate_cards(vector<int>cards, int rate)
{
  vector<int>xls;
  vector<int>xhs;
  vector<int>yls;
  vector<int>yhs;
  vector<int>mids;

  int row, column, mid, x1, x2, y1, y2;
  for(int c=0; c<cards.size(); c++)
    {
      switch(cards[c])
	{
	case 1:
	  row = 1;
	  column = 1;
	  break;
	case 2:
	  row = 1;
	  column = 2;
	  break;
	case 3:
	  row = 1;
	  column = 3;
	  break;
	case 4:
	  row = 1;
	  column = 4;
	  break;
	case 5:
	  row = 2;
	  column = 1;
	  break;
	case 6:
	  row = 2;
	  column = 2;
	  break;
	case 7:
	  row = 2;
	  column = 3;
	  break;
	case 8:
	  row = 2;
	  column = 4;
	  break;
	case 9:
	  row = 3;
	  column = 1;
	  break;
	case 10:
	  row = 3;
	  column = 2;
	  break;
	case 11:
	  row = 3;
	  column = 3;
	  break;
	case 12:
	  row = 3;
	  column = 4;
	  break;
	default:
	  break;
    }

  
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
  switch(card)
    {
    case 1:
      row = 1;
      column = 1;
      break;
    case 2:
      row = 1;
      column = 2;
      break;
    case 3:
      row = 1;
      column = 3;
      break;
    case 4:
      row = 1;
      column = 4;
      break;
    case 5:
      row = 2;
      column = 1;
      break;
    case 6:
      row = 2;
      column = 2;
      break;
    case 7:
      row = 2;
      column = 3;
      break;
    case 8:
      row = 2;
      column = 4;
      break;
    case 9:
      row = 3;
      column = 1;
      break;
    case 10:
      row = 3;
      column = 2;
      break;
    case 11:
      row = 3;
      column = 3;
      break;
    case 12:
      row = 3;
      column = 4;
      break;
    default:
      break;
    }
  
  x1 = ORIGIN_X + ((column - 1)*(CARD_WIDTH+COL_OFFSET))+1;
  x2 = x1 - 1 +CARD_WIDTH-1;
  y1 = ORIGIN_Y + ((row - 1)*(CARD_HEIGHT+ROW_OFFSET));
  y2 = y1+CARD_HEIGHT-1;

//Clear previous highlight
  if(x2 != 0)
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
  /*//Clear previous highlight
  if(cur_x2 != 0)
    {
      //1.Left Border
      move(cur_y1-1, cur_x1-2);
      addch(32);
      for(int i=0; i<CARD_HEIGHT+1; i++)
	{
	  move(cur_y1+i, cur_x1-2);
	  addch(32);
	}

      //2.Top Border
      move(cur_y1-1, cur_x1-1);
      addch(32);
      for(int j=0; j<CARD_WIDTH; j++)
	{
	  move(cur_y1-1, j+cur_x1-1+1);
	  addch(32);
	}

      //3.Right Border
      move(cur_y1-1, cur_x2+1);
      addch(32);
      for(int k=0; k<CARD_HEIGHT; k++)
	{
	  move(cur_y1+k, cur_x2+1);
	  addch(32);
	}

      //4.Bottom Border
      move(cur_y2+1, cur_x1-1);
      addch(32);
      for(int l=0; l<CARD_WIDTH; l++)
	{
	  move(cur_y2+1, l+cur_x1-1+1);
	  addch(32);
	}

    }
  */
  switch(card)
    {
    case 1:
      row = 1;
      column = 1;
      break;
    case 2:
      row = 1;
      column = 2;
      break;
    case 3:
      row = 1;
      column = 3;
      break;
    case 4:
      row = 1;
      column = 4;
      break;
    case 5:
      row = 2;
      column = 1;
      break;
    case 6:
      row = 2;
      column = 2;
      break;
    case 7:
      row = 2;
      column = 3;
      break;
    case 8:
      row = 2;
      column = 4;
      break;
    case 9:
      row = 3;
      column = 1;
      break;
    case 10:
      row = 3;
      column = 2;
      break;
    case 11:
      row = 3;
      column = 3;
      break;
    case 12:
      row = 3;
      column = 4;
      break;
    default:
      break;
    }
  
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
    curs_set(0);
    char user[15];
    splash_screen( user );

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
    
    /*start_color();

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_WHITE);
    init_pair(3, COLOR_YELLOW, COLOR_WHITE);
    init_pair(4, COLOR_BLUE, COLOR_WHITE);
    init_pair(5, COLOR_RED, COLOR_RED);
    init_pair(6, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(7, COLOR_BLUE, COLOR_BLUE);

    WINDOW *popup_win;
 
    getmaxyx(stdscr, row, column);
    popup_win = newwin(10, 30, row/2-5, column/2-15);
    keypad(popup_win, TRUE);
    box(popup_win, 0, 0);
  
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    
  
    //Draw 12 cards on the screen
    for ( int i=1; i<13; i++ )
    {
       draw_card(i, rand(), rand(), rand(), rand());
    }

    //Draw card IDs
    draw_card_IDs();
    refresh();

    int ch, lx, ly, card; 
  
    //Wait for user input
    /*
    for(;;)
    {
      move(30,10);
      ch = getch();
      card = ch - 96;

      switch(ch)
	{
	case 109:
	  {
	  touchwin(popup_win);
	  submenu_actions(popup_win);
	  break;
	  }
	
	case KEY_SPACE:
	  {
	  printw("WHOOSH!");
	  getyx(stdscr,ly,lx);
	  vector<int>cards;

	  if(choice_string.size()>0)
	    {
	      for(int d=0; d<choice_string.size(); d++)
		{
		  cards.push_back((int)choice_string[d] - 96);
		} 
	    }
	  
	  else
	    {
	      for(int t=0; t<12; t++)
		{
		  cards.push_back(t+1);
		}
	    }
	  
	  animate_cards(cards, 20000);
	  
	  //Redraw cards
	  for(int s=0; s<cards.size(); s++)
	    {
	      draw_card(s, rand(), rand(), rand(), rand());
	    }

	  move(ly,lx);
	  break;
	  }

	default:
	  mvprintw(33,10,"Key Pressed:%c", ch);
	  break;
	}

      if(choice_string.size()==3)
	{
	  //Empty choice string
	  choice_string = "";
	  //Delete all card highlights
	  for(int i=1; i<13; i++)
	    {
	      dehighlight_card(i);
	    }
	}

      if(choice_string.find((char)ch)==-1 && (ch>95 && ch<109))
	{
	  choice_string+= ((char)ch);
	  highlight_card(card);
	}

      move(32,18);
      clrtoeol();
      print_card_stats(ch);
      clrtoeol();
      refresh();
    }
    */  
    my_client->wait_for_input();
    cout << "nCurses has exited. " << endl;
    endwin();
    my_client->cleanup();

  }

}
