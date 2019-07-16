// Moonlander game for the Dublin Maker badge 2019 (50th anniversary of the first moon landing)


#include <stdint.h>
#include "sprite.h"
#include "console.h"
#include "moonlander.h"

void PlayMoonlander()
{
    int lunar_deltaV = 1; // DeltaV per clock tick due to lunar gravity
    int Vx,Vy; // LEM x and y velocities    
    int altitude;
    int fuel=2000;    
    int DeltaVUpdateCounter = 50;
    int DeltaVReload = 50;
    int Done=0;
    int TargetX;
    TargetX = Console.random(1,219);
    Vx = Vy = 0;
    sprite LEM(LEMImage1,100,20,10,12);    
    // clear the screen 
    Console.fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    // Draw some random stars
    for (int starcount = 0; starcount < 20; starcount++)
    {
        Console.putPixel(Console.random(0,SCREEN_WIDTH-1),Console.random(0, 200),COLOUR(255,255,255));
    }

    LEM.show();        
    Console.fillRectangle(0,200, SCREEN_WIDTH, 20, COLOUR(255,255,255));         
    Console.fillRectangle(TargetX,200,20,10,COLOUR(255,128,128));
    Console.print("Fuel=",5,5,SCREEN_HEIGHT-10,COLOUR(255,255,255),0);    
    while (!Done)
    {    
        
        if (fuel > 0) 
        {
            if (Console.Controller.getButtonState() & Console.Controller.Up)
            {            
                Vy += -3;
                fuel-=2;
                LEM.setimage(LEMImage2);            
            }
            else
            {                
                LEM.setimage(LEMImage1);            
            }
            if (Console.Controller.getButtonState() & Console.Controller.Left)
            {
                fuel-=1;
                LEM.move(LEM.getX()-1,LEM.getY());            
            }
            if (Console.Controller.getButtonState() & Console.Controller.Right)
            {
                fuel-=1;
                LEM.move(LEM.getX()+1,LEM.getY());            
            }
            if (fuel < 0)
                fuel = 0;
            if (Vy > 25)
                Vy = 25;
            if (Vy < -5)
                Vy = -5;
        }
        DeltaVUpdateCounter--;
        if (DeltaVUpdateCounter < 1)
        {   
            Vy+=lunar_deltaV ;
            if (Vy > 0)
                LEM.move(LEM.getX(),LEM.getY()+1);// LEM.getY()+Vy/100);         
            if (Vy < 0)
                LEM.move(LEM.getX(),LEM.getY()-1);// LEM.getY()+Vy/100);         
            DeltaVReload -=Vy;
            if (DeltaVReload < 1)
                DeltaVReload = 1;
            DeltaVUpdateCounter=DeltaVReload;
        }
        if (LEM.getY() > 188)
        {
            if (Vy > 20)
            {
                Console.fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOUR(255,0,0));
                Console.print("The Eagle has crashed",sizeof("The Eagle has crashed")-1,5,20,COLOUR(255,255,255),COLOUR(255,0,0));
                
            }
            else
            {
                if ( (LEM.getX() >= TargetX) && (LEM.getX() <= TargetX+20))
                {
                    // Landed on target and at a reasonable speed
                    Console.fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOUR(0,0,255));
                    Console.print("The Eagle has landed!",sizeof("The Eagle has landed!")-1,5,20,COLOUR(255,255,255),COLOUR(0,0,255));
                    
                }
                else
                {
                    // landed but in the wrong place
                    Console.fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOUR(255,255,0));
                    Console.print("The Eagle has missed!",sizeof("The Eagle has missed!")-1,5,20,COLOUR(255,255,255),COLOUR(255,255,0));
                    
                }
            }
            Console.Timer.sleep(2000);
            return;
        }
        Console.print(fuel,70,SCREEN_HEIGHT-10,COLOUR(255,255,255),0);
        Console.Timer.sleep(20);
        
    }
    
}
