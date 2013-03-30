import "stdhpp\stdapi.hcc";


namespace Numerical
{
	namespace Computing
	{

		class Program
		{
		
		public static void CalcReciprocalPowerOf2() //y = 2^-x
		{
			double x = 1;
			int n = 0;
			
			//Repeatedly divide x by 2 until it underflows to 0: in H++ it takes a lot of printed 0.0's 
			//representing really tiny numbers until it hits the real 0.0
			while(x > 1.0e-17)
			{
				x = x / 2;
				n++;
				printf(" 2^ - ", n, " = ", x);
			}
		}
		
		public static void CalcAddOneToReciprocalPowerOf2() //y = 1 + 2^-x
		{
			double x = 1, y = 2;
			int n = 0;
			
			//Repeatedly divide x by 2 until it underflows to 0
			
			while(y > 1)
			{
				x = x / 2;
				y = 1 + x;
				n++;
				printf(" 2^ - ", n, " = ", y);
			}
		}
		
		public static void TotalParallelResistance()
		{
			double R1 = 1.0;
			double R2[9];
			
			R2[0] = 1.0;
			R2[1] = 10.0;
			R2[2] = 1000.0;
			R2[3] = 1.0e5;
			R2[4] = 1.0e10;
			R2[5] = 0.1;
			R2[6] = 1.0e-5;
			R2[7] = 1.0e-10;
			R2[8] = 0;
			
			for(int i=0; i < sizeof(R2)/sizeof(double); ++i)
			{
				double total = 1 / (1/R1 + 1/R2[i]);
				printf("r1 = ", R1, "\tR2 = ",R2[i], "\t Total Resistance = ", total);
			}
		}
		
		public static void FindOneFromQuotient() // 1 / ( 1 / x)
		{
			double y = 0.0;
			
			for(int x=1; x <= 10; x++)
			{
				y = 1/(1/x);
				printf("y is ", x, "? == ", y);
			}
		}
		
		public static void ApproximateADerivativeByDifferenceQuotient()
		{
			int n = 1;
			double x = 1.0, h = 1.0, deriv = Math::Cos(x), diffquo, error;
			
			printf(" deriv = ", deriv);
			printf("\n h\t\t\tdiffquo\t\t\tabs(deriv - diffquo) \n");
			
			//let h range from 10^-1 down to 10^-20
			
			while(n <= 20)
			{
				h /= 10;
				diffquo = (Math::Sin(x+h)-Math::Sin(x))/h;  //DIFFERENCE QUOTIENT
				error = Math::Abs(deriv - diffquo);
				
				//I have to solve this problem for a future version of the floating point library: the floating point formatting
				if(n <=8)
					printf(h, "     \t\t", diffquo, "    \t", error);
				else if(n <=15)
					printf(h, "     \t", diffquo, "    \t", error);
				else if(n ==16)
					printf(h, "     \t", diffquo, "    \t\t\t", error);
				else
					printf(h, "     \t\t", diffquo, "    \t\t\t", error);
					
				n++;
			}
		}
		
		public static double ApproximateEulerConstant()
		{
		    double e = 0.0;
			double x = 1.0;
			int n= 100000000;
			e = Math::Pow((1 + x/n), n);
			
			printf("e = ", e, ", ln e = ", Math::Ln(e));
			
			return e;
		}

		public static int Factorial(int n)
		{
			if(n==0)
				return 1;
			return n * Factorial(n-1);
		}
		
		public static double ApproximateEulerConstantByTaylor()
		{
			double e = 0.0;
			double x = 1.0;
			int n = 20;
			for(int i=0; i < n; i++)
				e += Math::Pow(x, i) / Factorial(i);
			
			printf("e = ", e, ", ln e = ", Math::Ln(e));
			
			return e;
		}
		
		public static double CalcEulersGammaConstant()
		{
			double result = 0.0;
			int n = 100000;
			for(int i = 1; i< n; i++)
			{
				result += 1/i;
			}
			result -= Math::Ln(n);
			
			return result;
		}
		
		public static double CalcPowerOfReciprocalCubes()
		{
			double result = 0.0;
			int n = 1000;
			for(int i = 1; i< n; i++)
			{
				result += 1/Math::Pow(i, 3);
			}
			
			return result;			
		}
		
		public static double CalcHyperbolicFunctionsDemo(const double x)
		{
			double e_PowX = Sinh(x) + Cosh(x);
			
			return e_PowX;
		}
		
		public static double CalcHyperbolicDemo2()
		{
			return (Cosh(Math::pi())*Cosh(Math::pi())) - (Sinh(Math::pi())*Sinh(Math::pi()));
		}
				
		public static void main()
		{
			printf("Reciprocal power of 2:\n");
			CalcReciprocalPowerOf2();
			printf("1 + Reciprocal power of 2:\n");
			CalcAddOneToReciprocalPowerOf2();
			printf("\nTotal Parallel Resistance:\n");
			 TotalParallelResistance();
			printf("\nProblem: 1/(1/x):\n");
			FindOneFromQuotient();
			printf("Approximate A Derivative By Difference Quotient:\n");
			ApproximateADerivativeByDifferenceQuotient();
			printf("\nApproximating e:\n");
			double e1= ApproximateEulerConstant();
			printf("\nApproximating e using Taylor series:\n");
			double e2= ApproximateEulerConstantByTaylor();
			
			printf("\n\nCancellation phenomena:\n\ne1= ", e1, "\ne2= ", e2, "\n\n| e1 - e2 | = ", Math::Abs(e1-e2));
			
			printf("\nCalculating the Euler\'s gamma constant: ", CalcEulersGammaConstant());
	
			//printf("\nCalculating the Power of Reciprocal Cubes: ");
			printf("\nCalculating the Power of ", "Reciprocal Cubes: ",CalcPowerOfReciprocalCubes());
			printf("\nShould be: ", Math::Pow(Math::pi(), 3)/ 25.79436089);
			
			double x = Math::pi();
			double e_PowX = CalcHyperbolicFunctionsDemo(x);
			
			printf("\n x = pi = ", x);
			printf("\nValue of e^x == Sinh[x] + Cosh[x] == ", e_PowX,	
					"\n\t\t\t   Aprox. == ", Math::Exp(x));
				   
			printf("\n Cosh^2 pi - Sinh^2 pi == ", Math::Round(CalcHyperbolicDemo2()));
		}
		};	
	}
};