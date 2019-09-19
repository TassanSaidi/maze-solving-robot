//********************************************************************
//*             EEE3099S Line Following Robot Code                   *
//*==================================================================*
//* WRITTEN BY: Mueez Allie & Stefan Dominicus 		                 *
//* DATE CREATED: 19 September 2018                                  *
//* MODIFIED:                                                        *
//*==================================================================*
//* PROGRAMMED IN: Eclipse Luna Service Release 1 (4.4.1)            *
//* DEV. BOARD:    UCT STM32 Development Board                       *
//* TARGET:	   STMicroelectronics STM32F051C6                    	 *
//*==================================================================*
//* DESCRIPTION:                                                 	 *
//*                                                                  *
//********************************************************************
// INCLUDE FILES
//====================================================================
#include <stdio.h>
#include "stm32f0xx.h"
//====================================================================
//Constants
#include <cxxabi.h>

#define	goButton					GPIO_IDR_0		//Go Button
#define statusLED					GPIO_ODR_6		//Status LED on pin 6 of ord being used
#define	lineRightPin				GPIO_IDR_9		//Right Line Sensor
#define	lineCenterPin				GPIO_IDR_8		//Center Line Sensor
#define	lineLeftPin					GPIO_IDR_7		//Left Line Sensor
#define wideLeftPin					GPIO_IDR_12
#define wideRightPin				GPIO_IDR_13
#define startDelay					1000000			//Delay after the Go Button is released
#define flashDelay					1500000			//Delay used when flashing the Status LE

//global variables

    uint8_t mode = 0;				//0=ERROR, 1=READY, 2=DRIVE, 3=Right Turn, 4=Left Turn, 5=STOP
    uint16_t leftMotor;
    uint16_t  rightMotor;
    int total_points;
    int direction;				//stores direction that robot is facing 1234-->ENWS
    int time_distance;			//stores time based distance in x or y direction
    int coordinate[2][30];      //stores time based x and y co-ordinates of points
    int point[30];				//stores integer number that identifies point
    int type[30];				//stores the type of point, equal to number of paths leading to point -1
    int explored[30];           //stores the number of paths already explored leading to point
    int unexplored[4]={0,0,0,0};//stores the available unexplored paths that may be taken


//Function Declarations
 //Initialisation functions
    void initPorts(void);
    void init_NVIC(void);
    void init_EXTI(void);
    void update_data(void);   //updates data arrays
    int calc_distance(int);	  //calculates distance traveled from previous point
    int sel_direction(void);  //selects direction to move next based on order of preference
    void change_direction(int);//change direction based on input selected direction
    void follow_line(void);   //set motors to follow line in direction robot is facing
    void set_start(void);     //sets data values for starting point
    void check_explored(void); //sets global array of available unexplored paths
    int check_paths(void);    //returns number of paths surrounding the current point - 1


 //Interrupt Handler Functions
    void Go_Button_EXTI0_1_IRQHandler(void);
    void Sensor_EXTI_IRQHandler(void); // choose ports numbers for 14 and 15

 //General robot functions
    void stop(void); // code to make robot pause either at end of maze or at intersection for processing
    void move(void); // keep moving forward when on blacklike and no intersection is found
    void direction(void); // register direction when turning

//Maze Mapping Functions
void set_start(void){

	point[0]=0;
	coordinate[0][0]=0;
	coordinate[1][0]=0;
	Type[0]=0;
	explored[0]=check_paths();
	direction=2;
	total_points=1;


}


int check_paths(){

	int paths=0;

	paths = wideRightPin+WideleftPin+lineCentrePin;

	return paths;


}

void check_explored(){
	//sets available paths
	if (GPIO->IDR &right_wide){
		unexplored[1]=1;
	}

	if (GPIO->IDR &left_wide){
		unexplored[1]=1;
	}

	if (GPIO->IDR &right_wide){
		unexplored[1]=1;
	}

	if (GPIO->IDR &right_wide){
		unexplored[1]=1;
	}

	//sets unexplored available paths



}
// Main function
void main(void) {

    //Initialization

    direction();
    init_NVIC();
    init_EXTI();

    //Turn on Status LED on pin 6 of ODR
    GPIOB->ODR |= statusLED;

    //Loop forever
    for(;;);

}



//Interrupt handler for go button which is connected to GPIO B

void Go_Button_EXTI0_IRQHandler(void){
    /*apply debouncing later
     *
     *
     *

     * */

    EXTI->PR |= EXTI_PR_PR0; // clear interrupt pending bit since button was pressed


    EXTI-> IMR| EXTI0_IMR_MR0; // unmasking interrupt bit for button

    //Unmask interrupts for sensors
    EXTI->IMR |=EXT_IMR_MR4; //front sensor
    EXTI->IMR |=EXT_IMR_MR5;//centre sensor1
    EXTI->IMR |=EXT_IMR_MR6;//centresensor 2
    EXTI->IMR |=EXT_IMR_MR7;//left wide sensir
    EXTI->IMR |=EXT_IMR_MR8;//right wide sensor

    mode=1; // now ready to move

}

//assumes driving mode 2
void Sensor_EXTI_IRQHandler(void){

    EXTI->PR |= EXTI_PR_PR0;// removes interrupt pending state




}


//attatch edges to pushbutton(s) and configure external interrupt
void init_EXTI(void){



    RCC-> APB2ENR |=RCC_APB2ENR_SYSCFGCOMPEN;//ENABLE CLOCK

    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB; 	//Go Button
    SYSCFG->EXTICR[2] |= SYSCFG_EXTICR1_EXTI4_PB; 	//Front sensor
    SYSCFG->EXTICR[2] |= SYSCFG_EXTICR1_EXTI5_PB; 	//Centre 1
    SYSCFG->EXTICR[2] |= SYSCFG_EXTICR1_EXTI6_PB; 	//Centre 2
    SYSCFG->EXTICR[2] |= SYSCFG_EXTICR1_EXTI7_PB; 	//Wide left
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR1_EXTI9_PB; 	//Wide right



    //attach edges FTSR=falling ede RTSR=rising edge
    //Both edges attatched for flexibility (Black->White surface) and (White->Black)
    EXTI-> FTSR =EXTI_FTSR_TRO;
    EXTI-> FTSR =EXTI_FTSR_TR4;
    EXTI-> FTSR =EXTI_FTSR_TR5;
    EXTI-> FTSR =EXTI_FTSR_TR6;
    EXTI-> FTSR =EXTI_FTSR_TR7;
    EXTI-> FTSR =EXTI_FTSR_TR9;

    EXTI-> RTSR =EXTI_RTSR_TR0;
    EXTI-> RTSR =EXTI_RTSR_TR4;
    EXTI-> RTSR =EXTI_RTSR_TR5;
    EXTI-> RTSR =EXTI_RTSR_TR6;
    EXTI-> RTSR =EXTI_RTSR_TR7;
    EXTI-> RTSR =EXTI_RTSR_TR9;



    //Unmask interrupt for Go Button
    EXTI->IMR |= EXTI_IMR_MR0;




}
