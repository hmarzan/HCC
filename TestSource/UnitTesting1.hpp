import "stdhpp\stdapi.hcc";
import "stdhpp\TestVirtuals.hpp";
import "stdhpp\TestFunctionParams.hpp";
import "TestSwitch.hpp";


namespace UnitTesting
{
		namespace Objects
		{
			class ProxyObject
			{
				public virtual abstract int Compare(ProxyObject ref object);
			};

			class Automobile: ProxyObject
			{
				public Automobile()
				{
				
				}
				
				private double speed = 0.0;
				
				public double get:Speed()
				{
					return speed;
				}
				public void put:Speed(double val)
				{
					speed = val;
				}
				
				public virtual int Compare(ProxyObject ^ obj)
				{
					Automobile^ other = dynamic_cast(obj);
					if(speed < other.speed)
						return -1;
					else if(speed > other.speed)
						return 1;
					
					return 0;
				}
			};
			
			typename unsigned int __uint;
			
			class Array
			{
				protected ProxyObject ^ array = null;
				protected __uint _length = 0;
				public __uint get:length()
				{
					return _length;
				}
				
				public virtual void Destructor()
				{
					if(array!=null)
						destroy [] array;
				}				
				
				public virtual abstract ProxyObject^ getAt(__uint x);
				
				public virtual abstract void Swap(__uint i, __uint j);
			};

			class ArrayOfAutomobiles : Array
			{
				public ArrayOfAutomobiles(__uint len)
				{
					array = new Automobile[_length = len];
				}
				
				public virtual ProxyObject^ getAt(__uint x)
				{	
					return dynamic_cast(Automobile, &array[x]);
				}
				
				public virtual void Swap(__uint i, __uint j)
				{
					Automobile tmp = array[i];
					array[i] = array[j];
					array[j] = tmp;
				}
				
			};		
		};
	
	
	class SimpleObject
	{
		public virtual abstract bool lessThan(SimpleObject ref obj);
		public virtual abstract bool isEqual(SimpleObject ref obj);
		
		public virtual abstract void Swap(SimpleObject ref obj);
		
		public virtual void Destructor()
		{
			printf("Destroying a Simple Object...");
		}
	};
	
	//1
	class DoubleProxy: SimpleObject
	{
	public double val = 0.0;
	
	public DoubleProxy()
	{
		printf("Constructing a Double Proxy...");
	}
		public virtual bool lessThan(SimpleObject ref obj)
		{
			DoubleProxy^ ptr = dynamic_cast(&obj);
			
			return val < ptr.val;
		}
		
		public virtual bool isEqual(SimpleObject ref obj)
		{
			DoubleProxy^ ptr = dynamic_cast(DoubleProxy, &obj);
			
			return val < ptr.val;
		}
		
		public virtual void Swap(SimpleObject ref obj)
		{
			double tmp = val;
			
			DoubleProxy^ ptr = dynamic_cast(&obj);
			val = ptr.val;
			ptr.val = tmp;
		}

		public virtual void Destructor()
		{
			printf("Destroying a Double Proxy object...");
		}
		
	};
	
	class SwapperObject
	{
		public virtual abstract bool lessThan(SimpleObject ref obj1, SimpleObject ref obj2);
		public virtual abstract void Swap(SimpleObject ref obj1, SimpleObject ref obj2);
		
		public virtual abstract bool lessThanEx(SimpleObject[] array, unsigned int left, unsigned int right);
		public virtual abstract void SwapEx(SimpleObject[] array, unsigned int left, unsigned int right);
	};

	class DoubleSwapper : SwapperObject
	{
		public virtual bool lessThan(SimpleObject ref obj1, SimpleObject ref obj2)
		{
			DoubleProxy^ ptr1 = dynamic_cast(&obj1);
			DoubleProxy^ ptr2 = dynamic_cast(&obj2);
			
			return ptr1.val < ptr2.val;
		}
		public virtual void Swap(SimpleObject ref obj1, SimpleObject ref obj2)
		{
			DoubleProxy^ ptr1 = dynamic_cast(&obj1);
			DoubleProxy^ ptr2 = dynamic_cast(&obj2);

			double tmp = ptr1.val;
			ptr1.val = ptr2.val;
			ptr2.val = tmp;
		}
		
		public virtual bool lessThanEx(SimpleObject[] array, unsigned int left, unsigned int right)
		{
			DoubleProxy^ ptr1 = dynamic_cast(&array[left]);
			DoubleProxy^ ptr2 = dynamic_cast(DoubleProxy, &array[right]);
			
			return ptr1.val < ptr2.val;
		}
		public virtual void SwapEx(SimpleObject[] array, unsigned int left, unsigned int right)
		{
			DoubleProxy^ ptr1 = dynamic_cast(DoubleProxy, &array[left]);
			DoubleProxy^ ptr2 = dynamic_cast(&array[right]);

			double tmp = ptr1.val;
			ptr1.val = ptr2.val;
			ptr2.val = tmp;
		}
	};
	
	//4
	class SortDoubleProxy
	{
	public void Sort(DoubleProxy[] array, int n, SwapperObject ref swapper)
	{
		for(int index=1;index < n; index++)
		{
			for(int left=index - 1; left >= 0; --left)
			{
				//if(array[left + 1].lessThan(array[left]))
				if(swapper.lessThan(array[left + 1], array[left]))
				{
						//array[left + 1].Swap(array[left]);
						swapper.Swap(array[left + 1],array[left]);
				}
			}
		}
	}
	
