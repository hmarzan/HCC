import "stdhpp\stdapi.hcc";


using pow = Math::Pow2;

namespace Samples
{
	class Cotorra
	{
		private char[] what = null;
		
		//public Cotorra(){}
				
		public void Listen(string phrase)
		{
		//debugger;
			int n = strlen(phrase);
			if(what!=null)
				destroy[] what;
				
			what = new char[n + 1];
			strncpy(what, phrase, n);
			what[n] = '\0';
		}
		
		public void Talk()
		{
			printf(what);
		}
		
		public void Destructor()
		{
			printf("in the Destructor()...\n");
			if(what!=null)
				destroy []what;
		}
	};
	
	class SimpleProbability
	{
		public double []array = new double[20];
		
		public void Init()
		{
			array[0] = 3.1414124545234;
			array[1] = 3.0023452345234;
			array[2] = 2.9234655634676;
			array[3] = 3.87345634673673;
			array[4] = 3.99994523453452;
			array[5] = 4.123452345345345;
			array[6] = 4.0000223412342;
			array[7] = 4.0;
			array[8] = 4.0012341234124;
			array[9] = 2.7567845684758;
			array[10] = 3.9345634523;
			array[11] = 2.64756745674;
			array[12] = 3.905879567847563;			
		}
		
		public SimpleProbability()
		{
			Init();
		}
		
		
		public double CalcMedia()
		{
			int n = 12;
			double media = 0.0;
			for(int i=0; i <= n; i++)
			{
				media = media + array[i];
			}
			
			media = media / n;
			
			//printf("(1)And the mean is = ", media);			
			
			return media;
		}
		
		public double CalcVariance(double mean)
		{
			int n = 12;
			double variance = 0.0;
			for(int i=0; i<=n; i++)
			{
				//double pow = Math::Pow(Math::Abs(mean - array[i]), 2.0);
				//printf("pow [",i, "] = ", pow);
				variance = variance + Math::Pow(Math::Abs(mean - array[i]), 2.0);
			}
			
			variance = variance / (n-1);
			
			return variance;
		}
		
		public double CalcStdDev(double mean)
		{
			return Math::Sqrt(CalcVariance(mean));
		}
		
		public void Destructor()
		{
			printf("In the Simple Destructor()...\n");
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
			int i = Power(n, 2)-n+1;
			int m = Power(n, 2)+n-1;
			for(int j=0; j<n;j++){
				print(i+2*j);
				if(j < (n-1) && i < m)
					print("+");	
				//i+=2; //the next value in the series...
			}
			printf(" \t= ",Power(n, 3));
		}		
	};
	
	class Application
	{
		public static void main(int argc, string[] argv)
		{		
/*		
			//Cotorra^ lola = new Cotorra();
			Cotorra lola;
			
			//debugger;
			
			lola.Listen("Hello World from H++!");
			lola.Talk();
			
			string blabla = "H++ is a new language!";
			*/
			//lola.Listen(blabla);
			//lola.Talk();
			
			SimpleProbability^ simple = new SimpleProbability();
			
			double media = simple.CalcMedia();
				
			printf("(2)And the mean is = ", media);	

			printf("(*) (mean / 4.0)% = ", (media / 4.0)*100.0);
			
			double variance = simple.CalcVariance(media);
			
			printf("(3) the variance is = ", variance);			
			
			double stddev = simple.CalcStdDev(media);
			
			printf("(4) the std dev is = ", stddev);			
			
			printf("(5) media + stdev = ", media + stddev);
			
			//destroy lola;
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
						
			for(i=1; i<=n; i++)
				printf(simple.Fibonacci(i));
			destroy simple;
			
			printf("Gcd = ", Math::Gcd(14, Math::Gcd(28, Math::Gcd(7, 56))));
		}
	};
};