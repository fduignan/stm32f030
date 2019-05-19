#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "console.h"
#include "spi.h"
#include "timer.h"
#include "brick.h"
#include "sprite.h"
#define COURT_WIDTH 200
#define COURT_HEIGHT 200
// A rendition of the classic table tennis (ping-pong) game.
// The bat behaves in an interesting way.  If the ball hits the bat in the centre then the ball bounces
// parallel to the X axis.  As the point of impact moves away from the centre of the bat the angle of
// bounce (with respect to the X axis) increases - reaching almost 90 degrees at the extreme edges.
// What resolution is sufficient here?  Lets say +/-10 (for a bat that is 20 pixels high) So...
// If the ball hits at the centre : BallYVelocity -> 0
// If the ball hits the bat at any other point: BallYVelocity = DistanceFromCentre/2 (pos or neg)
// 
void Tennis(void)
{
    uint32_t Done=0;
    uint32_t TopX=0;
    int LeftSide=0; // If this is a '1' then 
    int8_t BallXVelocity = 1;
    int8_t BallYVelocity = 1;   
    uint32_t TopY=(SCREEN_HEIGHT-COURT_HEIGHT)/2;
    Console.fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_WIDTH, 0);    
    // Ask the user if they want to start or join a game    
    Console.print("Press Right for a new game",sizeof("Press Right for a new game")-1,10,10,COLOUR(255,255,255),COLOUR(0,0,0));
    Console.print("Press Left to join a game",sizeof("Press Left to join a game")-1,10,30,COLOUR(255,255,255),COLOUR(0,0,0));
    Console.print("Any other key to cancel",sizeof("Any other key to cancel")-1,10,50,COLOUR(255,255,255),COLOUR(0,0,0));
    uint16_t Choice=0;
    while(Choice==0)
    {
        Choice = Console.Controller.getButtonState();
        Console.Timer.sleep(50);
    }
    if (Choice==Console.Controller.Right)
    {
    }
    else if (Choice==Console.Controller.Left)
    {
        LeftSide = 1;
    }                    
    else 
    {
        return; // game cancelled
    }
    
    brick Bat(COURT_WIDTH, COURT_HEIGHT+TopY - 35, 3, 20 ,COLOUR(255,255,255));        
    brick Ball(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 3, 3,COLOUR(255,255,64));
    Bat.show();    
    if (!LeftSide) // Will assume the ball always starts on the right side so don't show yet
    {        
        Ball.show();    
    }
    
    Console.drawLine(0,TopY, SCREEN_WIDTH, TopY,COLOUR(255,64,64));
    Console.drawLine(0,TopY+COURT_HEIGHT, SCREEN_WIDTH, TopY+COURT_HEIGHT,COLOUR(255,64,64));
    if (!LeftSide)
    {
        Console.drawLine(TopX,TopY, TopX, TopY+COURT_HEIGHT,COLOUR(160,128,160));
    }
    else
    {
        Bat.move(SCREEN_WIDTH-COURT_WIDTH,Bat.getY());
        Console.drawLine(SCREEN_WIDTH-1,TopY, SCREEN_WIDTH-1, TopY+COURT_HEIGHT,COLOUR(160,128,160));
    }
    
    while(!Done)
    {
        // update the ball position
        Ball.move(Ball.getX()+BallXVelocity,Ball.getY()+BallYVelocity);
        if (Ball.getX() > COURT_WIDTH+TopX)
        {
            Ball.move(0,0);
        }
        // Hit the ball?
        if (Bat.touching(Ball.getX() + Ball.getWidth() , Ball.getY() + Ball.getHeight() / 2) || Bat.touching(Ball.getX() - Ball.getWidth() , Ball.getY() + Ball.getHeight() / 2) )
        {            
            BallXVelocity = -BallXVelocity;            
            BallYVelocity = -(((Bat.getY()+(Bat.getHeight()/2))-(Ball.getY() + (Ball.getHeight()/2))))/2;            
            Console.Sound.playTone(200,20);
        }
        // Did the ball hit the side of the court?
        if ( ( Ball.getY() + Ball.getHeight() >= (COURT_HEIGHT + TopY)) || (Ball.getY() - Ball.getHeight() <=  TopY))
        {
            BallYVelocity = -BallYVelocity;
        }
        // Temporary:
        if ( (Ball.getX() <=0) )
        {
            BallXVelocity = - BallXVelocity;
        }
        
        // Did the ball pass the bat?
        if ( (Ball.getX()-(Ball.getWidth()/2)) > (COURT_WIDTH+TopX))
        {
            Ball.move(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
            BallXVelocity = 1;
            BallYVelocity = 1;   
        }
        // Check if user has pushed the buttons
        if (Console.Controller.getButtonState() & Console.Controller.Up)
        {
            if (Bat.getY()>=1)
            {
                Bat.move(Bat.getX(),Bat.getY()-1);
            }            
        }
        if (Console.Controller.getButtonState() & Console.Controller.Down)
        {
            if (Bat.getY()<=SCREEN_HEIGHT - (Bat.getHeight()+1))
            {
                Bat.move(Bat.getX(),Bat.getY()+1);
            }                        
        }        
        
        Bat.redraw();
        Console.Timer.sleep(5);
    }
}