	private void Swap(double ref val1, double ref val2)
	{
		double tmp = val1;
		val1 = val2;
		val2 = tmp;
	}
	
	public void Sort2(double[] array, int n)
	{
		for(int index=1;index < n; index++)
		{
			for(int left=index - 1; left >= 0 ; left--)
			{
				if(array[left + 1] < array[left])
				{
					/* using a function with ref params is better for simplification of the algorithms
					double tmp = array[left + 1];
					array[left + 1] = array[left];
					array[left] = tmp;
					*/
					Swap(array[left + 1], array[left]);
				}
			}
		}	
	}
	
	public void SortEx(SimpleObject[] array, int n, SwapperObject ref swapper)
	{
		for(int index=1;index < n; index++)
		{
			for(int left=index - 1; left >= 0; --left)
			{
				if(swapper.lessThanEx(array, left + 1, left))
				{
					swapper.SwapEx(array, left + 1, left);
				}
			}
		}
	}

	public void	PrintObject(SimpleObject ref obj)
	{
		DoubleProxy^ dbl = dynamic_cast(&obj);

		printf("A double proxy = ", dbl.val);
		
	}
	
	};

	class HObject
	{
		public HObject()
		{
		}
		public virtual void Destructor()
		{
		
		}
	};
	
	class MonteCarlo
	{
		public static double PI(unsigned int trials)
		{
			unsigned int hits = 0;
			for(unsigned int i = 0; i < trials; ++i)
			{
				double x = Rnd::NextDblEx();
				double y = Rnd::NextDblEx();
				if(x * x + y * y < 1.0)
				{
					++hits;
				}
			}
			
			return 4.0 * hits / trials;
		}
	};

	class TestProperties
	{
	
		struct node
		{
			public node^ left = null;
			public node^ right = null;
			public HObject^ value = null;
			
			public node()
			{
			}
			
			public void Destructor()
			{		
				if(left!=null)
					destroy left;
				if(right!=null)
					destroy right;
				if(value!=null)
					destroy value;			
			}
		};

	
		private Shape[] shapes1 = new Circle[10];
		
			private Shape[] shapes2 = new Circle[4];
		
		private Shape^ shapes_ptr = new Circle[10]; //should this expression change the the decl from pointer to array inmediatelly?
		
		private Circle oshapes[5];
		
		public Shape^ get:Shapes()
		{
			return shapes1;
		}
		
		public void put:Shapes(Shape^ _shape)
		{
		
		//debugger;
			node link;
			
			link.left 	= new node();
			link.right 	= new node();
			link.value  = new HObject();
			
			/*TODO : destroy object.member
			destroy link.left;
			destroy link.right;
			destroy link.value;
			*/
			
			node^ left = link.left;
			destroy left;
			link.left = null;
			
			link.left = link.right;
			link.right = new node();
			
			node^ right = link.right;
			destroy right;
			
			link.right = link.left;
			link.left = new node();
			
			left = link.left;
			
			link.left = left;
				
			shapes1[0].Draw();
			shapes2[0].Draw();

			shapes_ptr[0].Draw();
			
			shapes1[1].Draw();
			shapes2[2].Draw();
			
			shapes_ptr[3].Draw();
			
			
			/*
			if(shapes1!=null)
				destroy shapes1; //must use destroy [] shapes for dynamic arrays;
			*/
			if(shapes1!=null)
				destroy [] shapes1;
			//
			shapes1 = _shape;
	
			const int n = 10;
			Circle local_circles[n];
			
			local_circles[0].Draw();
			
			local_circles[2].Draw();
			
			for(int i=0; i < n; i++)
				local_circles[i].Draw();

			
			Circle^ circlxx = new Circle[5];
			
			circlxx[0].Draw();
			
			//oshapes = circlxx;
		}	

		public virtual void Destructor()
		{
			using printf = Console::WriteLn;
			
			printf("**Before destroying all object in shapes1:");
			if(shapes1!=null)
				destroy [] shapes1;
		
			printf("**Before destroying all object in shapes2:");
			destroy [] shapes2;
		}		
	};

//using printf = Console::WriteLn;

	class HelloWorld
	{
		public HelloWorld()
		{
			Console::WriteLn("The HelloWorld constructor");
		}

		public void SayHello(string text)
		{
			Console::WriteLn("this is what I have to say for the first time in H++ :", text);
		}

		public void ShowHello(string text)
		{
			System::ShowMessage(text, "H++ First program", System::IconInformation);
		}

		public void Destructor()
		{
			Console::WriteLn("The HelloWorld destructor");
		}
	};
//const definitions in H++

	const int ten = 10;
	
	const Int16 minusten = -ten;
	const int hundred = 100;	
	const Int32 max_length = 80;	
	const Int64 max_64 = 42545234; //just a test
	
	const Int64 max_64_neg = -max_64;
		
	namespace misc1
	{
		double array[10];

		const double pi = 3.1415927;	
		const float e = 2.2114;
		
		const int ten_plus = -UnitTesting::minusten;	
		const char ch1 = 'a', ch2 = 'z';	
		const string hello = "hello, world.";
		
		const string hell = "human", boy = "mistakes";
		
		const double dX = 4.99, dY = 9.4567;
			
		enum greek { alpha, beta, gamma };
		
		enum days_of_week { monday = 1, 
							tuesday,
							wednesday,
							thursday,
							friday,
							saturday,
							sunday,
						};
	
		//type definitions
		typename greek gletter;	
		typename days_of_week days_type;
	}
	

	class TestIntegers
	{
	private int nWidth	= 0;
	private int nHeight	= 0;

