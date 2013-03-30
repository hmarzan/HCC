import "stdhpp\stdapi.hcc";

namespace Stanford
{
	class StanfordAlgorithms
	{
		public StanfordAlgorithms()
		{
		
		}
		public int Power(int x, int n)
		{ //O(n)
			if(n==0)
				return 1;
			else if(n==1)
				return x;
			int result = 1;
			while(n > 0){
				if(n%2==0){				
					//2^8 = (2 * 2) * (2 * 2) * (2 * 2) * (2 * 2)
					result *= (x * x);
					n-= 2;
				}else{				
					//2^7 = (2 * 2 * 2) * (2 * 2) * (2 * 2)
					result *= (x * x * x);
					n-= 3;
				}
			}
			return result;
		}
		
		public unsigned int Fibonacci(unsigned int n) //TOP-DOWN (DIV-&-CQR)
		{
			if(n==0 || n==1)
				return n;
			//else{			
			unsigned int a = Fibonacci((n + 1) div 2);
			unsigned int b = Fibonacci(((n + 1) div 2) - 1);

			if(n%2==0)
				return a * (a + 2 * b); //to avoid a a third mult. a * a + (2 * a * b)
			else
				return a * a + b * b;

			//}
		}	
		
		public void PrintPrettySeq1(int n)
		{
			int i = Power(n-1, 2) + 1;
			int m = Power(n,2) + 1;			
			for(int j=i; j < m; j++){
				print(j);
				if(j < (m-1))
					print("+");
			}
			printf("\t= ",Power(n-1, 3)," + ",Power(n, 3));
		}	

		public void PrintPrettySeq2(int n)
		{
			int i = Power(n, 2) - n+1;
			int m = Power(n, 2) + n-1;
			for(int j=1; j<=n;j++){				
				print(i); //print(i+2*j);   j=0...n-1
				if(j < n && i < m)	//if(j < (n-1) && i < m)
					print("+");	
				i+=2; //the next value in the series...
			}
			printf(" \t= ",Power(n, 3));
		}	
				
	};
	
	class Application
	{
		public static void main(int argc, string[] argv)
		{
			StanfordAlgorithms^ simple = new StanfordAlgorithms();
			
			print("Please, enter the number n = ");
			int n = readInt();
			printf("--------------------------------------------");
			printf("Calculate the Sum[i, {i,(n-1)^2 + 1},{m, 2n-1}]\t= \t(n-1)^3 + n^3.\n");
			printf("--------------------------------------------");
			for(int i=1; i<=n; i++)
				simple.PrintPrettySeq1(i);
			printf("--------------------------------------------");
			printf("Calculate the Sum[i+2*j, {j, 0}, {i,n^2-n+1},{m, n^2+n-1}, {n}]\t= \tn^3.\n");
			printf("--------------------------------------------");
			for(i=1; i<=n; i++)
				simple.PrintPrettySeq2(i);
			printf("--------------------------------------------");			
			printf("Fibonacci Sequence, up to: ", n);
			printf("--------------------------------------------");			
			for(i=1; i<=n; i++)
				print(simple.Fibonacci(i), ", ");
			printf("\n--------------------------------------------");			
			destroy simple;			
		}
	};
}