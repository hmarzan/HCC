import "stdhpp\stdapi.hcc";
//import "Algorithms.Sorting";

//using namespace Algorithms::Sorting;
//using namespace Algorithms::Sorting::Proxy;

class Program
{						  			
	public static void ShowMatrixForm(double [10][10] M, int rM, int cM)
	{
		for(int i=0;i<rM;i++){
			print("\n| ");
			for(int j=0;j<cM;j++)
				print(M[i][j],"  ");
			print("| ");
		}
	}
	public static double CalcTrace(double [10][10] M, int rM, int cM)
	{
		double Tr = 0.0;	
		int n = min(rM, cM);
		for(int i=0;i< n; i++){
		printf("M[",i,",",i,"]= ", M[i][i]);
			Tr += M[i][i];
		}
		return Tr;
	}

	public static void Traspose(double [10][10] M, int rM, int cM)
	{
		double Res[10][10];
		
		for(int i=0;i<rM;i++)
			for(int j=0;j<cM;j++)
				Res[j][i] = M[i][j];
						
		ShowMatrixForm(Res, rM, cM);
	}
	
	typename enum OpType { 
						Sum = 1, 
						Substract = 2,
				} OpType;	

	public static void SumMatrices(double [10][10] A, 
								   double[10][10]B, 
								   int rows, 
								   int cols, 
								   OpType what)
	{
		double Res[10][10];	
		double Result = 0.0;
		for(int i=0;i<rows;i++)
			for(int j=0;j<cols;j++){
				if(Sum==what)
					Result = A[i][j] + B[i][j];
				else
					Result = A[i][j] - B[i][j];
				
				Res[i][j] = Result;
			}
		
		ShowMatrixForm(Res, rows, cols);
	}

	public static void MultiplyMatrices(double [10][10]A, 
										double [10][10]B, 
										int rA, 
										int cA, 
										int rB, 
										int cB)
	{
		double Res[10][10];	
		if(cA==rB)
		{
			//(m*n)x(n*p) == (m*p)
			for(int i=0;i<rA;i++)					
				for(int j=0;j<cB;j++)
					Res[i][j] = 0.0;		
		
			for(i=0;i<rA;i++)
				for(j=0;j<cA;j++)
					for(int k=0;k<cA;k++)
					Res[i][j] +=  A[i][k] * B[k][j];

			ShowMatrixForm(Res, rA, cB);			
		}else
			printf("El numero de filas de B debe ser igual al numero de filas de A.");
	}

