import "TestSource\stdhpp\stdapi.hcc";

namespace UnitTesting //TestSwitch.hpp
{
	class TestSwitch
	{
		public const int open  = 1;
		public const int close = 2;
		public const int write = 3, 
						 read = 4;

		public static void RunTest(int option)
		{	
			switch(option)
			{
			case open:
				Console::WriteLn("Opening file...");
				break;
			case close:
				Console::WriteLn("Closing file...");
				break;
			case write:
				Console::WriteLn("Do Write to file...");
				break;
			case read:
				Console::WriteLn("Do Read from file...");
				break;
			case 5:
				Console::WriteLn("Do for 5 and follow to 6");
			case 6:
				{
				Console::WriteLn("Do 6, then finish");
				}
				break;
			default:
				Console::WriteLn("Invalid Option : ", option);
			};
		}
	};

	typename enum ShapeType
	{
		stCircle = 1,
		stSquare,
		stTriangle,
	}ShapeType;

	class Shape
	{		
		public virtual abstract ShapeType Type();
		public virtual abstract double ComputeArea();		
		public virtual abstract string Name();
		public virtual abstract void Draw();
	};

	class Circle: Shape
	{
		private double radius = 0.0;
		
		public Circle()
		{}

	public double get:Radius()
	{
		return radius;
	}

	public void put:Radius(double value)
	{
		radius = value;
	}

	protected double Circumference()
	{
		return Math::pi() * Radius;
	}

		public virtual ShapeType Type()
		{
			return stCircle;
		}		
		public virtual string Name()
		{
			return "Circle";
		}

		public virtual void Draw()
		{
			Console::WriteLn("Drawing a Circle!");
		}

		public virtual double ComputeArea()
		{
			return 2 * Circumference();
		}
	};

	class Square: Shape
	{
		private double dWidth = 0.0;
		private double dHeight = 0.0;

	public double get:Width()
	{
		return dWidth;
	}

	public void put:Width(double value)
	{
		dWidth = value;
	}

	public double get:Height()
	{
		return dHeight;
	}

	public void put:Height(double value)
	{
		dHeight = value;
	}

	public virtual double ComputeArea()
	{
		return Width * Height;
	}
		public virtual ShapeType Type()
		{
			return stSquare;
		}		
		public virtual string Name()
		{
			return "Square";
		}

		public virtual void Draw()
		{
			Console::WriteLn("Drawing a Square!");
		}

	};

	class Triangle: Shape
	{
		private double dBase = 0.0;
		private double dHigh = 0.0;

	public double get:Base()
	{
		return dBase;
	}

	public void put:Base(double value)
	{
		dBase = value;
	}

	public double get:High()
	{
		return dHigh;
	}

	public void put:High(double value)
	{
		dHigh = value;
	}
		public virtual ShapeType Type()
		{
			return stTriangle;
		}		
		public virtual string Name()
		{
			return "Triangle";
		}

		public virtual void Draw()
		{
			Console::WriteLn("Drawing a Triangle!");
		}


	public virtual double ComputeArea()
	{
		return Base * High / 2;
	}
	
	public void Destructor()
	{
		return;
	}
	};
}