	//all properties 
	public int get:Width()
	{
		return nWidth;
	}

	public void put:Width(int value)
	{
		nWidth = value;
	}

	public int get:Height()
	{
		return nHeight;
	}

	public void put:Height(int value)
	{
		nHeight = value;
	}

	public int getArea()
	{
		return Width * Height;
	}

	public double getArea2()
	{
		return nWidth * nHeight * 1.0;
	}

	public static Int64 bigValue = 2147483648; //0x0000000080000000h;

	public static void TestExpressions(void)
	{
		int hex_value = 0xDEADC0DEh;

		hex_value = hex_value << 0x10; //0xC0DE0000h

		hex_value = hex_value >> 8; //0x00C0DE00h

		hex_value = hex_value >> 8; //0x0000C0DEh
		
		short shift = 0x10;
		
		hex_value = hex_value << shift; ////0xC0DE0000h
		
		hex_value = hex_value >> shift; 

	//new type of expression down to zero from 4...
		int j = 0;

		volatile bool what = false;
				
		/*
		j = (4 + (--j))%4; //3
		j = (4 + (--j))%4; //1
		j = (4 + (--j))%4; //0
		j = (4 + (--j))%4; //3
		*/

		hex_value = 0;

		while(true)
		{
			j = (4 + (--j))%4; //321032103210
			
			Console::WriteLn(" j = (4 + (--j))%4 ==", j);
			
			if(++hex_value < 12)
				continue;

			what = (j%2)==0; //j==0

            break;
		}

		what = j==3; //false
		
		do{
			j = (4 + (--j))%4; //3210
			Console::WriteLn(" j = (4 + (--j))%4 ==", j);
			
			if(j==0)
				break;
			
		}while(j > 0);

		int x = 0;

		auto int y = 1, z = 2;

		volatile Int32 result = (y + 1) * z; //result==4		

		x++; //x==1
		++x; //x==2

		x += 14; //x==010h

	//	debugger;
		
		what = x==0x10h; //true
		
		Console::WriteLn("(what = x==0x10h)		= ", what);

		x /= 4; //x==4

		--x;
		x--; //x==2

		x = 7 mod x; //1

		what = x==1; //true
		
		Console::WriteLn("(what = x==1)			= ", what);

		x++; //x==2;

		x *= 10; //x==0x14h==20		

		x /= 5; //x==4

		x /=4; //x==1

		x %= 4; //x==1		

		y = 0x400h; //1024

		x = y div 15; // 68

		what = x == 68; //true
		
		Console::WriteLn("(what = x == 68) 		= ", what);

	//	debugger;
		
		z = 0x10; //16

		x = y * z; //16kb == 16 * 1024

		what = x==0x4000; //true
		
		Console::WriteLn("(what = x==0x4000) 		= ", what);

		y = z = x; //all must have 16384 or 04000h

		x = y div 4; //x==4096

		what = x==4096;
		
		Console::WriteLn("(what = x==4096) 		= ", what);

		z = x%3; //z==1

		what = z==1; //true

		y = x div 3; //==1365

		what = y==1365;		
		
		Console::WriteLn("(what = y==1365) 		= ", what);

		result = ftoi(x / 3); //==1365

		what = (result==1365.0);
		
		Console::WriteLn("(what = (result==1365.0)) 	= ", what);
		
		result = x div (3 + 1); //==1024

		what = (result==1024);
		
		Console::WriteLn("(what = (result==1024)) 	= ", what);

		auto short a = 20000, b = 4, c = 10;

		result = a * b * c; //800,000

		what = result==0xC3500h;
		
		Console::WriteLn("(what = result==0xC3500h) 	= ", what);

		result /= 0x10; //==50000

		what = (result==50000);
		
		Console::WriteLn("(what = (result==50000)) 	= ", what);

		x = result%3; //==2

		result = Math::Round(a / 3) * 5; //round(6,666.666666....) * 5 aprox. == 33,335

		what = (result==33335);
		
		Console::WriteLn("(what = (result==33335)) 	= ", what);

		a = 1; b= 4; c=4;

		double disc = Math::Sqr(b)- 4*a*c;

	//debugger;
		what = disc==0.0;
		
		Console::WriteLn("(what = disc==0.0) 		= ", what);
		
		double x1 = (-b + Math::Sqrt(disc)) / 2*a;
		what = x1==-2.0;
		
		Console::WriteLn("(what = x1==-2.0) 		= ", what);

		double x2 = (-b - Math::Sqrt(disc)) / 2*a;
		what = x2==-2.0;
		
		Console::WriteLn("(what = x2==-2.0) 		= ", what);

		x = 1;
		Console::WriteLn("the value of x is: ", x);

		++x;
		Console::WriteLn("the value of x is: ", x); //2

		--x;
		Console::WriteLn("the value of x is (1): ", x); //1

		Console::WriteLn("the value of x is (1): ", x++); //1 -->2

		Console::WriteLn("the value of x is (2): ", x--); //2 -->1

		Console::WriteLn("the value is (5): ", x++ - 4 / (-1)); //5

		Console::WriteLn("the value is (5): ", --x - 4 / (-1)); //5

		Console::WriteLn("the value is (3): ", (x++ - 4) / (-1)); //3

		Console::WriteLn("the value is (3): ", (--x - 4) / (-1)); //3

		x = ten * 100; //==0x400

		what = x ==1000; //true

		what = ten*50 == 500; //true

		using namespace misc1;

		gletter alett = alpha;	
		
		days_type lundi = monday, 
				mardi = tuesday;
				  
		int length = 15, width = 10;
		double area = length * width;	
		
		Console::WriteLn("l = ",length, "w = ",width);
		Console::WriteLn("(a = l * w) =", area);
	
		double radius, circ;
			
		char letter = 'x';
		
		double darea = (length * width) * 0.9;

		Console::WriteLn("(a = l * w * 0.9) = ", darea);
		
		a = 2;

	//debugger;
		
		bool ternary = 4 >= a ? 999 < 1000 && 1 > 0 : 777==77.7*10 ; 7 == 111;
		
		Console::WriteLn("Ternary (true) = ", ternary);

		ternary = (4 >= a) ? 999 > 1000 && 1 > 0 : 777 / 7 == 111;
		
		Console::WriteLn("Ternary (false ) = ", ternary);

		ternary = 4 < a ? 999 > 1000 && 1 > 0 : 777 / (3 + 4) == 111;
		
		Console::WriteLn("Ternary (true) = ", ternary);
		
		return;
	}

