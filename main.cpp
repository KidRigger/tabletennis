#include <allegro.h>
#include <allegro_primitives.h>
#include <allegro_font.h>
#include <allegro_color.h>
#include <allegro_ttf.h>
#include <allegro_native_dialog.h>
#include <math.h>
#include <sstream>

#define ScreenWidth 1440
#define ScreenHeight 900

const float FPS = 50.0;
const int PadW = 10;
const int PadH = 30;
const int PadShift = 130;

class Paddle
{
   private:
      int x,y;
      int width, height;
      ALLEGRO_COLOR self_color;
   public:
      Paddle(int _x, int _y, const int &_w, const int &_h, const ALLEGRO_COLOR &_color)
      {
         x = _x;
         y = _y;
         width = _w;
         height = _h;
         self_color = _color;
      }

      void Draw()
      {
         al_draw_filled_rectangle(x,y,x+width,y+height,self_color);
      }

      void MoveTo(int _x, int _y)
      {
         //bound condition
         if(x < 0)
            x = 0;
         else if(x > ScreenWidth-width)
            x = ScreenWidth - width;
         else
            x = _x;  //if in bounds

         //bound condition
         if(y < 0)
            y = 0;
         else if (y > ScreenHeight - height)
            y = ScreenHeight - height;
         else
            y = _y;  //if in bounds
      }

      void Translate(int _x, int _y)
      {
         x += _x;

         //bound check
         if (x > ScreenWidth - width)
            x = ScreenWidth - width;
         else if (x < 0)
            x = 0;

         y += _y;

         //bound check
         if (y > ScreenHeight - height)
            y = ScreenHeight - height;
         else if (y < 0)
            y = 0;
      }

      friend class ICollider;

};

class Ball
{
   private:
      int x, y, radius;
      ALLEGRO_COLOR self_color;
      int vx, vy, moveSpeed;

      void Bound(void)
      {  //Bounds the ball to stay in y bounds
         if(y > ScreenHeight-radius)
         {
            y = ScreenHeight-radius;
            vy = -1;
         }
         else if(y < radius)
         {
            y = radius;
            vy = 1;
         }
      }

      void ApplySpeed(void)
      {  //Applies the the movement delta
         x += vx * moveSpeed;
         y += vy * moveSpeed;
      }

   public:
      Ball(int _x, int _y, int _rad, const ALLEGRO_COLOR &_color, int _moveSpeed)
      {
         x = _x;
         y = _y;
         radius = _rad;
         self_color = _color;
         vx = 1;
         vy = 1;
         moveSpeed = _moveSpeed;
      }

      void Draw(void)
      {
         this->ApplySpeed();
         this->Bound();

         al_draw_filled_circle(x,y,radius,self_color);
      }

      void Reset(int _side)
      {
         x = ScreenWidth/2;
         y = ScreenHeight/2;
         vx = _side;
         vy = -vy;
      }

      friend class ICollider;
      friend class Manager;
};

class ICollider
{
   private:
      int side;
      Ball *ball;
      Paddle *pad;

      bool Collided()
      {  //collision fail check
         if (
            (pad->x  >= ball->x +  ball->radius) ||
            (pad->y  >= ball->y +  ball->radius) ||
            (ball->x >= pad->x  +  pad->width  ) ||
            (ball->y >= pad->y  +  pad->height )
            )
         {
            return false;
         }

         return true;
      }
   public:
      ICollider (Paddle &_pad, Ball &_ball, int _side)
      {
         pad = &_pad;
         ball = &_ball;
         side = _side;
      }

      void Collision()
      {
         if(this->Collided())
         {
            if (ball->y - pad->y < PadH/3)
               ball->vy = -1;
            else if(ball->y - pad->y > (2*PadH)/3)
               ball->vy = 1;

            ball->vx = side;
         }
      }
};

class ScoreUI
{
   private:
      int val;
      const ALLEGRO_FONT *font;
      ALLEGRO_COLOR self_color;
      int x,y,side;

   public:
      ScoreUI(int _x, int _y, const ALLEGRO_FONT *_font, ALLEGRO_COLOR _color, int _side)
      {
         val = 0;
         x = _x;
         y = _y;
         font = _font;
         self_color = _color;
         side = _side;
      }

      void Increment(void)
      {
         ++val;
      }

      void Reset(void)
      {
         val = 0;
      }

      void Draw()
      {
         std::stringstream str_value;
         str_value << val;
         const char* score = str_value.str().c_str();
         al_draw_text(font, self_color,
         (x + 75 - sizeof(score)*50*side), y, ALLEGRO_ALIGN_LEFT, score);
      }
      friend class Manager;
};

class Manager
{
  private:
      Ball * ball;
      ScoreUI * p1, * p2;

  public:
      Manager(Ball * _ball, ScoreUI * P1, ScoreUI * P2)
      {
         ball = _ball;
         p1 = P1;
         p2 = P2;
      }

