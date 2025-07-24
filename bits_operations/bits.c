
#include <stdio.h>

void positive_to_bits(void){
        int input = 0;
        scanf("%d",&input);
	int podopitniy = input;
	printf("Обратный вариант\n");
        while(podopitniy){
                printf("%d", (podopitniy & 1));
                podopitniy = podopitniy >> 1;
        }
	podopitniy = input;
	int bit_counter = (sizeof(input) * 8)-1;
	printf("Наверное неэффективный прямой вариант\n");
	while(bit_counter >= 0){
		int mask = 1 << bit_counter;
		printf("%d", (podopitniy & mask) >> bit_counter); 
		bit_counter-=1;
	}
	printf("\n");
	
}

void negative_to_bits(void){
        int input = 0;
        scanf("%d",&input);
        int podopitniy = input;
        printf("%d\n", podopitniy);
        int bit_counter = (sizeof(input) * 8)-1;
        printf("Число в дополнительном коде(для положительных равен прямому)\n");
        while(bit_counter >= 0){
                int mask = 1 << bit_counter;
                printf("%d", (podopitniy & mask) >> bit_counter);
                bit_counter-=1;
        }
        printf("\n");
	if(input < 0){ 
        	printf("Число в прямом коде\n");
		bit_counter = (sizeof(input) * 8)-1;
		podopitniy = input;
		for(int shift = 0;shift<bit_counter;shift++){ // перевод из дополнительного в прямой(вычитание и инверсия) 
			int mask = 1 << shift;
			if(((podopitniy & mask) >> shift) == 0){ // Проверяем крайний бит, равен ли он 0
				podopitniy = podopitniy | mask; // если равен, то меняем на 1 (я так сделал вычитание)
			}
			else{
				podopitniy = podopitniy & (~mask); // если равен 1, то заменяем его на 0, получаем обратный код
				podopitniy = ~podopitniy; // переводим обратный код в прямой
				break;
			}
		}
        	while(bit_counter >= 0){ // вывод прямого кода
                	int mask = 1 << bit_counter;
                	printf("%d", (podopitniy & mask) >> bit_counter);
                	bit_counter-=1;
        	}
        	printf("\n");
	}

}

void count_positive_bits(void){
	int input = 0;
        scanf("%d",&input);
	int counter = 0;
	if(input > 0){
		while(input){
			counter += (input & 1);
			input = input >> 1;
		}
		printf("%d\n",counter);
	}
	else{
		printf("Неверный ввод, число должно быть больше 0\n");	
	}
}

void change_third_byte(void){
        int input = 0;
        scanf("%d",&input);
	printf("Введите третий байт\n");
	unsigned input_byte = 0;
	scanf(" %u",&input_byte);
	char shift = 2*8;
	int shifted_char = input_byte << shift;
	input = input & ~(0xFF << shift);
	printf("%d\n", input | shifted_char);
}

int main(void){
	int choice = 0;
	char exit_flag = 1;
	while(exit_flag){
		printf("Выберите функцию\n 0 - positive to bits \n 1 - negative to bits \n 2 - count_positive_bits \n 3 - change_third_byte \n 4 - exit \n");
		scanf("%d",&choice);
		switch(choice){
			case 0:
				printf("Введите число:\n");
				positive_to_bits();
				break;
			case 1:
				printf("Введите число:\n");
				negative_to_bits();
				break;
			case 2:
				printf("Введите число:\n");
				count_positive_bits();
				break;
			case 3:
				printf("Введите число:\n");
				change_third_byte();
				break;
			case 4:
				exit_flag = 0;
				break;
		}
	}
}
