#include <stdio.h>


void square_matrix(void){
	int N;
        printf("Введите N: ");
	scanf("%d",&N);
	if(N<=0){
		printf("N должен быть больше 0\n");
	}
	else{
		int matrix[N][N];
		//for(int i = 0;i<N;i++){
		//	for(int j = N*i+1;j<=N+N*i;j++){
		//		printf("%d ",j); // Вывод без матриц	
		//	}
		//	printf("\n");
		//}
		for(int i = 0;i<N;i++){
			for(int j = 0;j<N;j++){
				matrix[i][j] = N*i+1+j;
				printf("%d ",matrix[i][j]);
			}
			printf("\n");
		}
	}
}

void reverse_output(void){
	int N;
	printf("Введите N: ");
	scanf("%d",&N);
        if(N<=0){
                printf("N должен быть больше 0\n");
        }
        else{
		printf("Введите массив: ");
		int array[N];
		for(int i = 0;i<N;i++){
			scanf("%d",&array[i]);
		}
		for(int i = 0;i<N;i++){
			int buf = array[N-i-1];
			if(N-i-1>i){
				array[N-i-1] = array[i];
				array[i] = buf;
			}
			printf("%d ",array[i]);
			
		}
		printf("\n");
	}	
}

void triangles(void){
	int N;
        printf("Введите N: ");
        scanf("%d",&N);
        if(N<=0){
                printf("N должен быть больше 0\n");
        }
        else{
		int matrix[N][N];
                for(int i = 0;i<N;i++){
			int buf = i+1;
                        for(int j = N-1;j>=0;j--){
				if(buf > 0){
					matrix[i][j] = 1;
					buf--;
				}
				else{
					matrix[i][j] = 0;
				}
			}
		}
		for(int i = 0;i<N;i++){
                	for(int j = 0;j<N;j++){
				printf("%d",matrix[i][j]);
			}
			printf("\n");
		}
	}
}

void ulitka(void){
	int N;
        printf("Введите N: ");
        scanf("%d",&N);  
        if(N<=0){
                printf("N должен быть больше 0\n");
        }
        else{
                int matrix[N][N];
		//int square_max = 0;
		//for(int i = 0;i<N;i++){
		//	square_max = (N+(N-1)+(N-1)+N-i-1);
		//	for(int j = 0;j<N;j++){
		//		//if(i == 0) matrix[i][j] = j+1;
		//		//printf("i = %d, j = %d: %d \n",i,j,(N+(N-1)+(N-j-1)+N-i-1));
		//		if(i == 0){ 
		//			printf("%d ",j+1);
		//			//if(j == 4) printf("\n");
		//		}
		//		if((i > 0 && j == 0) || (i == N-1)){
		//			printf("%d ", (N+(N-1)+(N-j-1)+N-i-1));
		//		}
		//		if(j == N-1 && (i > 0 && i < N-1)){
		//			printf("%d ", N+i);
		//		}
		//		if( (i == 1) && (j>0 && j<N-1)){
		//			printf("%d ", square_max+j);
		//		}
		//		
		//	}
		//	printf("\n");
		//}
		int num = 1;
		int top = 0;
		int bottom = N;
		int left = 0;
		int right = N;
		while(num <= N*N){
			for(int i = left;i< right;i++){
				matrix[top][i] = num;
				num++;
			}
			top++;
			for(int i = top;i<bottom;i++){
				matrix[i][right-1] = num;
				num++;
			}
			right--;
			for(int i = right-1;i>=left;i--){
				matrix[bottom-1][i] = num;
				num++;
			}
			bottom--;
			for(int i = bottom-1;i>=top;i--){
				matrix[i][left] = num;
				num++;
			}
			left++;
		}
		for(int i = 0;i<N;i++){
			for(int j = 0;j<N;j++){
				printf("%d ",matrix[i][j]);
			}
			printf("\n");
		}
	}
}

int main(void){
	int choice = 0;
	printf("0 - квадратная матрица \n 1 - массив в обратном порядке \n 2 - треугольник \n 3 - улитка \n 4 - выход \n Введите цифру: ");
	scanf("%d",&choice);
	switch(choice){
		case 0:
			square_matrix();
			break;
		case 1:
			reverse_output();
			break;
		case 2:
			triangles();
			break;
		case 3:
			ulitka();
			break;
		case 4:
			break;
	}
}
