import "stdhpp\stdapi.hcc";

namespace Mathematics {

namespace Experiments {

	class MatrixSuccesion
	{
		public MatrixSuccesion(){}
		
		public void ShowSuccesion(int m)
		{
			unsigned int A[16][16];			
			for(int n=1; n<=m; n++)
			{
				for(int i=0;i<n;i++)
					for(int j=i; j<n; j++)
					{
						A[i][j] = 1;						
						if((j-1) >=0)
							A[i][j] = A[i][j] + A[i][j-1];
						if((i-1) >=0)
							A[i][j] = A[i][j] + A[i-1][j];
						
						//We then apply it to a_ji = a_ij, and skip it:
						A[j][i] = A[i][j];
					}
					
				print("\n[");
				for(i=0;i<n; i++){
					for(j=0; j<n; j++)
					{						
						print(A[i][j]);
						if((j+1) < n)
							print(",\t");
					}					
					if((i+1) < n)
						print(",\n ");						
				}
				printf("]");				
			}
		}
	};
	class Application
	{
		public static void main(int argc, string[] argv)
		{
			MatrixSuccesion ^ mtx = new MatrixSuccesion();
			do{
				Console::ClearScreen();
				printf("The Matrix Succesion up to 16 matrices (Mathematics by Experiments Pag. 333).\n");
				print("Please, enter a number between 1 and 16: ");
				int m = readInt();
				if(m <=16)
					mtx.ShowSuccesion(m);
				print("\nContinue? 1 = Yes, Otherwise = No:");
				m = readInt();				
			}while(m==1);
			
			destroy mtx;
			
		}
	};
	}
}