	public int Calc1(int i, int ref j)
	{
		auto int nArea = j * Height;

		j = Width;	//reference keep this value

		j = i div 5;

		nArea = ftoi(j * Height * 1.0);
		//nArea = j * Height * 1.0;

		j *= 3; //new value for j

		return nArea;
	}

	public void Swap(int ref value1, int ref value2)
	{
		auto Int32 tmp = value1;
		value1 = value2;
		value2 = tmp;
	}

	public void Calc2(Int32 ref x, Int32 ref y)
	{
		int a = 1000;
		x = 9 * 3 * a; //27000
		y = a * 4 * 5; //20000
	}

	//members for specific integer expressions
	public void add(int v1, int v2, int ref res)
	{
		res = v1 + v2;

		Console::WriteLn("(+) the result is: ", res);
	}

	public void subs(int v1, int v2, int ref res)
	{
		res = v1 - v2;

		Console::WriteLn("(-) the result is: ", res);
	}

	public void multiply(int v1, int v2, int ref res)
	{
		res = v1 * v2;

		Console::WriteLn("(*) the result is: ", res);
	}

	public void divide(int v1, int v2, int ref res)
	{
		res = v1 div v2;

		Console::WriteLn("(div) the result is: ", res);
	}

	public void modulus(int v1, int v2, int ref res)
	{
		res = v1 % v2;

		Console::WriteLn("(mod) the result is: ", res);
	}

	public void divide2(int v1, int v2, int ref res)
	{
		res = Math::Round(v1 / v2);

		Console::WriteLn("(round(/)) the result is: ", res);
	}

	public void multiple_assignments(int ref param1, int param2)
	{
	int local = param2 = param1 = (0x400 * 1024) / 0x10;

		bool what = (local == 0x10000);
	}

	public int Factorial(int n)
	{
		//System::Debug::BreakPoint();
	
		if(n==0)
			return 1;
		else
			return n * Factorial(n - 1);
	}

	public void SwapDebug(int ref value1, int ref value2)
	{
		System::Debug::OutputString("Swaping values...");
		Swap(value1, value2);
	}

	public double member1 = 0.0;
	public double member2 = 1.0;

	public double my_array1[2];
	public int my_array2[2];
	};


	class TestFloatingPoint
	{
		public TestFloatingPoint()
		{
			System::Debug::OutputString("constructing a TestFloatingPoint() object...");
		}

		public void Destructor()
		{
			System::Debug::OutputString("destroying a TestFloatingPoint() object...");
			Console::WriteLn("destroying a TestFloatingPoint() object...");
		}
		public double Power(double X, int n)
		{
			if(n==0)
				return 1;

			if(n%2==0)
				return Power(X * X, n div 2);
			else
				return X * Power(X * X, n div 2);
		}


		public static double speed(double distance, double time)
		{
			return distance / time;
		}

		public double Area(double minor_base, double major_base, double height)
		{
			double area = ((minor_base + major_base) * height) / 2.0;

			return area;
		}
	};

	class TestArrays
	{
		private string name;

		public string get:Name()
		{
			return name;
		}
		public void put:Name(string value)
		{
			name = value;
		}

		public static void printChars(char [] array, int n)
		{		
			
			try
			{			
				//try{
					for(int i=0;i<n;++i)
						Console::WriteLn("Char at i:[", i, "] == ", array[i]);
						/*
				}catch(System::Exception::ACCESS_VIOLATION)
				{
					Console::WriteLn("Oops! there was a bug in this code!");
				}
						*/				
			}catch(System::Exception::ACCESS_VIOLATION)
			{
				Console::WriteLn("Oops! there was a serious bug in this code!");
			}
			catch(System::Exception::INTEGER_DIVIDE_BY_ZERO)
			{
				Console::WriteLn("Oops! there was a serious bug in this code!");
			}			
		}