      void ScoreUpdate(void)
      {
         if(ball->x < 0)
         {
            p2->Increment();
            ball->Reset(1);
         }
         else if(ball -> x > ScreenWidth)
         {
            p1->Increment();
            ball->Reset(-1);
         }

         if(p1 -> val == 11)
         {
            al_show_native_message_box(NULL,"Win","Victory","Player 1 wins!",NULL,NULL);
            exit(0);
         }
         else if(p2 -> val == 11)
         {
            al_show_native_message_box(NULL,"Win","Victory","Player 2 wins!",NULL,NULL);
            exit(0);
         }
      }
};

class Input
{
  private:
      int x[2];
      ALLEGRO_KEYBOARD_STATE keystate;
  public:

      Input()
      {
         al_install_keyboard();
      }

      bool update(void)
      {
         al_get_keyboard_state(&keystate);

         //Player 1 Arrow Up/Down
         if(al_key_down(&keystate, ALLEGRO_KEY_UP))
         {
            x[0] = -1;
         }
         else if(al_key_down(&keystate, ALLEGRO_KEY_DOWN))
         {
            x[0] = +1;
         }
         else
         {
            x[0] = 0;
         }

         //Player 2 W/S
         if(al_key_down(&keystate, ALLEGRO_KEY_W))
         {
            x[1] = -1;
         }
         else if(al_key_down(&keystate, ALLEGRO_KEY_S))
         {
            x[1] = 1;
         }
         else
         {
            x[1] = 0;
         }

         if (al_key_down(&keystate, ALLEGRO_KEY_ESCAPE))
         {
            return false;
         }

         return true;
      }

      //For output of the axes
      int GetAxis(int player)
      {
         return x[player-1];
      }
};



int main(int argc, char ** argv)
{

   if(!al_init())
   {  //checks for allegro
      al_show_native_message_box(NULL,"ERROR","Allegro Error","Allegro could not be initialized!",NULL,ALLEGRO_MESSAGEBOX_ERROR);
      return -1;
   }

   //Colors created
   ALLEGRO_COLOR color_black = al_map_rgb(0,0,0);
   ALLEGRO_COLOR color_white = al_map_rgb(255,255,255);
   //...

   //Create New Display
   ALLEGRO_DISPLAY *display = NULL;
   al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW|ALLEGRO_NOFRAME);
   display = al_create_display(ScreenWidth,ScreenHeight);
   if(!display)
   {  //checks for display
      al_show_native_message_box(display,"ERROR","Display Error","Display could not be created!",NULL,ALLEGRO_MESSAGEBOX_ERROR);
      return -1;
   }
   al_clear_to_color(color_black);
   //...

   //addons init
   al_init_primitives_addon();
   al_init_font_addon();
   al_init_ttf_addon();

   //font creation
   ALLEGRO_FONT *font = al_load_font("digital-7.ttf", 70, NULL);

   //input
   Input input;

   //timer
   ALLEGRO_TIMER *timer = NULL;
   timer = al_create_timer(1.0/FPS);

   if(!timer)
   {
      al_show_native_message_box(display,"ERROR","Timer Error","Allegro could not create timer!",NULL,ALLEGRO_MESSAGEBOX_ERROR);
      return -1;
   }

   //event queue
   ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
   al_register_event_source( event_queue, al_get_keyboard_event_source());
   al_register_event_source( event_queue, al_get_timer_event_source(timer));



   //create objects;
   Paddle player1(ScreenWidth-PadShift,425,PadW,PadH,color_white);
   Paddle player2(PadShift,425,PadW,PadH,color_white);
   Ball ball1(ScreenWidth/2,ScreenHeight/2,5,color_white,4);
   ScoreUI sc1(ScreenWidth/2 - 100, 50, font, color_white,1);
   ScoreUI sc2(ScreenWidth/2 - 100, 50, font, color_white,-1);
   Manager man(&ball1,&sc1,&sc2);

   //Colliders
   ICollider col1(player1,ball1,-1);
   ICollider col2(player2,ball1,1);

   bool isRunning = true;
   int moveSpeed = 5;

   al_start_timer(timer);

   while(isRunning)
   {  //Game loop
      ALLEGRO_EVENT event;    //Event handling
      al_wait_for_event(event_queue, &event);

      isRunning = input.update();      //Input update

      if(event.type == ALLEGRO_EVENT_TIMER)
      {  //Timer event
         al_clear_to_color(color_black);  //clrscr
         player1.Translate(0,input.GetAxis(1) * moveSpeed); //Moving P1
         player2.Translate(0,input.GetAxis(2) * moveSpeed); //Moving P2

         //Drawing all objects
         player1.Draw();
         player2.Draw();
         col1.Collision();
         col2.Collision();
         ball1.Draw();
         man.ScoreUpdate();
         sc1.Draw();
         sc2.Draw();


         al_draw_filled_rectangle(ScreenWidth/2 - 2, 0, ScreenWidth/2 + 2, ScreenHeight, color_white);

         //refresh screen
         al_flip_display();
      }
   }

   //Destructors
   al_destroy_font(font);
   al_destroy_event_queue(event_queue);
   al_destroy_timer(timer);
   al_destroy_display(display);
   //...
}


