import "TestSource\stdhpp\stdapi.hcc";


namespace UnitTesting
{
	class SummationSeries
	{
		// Sigma[1==1, n](i);
		public static unsigned int SimpleSum(unsigned int n)
		{
			unsigned int result = 0;			
			for(unsigned int i = 1; i<=n; ++i) 
				result += i;

			return result;
		}

		//Sigma[i==0, n](x^i);
		public static int GeometricSeriesSum(int x, unsigned int n)
		{
			int nx = ftoi(3.145464654);
			//this is a good test for H++ Code Generator: nested loops
			int sum = 0;			
			for(unsigned int i = 0; i<=n; ++i)
			{
				int prod = 1;				
				for(unsigned int j = 0; j< i; ++j)
					prod *= x;
				sum += prod;
			}
			return sum;
		}

		class Horner
		{
			//Sigma[i==0, n](a[i] * x^i);
			public static int Horner(int []a, unsigned int n, int x)
			{				
				int result = a[n];

				for(int i = n -1; i >=0; --i)
					result *= x + a[i];
				return result;
			}

			//Sigma[i==0, n](x^i); using Horner's rule
			public static int GeometricSeriesSum(int x, unsigned int n)
			{
				int sum = 0;				
				for(unsigned int i = 0; i<=n; ++i)
					sum = sum * x + 1;

				return sum;
			}

		};

		//Sigma[i==0, n](x^i); using the closed-form expression
		public static int GeometricSeriesClosedSum(int x, unsigned int n)
		{
			//the fastest one calculating arithmetic series summations
			return ftoi((Math::Pow(x, n + 1) - 1) / (x - 1));
		}

		//Computes the 'y' : the Euler's constant
		public static void ComputeRenderGamma(void)
		{
			double result = 0;	
			for(unsigned long i = 1; i<=500000; ++i)
				result += 1.0 / i - Math::log2( (i + 1.0) / i );

			Console::WriteLn("Gamma y = ", result);
		}

		public static int Ackermman(int m, int n)
		{
			if(m==0 and n >=0)
				return n + 1;
			else if(m>=1 && n==0)
				return Ackermman(m-1, 1);
			else	
				return Ackermman(m-1, Ackermman(m, n-1)); //for all other cases where: m,n >=1
		}
	};

	class Integers_Inline
	{
		public static void Test(void)
		{
			int a = 5, b = 4;

			Console::WriteLn("The values: a = ", a, ", and b = ", b);

			int c = max(a, b);

			Console::WriteLn("The max value is = ", c);

			int d = min(a, b);

			Console::WriteLn("The min value is = ", d);
		}
	};
}