		//implements the H++ statements before going into testing arrays...
		public static void DoTest(void)
		{
		//debugger;
				TestIntegers array2[4];				

				array2[0].Width  = 100;
				array2[0].Height = 200;				

				Int64 nArea = array2[0].Width;

				nArea = array2[0].Width * array2[0].Height;

				array2[0].member1 = Math::pi();
				array2[0].member1 = Math::log_e_base2();
				
				Console::WriteLn("(1)","nArea = 100 * 200 == ", nArea);

				array2[1].Width  = 100;
				array2[1].Height = 200;

				nArea = array2[1].Width * array2[1].Height;
				
				Console::WriteLn("(2)","nArea = 100 * 200 == ", nArea);

				array2[2].Width  = 100;
				array2[2].Height = 200;

				nArea = array2[2].Width * array2[2].Height;
				
				Console::WriteLn("(3)","nArea = 100 * 200 == ", nArea);

		//	debugger;
				array2[3].my_array1[0] = 9.99;
				array2[3].my_array1[1] = 3.1415927;
				array2[3].my_array2[0] = ftoi(9.99);
				array2[3].my_array2[1] = ftoi(3.1415927);
				
				Console::WriteLn("array2[3].my_array1[0] (double)== 9.99 == ", array2[3].my_array1[0]);
				Console::WriteLn("array2[3].my_array1[1] (double)== 3.1415927 == ", array2[3].my_array1[1]);
				
				Console::WriteLn("array2[3].my_array2[0] (int)== 10 == ", array2[3].my_array2[0]);
				Console::WriteLn("array2[3].my_array2[1] (int)== 3 == ", array2[3].my_array2[1]);
				
		//	debugger;
				static Circle array3[4];
				
				array3[0].Draw();
				
				array3[2].Draw();
				
				static double array4[4];
				
				array4[0] = Math::pi();
				array4[1] = Math::pi() * 2.1;
				array4[2] = Math::pi() * 4.1;
				array4[3] = Math::pi() * 8.1;
				
				for(int x=0; x < 4; x++)
					Console::WriteLn("array4[", x, "] = ", array4[x]);

			//debugger;
				
				string name = "Harold L. Marzan";
				printChars(name, StringHandling::StringLength(name));

				char my_name_is[20];
				
				strset(my_name_is, 0, sizeof(my_name_is));

				my_name_is[0] = 'H';
				my_name_is[1] = 'a';
				my_name_is[2] = 'r';
				my_name_is[3] = 'o';
				my_name_is[4] = 'l';
				my_name_is[5] = 'd';
				my_name_is[6] = '\0';
				

				//debugger;
				Console::WriteLn("My name is :", my_name_is);

				printChars(my_name_is, strlen(my_name_is));

		//debugger;
				char arr[10][20];
				
				arr[0][0] = 'H';
				arr[0][1] = 'E';
				arr[0][2] = 'L';
				arr[0][3] = 'L';
				arr[0][4] = 'O';
				arr[0][5] = '!';

				arr[0][0] = 'H';
				arr[1][1] = 'E';
				arr[2][2] = 'L';
				arr[3][3] = 'L';
				arr[4][4] = 'O';
				arr[5][5] = '!';
				
				for(unsigned int i=0; i< 6; i++)
					Console::WriteLn("Char at [", i , ", ",i, "]=", arr[i][i]);
				
			//	debugger;

				char testt[200];				
				string test = "Testing!!!";
				strncpy(testt, test, strlen(test));
				
				printChars(testt, StringHandling::StringLength(testt));

				//this is dangerous yet: TODO:
				//printChars(arr[9], StringHandling::StringLength(arr[9]));

				string arr_of_strings[4];

				arr_of_strings[0] = "Harold ";
				arr_of_strings[1] = "Lawrence ";
				arr_of_strings[2] = "Marzan ";
				arr_of_strings[3] = "Mercado.";
				
				for(i=0;i<4;++i)
				{
					printChars(arr_of_strings[i], StringHandling::StringLength(arr_of_strings[i]));
				}

				StringHandling::StringCopy(my_name_is, name, StringHandling::StringLength(name));
				printChars(my_name_is, StringHandling::StringLength(my_name_is));
		}
	};

