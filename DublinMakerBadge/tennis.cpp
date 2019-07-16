#include <stdint.h>
#include "../include/STM32F0x0.h"
#include "console.h"
#include "spi.h"
#include "timer.h"
#include "brick.h"
#include "sprite.h"
#define COURT_WIDTH 200
#define COURT_HEIGHT 200
#define MAX_SCORE 10
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
    uint8_t Message[4];
    uint32_t Done=0;
    uint32_t TopX=0;
    uint32_t Score=0;
    uint8_t OpponentAddress;
    int LeftSide=0; // If this is a '1' then 
    int8_t BallXVelocity = 1;
    int8_t BallYVelocity = 1;   
    uint32_t TopY=(SCREEN_HEIGHT-COURT_HEIGHT)/2;
    while(Console.Controller.getButtonState()); // wait for the button to be released
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
        OpponentAddress = 2;
        Console.Ibc.setAddress(1);
    }
    else if (Choice==Console.Controller.Left)
    {
        LeftSide = 1;
        OpponentAddress = 1;
        Console.Ibc.setAddress(2);
    }                    
    else 
    {
        return; // game cancelled
    }
    Console.fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_WIDTH, 0); 
    if (!LeftSide)
    {
        Console.print("Press Fire to serve",sizeof("Press Fire to serve")-1,10,30,COLOUR(255,255,255),COLOUR(0,0,0));
        Console.Timer.sleep(1000);
        Console.fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_WIDTH, 0); 
    }
    brick Bat(COURT_WIDTH, COURT_HEIGHT+TopY - 35, 2, 40 ,COLOUR(255,255,255));        
    brick Ball(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 5, 5,COLOUR(255,255,64));
    Bat.show();    
    if (!LeftSide) // Will assume the ball always starts on the right side so don't show yet
    {        
        Ball.show();    
    }    
    Console.drawLine(0,TopY-5, SCREEN_WIDTH, TopY-5,COLOUR(255,64,64));
    Console.drawLine(0,TopY+5+COURT_HEIGHT, SCREEN_WIDTH, TopY+5+COURT_HEIGHT,COLOUR(255,64,64));
    if (!LeftSide)
    {        
        if (Console.Ibc.getPacket(4,Message) > 0)
        {
            
        }
        Console.drawLine(TopX,TopY, TopX, TopY+COURT_HEIGHT,COLOUR(160,128,160));
    }
    else
    {
        Bat.move(SCREEN_WIDTH-COURT_WIDTH,Bat.getY());
        Console.drawLine(SCREEN_WIDTH-1,TopY, SCREEN_WIDTH-1, TopY+COURT_HEIGHT,COLOUR(160,128,160));
    }
    if (!LeftSide)
        while (Console.Controller.getButtonState() != Console.Controller.Fire);
    while(!Done)
    {
        Console.print(Score,100,5,COLOUR(255,255,255),COLOUR(0,0,0));
        if (Ball.visible() )
        {
            // update the ball position
            Ball.move(Ball.getX()+BallXVelocity,Ball.getY()+BallYVelocity);
            // Hit the ball?
            if ( (Bat.touching(Ball.getX() + (Ball.getWidth()/2) , Ball.getY() + (Ball.getHeight() / 2)) )|| (Bat.touching(Ball.getX() - (Ball.getWidth()/2) , Ball.getY() + (Ball.getHeight() / 2)) ) )
            {            
                BallXVelocity = -BallXVelocity;                                            
                BallYVelocity = -(((Bat.getY()+(Bat.getHeight()/2))-(Ball.getY() + (Ball.getHeight()/2))))/4;            
                
                Console.Sound.playTone(200,20);
            }
            // Did the ball hit the side of the court?
            if ( ( Ball.getY() + Ball.getHeight() >= (COURT_HEIGHT + TopY)) || (Ball.getY() - Ball.getHeight() <=  TopY))
            {
                BallYVelocity = -BallYVelocity;
            }
            if (!LeftSide)
            { // Right side of court
            
                if (Ball.getX() <= 0)
                {            
                    Message[0]=1;
                    Message[1]=Ball.getY();
                    Message[2]=BallXVelocity;
                    Message[3]=BallYVelocity;
                    Console.Ibc.sendPacket(OpponentAddress,0,4,Message);
                    Ball.hide();
                }
                // Did the ball pass the bat? 
                if (Ball.getX() > Bat.getX() ) 
                {
                    Message[0]=2; 
                    Message[1]=Score;
                    Message[2]=0;
                    Message[3]=0;
                    Console.Ibc.sendPacket(OpponentAddress,0,4,Message);
                    Ball.hide();
                }
            }
            else
            {
                // Left side of court
                if (Ball.getX() >= SCREEN_WIDTH)
                {                
                    Message[0]=1; 
                    Message[1]=Ball.getY();
                    Message[2]=BallXVelocity;
                    Message[3]=BallYVelocity;
                    Console.Ibc.sendPacket(OpponentAddress,0,4,Message);
                    Ball.hide();
                }
                // Did the ball pass the bat?
                if  (Ball.getX() < Bat.getX()) 
                {
                    
                    
                    Message[0]=2; 
                    Message[1]=Score;
                    Message[2]=0;
                    Message[3]=0;
                    Console.Ibc.sendPacket(OpponentAddress,0,4,Message);
                    Ball.hide();
                }                
            }
        }
        else            
        {
            if (Console.Ibc.getPacket(4,Message) > 0)
            {
                switch (Message[0]) {
                    case 0x1: { // Ball left other player's screen heading for this one
                        Ball.show();
                        if (LeftSide)                    
                            Ball.move(SCREEN_WIDTH-1,Message[1]);
                        else
                            Ball.move(1,Message[1]);                                                    
                        BallXVelocity=Message[2];
                        BallYVelocity=Message[3];
                        Console.Sound.playTone(800,10);        
                        break;
                    }
                    case 2: {
                        // Ball has been lost by other player.
                        Score++;                        
                        if (Score >= MAX_SCORE)
                        {
                            Console.fillRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOUR(0x00,0x00,0xff));
                            Console.print("You win!",sizeof("You win!")-1,50,30,COLOUR(0xff,0xff,0x00),COLOUR(0x00,0x00,0xff));
                            Console.print("Press fire to continue",sizeof("Press fire to continue")-1,50,50,COLOUR(0xff,0xff,00),COLOUR(0x00,0x00,0xff));
                            uint32_t timeout=1000;
                            while( (Console.Controller.getButtonState() != Console.Controller.Fire) && timeout--)
                            {
                                Console.Timer.sleep(50);
                            }
                            return;
                        }
                        Console.Sound.playTone(2000,10);
                        break;
                    }
                }                                
            }
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
        if (Console.Controller.getButtonState() & Console.Controller.Fire)
        {   
            if (!LeftSide)
            {
                if (Ball.visible() == 0)
                {
                    
                    Ball.move(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
                    BallXVelocity = 1;
                    BallYVelocity = 1;
                    Ball.show();    
                }
            }
        }
        Bat.redraw();
        Console.Timer.sleep(5);
        
    }
}