	public static void main()
	{
		double A[10][10];
		double B[10][10];
		
		double Tr = 0.0;
		bool bExitProgram = false;
		bool bReEnterMatrices = false;
		while(true)
		{
			Console::ClearScreen();
			printf("Programa de Matrices\n\n");
			printf("Entre las dimensiones de dos matrices:\n");
			print("Entre las filas de A: ");
			int rA = readInt();
			print("\nEntre las columnas de A: ");
			int cA = readInt();
			
			print("\nEntre las filas de B: ");
			int rB = readInt();
			print("\nEntre las columnas de B: ");
			int cB = readInt();
			
			Console::ClearScreen();

			char x = 'N';
			if(rA <=0 || rB <= 0)
			{
				printf("El numero de filas debe estar en 0 < r < 10 para ambas matrices. Desea empezar de nuevo (Y/N)?");
				x = getch();
				if(x=='y' || x=='Y')
					continue;
				break;
			}
			
			if(rA > 10 || rB > 10)
			{
				printf("El numero de filas debe estar en 0 < r < 10 para ambas matrices. Desea empezar de nuevo (Y/N)?");
				x = getch();
				if(x=='y' || x=='Y')
					continue;
				break;
			}
			
			if(cA <=0 || cB <= 0)
			{
				printf("El numero de columnas debe estar en 0 < c < 10 para ambas matrices. Desea empezar de nuevo (Y/N)?");
				x = getch();
				if(x=='y' || x=='Y')
					continue;
				break;
			}		
			
			if(cA > 10 || cB > 10)
			{
				printf("El numero de columnas debe estar en 0 < c < 10 para ambas matrices. Desea empezar de nuevo (Y/N)?");
				x = getch();
				if(x=='y' || x=='Y')
					continue;
				break;
			}
			//
			printf("Entre los valores para la matriz A[",rA,"][",cA,"]");
			for(int i=0;i<rA;i++)
				for(int j=0;j<cA;j++)
				{
					print("Elemento en A[", i,",",j,"] = ");
					A[i][j] = Console::ReadDouble();
				}
				
			printf("\nEntre los valores para la matriz B[",rB,"][",cB,"]");
			for(i=0;i<rB;i++)
				for(j=0;j<cB;j++)
				{
					print("Elemento en B[", i,",",j,"] = ");
					B[i][j] = Console::ReadDouble();
				}
				
			//Print the matrix...
			printf("Matriz A:\n");
			ShowMatrixForm(A, rA, cA);
			printf("\n\nMatriz B:\n");
			ShowMatrixForm(B, rB, cB);
			//
			do{
				printf("\n\nQue deseas hacer?\n");
				printf("0. Mostrar las Matrices\n1. Sumar las matrices.\n2. Restar las matrices.\n3. Multiplicar las matrices (A*B).\n4. Multiplicar las matrices (B*A).\n5. Calcular la traza de A\n6. Calcular la traza de B.\n7. Trasponer matriz A.\n8. Trasponer matriz B.\n9. Re-Entrar Matrices\n10. Salir\n\n");
				print(">:");
				int nOption = readInt();
				switch(nOption)
				{
				case 0:{
					printf("Matriz A:\n");
					ShowMatrixForm(A, rA, cA);
					printf("\n\nMatriz B:\n");
					ShowMatrixForm(B, rB, cB);
				}
				break;
				case 1:
				case 2:
				{
					if(rA!=rB || cA!=cB)
					{
						printf("Las matrices deben tener las mismas dimensiones para ser sumadas.");
					}else{
						printf("La matriz resultado:");
						SumMatrices(A, B, rA, rB, nOption == 1 ? Sum : Substract);
					}
				}
				break;
				case 3:{
					
					if(cA==rB)
					{
						printf("La matriz resultado C = A*B es:\n");
						MultiplyMatrices(A, B, rA, cA, rB, cB);					
					}else
						printf("El numero de filas de B debe ser igual al numero de columnas de A.");
				}
				break;
				case 4:
				{
					if(cB==rA)
					{
						printf("La matriz resultado C = B*A es:\n");
						MultiplyMatrices(B, A, rB, cB, rA, cA);					
					}else
						printf("El numero de filas de A debe ser igual al numero de columnas de B.");			
				}
				break;
				case 5:{
					Tr = CalcTrace(A, rA, cA);
					printf("\nLa Traza de la matriz es = ", Tr);				
					}
				break;	
				case 6:{
					Tr = CalcTrace(B, rB, cB);
					printf("\nLa Traza de la matriz es = ", Tr);	
					}
				break;
				case 7:{
						if(rA==cA)
						{
							printf("La Traspuesta de la matriz es:");
							Traspose(A, rA, cA);
						}else
							printf("Esta matriz no se puede trasponer ya que no es cuadrada.");
					}			
				break;
				case 8:{
						if(rB==cB)
						{
						printf("La Traspuesta de la matriz es:");
						Traspose(B, rB, cB);		
						}else
							printf("Esta matriz no se puede trasponer ya que no es cuadrada.");
					}
				break;
				case 9:
				{
					bReEnterMatrices = true;
				}
				break;
				case 10:
				{
					bExitProgram = true;
				}
				break;
				default:
				{
					printf("Entrada desconocida. Favor seleccionar una opcion.");
				}
				};
			
				if(false==bExitProgram)
				{
					printf("\n\nPresione una tecla para regresar...");
					char ch = getch();
					//Console::ClearScreen();					
				}
			}while(false==bExitProgram && false==bReEnterMatrices);
			
			if(bExitProgram)
				break;
			if(bReEnterMatrices){
				bReEnterMatrices = false;
				continue;
			}
		} //end-while
	}
};