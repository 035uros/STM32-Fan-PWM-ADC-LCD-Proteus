#include "stm32f103x6.h"

void btn(void);									//декларација функције и њених параметара
void delay(int num);							//декларација функције и њених параметара
//void ispisbroja(int broj);					//декларација функције и њених параметара у случају да користимо 7-сегменти дисплеј


int automod=1;									//флег који користимо ради превенције беспотребног приступа регистру тајмера 3
//функција која проверава стање контролних тастера
void btn(void)
{
    if (!((GPIOA->IDR >> 4) & 1)){				//проверамо да ли је пин PA4 "low"
    	if(automod == 1){						//уколико је то случај проверавамо да ли флег указује на то да настаје промена из ручног режима рада
    	TIM3->CR1 |= TIM_CR1_CEN;				//уколико је то случај омогућавамо counter тајмера уписивањем 1 на бит 0, стр. 405
    	automod = 0;							//постављамо флег на 0 како не бисмо понављали ову операцију убудуће
    	}
    }
    if((GPIOA->IDR >> 4) & 1){					//у случају да је пин PA4 "high", знамо да је у питању ручни режим рада
    	automod = 1;							//ресетујемо флег како бисмо могли поново до укључимо аутоматски режим
    	TIM3->CR1 &= ~TIM_CR1_CEN;				//уписујемо 0 на бит 0 контролног регистра тајмера 1 и тако ономогућавамо бројач, стр. 405
    	if (!((GPIOA->IDR >> 5) & 1)){			//уколико је притиснут тастер повезан на пин PA5, реч је о операцији убрзања мотора
    		if(TIM2->CCR4 - 50 >= 0){			//гранични услов како не бисмо дозволили "испадање" из опсега
    			TIM2->CCR4=TIM2->CCR4-50;		//смањујемо вредност Control capture регистра за канал 4 тајмера 2 и тиме мењамо duty cycle односно убрзавамо рад мотора
    			delay(1000);					//осигуравамо да је операција извршена
    		}else{
    			TIM2->CCR4=0;					//у случају излажења из опсега постављамо CCR на 0, односно PWM сигнал ће констатно бити "high" и тиме добијамо максималну брзину окретања
    			delay(1000);					//осигуравамо да је операција извршена
        	}
    	}
    	if(!(GPIOA->IDR >> 6 & 1)){				//уколико је притиснут тастер повезан на пин PA6, реч је о операцији успоравања мотора
    		if(TIM2->CCR4 + 50 <= 999){			//гранични услов како не бисмо дозволили "испадање" из опсега
        	TIM2->CCR4=TIM2->CCR4+50;			//повећамо вредност Control capture регистра за канал 4 тајмера 2 и тиме мењамо duty cycle дносно успоравамо рад мотора
        	delay(1000);						//осигуравамо да је операција извршена
    		}else{
    			TIM2->CCR4=999;					//у случају излажења из опсега постављамо CCR на 999, односно PWM сигнал ће констатно бити "low" и тиме у потпуности обустављамо окретање мотора
    			delay(1000);					//осигуравамо да је операција извршена
        	}
    	}

    }
}


//фукнција за генерисање PWM сигнала и декларацију тастера
void pwm(void){

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; 				// омогућавамо клок порта А уписивањем 1 на бит 2 APB2ENR регистра, стр. 114

	//декларација тастера
	GPIOA->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4); 	// бришемо подешавања за PA4
	GPIOA->CRL |= (GPIO_CRL_MODE4_0 | GPIO_CRL_CNF4_0); // сетујемо режими у Alternate function push-pull

	GPIOA->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5); 	// бришемо подешавања за PA5
	GPIOA->CRL |= (GPIO_CRL_MODE5_0 | GPIO_CRL_CNF5_0); // сетујемо режими у Alternate function push-pull

	GPIOA->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_CNF6); 	// бришемо подешавања за PA6
	GPIOA->CRL |= (GPIO_CRL_MODE6_0 | GPIO_CRL_CNF6_0); // сетујемо режими у Alternate function push-pull

	GPIOA->BSRR |= GPIO_BSRR_BS4; 						// PA4 high, декларишемо default вредност на логичку јединицу уписивањем 1 у bit set бит BSSR регистра, стр. 173
	GPIOA->BSRR |= GPIO_BSRR_BS5;						// PA5 high, декларишемо default вредност на логичку јединицу уписивањем 1 у bit set бит BSSR регистра, стр. 173
	GPIOA->BSRR |= GPIO_BSRR_BS6; 						// PA6 high, декларишемо default вредност на логичку јединицу уписивањем 1 у bit set бит BSSR регистра, стр. 173

	GPIOA->CRL &= ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3); 	// бришемо подешавања за PA3
	GPIOA->CRL |= (GPIO_CRL_MODE3_1 | GPIO_CRL_CNF3_1); // сетујемо режими у Alternate function push-pull, макс. брзина 2MHz

	// Одабрали смо тајмер TIM2, канал 4

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 				//омогућавамо клок тајмера 2 уписивањем 1 на бит 0 APB1ENR регистра, стр. 114

	// За добијање жељене брзине рада тајмера користимо следећу формулу и на основу ње уписујемо вредности у регистре
	// Timer driving frequency = 80 MHz/(l + PSC) = 80 MHz/(1+7) = 10MHz
	// frequency = 10MHz/(1 + ARR) = 10MHz/1000 = 10kHz

	TIM2->PSC = 7; 										// preslaler, служи да подели фреквенцију самог микроконтролера, стр. 419

	TIM2->ARR = 999;									// auto reaload регистрар, означава нам "до колико" бројимо, стр. 419

	TIM2->CCR4 = 500; 									// ширину имуплса подешавамо уписивањем вредности у Capture/Compare регистар, вредност ће бити 1 док је вредност бројача мања од Capture/Compare регистра, стр. 421 - стартна вредност означава рад на 50% duty cycle

	TIM2->CCMR2 &= ~TIM_CCMR2_OC4M; 					// Бришемо вредност Output compare 4 мода, стр. 416

	TIM2->CCMR2 |= (0b110 << TIM_CCMR2_OC4M_Pos); 		// Уписујемо 110 у OC4 битове регистра 2 capture/compare mode и тиме сетујемо режим рада у PWM мод 1, стр. 387

	TIM2->CCMR2 &= ~TIM_CCMR2_OC4PE;					// пишемо 0 у Output compare 4 preload enable биту што нам омогућава да мењамо вредност CCR4 регистра у било ком тренутку и да његова нова вредност ступа на стагу истог тренутка

	TIM2->CCER &= ~TIM_CCER_CC4P; 						// подешавамо поларитет Capture/Compare ->  0 = active high, capture/compare enable регистар стр. 417

	TIM2->CCER |= TIM_CCER_CC4E;						// сетујемо Capture/Compare 4 output enable бит и тиме омогућавамо излазни сигнал на његовом унапред декларисаном пину.

	TIM2->CR1 &= ~TIM_CR1_ARPE; 						// пишемо 0 у Auto-reload preload enable бит контролног регистра 1, тиме онемогућујемо баферовање ARR регситра, стр. 404

	TIM2->CR1 |= TIM_CR1_CEN;							// уписивањем 1 на бит 0 контролног регистра 1 омогућујемо бројач тајмера 2, стр. 405

}