	namespace Test1
	{
		class TestRunner
		{
			public static void main(int argc, string[] argv)
			{
			//debugger;
				for(int ac=0; ac < argc; ac++)
				{
					Console::WriteLn("Argument: [",ac,"]= ", argv[ac]);
				}
				
				bool bUnattended = argc > 1 && (argv[1] == "/Silent");
				
				bUnattended = bUnattended || 5 >= 5.99;

__HELLO_PART:
				HelloWorld hello();

				string say = "Hello World from H++!!!";

				hello.SayHello(say);

				if(false==bUnattended)
					hello.ShowHello(say);

	//		debugger;
				TestIntegers tester1;

				tester1.Width	=  0x400h * 1024; //1mb
				tester1.Height	= 0x20h; //

				int nWidth	= tester1.Width,
					nHeight = tester1.Height;

				auto Int64 nArea = tester1.Width * tester1.Height;
				
				Console::WriteLn("nArea = ", nArea);

				with(tester1)
				{
					.Width	= 0x666h;
					.Height = 0x777h;
				}

				//int __CALC_FACTORIAL = 0;

				//goto __CALC_FACTORIAL;

		//		debugger;

				nArea = nWidth * nHeight;

				Console::WriteLn("nArea = ", nArea);
				
				int n = 10;

				Integers_Inline::Test();
				
				for(int index=0; index<50; index++)
				{
					Console::WriteLn("Current Index :", index );
				}

				for(;;)
				{
					//infinite loop
					if(--index==0)
						break;
				}

		//		debugger;
				bool what = !(index==0);

__CALC_FACTORIAL:

				int nf = tester1.Factorial(n);
				
				Console::WriteLn("Factorial of ", n, " = ", nf);

				--n;
				if(n==0)
					goto __END_FACTORIAL;

				goto __CALC_FACTORIAL;

__END_FACTORIAL:

				//only the reference param will be assigned when the function return to main...
				int myRef1 = 0;
				int myVar1 = 0;

		//	debugger;

				tester1.multiple_assignments(myRef1, myVar1);

				Console::WriteLn("The value of myRef1 must be 65536 ==", myRef1);
				Console::WriteLn("The value of myVar1 is not 65536 but zero (0) == ", myVar1);

				Console::WriteLn("(int)the area is: ", tester1.getArea());
				Console::WriteLn("(double)the area is: ", tester1.getArea2());

				auto Int32 result = 0;
				tester1.add(1024, 3072, result); //result==4096

				tester1.subs(1024, 3072, result); //result==-2048
				
				tester1.subs(3072, 1024, result); //result== 2048

				tester1.modulus(4096, 6, result); //result==4

				tester1.multiply(3, 927, result); //result==2781==0xADDh

				tester1.divide(4096, 6, result); //result==682

				tester1.divide2(4096, 6, result); //result==682.66666 aprox. 683				

	//		debugger;
				//test all kind of H++ integer expressions...
				TestIntegers::TestExpressions();
				
				int val1 = 1, val2 = 2;

				what = false;

				tester1.Swap(val1, val2);

				what = val1 == 2 && val2==1; //true
				
				Console::WriteLn("what == True ?", what);

				tester1.Calc2(val1, val2);

	//		debugger;
				what = val1 == 27000 && val2==20000; //true
				
				Console::WriteLn("what == True ?", what);

				tester1.Swap(val1, val2);

				what = val1 == 20000 && val2==27000; //true
				
				Console::WriteLn("what == True ?", what);
				
				tester1.SwapDebug(val1, val2);

				what = val1 == 27000 && val2==20000; //true
				
				Console::WriteLn("what == True ?", what);

				tester1.Calc1(val1, val2);

				what = val2!=27000; //true

				val1 = val1 << 24;
				
				what = val1 == 0x78000000;
				
				Console::WriteLn("(what = val1 == 0x78000000) = ", what);				

				if(false==bUnattended)
					hello.ShowHello("H++ Unit Testing : Second Phase");
	//		debugger;
				TestFloatingPoint fpTester();

				for(index=0; index <=32; index++)
				{
					double _pow = fpTester.Power(2.0, index);
				
					Console::WriteLn("pow(2, ", index, ") = ", _pow);
				}

				int array1[20];

		//	debugger;
				for(index = 0; index < sizeof(array1) / sizeof(int); )
				{
					array1[index] = ftoi((index + 1) * 9.45);
					Console::WriteLn("Result for array1[", index, "] = ", array1[index]);
					index++;
				}

				TestArrays::DoTest();

	//		debugger;
				
				Rnd rnd(Math::Round(0x400 * Math::log_10_base2() + 1.0));

	//			goto _BlockRndDoubles;

				Console::WriteLn("Pseudo-Random Integer values:");
				Console::WriteLn("(1)A pseudo-random value: ", rnd.NextInt());
				Console::WriteLn("(2)A pseudo-random value: ", rnd.NextInt());
				Console::WriteLn("(3)A pseudo-random value: ", rnd.NextInt());
				Console::WriteLn("(4)A pseudo-random value: ", rnd.NextInt());
				Console::WriteLn("(5)A pseudo-random value: ", rnd.NextInt());
				Console::WriteLn("(6)A pseudo-random value: ", rnd.NextInt());

_BlockRndDoubles:

				Console::WriteLn("Pseudo-Random Double values:");
				Console::WriteLn("(1)A pseudo-random value: ", rnd.NextDbl());
				Console::WriteLn("(2)A pseudo-random value: ", rnd.NextDbl());
				Console::WriteLn("(3)A pseudo-random value: ", rnd.NextDbl());
				Console::WriteLn("(4)A pseudo-random value: ", rnd.NextDbl());
				Console::WriteLn("(5)A pseudo-random value: ", rnd.NextDbl());
				Console::WriteLn("(6)A pseudo-random value: ", rnd.NextDbl());
				//

				using namespace UnitTesting::TestVirtuals;

				double res = 0.0;

				SimpleRV srv();

				res = srv.Sample();
				Console::WriteLn("(1)A simple random variable has : ", res, " as its value.");
				res = srv.Sample();
				Console::WriteLn("(2)A simple random variable has : ", res, " as its value.");
				res = srv.Sample();
				Console::WriteLn("(3)A simple random variable has : ", res, " as its value.");
				res = srv.Sample();
				Console::WriteLn("(4)A simple random variable has : ", res, " as its value.");
				Console::WriteLn("my name is", srv.Name());

				res = 1.0;

				UniformRV urv(Math::log_e_base2(), Math::log_2_base_e());

				res = urv.Sample();
				Console::WriteLn("(1)An uniform random variable has : ", res, " as its value.");
				res = urv.Sample();
				Console::WriteLn("(2)An uniform random variable has : ", res, " as its value.");
				res = urv.Sample();
				Console::WriteLn("(3)An uniform random variable has : ", res, " as its value.");
				res = urv.Sample();
				Console::WriteLn("(4)An uniform random variable has : ", res, " as its value.");
				Console::WriteLn("my name is", urv.Name());

				res = 0.0;

				ExponentialRV erv(Math::pi());

				res = erv.Sample();
				Console::WriteLn("(1)An exponential random variable has : ", res, " as its value.");
				res = erv.Sample();
				Console::WriteLn("(2)An exponential random variable has : ", res, " as its value.");
				res = erv.Sample();
				Console::WriteLn("(3)An exponential random variable has : ", res, " as its value.");
				res = erv.Sample();
				Console::WriteLn("(4)An exponential random variable has : ", res, " as its value.");
				Console::WriteLn("my name is", erv.Name());

				
//				goto _Alpha;
				TestSwitch::RunTest(TestSwitch::open);
//				goto _Omega;
				
				Circle obj;

				obj.Radius = 115.456;
				
	using cout = Console::WriteLn;
				
				cout("Circle.Radius = ", obj.Radius);

				auto double area = obj.ComputeArea();
				
				cout("Circle.Area = ", area);

		//debugger;		
				Circle obj2 = obj;				

				what = (area == obj2.ComputeArea());
				
				cout("what must be true = ", what);
				
				if(obj==obj2)
				{
					cout("obj==obj2");
				}

				Square sq;

				sq.Width = 500.456;
				sq.Height = 1045.4564;

				sq.ComputeArea();
				
				cout("sq.Width = ", sq.Width, ", sq.Height = ", sq.Height, ", area = ", sq.ComputeArea());

				Triangle tri;

				tri.Base = 450.4564;
				tri.High = 4572.4564;

				tri.ComputeArea();
				
				cout("tri.Base = ", tri.Base, ", tri.High = ", tri.High, ", area = ", tri.ComputeArea());

using printf = Console::WriteLn;
				
				printf("I'm tired of waiting!!!");
				SimpleRV cant;
				
				Console::WriteLn("My Name is: ", cant.Name());
				
				if(false==bUnattended)
					hello.ShowHello("H++ Unit Testing : Third Phase");
	//		debugger;
				TestIntegers xtarget,
							 ysource;

				ysource.Width = 0x666;
				ysource.Height = 0x777;
				
				Console::WriteLn("ysource.Width = ", ysource.Width, ", ysource.Height = ", ysource.Height);
				
				try{
					debugger; //this breakpoint will never be reached by a debugger because will be catched inmediately
					//
					Console::WriteLn("This line will never be executed!");
					//
				}catch(System::Exception::BREAKPOINT)
				{
					Console::WriteLn("**A breakpoint was catched!**");
				}

				//to test the object copy feature...
				xtarget = ysource;
				
				Console::WriteLn("(copied)xtarget.Width = ", xtarget.Width, ", (copied)xtarget.Height = ", xtarget.Height);

				val1 = 5, val2 = 3;					
				
				Integers_Inline::Test(void);
				
				Console::WriteLn("SummationSeries::ComputeRenderGamma() series: ");
				SummationSeries::ComputeRenderGamma(void);
				
				cout("Degrees to Radians of ", 90, " is = ", Math::DegreesToRadians(90.0), " Radians.");
				cout("Radians to Degrees of ", 1.57079, " is = ", Math::RadiansToDegrees(1.57079), " Degrees.");
				
		//	debugger;
				double^ p = new double;

				
				*p = 3.141516927;
				
				cout("*p (3.1415...)= ", *p);

				res = *p;
				
				cout("res (3.1415...)= ", res);

				destroy p;
/*	TODO: 
				p = &res;

				*p = 9.99;
				
				cout("res = (9.99)", res);
				*/

		//	debugger;	
				TestFloatingPoint^ fpTester_ptr = new TestFloatingPoint();

				_pow = fpTester_ptr.Power(2.0, 64);

				destroy fpTester_ptr;

		//debugger;
				int [] iptr = new int[10];	//10 * 10 * 4 = 400 bytes

				iptr[0] = 145;
				iptr[9] = 541;
				iptr[6] = 774;
				iptr[7] = 991;
				iptr[2] = 123;
				iptr[1] = 667;
				
				printf("iptr[0] == 145 == ", iptr[0]);
				printf("iptr[9] == 541 == ", iptr[9]);
				printf("iptr[7] == 991 == ", iptr[7]);
				printf("iptr[2] == 123 == ", iptr[2]);
				printf("iptr[1] == 667 == ", iptr[1]);
				printf("iptr[6] == 774 == ", iptr[6]);

				destroy iptr;

				iptr = null;
				
				int yes[10][10];
				yes[0][0] = 145;
				yes[1][9] = 541;
				yes[2][7] = 334;
				yes[3][9] = 123;
				
				printf("yes[0][0] == 145 == ", yes[0][0]);
				printf("yes[1][9] == 541 == ", yes[1][9]);
				printf("yes[2][7] == 334 == ", yes[2][7]);
				printf("yes[3][9] == 123 == ", yes[3][9]);
				
				double numbers[10];
				
			//	debugger;
				
				numbers[0] = Math::pi();
				numbers[1] = Math::pi() * 2.1;
				numbers[2] = Math::pi() * 4.1;
				numbers[3] = Math::pi() * 8.1;
				numbers[4] = Math::pi() * 16.1;
				numbers[5] = Math::pi() * 32.1;
				numbers[6] = Math::pi() * 64.1;
				numbers[7] = Math::pi() * 128.1;
				numbers[8] = Math::pi() * 256.1;
				numbers[9] = Math::pi() * 512.1;
				
				int sz = sizeof(numbers)/sizeof(double);
				
				for(index=0; index < sz; index++)
				{
					printf("numbers[", index,"] = ", numbers[index]);
				}
				
		//	debugger;
				Circle objs[3];
				
				objs[1].Radius = Math::pi();
				
				objs[1].Draw();
				
				cout("Circle[1].Radius = 3.14 = ", objs[1].Radius);
				
				objs[2] = objs[1]; //copying this object
				
				cout("Circle[2].Radius = 3.14 = ", objs[2].Radius);
				
				Circle dest;
				
				dest = objs[1];
				
				cout("Circle.Radius = 3.14 = ", dest.Radius);
				
				objs[0] = dest;
				
				cout("Circle[0].Radius = 3.14 = ", objs[0].Radius);
				
				int ^ xptr = null;

				xptr = new int;
				
			if(false==bUnattended)
				hello.ShowHello("H++ Unit Testing : Fourth Phase");
				
	//		debugger;
				//testing sizeof operators
				int npi = sizeof(misc1::pi);

				npi = sizeof pi;

				int nint = sizeof(int),
					ndbl = sizeof(double),
					ni64 = sizeof Int64,
					ntit = sizeof(TestIntegers),
					nptr = sizeof(p),
					narr = sizeof(iptr),
					nar2 = sizeof(array1),
					nar3 = sizeof(misc1::array);

				nint = sizeof int + sizeof double;

				nint = sizeof(int) + sizeof(Int64);

				if(nint >= ndbl && ni64 < ntit)
				{
					Console::WriteLn("This is getting complex and better each time!");
				}				
				
				destroy xptr;
				
				int [] ii2ptr = null;

				xptr = xptr + 1;
				
	//		debugger;
				string name1 = "Harold", 
					   name2 = "Harold L.",
					   name3 = "Harold L. Marzan",
					   name4 = name1;
				
				if(name1 <= name2)
				{
					Console::WriteLn("name1 =< name2");
				}
				if(strlen(name3) > strlen(name2))
				{
					Console::WriteLn("name3.length > name2.length");
				}
				
				if(name2 <= name3)
				{
					Console::WriteLn("name2 <= name3");
				}
				
				//this expression is not correctly generated
				if(!(name3 < name2))
				{
					Console::WriteLn("!(name3 < name2)");
				}
	
				if(false==!(name2 < name3))
				{
					Console::WriteLn("false==!(name2 < name3)");
				}
				
				if(name1==name4)
				{
					Console::WriteLn("It's working as designed!");
				}
				
				if(name1==name3)
				{
					Console::WriteLn("I should never get here!");
				}				
//debugger;
				if(name1!=name3)
				{
					Console::WriteLn("Yes (name1 != name3)!");
				}				
				
		//	debugger;
				UnitTesting::TestIntegers::bigValue *= 4;
				
			//debugger;
				UnitTesting::TestIntegers::bigValue /= 4;

			//debugger;
				UnitTesting::TestIntegers::bigValue += 1024;
				
			//debugger;
				UnitTesting::TestIntegers::bigValue -= 1024;
				
	//	debugger;
				Shape^ obj_ptr = ShapeFactoryCreator::Create("Circle");				
				
				obj_ptr.Draw();
				
				Visitor visitor;				
				visitor.Visit(obj_ptr);				
				
				Circle^ circl = dynamic_cast(obj_ptr); //valid
				
				circl = dynamic_cast(Circle, obj_ptr); //valid
				
				//Square^ squar = dynamic_cast(Circle, obj_ptr); //must fail
				//squar = dynamic_cast(Circle, obj_ptr); //must fail
				
				circl.DrawSphere();
				
				destroy dynamic_cast(Circle, obj_ptr);
				
				//destroy circl;
				
				obj_ptr = ShapeFactoryCreator::Create("Square");				
				visitor.Visit(obj_ptr);				
				
				destroy obj_ptr; //dynamic_cast(Square, obj_ptr);
				
				obj_ptr = ShapeFactoryCreator::Create("Triangle");				
				visitor.Visit(obj_ptr);
				
				destroy obj_ptr; //dynamic_cast(Triangle, obj_ptr);
				
		//debugger;				
				TestProperties testProp;
				
				Circle^ shapes = new Circle[20]; 
				testProp.Shapes = shapes;			//TODO: the destructor is creating memory leaks because the array size was not promoted in the data member
	//	debugger;	
				SortDoubleProxy sorter;
				n = 10;
				DoubleProxy [] array = new DoubleProxy[n];
				//initialization	
				array[0].val = 19.2;
				array[1].val = 3.2345;
				array[2].val = 6.3456;
				array[3].val = 11.5676;
				array[4].val = 7.2345;
				array[5].val = 12.4567;
				array[6].val = 9.2345;
				array[7].val = 15.123;
				array[8].val = 1.2;
				array[9].val = 18.1234;

				for(index=0; index < n; index++)
					cout("array[", index,"]= ", array[index].val);
			
				sorter.PrintObject(array[9]);
	//	debugger;
				DoubleSwapper swapper;
				
				sorter.Sort(array, n, swapper);
				
				sorter.SortEx(array, n, swapper);
				
				cout("Now sorted:");
				
				for(index=0; index < n; index++)
					cout("array[", index,"]= ", array[index].val);
				
				sorter.PrintObject(array[9]);
				
				destroy [] array;
				
				const int nn = 10;
				double arr[nn];
				
			//	debugger;
				
				arr[0]= 19.2;
				arr[1]= 3.2345;
				arr[2]= 6.3456;
				arr[3]= 11.5676;
				arr[4]= 7.2345;
				arr[5]= 12.4567;
				arr[6]= 9.2345;
				arr[7]= 15.123;
				arr[8]= 1.2;
				arr[9]= 18.1234;

				for(index=0; index < nn; index++)
					cout("array[", index,"]= ", arr[index]);
				
				sorter.Sort2(arr, n);
				
				cout("Now sorted:");
				
				for(index=0; index < nn; index++)
					cout("array[", index,"]= ", arr[index]);
					
					
				cout("\nComputing the PI constant...\n");
				double PI = MonteCarlo::PI(10000000);
				cout("The PI value = ", PI, "\n\n");
				
			}
		};
	}
}